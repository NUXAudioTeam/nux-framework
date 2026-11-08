// Non-Windows: no native MIDI backend yet, so the factory honestly
// returns the offline backend (see the factory header).
#include <nxf/midi/MidiBackendFactory.h>
#include <nxf/midi/OfflineMidiBackend.h>

namespace nxf::midi {

std::unique_ptr<IMidiBackend> createSystemMidiBackend()
{
    return std::make_unique<OfflineMidiBackend>();
}

} // namespace nxf::midi
