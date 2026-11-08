#pragma once

#include <nxf/core/FixedBuffer.h>

#include <cstddef>
#include <cstdint>

namespace nxf::audio {

/// Non-owning mutable view over interleaved float samples.
/// Layout: frame-major - sample(channel, frame) lives at
/// samples[frame * channels + channel]. Indices are preconditions,
/// not checked: views are used on the audio thread, where the frame
/// count is known before the block starts (REALTIME_AUDIO_RULES.md).
struct AudioBufferView {
    float* samples = nullptr;
    int channels = 0;
    int frames = 0;
    // Methods are const: the view is a handle - the samples it
    // points at stay mutable, exactly like std::span.
    [[nodiscard]] float& sample(int channel, int frame) const noexcept {
        return samples[static_cast<std::size_t>(frame) * static_cast<std::size_t>(channels) + static_cast<std::size_t>(channel)];
    }
    void setSample(int channel, int frame, float value) const noexcept { sample(channel, frame) = value; }
    void clear() const noexcept { for (int i = 0; i < frames * channels; ++i) samples[i] = 0.0f; }
};

/// Non-owning read-only view, same layout as AudioBufferView.
struct ConstAudioBufferView {
    const float* samples = nullptr;
    int channels = 0;
    int frames = 0;
    [[nodiscard]] float sample(int channel, int frame) const noexcept {
        return samples[static_cast<std::size_t>(frame) * static_cast<std::size_t>(channels) + static_cast<std::size_t>(channel)];
    }
};

/// An owning interleaved float audio buffer, allocated once at
/// construction (via nxf::core::FixedBuffer). Construction is not
/// real-time-safe; everything after it is allocation-free.
class AudioBuffer {
public:
    AudioBuffer() = default;
    AudioBuffer(int channels, int frames)
        : channels_(channels > 0 ? channels : 0),
          frames_(frames > 0 ? frames : 0),
          data_(static_cast<std::size_t>(channels_) * static_cast<std::size_t>(frames_)) {}

    [[nodiscard]] int channels() const noexcept { return channels_; }
    [[nodiscard]] int frames() const noexcept { return frames_; }
    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }
    [[nodiscard]] float sample(int channel, int frame) const noexcept { return constView().sample(channel, frame); }
    void setSample(int channel, int frame, float value) noexcept { view().setSample(channel, frame, value); }
    void clear() noexcept { data_.fill(0.0f); }
    [[nodiscard]] AudioBufferView view() noexcept { return {data_.data(), channels_, frames_}; }
    [[nodiscard]] ConstAudioBufferView constView() const noexcept { return {data_.data(), channels_, frames_}; }

private:
    int channels_ = 0;
    int frames_ = 0;
    nxf::core::FixedBuffer<float> data_;
};

} // namespace nxf::audio
