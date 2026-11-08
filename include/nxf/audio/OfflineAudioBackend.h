#pragma once

#include <nxf/audio/AudioDevice.h>

#include <atomic>

namespace nxf::audio {

/// A stream with no hardware behind it. It never calls the callback
/// by itself: the owner pumps processBlock() explicitly, which makes
/// stream lifecycle and callback logic testable offline and lets
/// applications render without a device. The callback therefore runs
/// on the caller's thread - this is a test/offline tool, not a
/// real-time device, and it is named and documented as such.
class OfflineAudioStream final : public IAudioStream {
public:
    [[nodiscard]] nxf::core::Error open(const AudioStreamConfig& config, IAudioCallback& callback) override;
    [[nodiscard]] nxf::core::Error start() override;
    [[nodiscard]] nxf::core::Error stop() override;
    void close() override;
    [[nodiscard]] StreamState state() const noexcept override { return state_; }
    [[nodiscard]] double sampleRate() const noexcept override { return config_.sampleRate; }
    [[nodiscard]] std::uint64_t streamTimeFrames() const noexcept override { return streamTime_.load(); }

    /// Delivers exactly one block (config.framesPerCallback frames of
    /// silent input) to the callback. Running state only.
    [[nodiscard]] nxf::core::Error processBlock();

private:
    AudioStreamConfig config_;
    IAudioCallback* callback_ = nullptr;
    StreamState state_ = StreamState::Closed;
    AudioBuffer inputBuffer_;
    AudioBuffer outputBuffer_;
    std::atomic<std::uint64_t> streamTime_{0};
};

/// Backend that enumerates a single simulated device. Named
/// "offline" so a simulated pass can never be mistaken for hardware
/// (the same honesty rule as the verification ladder in NUX DAW).
class OfflineAudioBackend final : public IAudioBackend {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "offline"; }
    [[nodiscard]] nxf::core::Result<std::vector<AudioDeviceInfo>> enumerateDevices() override;
    [[nodiscard]] nxf::core::Result<std::unique_ptr<IAudioStream>> createStream() override;
};

} // namespace nxf::audio
