#include <nxf/project/ProjectFile.h>
#include <nxf/platform/FileSystem.h>

#include <filesystem>

namespace nxf::project {
namespace {
using nxf::core::Error;
constexpr std::string_view header = "NXF-PROJECT 1";
} // namespace

Error saveProject(ProjectDocument& document, const std::string& path)
{
    if (path.empty()) return Error::invalidArgument("save path is empty");
    const std::string content = std::string{header} + "\n" + serializeValue(document.root()) + "\n";
    const std::string tempPath = path + ".tmp";
    if (const auto written = nxf::platform::writeTextFile(tempPath, content); !written) return written;

    std::error_code ec;
    std::filesystem::remove(path, ec);
    ec.clear();
    std::filesystem::rename(tempPath, path, ec);
    if (ec) {
        std::error_code cleanup;
        std::filesystem::remove(tempPath, cleanup);
        return Error::internal("could not move the saved project into place: " + ec.message());
    }
    document.markSaved();
    return Error::success();
}

nxf::core::Result<ProjectDocument> loadProject(const std::string& path)
{
    using R = nxf::core::Result<ProjectDocument>;
    const auto content = nxf::platform::readTextFile(path);
    if (!content.hasValue()) return R::failure(content.error());

    const std::string& text = content.value();
    const auto newline = text.find('\n');
    if (newline == std::string::npos || text.substr(0, newline) != header)
        return R::failure(Error::invalidArgument("not an NXF project file"));

    auto root = parseValue(text.substr(newline + 1));
    if (!root.hasValue()) return R::failure(root.error());
    if (!root.value().isObject()) return R::failure(Error::invalidArgument("project root is not an object"));

    const Value* version = root.value().get("formatVersion");
    if (version == nullptr || !version->isInt())
        return R::failure(Error::invalidArgument("project has no format version"));
    if (version->asInt() > ProjectDocument::currentFormatVersion)
        return R::failure(Error::unavailable("project was written by a newer NXF format"));
    if (version->asInt() < 1)
        return R::failure(Error::invalidArgument("project format version is invalid"));

    ProjectDocument document{root.value()};
    document.markSaved();
    return R::success(std::move(document));
}

} // namespace nxf::project
