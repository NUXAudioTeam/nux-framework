#pragma once

#include <algorithm>
#include <cmath>

namespace nxf::dsp {

/// Level conversions used across the DSP module. gainToDecibels has
/// a floor of -100 dB so silence is a number, never -inf; the
/// functions are exact inverses above that floor.

[[nodiscard]] inline float decibelsToGain(double decibels) {
    return static_cast<float>(std::pow(10.0, decibels / 20.0));
}
[[nodiscard]] inline float gainToDecibels(float gain) {
    if (gain <= 0.0f) return -100.0f;
    return static_cast<float>((std::max)(-100.0, 20.0 * std::log10(static_cast<double>(gain))));
}

} // namespace nxf::dsp
