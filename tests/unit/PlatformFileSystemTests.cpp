#include <nxf/platform/FileSystem.h>
#include "TestSupport.h"
#include <filesystem>
#include <string>
int main() {
    using namespace nxf::platform;
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "nxf_platform_tests";
    std::error_code ec; fs::remove_all(dir, ec);
    NXF_CHECK(createDirectory((dir / "sub").string()).isSuccess());
    NXF_CHECK(exists(dir.string()));
    NXF_CHECK(exists((dir / "sub").string()));
    const std::string file = (dir / "hello.txt").string();
    NXF_CHECK(!exists(file));
    NXF_CHECK(writeTextFile(file, "NXF platform").isSuccess());
    NXF_CHECK(exists(file));
    const auto size = fileSizeBytes(file);
    NXF_CHECK(size.hasValue());
    NXF_CHECK(size.value() == 12);
    const auto text = readTextFile(file);
    NXF_CHECK(text.hasValue());
    NXF_CHECK(text.value() == "NXF platform");
    NXF_CHECK(writeTextFile(file, "x").isSuccess());          // overwrite/truncate
    NXF_CHECK(readTextFile(file).value() == "x");
    const auto missing = readTextFile((dir / "nope.txt").string());
    NXF_CHECK(!missing.hasValue());                          // failure as a value
    NXF_CHECK(missing.error().code() == nxf::core::ErrorCode::Unavailable);
    const auto listing = listDirectory(dir.string());
    NXF_CHECK(listing.hasValue());
    bool sawFile = false, sawSub = false;
    for (const auto& name : listing.value()) { if (name == "hello.txt") sawFile = true; if (name == "sub") sawSub = true; }
    NXF_CHECK(sawFile); NXF_CHECK(sawSub);
    const auto tmp = tempDirectory();
    NXF_CHECK(tmp.hasValue()); NXF_CHECK(!tmp.value().empty()); NXF_CHECK(exists(tmp.value()));
    NXF_CHECK(removeFile(file).isSuccess());
    NXF_CHECK(!exists(file));
    fs::remove_all(dir, ec);
    return nxf::test::report("PlatformFileSystemTests");
}
