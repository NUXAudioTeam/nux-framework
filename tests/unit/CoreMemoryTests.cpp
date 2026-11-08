#include <nxf/core/FixedBuffer.h>
#include "TestSupport.h"
#include <cstddef>
int main() {
    using nxf::core::FixedBuffer;
    FixedBuffer<float> buf(16);
    NXF_CHECK(buf.size() == 16);
    NXF_CHECK(!buf.empty());
    buf.fill(0.5f);
    for (float x : buf.span()) NXF_CHECK(x == 0.5f);
    buf[3] = 1.0f;
    NXF_CHECK(buf.span()[3] == 1.0f);
    const FixedBuffer<int> emptyBuf(0);
    NXF_CHECK(emptyBuf.empty());
    NXF_CHECK(emptyBuf.size() == 0);
    FixedBuffer<float> movedBuf = std::move(buf);   // ownership transfers
    NXF_CHECK(movedBuf.size() == 16);
    NXF_CHECK(buf.size() == 0);
    return nxf::test::report("CoreMemoryTests");
}
