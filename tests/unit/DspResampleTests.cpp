#include <nxf/dsp/Resampler.h>
#include "TestSupport.h"
#include <cmath>
#include <vector>
int main() {
    using namespace nxf::dsp;
    const std::vector<float> dc(480, 0.5f);
    const auto up = resampleLinear(dc, 2.0);          // 48k -> 96k
    NXF_CHECK(up.size() == 960);
    NXF_CHECK(std::fabs(up[500] - 0.5f) < 1e-5f);     // DC survives resampling
    const auto down = resampleLinear(dc, 0.5);        // 48k -> 24k
    NXF_CHECK(down.size() == 240);
    NXF_CHECK(std::fabs(down[100] - 0.5f) < 1e-5f);
    NXF_CHECK(resampleLinear(dc, 1.0) == dc);
    NXF_CHECK(resampleLinear({}, 2.0).empty());
    return nxf::test::report("DspResampleTests");
}
