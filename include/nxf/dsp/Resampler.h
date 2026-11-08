#pragma once

#include <span>
#include <vector>

namespace nxf::dsp {

/// One-shot linear-interpolation resampling. ratio is
/// targetRate / sourceRate (2.0 doubles the sample count, 0.5 halves
/// it). Linear interpolation is deliberately the Phase 5 baseline:
/// simple, predictable and allocation-owning - it is a conversion
/// utility for import/render paths, not the final quality resampler
/// (a windowed-sinc design is future DSP work). Output length is
/// floor(input * ratio); DC and steady tones survive it (tested).
[[nodiscard]] std::vector<float> resampleLinear(std::span<const float> input, double ratio);

} // namespace nxf::dsp
