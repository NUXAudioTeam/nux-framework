#pragma once

#include <nxf/midi/MidiDevice.h>

#include <memory>

namespace nxf::midi {

/// Windows: the WinMM backend. Platforms without a backend yet get
/// the offline backend - check name(); simulated is never hardware.
[[nodiscard]] std::unique_ptr<IMidiBackend> createSystemMidiBackend();

} // namespace nxf::midi
