#pragma once

#include <nxf/midi/MidiMessage.h>

#include <array>
#include <cstddef>
#include <cstdint>

namespace nxf::midi {

/// Follows an incoming MIDI clock (24 pulses per quarter note).
/// Feed every message with a microsecond timestamp; the tracker
/// reports run state, beat position and a tempo estimate averaged
/// over the most recent clock intervals. Pure logic - tested by
/// simulating a 120 BPM clock.
class MidiClockTracker {
public:
    void process(const MidiMessage& message, std::uint64_t timestampMicroseconds) {
        switch (message.type()) {
            case MidiMessageType::Start:
                running_ = true;
                clockCount_ = 0;
                intervalCount_ = 0;
                intervalIndex_ = 0;
                hasLastTimestamp_ = false;
                break;
            case MidiMessageType::Continue:
                running_ = true;
                hasLastTimestamp_ = false;
                break;
            case MidiMessageType::Stop:
                running_ = false;
                break;
            case MidiMessageType::TimingClock:
                if (!running_) break;
                ++clockCount_;
                if (hasLastTimestamp_ && timestampMicroseconds > lastTimestamp_) {
                    intervals_[intervalIndex_] = timestampMicroseconds - lastTimestamp_;
                    intervalIndex_ = (intervalIndex_ + 1) % intervals_.size();
                    if (intervalCount_ < intervals_.size()) ++intervalCount_;
                }
                lastTimestamp_ = timestampMicroseconds;
                hasLastTimestamp_ = true;
                break;
            default: break;
        }
    }

    [[nodiscard]] bool isRunning() const noexcept { return running_; }
    [[nodiscard]] double beatPosition() const noexcept { return static_cast<double>(clockCount_) / 24.0; }
    [[nodiscard]] double tempoBpm() const noexcept {
        if (intervalCount_ == 0) return 0.0;
        double total = 0.0;
        for (std::size_t i = 0; i < intervalCount_; ++i) total += static_cast<double>(intervals_[i]);
        const double averageClockMicros = total / static_cast<double>(intervalCount_);
        return 60'000'000.0 / (averageClockMicros * 24.0);
    }

private:
    bool running_ = false;
    std::uint64_t clockCount_ = 0;
    std::array<std::uint64_t, 24> intervals_{};
    std::size_t intervalCount_ = 0;
    std::size_t intervalIndex_ = 0;
    std::uint64_t lastTimestamp_ = 0;
    bool hasLastTimestamp_ = false;
};

} // namespace nxf::midi
