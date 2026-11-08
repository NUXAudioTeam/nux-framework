#pragma once

#include <span>
#include <vector>

namespace nxf::dsp {

/// A Schroeder-style reverb: eight parallel damped comb filters
/// feeding four series all-pass filters (the classic freeverb
/// topology, implemented here from the published algorithm).
/// prepare() allocates; process() is allocation-free. Mono: run one
/// instance per channel with slightly different tunings for stereo.
class Reverb {
public:
    void prepare(double sampleRate);
    /// 0..1 - maps to comb feedback (decay time).
    void setRoomSize(float size) noexcept;
    /// 0..1 - low-pass damping inside the combs (brightness of the tail).
    void setDamping(float damping) noexcept;
    void setMix(float mix) noexcept { mix_ = mix < 0.0f ? 0.0f : (mix > 1.0f ? 1.0f : mix); }
    void reset() noexcept;

    [[nodiscard]] float processSample(float input) noexcept;
    void process(std::span<float> samples) noexcept { for (auto& s : samples) s = processSample(s); }

private:
    struct Comb {
        std::vector<float> buffer;
        std::size_t index = 0;
        float filterState = 0.0f;
        float process(float input, float feedback, float damping) noexcept;
    };
    struct AllPass {
        std::vector<float> buffer;
        std::size_t index = 0;
        float process(float input, float feedback) noexcept;
    };
    std::vector<Comb> combs_;
    std::vector<AllPass> allPasses_;
    float roomSize_ = 0.7f;
    float damping_ = 0.3f;
    float mix_ = 0.33f;
};

} // namespace nxf::dsp
