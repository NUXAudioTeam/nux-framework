#include <nxf/graphics/Waveform.h>

#include <algorithm>

namespace nxf::graphics {

std::vector<WaveformPeak> computeWaveformPeaks(std::span<const float> samples, int columns)
{
    if (samples.empty() || columns <= 0) return {};
    std::vector<WaveformPeak> peaks(static_cast<std::size_t>(columns));
    for (int column = 0; column < columns; ++column) {
        const auto begin = static_cast<std::size_t>(column) * samples.size() / static_cast<std::size_t>(columns);
        auto end = static_cast<std::size_t>(column + 1) * samples.size() / static_cast<std::size_t>(columns);
        if (end <= begin) end = begin + 1;
        float low = samples[begin];
        float high = samples[begin];
        for (std::size_t i = begin; i < end && i < samples.size(); ++i) {
            low = (std::min)(low, samples[i]);
            high = (std::max)(high, samples[i]);
        }
        peaks[static_cast<std::size_t>(column)] = {low, high};
    }
    return peaks;
}

} // namespace nxf::graphics
