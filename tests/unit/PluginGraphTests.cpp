#include <nxf/plugin/PluginManager.h>
#include <nxf/plugin/PluginNode.h>
#include <nxf/audio/AudioGraph.h>
#include "TestSupport.h"
#include <cmath>
int main() {
    using namespace nxf::plugin;
    PluginManager manager;
    manager.registerAdapter(std::make_shared<NativeTestPluginFormatAdapter>());
    PluginDescriptor sineDesc;
    sineDesc.id = "test.sine"; sineDesc.format = "nxf-test"; sineDesc.factoryKey = "sine";
    sineDesc.outputChannels = 1;
    auto sine = std::move(manager.createInstance(sineDesc).value());
    NXF_CHECK(sine->prepare(48000.0, 4800, 1).isSuccess());
    NXF_CHECK(sine->setParameterNormalized(0, 0.5));           // frequency param

    nxf::audio::AudioGraph graph;
    graph.addNode(std::make_shared<PluginNode>(std::move(sine)));
    NXF_CHECK(graph.prepare(48000.0, 1, 4800).isSuccess());
    nxf::audio::AudioBuffer input(1, 4800), output(1, 4800);
    NXF_CHECK(graph.process(input.constView(), output.view(), 0).isSuccess());
    double sum = 0; int nonzero = 0;
    for (int f = 0; f < 4800; ++f) { const float v = output.sample(0, f); sum += v * v; if (v != 0.0f) ++nonzero; }
    NXF_CHECK(nonzero > 4000);                                 // the hosted instrument really sounds
    NXF_CHECK(std::sqrt(sum / 4800.0) > 0.1);
    return nxf::test::report("PluginGraphTests");
}
