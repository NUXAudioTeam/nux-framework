#include <nxf/plugin/PluginFormatAdapter.h>
#include <nxf/audio/AudioBuffer.h>
#include "TestSupport.h"

#include <cmath>
#include <cstdlib>
#include <string>

int main()
{
    using namespace nxf::plugin;

    Vst3FormatAdapter adapter;
    NXF_CHECK(adapter.formatName() == "vst3");

    // Scanning must always fail cleanly for bad candidates, with
    // or without the SDK integrated.
    NXF_CHECK(!adapter.scanFile("no-such-plugin.vst3").hasValue());
    NXF_CHECK(!adapter.scanFile("not-a-plugin.txt").hasValue());

    PluginDescriptor fake;
    fake.format = "vst3";
    fake.factoryKey = "00000000-0000-0000-0000-000000000000";
    fake.path = "no-such-plugin.vst3";
    NXF_CHECK(!adapter.createInstance(fake).hasValue());

    // The real end-to-end proof needs an actual .vst3 binary.
    // CI and local runs provide one (built from the SDK samples)
    // through this environment variable; without it the test only
    // covers the failure paths and says so.
    const char* pluginPathEnv = std::getenv("NXF_TEST_VST3_PLUGIN");
    if (pluginPathEnv == nullptr || pluginPathEnv[0] == '\0') {
        std::cout << "Vst3HostingTests: no NXF_TEST_VST3_PLUGIN set - "
                     "real plug-in scan/process checks skipped (sdk integrated: "
                  << (isVst3SdkIntegrated() ? "yes" : "no") << ")\n";
        return nxf::test::report("Vst3HostingTests");
    }

    NXF_CHECK(isVst3SdkIntegrated());
    auto scanned = adapter.scanFile(pluginPathEnv);
    NXF_CHECK(scanned.hasValue());
    if (!scanned.hasValue())
        return nxf::test::report("Vst3HostingTests");
    const PluginDescriptor descriptor = scanned.value();
    NXF_CHECK(descriptor.format == "vst3");
    NXF_CHECK(!descriptor.id.empty());
    NXF_CHECK(!descriptor.name.empty());
    NXF_CHECK(descriptor.outputChannels > 0);

    auto created = adapter.createInstance(descriptor);
    NXF_CHECK(created.hasValue());
    if (!created.hasValue())
        return nxf::test::report("Vst3HostingTests");
    auto plugin = std::move(created.value());
    NXF_CHECK(plugin->state() == PluginState::Created);

    const int channels = descriptor.outputChannels > 0 ? descriptor.outputChannels : 2;
    NXF_CHECK(plugin->prepare(48000.0, 512, channels).isSuccess());
    NXF_CHECK(plugin->state() == PluginState::Prepared);

    if (!plugin->parameters().empty()) {
        NXF_CHECK(plugin->setParameterNormalized(0, 0.5));
        NXF_CHECK(std::fabs(plugin->parameterNormalized(0) - 0.5) < 1e-9);
        NXF_CHECK(!plugin->setParameterNormalized(9999, 0.5));
    }

    nxf::audio::AudioBuffer input(channels, 512), output(channels, 512);
    for (int c = 0; c < channels; ++c)
        for (int f = 0; f < 512; ++f)
            input.setSample(c, f, 0.25f);
    plugin->process(input.constView(), output.view(), {}, 48000.0, 0);
    bool allFinite = true;
    for (int c = 0; c < channels; ++c)
        for (int f = 0; f < 512; ++f)
            allFinite = allFinite && std::isfinite(output.sample(c, f));
    NXF_CHECK(allFinite);

    // State round trip: restoring our own chunk must be accepted
    // (an empty chunk from plug-ins without state is fine - then
    // there is nothing to restore and we skip honestly).
    const auto state = plugin->saveState();
    if (!state.empty())
        NXF_CHECK(plugin->restoreState(state).isSuccess());

    plugin->suspend();
    NXF_CHECK(plugin->state() == PluginState::Created);
    return nxf::test::report("Vst3HostingTests");
}
