#include <nxf/dsp/Compressor.h>
#include <nxf/dsp/Level.h>
#include "TestSupport.h"
#include <cmath>
#include <vector>
int main() {
    using namespace nxf::dsp;
    // Static curve: above threshold, output = T + (in - T)/ratio
    NXF_CHECK(std::fabs(Compressor::expectedOutputDb(-10.0, -20.0, 4.0) - (-17.5)) < 1e-9);
    NXF_CHECK(Compressor::expectedOutputDb(-30.0, -20.0, 4.0) == -30.0);  // below: untouched

    constexpr double sr = 48000.0;
    {   // quiet signal (peak 0.1 = -20 dBFS) under a -10 dB threshold: untouched
        Compressor comp; comp.prepare(sr); comp.setThresholdDb(-10.0); comp.setRatio(4.0);
        std::vector<float> v(4800, 0.1f); comp.process(v);
        NXF_CHECK(std::fabs(v[4799] - 0.1f) < 1e-4f);
        NXF_CHECK(comp.gainReductionDb() > -0.1);
    }
    {   // loud DC at 0.9 (-0.9 dBFS), threshold -20, ratio 10: gain pulled well down
        Compressor comp; comp.prepare(sr); comp.setThresholdDb(-20.0); comp.setRatio(10.0);
        comp.setAttackSeconds(0.001); comp.setReleaseSeconds(0.05);
        std::vector<float> v(48000, 0.9f); comp.process(v);
        NXF_CHECK(v[47999] > 0.0f && v[47999] < 0.35f);
        NXF_CHECK(comp.gainReductionDb() < -5.0);
    }
    return nxf::test::report("DspDynamicsTests");
}
