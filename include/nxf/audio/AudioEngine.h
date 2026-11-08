#pragma once

#include <nxf/audio/AudioDevice.h>
#include <nxf/audio/AudioGraph.h>
#include <nxf/audio/Transport.h>

namespace nxf::audio {

/// The Phase 4 engine: a prepared AudioGraph driven by a Transport.
///
/// render() is the audio-thread entry point (also used directly for
/// offline rendering in tests): while the transport is playing it
/// renders the graph and advances the position; while stopped it
/// writes silence and the position does not move. It allocates and
/// locks nothing. Structural changes go through graph() and must be
/// followed by prepare() while no rendering is happening.
class AudioEngine {
public:
    [[nodiscard]] nxf::core::Error prepare(double sampleRate, int channels, int maxFrames) {
        sampleRate_ = sampleRate;
        return graph_.prepare(sampleRate, channels, maxFrames);
    }

    void render(const ConstAudioBufferView& input, const AudioBufferView& output) {
        if (!transport_.isPlaying() || !graph_.isPrepared()) {
            output.clear();
            return;
        }
        if (graph_.process(input, output, transport_.positionFrames()).isSuccess())
            transport_.advance(static_cast<std::uint64_t>(output.frames));
        else
            output.clear();
    }

    [[nodiscard]] AudioGraph& graph() noexcept { return graph_; }
    [[nodiscard]] Transport& transport() noexcept { return transport_; }
    [[nodiscard]] double sampleRate() const noexcept { return sampleRate_; }

private:
    AudioGraph graph_;
    Transport transport_;
    double sampleRate_ = 0.0;
};

/// Adapts an AudioEngine to a stream callback (Phase 3 interface):
/// hand this to IAudioStream::open() and the engine becomes the
/// device's audio processor.
class EngineCallback final : public IAudioCallback {
public:
    explicit EngineCallback(AudioEngine& engine) : engine_(engine) {}
    void processAudio(const ConstAudioBufferView& input, const AudioBufferView& output,
                      double, std::uint64_t) override {
        engine_.render(input, output);
    }
private:
    AudioEngine& engine_;
};

} // namespace nxf::audio
