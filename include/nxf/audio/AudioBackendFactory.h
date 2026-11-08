#pragma once

#include <nxf/audio/AudioDevice.h>

#include <memory>

namespace nxf::audio {

/// The platform's real backend: WASAPI on Windows. On platforms
/// without a backend yet this returns the offline backend - check
/// name() if the distinction matters; a simulated device is never
/// dressed up as hardware.
[[nodiscard]] std::unique_ptr<IAudioBackend> createSystemBackend();

} // namespace nxf::audio
