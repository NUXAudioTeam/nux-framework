#include <nxf/audio/Transport.h>
#include "TestSupport.h"
int main() {
    using namespace nxf::audio;
    Transport transport;
    NXF_CHECK(!transport.isPlaying());
    NXF_CHECK(transport.positionFrames() == 0);
    NXF_CHECK(transport.tempoBpm() == 120.0);
    transport.advance(480);                       // stopped: no movement
    NXF_CHECK(transport.positionFrames() == 0);
    transport.play();
    transport.advance(480);
    NXF_CHECK(transport.positionFrames() == 480);
    NXF_CHECK(Transport::framesPerBeat(48000.0, 120.0) == 24000.0);
    NXF_CHECK(transport.framesToSeconds(48000, 48000.0) == 1.0);
    transport.setLoop(1000, 2000, true);
    transport.seek(1500);
    transport.advance(600);                       // 2100 wraps to 1100
    NXF_CHECK(transport.positionFrames() == 1100);
    transport.stop();
    transport.advance(100);
    NXF_CHECK(transport.positionFrames() == 1100);
    transport.rewind();
    NXF_CHECK(transport.positionFrames() == 0);
    return nxf::test::report("TransportTests");
}
