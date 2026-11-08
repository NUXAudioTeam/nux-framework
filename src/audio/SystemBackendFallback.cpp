// Non-Windows system backend: no native backend exists yet, so the
// factory honestly returns the offline backend (see the header).
#include <nxf/audio/AudioBackendFactory.h>
#include <nxf/audio/OfflineAudioBackend.h>

namespace nxf::audio {

std::unique_ptr<IAudioBackend> createSystemBackend()
{
    return std::make_unique<OfflineAudioBackend>();
}

} // namespace nxf::audio
