#pragma once

#include <nxf/audio/AudioBuffer.h>

#include <atomic>
#include <cstdint>

namespace nxf::audio {

/// One processing step in an AudioGraph. process() runs on the audio
/// thread: no allocation, no blocking, no I/O (REALTIME_AUDIO_RULES).
/// prepare() is called off the audio thread before rendering starts,
/// whenever the sample rate / channel count / maximum block size is
/// known; implementations allocate there, never in process().
/// Parameters that the UI changes while audio runs are atomics in
/// the concrete nodes below - that is the required pattern.
class IAudioNode {
public:
    virtual ~IAudioNode() = default;
    virtual void prepare(double sampleRate, int channels, int maxFrames) = 0;
    virtual void process(const ConstAudioBufferView& input, const AudioBufferView& output,
                         double sampleRate, std::uint64_t streamTimeFrames) = 0;
};

/// Multiplies its input by a gain. The gain is an atomic: the UI may
/// change it while the audio thread reads it, with no lock.
class GainNode final : public IAudioNode {
public:
    void setGain(float gain) noexcept { gain_.store(gain); }
    [[nodiscard]] float gain() const noexcept { return gain_.load(); }
    void prepare(double, int, int) override {}
    void process(const ConstAudioBufferView& input, const AudioBufferView& output, double, std::uint64_t) override;
private:
    std::atomic<float> gain_{1.0f};
};

/// Generates a sine wave (ignores its input). A signal source for
/// instruments and tests - the DSP building blocks live in Phase 5.
class SineSourceNode final : public IAudioNode {
public:
    void setFrequency(double hertz) noexcept { frequency_.store(hertz); }
    void setAmplitude(float amplitude) noexcept { amplitude_.store(amplitude); }
    void prepare(double, int, int) override { phase_ = 0.0; }
    void process(const ConstAudioBufferView& input, const AudioBufferView& output,
                 double sampleRate, std::uint64_t) override;
private:
    std::atomic<double> frequency_{440.0};
    std::atomic<float> amplitude_{0.5f};
    double phase_ = 0.0;   // audio thread only
};

/// Outputs a constant level (ignores its input). Mainly a test and
/// calibration source: its output is exact, which proofs rely on.
class ConstantSourceNode final : public IAudioNode {
public:
    void setLevel(float level) noexcept { level_.store(level); }
    void prepare(double, int, int) override {}
    void process(const ConstAudioBufferView& input, const AudioBufferView& output, double, std::uint64_t) override;
private:
    std::atomic<float> level_{0.0f};
};

} // namespace nxf::audio
