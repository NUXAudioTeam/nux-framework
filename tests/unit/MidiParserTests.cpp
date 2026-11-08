#include <nxf/midi/MidiStreamParser.h>
#include <nxf/midi/MidiEventQueue.h>
#include "TestSupport.h"
#include <vector>
int main() {
    using namespace nxf::midi;
    std::vector<MidiMessage> got;
    MidiStreamParser parser([&](const MidiMessage& m) { got.push_back(m); });
    parser.feed({0x90, 60, 100, 62, 90});              // running status: two note-ons
    NXF_CHECK(got.size() == 2);
    NXF_CHECK(got[0].noteNumber() == 60);
    NXF_CHECK(got[1].noteNumber() == 62); NXF_CHECK(got[1].velocity() == 90);
    got.clear();
    parser.feed({0xF8, 0x90, 64, 10, 0xF8});           // realtime clock interleaved, harms nothing
    NXF_CHECK(got.size() == 3);
    NXF_CHECK(got[0].type() == MidiMessageType::TimingClock);
    NXF_CHECK(got[1].noteNumber() == 64);
    NXF_CHECK(got[2].type() == MidiMessageType::TimingClock);
    got.clear();
    parser.feed({0xC0, 5});                            // program change (1 data byte)
    NXF_CHECK(got.size() == 1);
    NXF_CHECK(got[0].type() == MidiMessageType::ProgramChange);
    got.clear();
    parser.feed({0xF0, 0x7E, 0x01}); parser.feed({0x02, 0xF7});   // sysex across chunks
    NXF_CHECK(got.size() == 1);
    NXF_CHECK(got[0].type() == MidiMessageType::SystemExclusive);
    NXF_CHECK(got[0].sysexData().size() == 5);
    got.clear();
    parser.feed({0x90, 70}); parser.feed({80});        // message split across feeds
    NXF_CHECK(got.size() == 1); NXF_CHECK(got[0].velocity() == 80);

    MidiEventQueue queue;                              // SPSC event queue (core queue underneath)
    NXF_CHECK(queue.tryPush(MidiEvent{MidiMessage::noteOn(1, 60, 100), 1234}));
    NXF_CHECK(queue.tryPush(MidiEvent{MidiMessage::noteOff(1, 60, 0), 2000}));
    MidiEvent event;
    NXF_CHECK(queue.tryPop(event)); NXF_CHECK(event.timestampFrames == 1234); NXF_CHECK(event.message.isNoteOn());
    NXF_CHECK(queue.tryPop(event)); NXF_CHECK(event.timestampFrames == 2000);
    NXF_CHECK(!queue.tryPop(event));
    return nxf::test::report("MidiParserTests");
}
