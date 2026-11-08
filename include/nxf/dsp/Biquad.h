#pragma once

#include <span>

namespace nxf::dsp {

/// A second-order IIR filter (RBJ cookbook designs) with double
/// precision coefficients and state. One instance processes one
/// channel; coefficients are set off the audio thread, process() is
/// allocation-free. Frequency/Q/gain setters fully redesign the
/// filter and keep the running state (call reset() to clear it).
class Biquad {
public:
    void setLowPass(double sampleRate, double frequencyHz, double q);
    void setHighPass(double sampleRate, double frequencyHz, double q);
    void setPeaking(double sampleRate, double frequencyHz, double q, double gainDb);
    void setLowShelf(double sampleRate, double frequencyHz, double q, double gainDb);
    void setHighShelf(double sampleRate, double frequencyHz, double q, double gainDb);
    void reset() noexcept { x1_ = x2_ = y1_ = y2_ = 0.0; }

    [[nodiscard]] float processSample(float input) noexcept {
        const double out = b0_ * input + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;
        x2_ = x1_; x1_ = input;
        y2_ = y1_; y1_ = out;
        return static_cast<float>(out);
    }
    void process(std::span<float> samples) noexcept {
        for (auto& sample : samples) sample = processSample(sample);
    }

private:
    void normalise(double a0, double b0, double b1, double b2, double a1, double a2) noexcept;
    double b0_ = 1.0, b1_ = 0.0, b2_ = 0.0, a1_ = 0.0, a2_ = 0.0;
    double x1_ = 0.0, x2_ = 0.0, y1_ = 0.0, y2_ = 0.0;
};

} // namespace nxf::dsp
