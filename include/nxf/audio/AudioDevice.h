#pragma once

#include <nxf/audio/AudioBuffer.h>
#include <nxf/core/Error.h>
#include <nxf/core/Result.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace nxf::audio {

/// One audio device as reported by a backend. IDs are backend-owned
/// strings: persist them in settings, never parse them.
struct AudioDeviceInfo {
    std::string id;
    std::string name;
    int maxInputChannels = 0;
    int maxOutputChannels = 0;
    double defaultSampleRate = 0.0;
    bool isDefaultInput = false;
    bool isDefaultOutput = false;
};

struct AudioStreamConfig {
    std::string outputDeviceId;   // empty = system default output
    std::string inputDeviceId;    // empty = system default input
    double sampleRate = 48000.0;
    int inputChannels = 0;
    int outputChannels = 2;
    int framesPerCallback = 512;  // a request/hint; devices may deliver other block sizes
};

/// Validates a stream configuration before any device is touched.
[[nodiscard]] nxf::core::Error validateAudioStreamConfig(const AudioStreamConfig& config);

enum class StreamState { Closed, Open, Running };

/// Implemented by the application/engine. processAudio runs on the
/// stream's audio thread and must obey docs/REALTIME_AUDIO_RULES.md:
/// no allocation, no blocking, no I/O. Views are interleaved float;
/// input is silence when the device underruns.
class IAudioCallback {
public:
    virtual ~IAudioCallback() = default;
    virtual void processAudio(const ConstAudioBufferView& input,
                              const AudioBufferView& output,
                              double sampleRate,
                              std::uint64_t streamTimeFrames) = 0;
};

/// A configured audio stream. Control methods (open/start/stop) are
/// called from non-audio threads; the callback runs on the stream's
/// own audio thread between start() and stop().
class IAudioStream {
public:
    virtual ~IAudioStream() = default;
    [[nodiscard]] virtual nxf::core::Error open(const AudioStreamConfig& config, IAudioCallback& callback) = 0;
    [[nodiscard]] virtual nxf::core::Error start() = 0;
    [[nodiscard]] virtual nxf::core::Error stop() = 0;
    virtual void close() = 0;
    [[nodiscard]] virtual StreamState state() const noexcept = 0;
    [[nodiscard]] virtual double sampleRate() const noexcept = 0;
    [[nodiscard]] virtual std::uint64_t streamTimeFrames() const noexcept = 0;
};

/// A source of devices and streams (WASAPI, offline, ...).
/// Enumeration may be called from any non-audio thread and must
/// report honestly: an empty list means no device was found, not an
/// error to hide.
class IAudioBackend {
public:
    virtual ~IAudioBackend() = default;
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual nxf::core::Result<std::vector<AudioDeviceInfo>> enumerateDevices() = 0;
    [[nodiscard]] virtual nxf::core::Result<std::unique_ptr<IAudioStream>> createStream() = 0;
};

} // namespace nxf::audio
