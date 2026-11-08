#include <nxf/midi/MidiClock.h>
#include "TestSupport.h"
#include <cmath>
int main() {
    using namespace nxf::midi;
    MidiClockTracker clock;
    NXF_CHECK(!clock.isRunning());
    clock.process(MidiMessage::start(), 0);
    NXF_CHECK(clock.isRunning());
    NXF_CHECK(clock.beatPosition() == 0.0);
    // 120 BPM: a quarter note is 500 ms -> each of the 24 clocks is 500000/24 us
    std::uint64_t t = 0;
    for (int beat = 0; beat < 4; ++beat)
        for (int i = 0; i < 24; ++i) { t += 500000 / 24; clock.process(MidiMessage::timingClock(), t); }
    NXF_CHECK(std::fabs(clock.beatPosition() - 4.0) < 1e-9);
    NXF_CHECK(std::fabs(clock.tempoBpm() - 120.0) < 2.0);
    clock.process(MidiMessage::stop(), t);
    NXF_CHECK(!clock.isRunning());
    clock.process(MidiMessage::start(), t);            // restart resets position
    NXF_CHECK(clock.beatPosition() == 0.0);
    return nxf::test::report("MidiClockTests");
}
