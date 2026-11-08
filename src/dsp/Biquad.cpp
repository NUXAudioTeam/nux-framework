#include <nxf/dsp/Biquad.h>

#include <cmath>

namespace nxf::dsp {
namespace {
constexpr double pi = 3.14159265358979323846;
struct Common { double alpha, cosW; };
Common common(double sampleRate, double frequency, double q) {
    const double w = 2.0 * pi * frequency / sampleRate;
    return {std::sin(w) / (2.0 * q), std::cos(w)};
}
} // namespace

void Biquad::normalise(double a0, double b0, double b1, double b2, double a1, double a2) noexcept
{
    b0_ = b0 / a0; b1_ = b1 / a0; b2_ = b2 / a0; a1_ = a1 / a0; a2_ = a2 / a0;
}

void Biquad::setLowPass(double sampleRate, double frequencyHz, double q)
{
    const auto c = common(sampleRate, frequencyHz, q);
    normalise(1.0 + c.alpha, (1.0 - c.cosW) / 2.0, 1.0 - c.cosW, (1.0 - c.cosW) / 2.0, -2.0 * c.cosW, 1.0 - c.alpha);
}

void Biquad::setHighPass(double sampleRate, double frequencyHz, double q)
{
    const auto c = common(sampleRate, frequencyHz, q);
    normalise(1.0 + c.alpha, (1.0 + c.cosW) / 2.0, -(1.0 + c.cosW), (1.0 + c.cosW) / 2.0, -2.0 * c.cosW, 1.0 - c.alpha);
}

void Biquad::setPeaking(double sampleRate, double frequencyHz, double q, double gainDb)
{
    const auto c = common(sampleRate, frequencyHz, q);
    const double a = std::pow(10.0, gainDb / 40.0);
    normalise(1.0 + c.alpha / a, 1.0 + c.alpha * a, -2.0 * c.cosW, 1.0 - c.alpha * a, -2.0 * c.cosW, 1.0 - c.alpha / a);
}

void Biquad::setLowShelf(double sampleRate, double frequencyHz, double q, double gainDb)
{
    const auto c = common(sampleRate, frequencyHz, q);
    const double a = std::pow(10.0, gainDb / 40.0);
    const double s = 2.0 * std::sqrt(a) * c.alpha;
    normalise((a + 1.0) + (a - 1.0) * c.cosW + s,
              a * ((a + 1.0) - (a - 1.0) * c.cosW + s),
              2.0 * a * ((a - 1.0) - (a + 1.0) * c.cosW),
              a * ((a + 1.0) - (a - 1.0) * c.cosW - s),
              -2.0 * ((a - 1.0) + (a + 1.0) * c.cosW),
              (a + 1.0) + (a - 1.0) * c.cosW - s);
}

void Biquad::setHighShelf(double sampleRate, double frequencyHz, double q, double gainDb)
{
    const auto c = common(sampleRate, frequencyHz, q);
    const double a = std::pow(10.0, gainDb / 40.0);
    const double s = 2.0 * std::sqrt(a) * c.alpha;
    normalise((a + 1.0) - (a - 1.0) * c.cosW + s,
              a * ((a + 1.0) + (a - 1.0) * c.cosW + s),
              -2.0 * a * ((a - 1.0) + (a + 1.0) * c.cosW),
              a * ((a + 1.0) + (a - 1.0) * c.cosW - s),
              2.0 * ((a - 1.0) - (a + 1.0) * c.cosW),
              (a + 1.0) - (a - 1.0) * c.cosW - s);
}

} // namespace nxf::dsp
