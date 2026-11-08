#include <nxf/audio/AudioGraph.h>
#include <nxf/audio/AudioNode.h>
#include "TestSupport.h"
#include <cmath>
namespace {
bool approx(float a, float b, float eps = 1e-5f) { return std::fabs(a - b) < eps; }
}
int main() {
    using namespace nxf::audio;
    {   // external input -> gain 0.5 -> out
        AudioGraph graph;
        auto gain = std::make_shared<GainNode>(); gain->setGain(0.5f);
        const auto gainId = graph.addNode(gain);
        NXF_CHECK(gainId == 0);
        NXF_CHECK(graph.prepare(48000.0, 2, 128).isSuccess());
        AudioBuffer input(2, 128); for (int f = 0; f < 128; ++f) { input.setSample(0, f, 0.5f); input.setSample(1, f, -0.5f); }
        AudioBuffer output(2, 128);
        NXF_CHECK(graph.process(input.constView(), output.view(), 0).isSuccess());
        NXF_CHECK(approx(output.sample(0, 10), 0.25f));
        NXF_CHECK(approx(output.sample(1, 10), -0.25f));
    }
    {   // two sources mixed into one gain: 0.2 + 0.3, then x0.5 = 0.25
        AudioGraph graph;
        auto a = std::make_shared<ConstantSourceNode>(); a->setLevel(0.2f);
        auto b = std::make_shared<ConstantSourceNode>(); b->setLevel(0.3f);
        auto g = std::make_shared<GainNode>(); g->setGain(0.5f);
        const auto aId = graph.addNode(a);
        const auto bId = graph.addNode(b);
        const auto gId = graph.addNode(g);
        NXF_CHECK(graph.connect(aId, gId).isSuccess());
        NXF_CHECK(graph.connect(bId, gId).isSuccess());
        NXF_CHECK(!graph.connect(aId, 99).isSuccess());
        NXF_CHECK(graph.prepare(48000.0, 1, 64).isSuccess());
        AudioBuffer input(1, 64); AudioBuffer output(1, 64);
        NXF_CHECK(graph.process(input.constView(), output.view(), 0).isSuccess());
        NXF_CHECK(approx(output.sample(0, 5), 0.25f));   // (0.2 + 0.3) mixed, then x 0.5
    }
    {   // sine source actually oscillates: RMS of 440 Hz at 48k over 4800 frames ~ A/sqrt(2)
        AudioGraph graph;
        auto sine = std::make_shared<SineSourceNode>(); sine->setFrequency(440.0); sine->setAmplitude(0.8f);
        graph.addNode(sine);
        NXF_CHECK(graph.prepare(48000.0, 1, 4800).isSuccess());
        AudioBuffer input(1, 4800); AudioBuffer output(1, 4800);
        NXF_CHECK(graph.process(input.constView(), output.view(), 0).isSuccess());
        double sum = 0; for (int f = 0; f < 4800; ++f) sum += static_cast<double>(output.sample(0, f)) * output.sample(0, f);
        const double rms = std::sqrt(sum / 4800.0);
        NXF_CHECK(rms > 0.5 && rms < 0.63);
    }
    {   // a cycle must be refused at prepare time, never rendered
        AudioGraph graph;
        auto a = std::make_shared<GainNode>(); auto b = std::make_shared<GainNode>();
        const auto aId = graph.addNode(a); const auto bId = graph.addNode(b);
        NXF_CHECK(graph.connect(aId, bId).isSuccess());
        NXF_CHECK(graph.connect(bId, aId).isSuccess());
        NXF_CHECK(!graph.prepare(48000.0, 2, 64).isSuccess());
    }
    return nxf::test::report("AudioGraphTests");
}
