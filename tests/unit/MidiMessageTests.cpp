#include <nxf/midi/MidiMessage.h>
#include "TestSupport.h"
int main() {
    using namespace nxf::midi;
    const auto on = MidiMessage::noteOn(1, 60, 100);
    NXF_CHECK(on.type() == MidiMessageType::NoteOn);
    NXF_CHECK(on.isNoteOn());
    NXF_CHECK(!on.isNoteOff());
    NXF_CHECK(on.channel() == 1);
    NXF_CHECK(on.noteNumber() == 60);
    NXF_CHECK(on.velocity() == 100);
    NXF_CHECK(on.size() == 3);
    NXF_CHECK(on.byte(0) == 0x90); NXF_CHECK(on.byte(1) == 60); NXF_CHECK(on.byte(2) == 100);
    const auto vel0 = MidiMessage::noteOn(1, 60, 0);
    NXF_CHECK(vel0.isNoteOff());                       // note-on at velocity 0 IS note-off
    const auto off = MidiMessage::noteOff(16, 127, 64);
    NXF_CHECK(off.channel() == 16); NXF_CHECK(off.byte(0) == 0x8F);
    const auto cc = MidiMessage::controlChange(2, 7, 90);
    NXF_CHECK(cc.type() == MidiMessageType::ControlChange);
    NXF_CHECK(cc.controllerNumber() == 7); NXF_CHECK(cc.controllerValue() == 90);
    const auto bendUp = MidiMessage::pitchBend(1, 16383);
    NXF_CHECK(bendUp.pitchBend() == 16383);
    NXF_CHECK(MidiMessage::pitchBend(1, 8192).pitchBend() == 8192);
    const auto parsed = MidiMessage::fromBytes({0x93, 65, 0x7F});
    NXF_CHECK(parsed.hasValue());
    NXF_CHECK(parsed.value().channel() == 4);
    NXF_CHECK(parsed.value().noteNumber() == 65);
    NXF_CHECK(!MidiMessage::fromBytes({0x90, 60}).hasValue());       // truncated
    NXF_CHECK(!MidiMessage::fromBytes({60, 100}).hasValue());        // no status byte
    NXF_CHECK(!MidiMessage::noteOn(0, 60, 100).isNoteOn());           // invalid channel: inert
    NXF_CHECK(MidiMessage::timingClock().type() == MidiMessageType::TimingClock);
    NXF_CHECK(MidiMessage::timingClock().size() == 1);
    return nxf::test::report("MidiMessageTests");
}
