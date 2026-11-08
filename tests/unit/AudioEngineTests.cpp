#include <nxf/audio/AudioEngine.h>
#include <nxf/audio/OfflineAudioBackend.h>
#include "TestSupport.h"
#include <atomic>
#include <cmath>
namespace {
struct CaptureCallback final : nxf::audio::IAudioCallback {
    nxf::audio::AudioEngine& engine;
    std::atomic<float> peak{0.0f};
    explicit CaptureCallback(nxf::audio::AudioEngine& e) : engine(e) {}
    void processAudio(const nxf::audio::ConstAudioBufferView& input, const nxf::audio::AudioBufferView& output,
                      double sampleRate, std::uint64_t time) override {
        engine.render(input, output);
        (void)sampleRate; (void)time;
        for (int f = 0; f < output.frames; ++f) for (int ch = 0; ch < output.channels; ++ch)
            peak = std::max(peak.load(), std::fabs(output.sample(ch, f)));
    }
};
}
int main() {
    using namespace nxf::audio;
    AudioEngine engine;
    auto sine = std::make_shared<SineSourceNode>(); sine->setFrequency(440.0); sine->setAmplitude(0.5f);
    engine.graph().addNode(sine);
    NXF_CHECK(engine.prepare(48000.0, 2, 256).isSuccess());
    AudioBuffer input(2, 256); AudioBuffer output(2, 256);
    engine.render(input.constView(), output.view());       // stopped: silence, no advance
    NXF_CHECK(output.sample(0, 100) == 0.0f);
    NXF_CHECK(engine.transport().positionFrames() == 0);
    engine.transport().play();
    engine.render(input.constView(), output.view());
    NXF_CHECK(output.sample(0, 100) != 0.0f);
    NXF_CHECK(engine.transport().positionFrames() == 256);
    engine.transport().stop();
    // through the Phase 3 offline stream, exactly like a device callback
    OfflineAudioBackend backend;
    auto stream = std::move(backend.createStream().value());
    CaptureCallback callback(engine);
    AudioStreamConfig config; config.outputChannels = 2; config.framesPerCallback = 256;
    NXF_CHECK(stream->open(config, callback).isSuccess());
    NXF_CHECK(stream->start().isSuccess());
    engine.transport().play();
    auto offline = dynamic_cast<OfflineAudioStream*>(stream.get());
    NXF_CHECK(offline->processBlock().isSuccess());
    NXF_CHECK(callback.peak.load() > 0.1f);
    NXF_CHECK(stream->stop().isSuccess());
    stream->close();
    return nxf::test::report("AudioEngineTests");
}
