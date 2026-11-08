#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace nxf::dsp {

/// A feedback delay over a circular buffer allocated in prepare().
/// Delay time, feedback and wet/dry mix are plain settings (call
/// them off the audio thread); process() is allocation-free and
/// sample-exact, which the echo tests pin down.
class DelayLine {
public:
    void prepare(double sampleRate, double maxDelaySeconds);
    void setDelaySamples(int samples) noexcept;
    void setDelaySeconds(double seconds) { setDelaySamples(static_cast<int>(seconds * sampleRate_)); }
    void setFeedback(float feedback) noexcept { feedback_ = feedback < 0.0f ? 0.0f : (feedback > 0.95f ? 0.95f : feedback); }
    void setMix(float mix) noexcept { mix_ = mix < 0.0f ? 0.0f : (mix > 1.0f ? 1.0f : mix); }
    void reset() noexcept;

    [[nodiscard]] float processSample(float input) noexcept;
    void process(std::span<float> samples) noexcept { for (auto& s : samples) s = processSample(s); }

private:
    std::vector<float> buffer_;
    double sampleRate_ = 0.0;
    std::size_t writeIndex_ = 0;
    int delaySamples_ = 0;
    float feedback_ = 0.35f;
    float mix_ = 0.35f;
};

} // namespace nxf::dsp
