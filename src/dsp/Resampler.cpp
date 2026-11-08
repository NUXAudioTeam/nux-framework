#include <nxf/dsp/Resampler.h>

#include <cmath>

namespace nxf::dsp {

std::vector<float> resampleLinear(std::span<const float> input, double ratio)
{
    if (input.empty() || ratio <= 0.0) return {};
    if (ratio == 1.0) return {input.begin(), input.end()};
    const auto outSize = static_cast<std::size_t>(static_cast<double>(input.size()) * ratio);
    std::vector<float> output;
    output.reserve(outSize);
    for (std::size_t i = 0; i < outSize; ++i) {
        const double position = static_cast<double>(i) / ratio;
        const auto index = static_cast<std::size_t>(position);
        const float fraction = static_cast<float>(position - static_cast<double>(index));
        const float a = input[index];
        const float b = input[std::min(index + 1, input.size() - 1)];
        output.push_back(a + (b - a) * fraction);
    }
    return output;
}

} // namespace nxf::dsp
