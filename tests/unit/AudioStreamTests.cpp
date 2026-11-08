#include <nxf/audio/AudioDevice.h>
#include <nxf/audio/OfflineAudioBackend.h>
#include "TestSupport.h"
#include <atomic>
namespace {
struct ProbeCallback final : nxf::audio::IAudioCallback {
    std::atomic<int> blocks{0};
    std::atomic<int> lastFrames{0};
    void processAudio(const nxf::audio::ConstAudioBufferView& input,
                      const nxf::audio::AudioBufferView& output,
                      double sampleRate, std::uint64_t streamTimeFrames) override {
        (void)streamTimeFrames;
        NXF_CHECK(sampleRate == 48000.0);
        for (int f = 0; f < output.frames; ++f)
            for (int ch = 0; ch < output.channels; ++ch)
                output.setSample(ch, f, input.channels > 0 ? input.sample(ch % input.channels, f) : 0.25f);
        ++blocks; lastFrames = output.frames;
    }
};
}
int main() {
    using namespace nxf::audio;
    OfflineAudioBackend backend;
    NXF_CHECK(backend.name() == "offline");
    const auto devices = backend.enumerateDevices();
    NXF_CHECK(devices.hasValue());
    NXF_CHECK(devices.value().size() == 1);
    NXF_CHECK(devices.value()[0].maxOutputChannels == 2);
    NXF_CHECK(devices.value()[0].maxInputChannels == 2);
    NXF_CHECK(devices.value()[0].defaultSampleRate == 48000.0);

    auto streamResult = backend.createStream();
    NXF_CHECK(streamResult.hasValue());
    auto stream = std::move(streamResult.value());
    ProbeCallback callback;
    AudioStreamConfig bad; bad.sampleRate = 0;
    NXF_CHECK(!stream->open(bad, callback).isSuccess());     // invalid config refused
    AudioStreamConfig config; config.inputChannels = 2; config.outputChannels = 2; config.framesPerCallback = 256;
    NXF_CHECK(stream->open(config, callback).isSuccess());
    NXF_CHECK(stream->state() == StreamState::Open);
    auto offline = dynamic_cast<OfflineAudioStream*>(stream.get());
    NXF_CHECK(offline != nullptr);
    NXF_CHECK(!offline->processBlock().isSuccess());         // not started yet
    NXF_CHECK(stream->start().isSuccess());
    NXF_CHECK(stream->state() == StreamState::Running);
    NXF_CHECK(offline->processBlock().isSuccess());
    NXF_CHECK(callback.blocks == 1);
    NXF_CHECK(callback.lastFrames == 256);
    NXF_CHECK(stream->streamTimeFrames() == 256);
    NXF_CHECK(stream->stop().isSuccess());
    NXF_CHECK(stream->state() == StreamState::Open);
    stream->close();
    NXF_CHECK(stream->state() == StreamState::Closed);
    stream->close();                                          // idempotent
    return nxf::test::report("AudioStreamTests");
}
