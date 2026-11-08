#include <nxf/plugin/PluginManager.h>
#include <nxf/audio/AudioBuffer.h>
#include "TestSupport.h"
#include <array>
#include <cmath>
int main() {
    using namespace nxf::plugin;
    PluginManager manager;
    manager.registerAdapter(std::make_shared<NativeTestPluginFormatAdapter>());
    PluginDescriptor gainDesc;
    gainDesc.id = "test.gain"; gainDesc.format = "nxf-test"; gainDesc.factoryKey = "gain";
    gainDesc.inputChannels = 2; gainDesc.outputChannels = 2;

    auto created = manager.createInstance(gainDesc);
    NXF_CHECK(created.hasValue());
    auto plugin = std::move(created.value());
    NXF_CHECK(plugin->descriptor().id == "test.gain");
    NXF_CHECK(plugin->state() == PluginState::Created);
    NXF_CHECK(plugin->parameters().size() == 1);
    NXF_CHECK(plugin->parameters()[0].id == "gain");

    nxf::audio::AudioBuffer input(2, 128), output(2, 128);
    for (int f = 0; f < 128; ++f) { input.setSample(0, f, 0.5f); input.setSample(1, f, 0.5f); }
    plugin->process(input.constView(), output.view(), {}, 48000.0, 0);   // not prepared: silence rules
    NXF_CHECK(output.sample(0, 10) == 0.0f);
    NXF_CHECK(plugin->prepare(48000.0, 128, 2).isSuccess());
    NXF_CHECK(plugin->state() == PluginState::Prepared);
    NXF_CHECK(plugin->setParameterNormalized(0, 0.25));        // gain param 0..2 -> 0.5
    NXF_CHECK(std::fabs(plugin->parameterNormalized(0) - 0.25) < 1e-9);
    plugin->process(input.constView(), output.view(), {}, 48000.0, 0);
    NXF_CHECK(std::fabs(output.sample(0, 10) - 0.25f) < 1e-6f);
    NXF_CHECK(!plugin->setParameterNormalized(7, 0.5));        // no such parameter
    NXF_CHECK(plugin->setParameterNormalized(0, 2.0));         // clamps to 1.0
    NXF_CHECK(plugin->parameterNormalized(0) == 1.0);

    const auto saved = plugin->saveState();
    NXF_CHECK(!saved.empty());
    NXF_CHECK(plugin->setParameterNormalized(0, 0.0));
    NXF_CHECK(plugin->restoreState(saved).isSuccess());
    NXF_CHECK(plugin->parameterNormalized(0) == 1.0);
    const std::array<std::uint8_t, 2> garbage{0x00, 0x01};
    NXF_CHECK(!plugin->restoreState(garbage).isSuccess());   // garbage state refused

    plugin->suspend();
    NXF_CHECK(plugin->state() == PluginState::Created);
    plugin->reset();
    return nxf::test::report("PluginInstanceTests");
}
