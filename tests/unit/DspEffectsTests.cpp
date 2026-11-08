#include <nxf/dsp/DelayLine.h>
#include <nxf/dsp/Reverb.h>
#include "TestSupport.h"
#include <cmath>
#include <vector>
int main() {
    using namespace nxf::dsp;
    {   // delay of exactly 4 samples, no feedback: one echo, sample-perfect
        DelayLine delay; delay.prepare(48000.0, 1.0); delay.setDelaySamples(4); delay.setFeedback(0.0f); delay.setMix(1.0f);
        std::vector<float> v(16, 0.0f); v[0] = 1.0f; delay.process(v);
        NXF_CHECK(v[0] == 0.0f);
        NXF_CHECK(v[4] == 1.0f);
        NXF_CHECK(v[8] == 0.0f);
    }
    {   // feedback 0.5: echoes decay 1, 0.5, 0.25
        DelayLine delay; delay.prepare(48000.0, 1.0); delay.setDelaySamples(4); delay.setFeedback(0.5f); delay.setMix(1.0f);
        std::vector<float> v(16, 0.0f); v[0] = 1.0f; delay.process(v);
        NXF_CHECK(std::fabs(v[4] - 1.0f) < 1e-6f);
        NXF_CHECK(std::fabs(v[8] - 0.5f) < 1e-6f);
        NXF_CHECK(std::fabs(v[12] - 0.25f) < 1e-6f);
    }
    {   // reverb: an impulse grows a tail that decays and stays finite/bounded
        Reverb reverb; reverb.prepare(48000.0); reverb.setMix(1.0f);
        std::vector<float> v(48000, 0.0f); v[0] = 1.0f; reverb.process(v);
        double peak = 0, early = 0, late = 0;
        for (int i = 0; i < 48000; ++i) {
            peak = std::max(peak, static_cast<double>(std::fabs(v[static_cast<std::size_t>(i)])));
            if (i >= 100 && i < 10000) early += std::fabs(v[static_cast<std::size_t>(i)]);
            if (i >= 38000) late += std::fabs(v[static_cast<std::size_t>(i)]);
        }
        NXF_CHECK(std::isfinite(peak));
        NXF_CHECK(peak > 0.01 && peak < 4.0);
        NXF_CHECK(early > 1.0);        // there really is a tail
        NXF_CHECK(late < early);       // and it decays
    }
    return nxf::test::report("DspEffectsTests");
}
