#include <nxf/dsp/Fft.h>
#include <nxf/dsp/Level.h>
#include "TestSupport.h"
#include <cmath>
#include <complex>
#include <vector>
int main() {
    using namespace nxf::dsp;
    NXF_CHECK(decibelsToGain(0.0) == 1.0f);
    NXF_CHECK(std::fabs(decibelsToGain(-6.0206) - 0.5) < 1e-3);
    NXF_CHECK(std::fabs(gainToDecibels(2.0f) - 6.0206) < 1e-3);
    NXF_CHECK(gainToDecibels(0.0f) == -100.0f);   // floor, never -inf

    Fft fft(1024);
    NXF_CHECK(fft.size() == 1024);
    {   // impulse: energy in every bin
        std::vector<std::complex<float>> data(1024);
        data[0] = 1.0f;
        fft.forward(data);
        for (const auto& bin : data) NXF_CHECK(std::fabs(std::abs(bin) - 1.0f) < 1e-3f);
    }
    {   // sine exactly on bin 8: only bins 8 and 1016 light up, magnitude N/2
        std::vector<std::complex<float>> data(1024);
        for (int i = 0; i < 1024; ++i) data[i] = std::sin(2.0 * 3.14159265358979 * 8.0 * i / 1024.0);
        fft.forward(data);
        NXF_CHECK(std::fabs(std::abs(data[8]) - 512.0f) < 1.0f);
        NXF_CHECK(std::abs(data[100]) < 0.01f);
        fft.inverse(data);   // round trip restores the sine
        NXF_CHECK(std::fabs(data[100].real() - static_cast<float>(std::sin(2.0 * 3.14159265358979 * 8.0 * 100 / 1024.0))) < 1e-3f);
        NXF_CHECK(std::fabs(data[100].imag()) < 1e-3f);
    }
    return nxf::test::report("DspFftTests");
}
