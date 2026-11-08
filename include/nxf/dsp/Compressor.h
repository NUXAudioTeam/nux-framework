#pragma once

#include <span>

namespace nxf::dsp {

/// A feed-forward compressor with attack/release envelope smoothing.
/// One instance per channel. The static curve is exposed
/// (expectedOutputDb) so tools and tests can reason about the
/// processor without running audio through it.
class Compressor {
public:
    void prepare(double sampleRate);
    void setThresholdDb(double db) { thresholdDb_ = db; }
    void setRatio(double ratio) { ratio_ = ratio < 1.0 ? 1.0 : ratio; }
    void setAttackSeconds(double seconds) { attackSeconds_ = seconds; updateCoefficients(); }
    void setReleaseSeconds(double seconds) { releaseSeconds_ = seconds; updateCoefficients(); }
    void setMakeupGainDb(double db) { makeupDb_ = db; }
    [[nodiscard]] double gainReductionDb() const noexcept { return currentGainDb_; }

    [[nodiscard]] static constexpr double expectedOutputDb(double inputDb, double thresholdDb, double ratio) noexcept {
        return inputDb <= thresholdDb ? inputDb : thresholdDb + (inputDb - thresholdDb) / (ratio < 1.0 ? 1.0 : ratio);
    }

    [[nodiscard]] float processSample(float input);
    void process(std::span<float> samples) { for (auto& s : samples) s = processSample(s); }

private:
    void updateCoefficients();
    double sampleRate_ = 0.0;
    double thresholdDb_ = -18.0;
    double ratio_ = 3.0;
    double attackSeconds_ = 0.003;
    double releaseSeconds_ = 0.25;
    double makeupDb_ = 0.0;
    double attackCoeff_ = 0.0;
    double releaseCoeff_ = 0.0;
    double currentGainDb_ = 0.0;
};

} // namespace nxf::dsp
