#include <nxf/core/Time.h>
#include "TestSupport.h"
#include <cstdint>
int main() {
    using namespace nxf::core;
    const std::uint64_t a = monotonicNanoseconds();
    const std::uint64_t b = monotonicNanoseconds();
    NXF_CHECK(b >= a);                       // never goes backwards
    NXF_CHECK(nanosecondsToSeconds(1500000000ULL) == 1.5);
    NXF_CHECK(nanosecondsToSeconds(0) == 0.0);
    NXF_CHECK(secondsToNanoseconds(2.0) == 2000000000ULL);
    return nxf::test::report("CoreTimeTests");
}
