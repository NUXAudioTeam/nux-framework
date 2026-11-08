#include <nxf/platform/Timing.h>
#include <nxf/core/Time.h>
#include "TestSupport.h"
#include <cstdint>
int main() {
    using namespace nxf::platform;
    const std::uint64_t start = nxf::core::monotonicNanoseconds();
    sleepMilliseconds(25);
    const std::uint64_t elapsed = nxf::core::monotonicNanoseconds() - start;
    NXF_CHECK(elapsed >= 10'000'000ULL);       // at least ~10 ms (scheduler tolerant)
    NXF_CHECK(elapsed < 5'000'000'000ULL);     // and not absurdly more
    yieldThread();                             // must simply return
    return nxf::test::report("PlatformTimingTests");
}
