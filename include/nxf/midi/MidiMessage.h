#pragma once

#include <nxf/core/Result.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <vector>

namespace nxf::midi {

enum class MidiMessageType {
    Unknown, NoteOff, NoteOn, PolyKeyPressure, ControlChange, ProgramChange,
    ChannelPressure, PitchBend, SystemExclusive, TimingClock, Start, Continue,
    Stop, ActiveSensing, SystemReset
};

/// One immutable MIDI message. Channel numbers in this API are
/// 1..16 (the wire stores 0..15). Factory functions validate their
/// arguments and return an inert Unknown message on bad input, so a
/// MidiMessage is always safe to inspect. A note-on with velocity 0
/// is a note-off (isNoteOff), as the spec requires.
class MidiMessage {
public:
    MidiMessage() = default;

    [[nodiscard]] static MidiMessage noteOn(int channel, int note, int velocity);
    [[nodiscard]] static MidiMessage noteOff(int channel, int note, int velocity);
    [[nodiscard]] static MidiMessage controlChange(int channel, int controller, int value);
    [[nodiscard]] static MidiMessage programChange(int channel, int program);
    [[nodiscard]] static MidiMessage pitchBend(int channel, int value14);
    [[nodiscard]] static MidiMessage timingClock();
    [[nodiscard]] static MidiMessage start();
    [[nodiscard]] static MidiMessage cont();
    [[nodiscard]] static MidiMessage stop();
    [[nodiscard]] static MidiMessage systemExclusive(std::vector<std::uint8_t> bytes);

    /// Parses exactly one complete message (no running status).
    [[nodiscard]] static nxf::core::Result<MidiMessage> fromBytes(std::span<const std::uint8_t> bytes);
    [[nodiscard]] static nxf::core::Result<MidiMessage> fromBytes(std::initializer_list<std::uint8_t> bytes) {
        return fromBytes(std::span<const std::uint8_t>{bytes.begin(), bytes.size()});
    }

    [[nodiscard]] MidiMessageType type() const noexcept { return type_; }
    [[nodiscard]] std::size_t size() const noexcept { return type_ == MidiMessageType::SystemExclusive ? sysex_.size() : size_; }
    [[nodiscard]] std::uint8_t byte(std::size_t index) const noexcept {
        return type_ == MidiMessageType::SystemExclusive ? sysex_[index] : bytes_[index];
    }
    [[nodiscard]] const std::vector<std::uint8_t>& sysexData() const noexcept { return sysex_; }

    [[nodiscard]] int channel() const noexcept;   // 1..16, or 0 for system messages
    [[nodiscard]] bool isNoteOn() const noexcept { return type_ == MidiMessageType::NoteOn && velocity() > 0; }
    [[nodiscard]] bool isNoteOff() const noexcept {
        return type_ == MidiMessageType::NoteOff || (type_ == MidiMessageType::NoteOn && velocity() == 0);
    }
    [[nodiscard]] int noteNumber() const noexcept;
    [[nodiscard]] int velocity() const noexcept;
    [[nodiscard]] int controllerNumber() const noexcept;
    [[nodiscard]] int controllerValue() const noexcept;
    [[nodiscard]] int programNumber() const noexcept;
    [[nodiscard]] int pitchBend() const noexcept;   // 0..16383, centre 8192

private:
    MidiMessage(MidiMessageType type, std::uint8_t b0, std::uint8_t b1, std::uint8_t b2, std::size_t size);
    explicit MidiMessage(std::vector<std::uint8_t> sysex);
    MidiMessageType type_ = MidiMessageType::Unknown;
    std::array<std::uint8_t, 3> bytes_{};
    std::size_t size_ = 0;
    std::vector<std::uint8_t> sysex_;
};

} // namespace nxf::midi
