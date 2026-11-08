#include <nxf/dsp/DelayLine.h>

#include <algorithm>
#include <cmath>

namespace nxf::dsp {

void DelayLine::prepare(double sampleRate, double maxDelaySeconds)
{
    sampleRate_ = sampleRate;
    buffer_.assign(static_cast<std::size_t>(std::ceil(sampleRate * maxDelaySeconds)) + 1, 0.0f);
    writeIndex_ = 0;
    delaySamples_ = 0;
}

void DelayLine::setDelaySamples(int samples) noexcept
{
    if (buffer_.empty()) { delaySamples_ = 0; return; }
    delaySamples_ = (std::max)(0, (std::min)(samples, static_cast<int>(buffer_.size()) - 1));
}

void DelayLine::reset() noexcept
{
    std::fill(buffer_.begin(), buffer_.end(), 0.0f);
    writeIndex_ = 0;
}

float DelayLine::processSample(float input) noexcept
{
    if (buffer_.empty()) return input;
    const std::size_t size = buffer_.size();
    const std::size_t readIndex = (writeIndex_ + size - static_cast<std::size_t>(delaySamples_)) % size;
    const float delayed = buffer_[readIndex];
    buffer_[writeIndex_] = input + delayed * feedback_;
    writeIndex_ = (writeIndex_ + 1) % size;
    return input * (1.0f - mix_) + delayed * mix_;
}

} // namespace nxf::dsp
