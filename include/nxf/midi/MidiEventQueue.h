#pragma once

#include <nxf/core/SpscQueue.h>
#include <nxf/midi/MidiMessage.h>

#include <cstdint>

namespace nxf::midi {

/// A message stamped with a frame position in the stream's timeline.
struct MidiEvent {
    MidiMessage message;
    std::uint64_t timestampFrames = 0;
};

/// Single-producer/single-consumer queue of MIDI events (built on
/// the core SPSC queue): one thread receives, one thread consumes.
/// Fixed capacity of 1024 events, allocated up front; a full queue
/// refuses the push instead of overwriting.
class MidiEventQueue {
public:
    [[nodiscard]] bool tryPush(MidiEvent event) { return queue_.tryPush(std::move(event)); }
    [[nodiscard]] bool tryPop(MidiEvent& out) { return queue_.tryPop(out); }
    [[nodiscard]] bool empty() const { return queue_.empty(); }
    [[nodiscard]] static constexpr std::size_t capacity() noexcept { return decltype(queue_)::capacity(); }

private:
    nxf::core::SpscQueue<MidiEvent, 1024> queue_;
};

} // namespace nxf::midi
