#include <nxf/dsp/Reverb.h>

#include <algorithm>
#include <cmath>

namespace nxf::dsp {
namespace {
// Delay lengths at 44.1 kHz (scaled by sample rate in prepare()).
constexpr int combTunings[] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
constexpr int allPassTunings[] = {556, 441, 341, 225};
constexpr float allPassFeedback = 0.5f;
constexpr float inputGain = 0.25f;
} // namespace

float Reverb::Comb::process(float input, float feedback, float damping) noexcept
{
    if (buffer.empty()) return input;
    const float output = buffer[index];
    filterState = output * (1.0f - damping) + filterState * damping;
    buffer[index] = input + filterState * feedback;
    index = (index + 1) % buffer.size();
    return output;
}

float Reverb::AllPass::process(float input, float feedback) noexcept
{
    if (buffer.empty()) return input;
    const float buffered = buffer[index];
    const float output = -input + buffered;
    buffer[index] = input + buffered * feedback;
    index = (index + 1) % buffer.size();
    return output;
}

void Reverb::prepare(double sampleRate)
{
    const double scale = sampleRate / 44100.0;
    combs_.clear();
    for (const int tuning : combTunings) {
        Comb comb;
        comb.buffer.assign(static_cast<std::size_t>(std::max(1.0, tuning * scale)), 0.0f);
        combs_.push_back(std::move(comb));
    }
    allPasses_.clear();
    for (const int tuning : allPassTunings) {
        AllPass pass;
        pass.buffer.assign(static_cast<std::size_t>(std::max(1.0, tuning * scale)), 0.0f);
        allPasses_.push_back(std::move(pass));
    }
}

void Reverb::setRoomSize(float size) noexcept
{
    roomSize_ = size < 0.0f ? 0.0f : (size > 1.0f ? 1.0f : size);
}

void Reverb::setDamping(float damping) noexcept
{
    damping_ = damping < 0.0f ? 0.0f : (damping > 1.0f ? 1.0f : damping);
}

void Reverb::reset() noexcept
{
    for (auto& comb : combs_) {
        std::fill(comb.buffer.begin(), comb.buffer.end(), 0.0f);
        comb.index = 0;
        comb.filterState = 0.0f;
    }
    for (auto& pass : allPasses_) {
        std::fill(pass.buffer.begin(), pass.buffer.end(), 0.0f);
        pass.index = 0;
    }
}

float Reverb::processSample(float input) noexcept
{
    const float feedback = 0.7f + roomSize_ * 0.28f;
    float wet = 0.0f;
    for (auto& comb : combs_) wet += comb.process(input * inputGain, feedback, damping_);
    for (auto& pass : allPasses_) wet = pass.process(wet, allPassFeedback);
    return input * (1.0f - mix_) + wet * mix_;
}

} // namespace nxf::dsp
