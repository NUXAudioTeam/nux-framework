#include <nxf/dsp/Biquad.h>
#include <nxf/dsp/ThreeBandEq.h>
#include "TestSupport.h"
#include <cmath>
#include <vector>
namespace {
double rmsOf(const std::vector<float>& v, std::size_t from) {
    double s = 0; for (std::size_t i = from; i < v.size(); ++i) s += static_cast<double>(v[i]) * v[i];
    return std::sqrt(s / static_cast<double>(v.size() - from));
}
std::vector<float> sine(double freq, double sr, int n) {
    std::vector<float> v(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) v[static_cast<std::size_t>(i)] = static_cast<float>(std::sin(2.0 * 3.14159265358979 * freq * i / sr));
    return v;
}
}
int main() {
    using namespace nxf::dsp;
    constexpr double sr = 48000.0;
    {   // low-pass 1 kHz: 100 Hz passes (~1.0), 12 kHz is strongly cut
        auto input = sine(100.0, sr, 4800); auto out = input;
        Biquad lp; lp.setLowPass(sr, 1000.0, 0.7071); lp.process(out);
        NXF_CHECK(rmsOf(out, 2400) > 0.65 && rmsOf(out, 2400) < 0.76);
        auto input2 = sine(12000.0, sr, 4800); auto out2 = input2;
        Biquad lp2; lp2.setLowPass(sr, 1000.0, 0.7071); lp2.process(out2);
        NXF_CHECK(rmsOf(out2, 2400) < 0.05);
        // DC passes a low-pass untouched once settled
        std::vector<float> dc(480, 1.0f); Biquad lp3; lp3.setLowPass(sr, 1000.0, 0.7071); lp3.process(dc);
        NXF_CHECK(std::fabs(dc[479] - 1.0f) < 1e-3f);
    }
    {   // high-pass 1 kHz: DC dies, 12 kHz passes
        std::vector<float> dc(4800, 1.0f); Biquad hp; hp.setHighPass(sr, 1000.0, 0.7071); hp.process(dc);
        NXF_CHECK(std::fabs(dc[4799]) < 1e-3f);
        auto input = sine(12000.0, sr, 4800); auto out = input;
        Biquad hp2; hp2.setHighPass(sr, 1000.0, 0.7071); hp2.process(out);
        NXF_CHECK(rmsOf(out, 2400) > 0.65);
    }
    {   // peaking +6 dB at 1 kHz: tone at the centre roughly doubles
        auto input = sine(1000.0, sr, 4800); auto out = input;
        Biquad peak; peak.setPeaking(sr, 1000.0, 1.0, 6.0); peak.process(out);
        const double ratio = rmsOf(out, 2400) / rmsOf(input, 2400);
        NXF_CHECK(ratio > 1.8 && ratio < 2.2);
    }
    {   // flat EQ is transparent; +12 dB low shelf lifts a 100 Hz tone hard
        ThreeBandEq eq; eq.prepare(sr);
        auto input = sine(100.0, sr, 4800); auto flat = input; eq.process(flat);
        NXF_CHECK(std::fabs(rmsOf(flat, 2400) / rmsOf(input, 2400) - 1.0) < 0.01);
        ThreeBandEq boosted; boosted.prepare(sr); boosted.setLowGainDb(12.0);
        auto out = input; boosted.process(out);
        NXF_CHECK(rmsOf(out, 2400) / rmsOf(input, 2400) > 3.0);
    }
    return nxf::test::report("DspFilterTests");
}
