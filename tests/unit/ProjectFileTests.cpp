#include <nxf/project/ProjectFile.h>
#include <nxf/platform/FileSystem.h>
#include "TestSupport.h"
#include <filesystem>
int main() {
    using namespace nxf::project;
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "nxf_project_tests";
    std::error_code ec; fs::remove_all(dir, ec); fs::create_directories(dir);
    const std::string path = (dir / "song.nxfproj").string();

    ProjectDocument doc("Saved Song");
    doc.setValue("settings.tempo", Value::real(99.5));
    doc.setValue("settings.title", Value::string("A \"quoted\" title"));
    NXF_CHECK(saveProject(doc, path).isSuccess());
    NXF_CHECK(!doc.isDirty());                                // saving cleans the document
    NXF_CHECK(nxf::platform::exists(path));
    NXF_CHECK(!nxf::platform::exists(path + ".tmp"));         // atomic save leaves no temp behind
    const auto raw = nxf::platform::readTextFile(path);
    NXF_CHECK(raw.hasValue());
    NXF_CHECK(raw.value().rfind("NXF-PROJECT 1", 0) == 0);    // magic + format version header

    const auto loaded = loadProject(path);
    NXF_CHECK(loaded.hasValue());
    NXF_CHECK(loaded.value().name() == "Saved Song");
    NXF_CHECK(loaded.value().value("settings.tempo")->asReal() == 99.5);
    NXF_CHECK(loaded.value().value("settings.title")->asString() == "A \"quoted\" title");
    NXF_CHECK(!loaded.value().isDirty());

    NXF_CHECK(!loadProject((dir / "missing.nxfproj").string()).hasValue());

    // a file from a newer format is refused, never guessed at
    NXF_CHECK(nxf::platform::writeTextFile((dir / "future.nxfproj").string(),
        "NXF-PROJECT 1\n{ formatVersion = 99; name = \"Future\"; }").isSuccess());
    const auto future = loadProject((dir / "future.nxfproj").string());
    NXF_CHECK(!future.hasValue());
    NXF_CHECK(future.error().code() == nxf::core::ErrorCode::Unavailable);

    NXF_CHECK(nxf::platform::writeTextFile((dir / "corrupt.nxfproj").string(), "NXF-PROJECT 1\n{ broken = ; }").isSuccess());
    NXF_CHECK(!loadProject((dir / "corrupt.nxfproj").string()).hasValue());
    NXF_CHECK(nxf::platform::writeTextFile((dir / "alien.nxfproj").string(), "NOT OUR FILE").isSuccess());
    NXF_CHECK(!loadProject((dir / "alien.nxfproj").string()).hasValue());

    fs::remove_all(dir, ec);
    return nxf::test::report("ProjectFileTests");
}
