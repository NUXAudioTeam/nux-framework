#include <nxf/dsp/Compressor.h>
#include <nxf/dsp/Level.h>

#include <algorithm>
#include <cmath>

namespace nxf::dsp {

void Compressor::prepare(double sampleRate)
{
    sampleRate_ = sampleRate;
    currentGainDb_ = 0.0;
    updateCoefficients();
}

void Compressor::updateCoefficients()
{
    if (sampleRate_ <= 0.0) return;
    attackCoeff_ = std::exp(-1.0 / (attackSeconds_ * sampleRate_));
    releaseCoeff_ = std::exp(-1.0 / (releaseSeconds_ * sampleRate_));
}

float Compressor::processSample(float input)
{
    const double inputDb = static_cast<double>(gainToDecibels(std::fabs(input)));
    const double desiredGainDb = (std::min)(0.0, expectedOutputDb(inputDb, thresholdDb_, ratio_) - inputDb);
    const double coeff = desiredGainDb < currentGainDb_ ? attackCoeff_ : releaseCoeff_;
    currentGainDb_ = coeff * currentGainDb_ + (1.0 - coeff) * desiredGainDb;
    return input * decibelsToGain(currentGainDb_ + makeupDb_);
}

} // namespace nxf::dsp
