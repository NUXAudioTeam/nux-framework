#include <nxf/audio/AudioNode.h>

#include <cmath>

namespace nxf::audio {

void GainNode::process(const ConstAudioBufferView& input, const AudioBufferView& output, double, std::uint64_t)
{
    const float gain = gain_.load(std::memory_order_relaxed);
    for (int frame = 0; frame < output.frames; ++frame)
        for (int ch = 0; ch < output.channels; ++ch)
            output.setSample(ch, frame, (ch < input.channels ? input.sample(ch, frame) : 0.0f) * gain);
}

void SineSourceNode::process(const ConstAudioBufferView&, const AudioBufferView& output, double sampleRate, std::uint64_t)
{
    constexpr double twoPi = 6.28318530717958647692;
    const double increment = twoPi * frequency_.load(std::memory_order_relaxed) / sampleRate;
    const float amplitude = amplitude_.load(std::memory_order_relaxed);
    for (int frame = 0; frame < output.frames; ++frame) {
        const float value = amplitude * static_cast<float>(std::sin(phase_));
        phase_ += increment;
        if (phase_ >= twoPi) phase_ -= twoPi;
        for (int ch = 0; ch < output.channels; ++ch) output.setSample(ch, frame, value);
    }
}

void ConstantSourceNode::process(const ConstAudioBufferView&, const AudioBufferView& output, double, std::uint64_t)
{
    const float level = level_.load(std::memory_order_relaxed);
    for (int frame = 0; frame < output.frames; ++frame)
        for (int ch = 0; ch < output.channels; ++ch) output.setSample(ch, frame, level);
}

} // namespace nxf::audio
