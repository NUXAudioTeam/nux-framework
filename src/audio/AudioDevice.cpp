#include <nxf/audio/AudioDevice.h>

namespace nxf::audio {

nxf::core::Error validateAudioStreamConfig(const AudioStreamConfig& config)
{
    using nxf::core::Error;
    if (config.sampleRate <= 0.0) return Error::invalidArgument("sample rate must be positive");
    if (config.inputChannels < 0 || config.outputChannels < 0) return Error::invalidArgument("channel counts cannot be negative");
    if (config.inputChannels == 0 && config.outputChannels == 0) return Error::invalidArgument("a stream needs input or output channels");
    if (config.framesPerCallback <= 0) return Error::invalidArgument("frames per callback must be positive");
    return Error::success();
}

} // namespace nxf::audio
