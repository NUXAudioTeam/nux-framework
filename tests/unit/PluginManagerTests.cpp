#include <nxf/plugin/PluginManager.h>
#include <nxf/platform/FileSystem.h>
#include "TestSupport.h"
#include <filesystem>
namespace {
void writeFile(const std::string& path, const std::string& text) {
    NXF_CHECK(nxf::platform::writeTextFile(path, text).isSuccess());
}
}
int main() {
    using namespace nxf::plugin;
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "nxf_plugin_tests";
    std::error_code ec; fs::remove_all(dir, ec); fs::create_directories(dir);

    writeFile((dir / "testgain.nxfplug").string(),
        "{ id = \"test.gain\"; name = \"Test Gain\"; manufacturer = \"NUX Audio Team\"; version = \"1.0\"; "
        "category = \"effect\"; behavior = \"gain\"; inputChannels = 2; outputChannels = 2; acceptsMidi = false; }");
    writeFile((dir / "testsine.nxfplug").string(),
        "{ id = \"test.sine\"; name = \"Test Sine\"; manufacturer = \"NUX Audio Team\"; version = \"1.0\"; "
        "category = \"instrument\"; behavior = \"sine\"; inputChannels = 0; outputChannels = 2; acceptsMidi = true; }");
    writeFile((dir / "broken.nxfplug").string(), "{ id = ; }");
    writeFile((dir / "notes.txt").string(), "not a plugin");

    PluginManager manager;
    manager.registerAdapter(std::make_shared<NativeTestPluginFormatAdapter>());
    manager.registerAdapter(std::make_shared<Vst3FormatAdapter>());
    const auto result = manager.scanDirectory(dir.string());
    NXF_CHECK(result.descriptors.size() == 2);                 // broken file skipped, txt ignored
    NXF_CHECK(result.errors.size() == 1);
    const auto* gain = manager.findPlugin("test.gain");
    NXF_CHECK(gain != nullptr);
    NXF_CHECK(gain->name == "Test Gain");
    NXF_CHECK(gain->format == "nxf-test");
    NXF_CHECK(gain->isEffect());
    const auto* sine = manager.findPlugin("test.sine");
    NXF_CHECK(sine != nullptr); NXF_CHECK(sine->isInstrument()); NXF_CHECK(sine->acceptsMidi);
    NXF_CHECK(manager.findPlugin("no.such.plugin") == nullptr);

    // duplicate ids: first registration wins, second is an error not a silent overwrite
    const auto again = manager.scanDirectory(dir.string());
    NXF_CHECK(again.descriptors.empty());
    NXF_CHECK(again.errors.size() == 3);                       // 2 duplicates + 1 broken

    // descriptor <-> Value round trip (the .nxfplug file format itself)
    const auto value = gain->toValue();
    const auto restored = PluginDescriptor::fromValue(value);
    NXF_CHECK(restored.hasValue());
    NXF_CHECK(restored.value() == *gain);
    NXF_CHECK(!PluginDescriptor::fromValue(nxf::project::Value::integer(3)).hasValue());

    // the VST3 seam is honest: present, named, and unavailable without the SDK
    auto vst3 = manager.findAdapter("vst3");
    NXF_CHECK(vst3 != nullptr);
    NXF_CHECK(vst3->formatName() == "vst3");
    NXF_CHECK(!vst3->createInstance(*gain).hasValue());

    fs::remove_all(dir, ec);
    return nxf::test::report("PluginManagerTests");
}
