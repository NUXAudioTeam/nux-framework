#pragma once

#include <nxf/midi/MidiMessage.h>

#include <vector>

namespace nxf::midi {

/// One routing rule. inputChannel 0 means "any channel";
/// outputChannel 0 means "keep the input channel". Notes can be
/// transposed (a transposed note outside 0..127 is dropped, never
/// wrapped). Per-type switches filter whole message families.
struct MidiRoute {
    int inputChannel = 0;
    int outputChannel = 0;
    int transposeSemitones = 0;
    bool passNotes = true;
    bool passControlChange = true;
    bool passPitchBend = true;
    bool passProgramChange = true;
    bool passOther = true;
};

/// Applies routing rules to messages. A message that matches no
/// rule produces no output. Configuration (addRoute) happens off
/// the processing path; route() itself is a pure function of the
/// message and the rules.
class MidiRouter {
public:
    void addRoute(MidiRoute route) { routes_.push_back(route); }
    void clear() { routes_.clear(); }
    [[nodiscard]] std::vector<MidiMessage> route(const MidiMessage& message) const;

private:
    std::vector<MidiRoute> routes_;
};

} // namespace nxf::midi
