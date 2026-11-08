#pragma once

#include <nxf/dsp/Biquad.h>

#include <span>

namespace nxf::dsp {

/// Low shelf (200 Hz) + peaking (1 kHz) + high shelf (4 kHz) in
/// series - the classic channel EQ shape. Gains are in dB; at 0 dB
/// everywhere the EQ is transparent (pinned by a test).
class ThreeBandEq {
public:
    void prepare(double sampleRate) {
        sampleRate_ = sampleRate;
        update();
    }
    void setLowGainDb(double db) { lowDb_ = db; update(); }
    void setMidGainDb(double db) { midDb_ = db; update(); }
    void setHighGainDb(double db) { highDb_ = db; update(); }
    void reset() { low_.reset(); mid_.reset(); high_.reset(); }
    void process(std::span<float> samples) {
        low_.process(samples);
        mid_.process(samples);
        high_.process(samples);
    }

private:
    void update() {
        if (sampleRate_ <= 0.0) return;
        low_.setLowShelf(sampleRate_, 200.0, 0.7071, lowDb_);
        mid_.setPeaking(sampleRate_, 1000.0, 0.9, midDb_);
        high_.setHighShelf(sampleRate_, 4000.0, 0.7071, highDb_);
    }
    Biquad low_, mid_, high_;
    double sampleRate_ = 0.0;
    double lowDb_ = 0.0, midDb_ = 0.0, highDb_ = 0.0;
};

} // namespace nxf::dsp
