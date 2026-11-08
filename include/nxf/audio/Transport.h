#pragma once

#include <atomic>
#include <cstdint>

namespace nxf::audio {

/// Playback position and tempo, shared between the UI (which calls
/// play/stop/seek) and the audio side (which calls advance() once per
/// rendered block). Every field is atomic, so no thread ever locks.
/// Positions are in frames at the current stream's sample rate.
class Transport {
public:
    void play() noexcept { playing_.store(true); }
    void stop() noexcept { playing_.store(false); }
    [[nodiscard]] bool isPlaying() const noexcept { return playing_.load(); }

    void seek(std::uint64_t frame) noexcept { position_.store(frame); }
    void rewind() noexcept { position_.store(0); }
    [[nodiscard]] std::uint64_t positionFrames() const noexcept { return position_.load(); }

    void setTempoBpm(double bpm) noexcept { bpm_.store(bpm > 0.0 ? bpm : 120.0); }
    [[nodiscard]] double tempoBpm() const noexcept { return bpm_.load(); }

    /// Loop over [startFrame, endFrame) while enabled.
    void setLoop(std::uint64_t startFrame, std::uint64_t endFrame, bool enabled) noexcept {
        loopStart_.store(startFrame);
        loopEnd_.store(endFrame);
        loopEnabled_.store(enabled && endFrame > startFrame);
    }
    [[nodiscard]] bool loopEnabled() const noexcept { return loopEnabled_.load(); }

    /// Audio side: move the position after rendering a block.
    /// Does nothing while stopped (a stopped transport never drifts).
    void advance(std::uint64_t frames) noexcept {
        if (!playing_.load()) return;
        std::uint64_t next = position_.load() + frames;
        if (loopEnabled_.load()) {
            const std::uint64_t start = loopStart_.load();
            const std::uint64_t end = loopEnd_.load();
            if (end > start && next >= end) {
                const std::uint64_t length = end - start;
                next = start + (next - start) % length;
            }
        }
        position_.store(next);
    }

    [[nodiscard]] static constexpr double framesPerBeat(double sampleRate, double bpm) noexcept {
        return sampleRate * 60.0 / bpm;
    }
    [[nodiscard]] static constexpr double framesToSeconds(std::uint64_t frames, double sampleRate) noexcept {
        return static_cast<double>(frames) / sampleRate;
    }

private:
    std::atomic<bool> playing_{false};
    std::atomic<std::uint64_t> position_{0};
    std::atomic<double> bpm_{120.0};
    std::atomic<bool> loopEnabled_{false};
    std::atomic<std::uint64_t> loopStart_{0};
    std::atomic<std::uint64_t> loopEnd_{0};
};

} // namespace nxf::audio
