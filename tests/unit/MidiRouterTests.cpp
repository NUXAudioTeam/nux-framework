#include <nxf/midi/MidiRouter.h>
#include "TestSupport.h"
int main() {
    using namespace nxf::midi;
    MidiRouter router;
    MidiRoute remap; remap.inputChannel = 1; remap.outputChannel = 10; remap.transposeSemitones = 12;
    router.addRoute(remap);
    const auto out = router.route(MidiMessage::noteOn(1, 60, 100));
    NXF_CHECK(out.size() == 1);
    NXF_CHECK(out[0].channel() == 10);
    NXF_CHECK(out[0].noteNumber() == 72);
    NXF_CHECK(router.route(MidiMessage::noteOn(2, 60, 100)).empty());   // other channel: no route
    const auto cc = router.route(MidiMessage::controlChange(1, 7, 90));
    NXF_CHECK(cc.size() == 1); NXF_CHECK(cc[0].channel() == 10); NXF_CHECK(cc[0].noteNumber() == -1);
    NXF_CHECK(router.route(MidiMessage::noteOn(1, 120, 100)).empty());  // transposed out of range: dropped, not wrapped

    MidiRouter passthrough;
    MidiRoute all; all.inputChannel = 0;               // any channel, keep channel
    passthrough.addRoute(all);
    NXF_CHECK(passthrough.route(MidiMessage::noteOn(5, 60, 1)).size() == 1);
    NXF_CHECK(passthrough.route(MidiMessage::noteOn(5, 60, 1))[0].channel() == 5);
    MidiRoute notesOnly; notesOnly.inputChannel = 0; notesOnly.passControlChange = false;
    MidiRouter filtered; filtered.addRoute(notesOnly);
    NXF_CHECK(filtered.route(MidiMessage::controlChange(1, 1, 1)).empty());
    return nxf::test::report("MidiRouterTests");
}
