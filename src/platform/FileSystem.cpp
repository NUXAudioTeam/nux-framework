#include <nxf/platform/FileSystem.h>

#include <filesystem>
#include <fstream>
#include <system_error>

namespace nxf::platform {
namespace {
namespace fs = std::filesystem;
using nxf::core::Error;

[[nodiscard]] fs::path toPath(std::string_view path) { return fs::path{std::string{path}}; }
[[nodiscard]] Error ioError(std::string_view what, const std::error_code& ec) {
    return Error::unavailable(std::string{what} + ": " + ec.message());
}
} // namespace

bool exists(std::string_view path)
{
    std::error_code ec;
    return !path.empty() && fs::exists(toPath(path), ec);
}

nxf::core::Result<std::uint64_t> fileSizeBytes(std::string_view path)
{
    std::error_code ec;
    const auto size = fs::file_size(toPath(path), ec);
    if (ec) return nxf::core::Result<std::uint64_t>::failure(ioError("file size", ec));
    return nxf::core::Result<std::uint64_t>::success(static_cast<std::uint64_t>(size));
}

nxf::core::Result<std::string> readTextFile(std::string_view path)
{
    std::ifstream in{toPath(path), std::ios::binary};
    if (!in) return nxf::core::Result<std::string>::failure(Error::unavailable("cannot open file for reading"));
    std::string contents{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    if (in.bad()) return nxf::core::Result<std::string>::failure(Error::internal("failed while reading file"));
    return nxf::core::Result<std::string>::success(std::move(contents));
}

Error writeTextFile(std::string_view path, std::string_view contents)
{
    std::ofstream out{toPath(path), std::ios::binary | std::ios::trunc};
    if (!out) return Error::unavailable("cannot open file for writing");
    out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    if (!out) return Error::internal("failed while writing file");
    return Error::success();
}

nxf::core::Result<std::vector<std::string>> listDirectory(std::string_view path)
{
    std::error_code ec;
    std::vector<std::string> names;
    for (const auto& entry : fs::directory_iterator{toPath(path), ec}) {
        names.push_back(entry.path().filename().string());
    }
    if (ec) return nxf::core::Result<std::vector<std::string>>::failure(ioError("list directory", ec));
    return nxf::core::Result<std::vector<std::string>>::success(std::move(names));
}

Error createDirectory(std::string_view path)
{
    std::error_code ec;
    fs::create_directories(toPath(path), ec);
    if (ec) return ioError("create directory", ec);
    return Error::success();
}

Error removeFile(std::string_view path)
{
    std::error_code ec;
    fs::remove(toPath(path), ec);
    if (ec) return ioError("remove file", ec);
    return Error::success();
}

nxf::core::Result<std::string> tempDirectory()
{
    std::error_code ec;
    const auto dir = fs::temp_directory_path(ec);
    if (ec) return nxf::core::Result<std::string>::failure(ioError("temp directory", ec));
    return nxf::core::Result<std::string>::success(dir.string());
}

} // namespace nxf::platform
