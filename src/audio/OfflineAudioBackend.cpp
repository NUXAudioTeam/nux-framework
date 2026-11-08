#include <nxf/audio/OfflineAudioBackend.h>

namespace nxf::audio {

nxf::core::Error OfflineAudioStream::open(const AudioStreamConfig& config, IAudioCallback& callback)
{
    if (state_ == StreamState::Running) return nxf::core::Error::unavailable("stop the stream before reopening");
    if (const auto valid = validateAudioStreamConfig(config); !valid) return valid;
    config_ = config;
    callback_ = &callback;
    inputBuffer_ = AudioBuffer{config.inputChannels, config.framesPerCallback};
    outputBuffer_ = AudioBuffer{config.outputChannels, config.framesPerCallback};
    inputBuffer_.clear();
    outputBuffer_.clear();
    streamTime_ = 0;
    state_ = StreamState::Open;
    return nxf::core::Error::success();
}

nxf::core::Error OfflineAudioStream::start()
{
    if (state_ != StreamState::Open) return nxf::core::Error::unavailable("stream is not open");
    state_ = StreamState::Running;
    return nxf::core::Error::success();
}

nxf::core::Error OfflineAudioStream::stop()
{
    if (state_ == StreamState::Running) state_ = StreamState::Open;
    return nxf::core::Error::success();
}

void OfflineAudioStream::close()
{
    (void)stop();
    callback_ = nullptr;
    state_ = StreamState::Closed;
}

nxf::core::Error OfflineAudioStream::processBlock()
{
    if (state_ != StreamState::Running || callback_ == nullptr)
        return nxf::core::Error::unavailable("stream is not running");
    callback_->processAudio(inputBuffer_.constView(), outputBuffer_.view(), config_.sampleRate, streamTime_.load());
    streamTime_ += static_cast<std::uint64_t>(config_.framesPerCallback);
    return nxf::core::Error::success();
}

nxf::core::Result<std::vector<AudioDeviceInfo>> OfflineAudioBackend::enumerateDevices()
{
    AudioDeviceInfo device;
    device.id = "nxf-offline";
    device.name = "NXF Offline Device (simulated, no hardware)";
    device.maxInputChannels = 2;
    device.maxOutputChannels = 2;
    device.defaultSampleRate = 48000.0;
    device.isDefaultInput = true;
    device.isDefaultOutput = true;
    return nxf::core::Result<std::vector<AudioDeviceInfo>>::success({device});
}

nxf::core::Result<std::unique_ptr<IAudioStream>> OfflineAudioBackend::createStream()
{
    return nxf::core::Result<std::unique_ptr<IAudioStream>>::success(
        std::unique_ptr<IAudioStream>{std::make_unique<OfflineAudioStream>()});
}

} // namespace nxf::audio
