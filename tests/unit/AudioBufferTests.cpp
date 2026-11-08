#include <nxf/audio/AudioBuffer.h>
#include "TestSupport.h"
int main() {
    using nxf::audio::AudioBuffer;
    AudioBuffer buf(2, 4);
    NXF_CHECK(buf.channels() == 2);
    NXF_CHECK(buf.frames() == 4);
    NXF_CHECK(buf.size() == 8);
    buf.setSample(0, 0, 0.5f); buf.setSample(1, 0, -0.5f); buf.setSample(1, 3, 1.0f);
    NXF_CHECK(buf.sample(0, 0) == 0.5f);
    NXF_CHECK(buf.sample(1, 0) == -0.5f);
    NXF_CHECK(buf.sample(1, 3) == 1.0f);
    NXF_CHECK(buf.sample(0, 1) == 0.0f);
    buf.clear();
    NXF_CHECK(buf.sample(1, 3) == 0.0f);
    auto view = buf.view();
    NXF_CHECK(view.channels == 2); NXF_CHECK(view.frames == 4);
    view.setSample(0, 2, 0.25f);
    NXF_CHECK(buf.sample(0, 2) == 0.25f);
    const auto& constBuf = buf;
    NXF_CHECK(constBuf.constView().sample(0, 2) == 0.25f);
    AudioBuffer empty(0, 0);
    NXF_CHECK(empty.empty());
    return nxf::test::report("AudioBufferTests");
}
