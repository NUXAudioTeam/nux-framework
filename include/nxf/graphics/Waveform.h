#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace nxf::graphics {

/// The min/max envelope of one pixel column of audio.
struct WaveformPeak {
    float minimum = 0.0f;
    float maximum = 0.0f;
};

/// Reduces samples to one peak per output column - the standard
/// waveform overview computation, separated from drawing so it can
/// be tested exactly.
[[nodiscard]] std::vector<WaveformPeak> computeWaveformPeaks(std::span<const float> samples, int columns);

} // namespace nxf::graphics
