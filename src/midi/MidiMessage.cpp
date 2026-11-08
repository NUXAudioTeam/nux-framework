#include <nxf/midi/MidiMessage.h>

namespace nxf::midi {
namespace {
bool channelOk(int c) { return c >= 1 && c <= 16; }
bool dataOk(int v) { return v >= 0 && v <= 127; }
std::uint8_t statusFor(MidiMessageType type, int channel) {
    std::uint8_t high = 0;
    switch (type) {
        case MidiMessageType::NoteOff: high = 0x80; break;
        case MidiMessageType::NoteOn: high = 0x90; break;
        case MidiMessageType::PolyKeyPressure: high = 0xA0; break;
        case MidiMessageType::ControlChange: high = 0xB0; break;
        case MidiMessageType::ProgramChange: high = 0xC0; break;
        case MidiMessageType::ChannelPressure: high = 0xD0; break;
        case MidiMessageType::PitchBend: high = 0xE0; break;
        default: break;
    }
    return static_cast<std::uint8_t>(high | static_cast<std::uint8_t>(channel - 1));
}
MidiMessageType typeOf(std::uint8_t status) {
    if (status == 0xF0) return MidiMessageType::SystemExclusive;
    if (status == 0xF8) return MidiMessageType::TimingClock;
    if (status == 0xFA) return MidiMessageType::Start;
    if (status == 0xFB) return MidiMessageType::Continue;
    if (status == 0xFC) return MidiMessageType::Stop;
    if (status == 0xFE) return MidiMessageType::ActiveSensing;
    if (status == 0xFF) return MidiMessageType::SystemReset;
    switch (status & 0xF0) {
        case 0x80: return MidiMessageType::NoteOff;
        case 0x90: return MidiMessageType::NoteOn;
        case 0xA0: return MidiMessageType::PolyKeyPressure;
        case 0xB0: return MidiMessageType::ControlChange;
        case 0xC0: return MidiMessageType::ProgramChange;
        case 0xD0: return MidiMessageType::ChannelPressure;
        case 0xE0: return MidiMessageType::PitchBend;
        default: return MidiMessageType::Unknown;
    }
}
std::size_t expectedSize(std::uint8_t status) {
    if (status < 0xF0) {
        const auto high = status & 0xF0;
        return (high == 0xC0 || high == 0xD0) ? 2 : 3;
    }
    return 1;   // realtime/system single-byte messages handled here
}
} // namespace

MidiMessage::MidiMessage(MidiMessageType type, std::uint8_t b0, std::uint8_t b1, std::uint8_t b2, std::size_t size)
    : type_(type), bytes_{b0, b1, b2}, size_(size) {}

MidiMessage::MidiMessage(std::vector<std::uint8_t> sysex)
    : type_(MidiMessageType::SystemExclusive), sysex_(std::move(sysex)) {}

MidiMessage MidiMessage::noteOn(int channel, int note, int velocity) {
    if (!channelOk(channel) || !dataOk(note) || !dataOk(velocity)) return {};
    return {MidiMessageType::NoteOn, statusFor(MidiMessageType::NoteOn, channel),
            static_cast<std::uint8_t>(note), static_cast<std::uint8_t>(velocity), 3};
}
MidiMessage MidiMessage::noteOff(int channel, int note, int velocity) {
    if (!channelOk(channel) || !dataOk(note) || !dataOk(velocity)) return {};
    return {MidiMessageType::NoteOff, statusFor(MidiMessageType::NoteOff, channel),
            static_cast<std::uint8_t>(note), static_cast<std::uint8_t>(velocity), 3};
}
MidiMessage MidiMessage::controlChange(int channel, int controller, int value) {
    if (!channelOk(channel) || !dataOk(controller) || !dataOk(value)) return {};
    return {MidiMessageType::ControlChange, statusFor(MidiMessageType::ControlChange, channel),
            static_cast<std::uint8_t>(controller), static_cast<std::uint8_t>(value), 3};
}
MidiMessage MidiMessage::programChange(int channel, int program) {
    if (!channelOk(channel) || !dataOk(program)) return {};
    return {MidiMessageType::ProgramChange, statusFor(MidiMessageType::ProgramChange, channel),
            static_cast<std::uint8_t>(program), 0, 2};
}
MidiMessage MidiMessage::pitchBend(int channel, int value14) {
    if (!channelOk(channel) || value14 < 0 || value14 > 16383) return {};
    return {MidiMessageType::PitchBend, statusFor(MidiMessageType::PitchBend, channel),
            static_cast<std::uint8_t>(value14 & 0x7F), static_cast<std::uint8_t>((value14 >> 7) & 0x7F), 3};
}
MidiMessage MidiMessage::timingClock() { return {MidiMessageType::TimingClock, 0xF8, 0, 0, 1}; }
MidiMessage MidiMessage::start() { return {MidiMessageType::Start, 0xFA, 0, 0, 1}; }
MidiMessage MidiMessage::cont() { return {MidiMessageType::Continue, 0xFB, 0, 0, 1}; }
MidiMessage MidiMessage::stop() { return {MidiMessageType::Stop, 0xFC, 0, 0, 1}; }
MidiMessage MidiMessage::systemExclusive(std::vector<std::uint8_t> bytes) {
    if (bytes.size() < 2 || bytes.front() != 0xF0 || bytes.back() != 0xF7) return {};
    return MidiMessage{std::move(bytes)};
}

nxf::core::Result<MidiMessage> MidiMessage::fromBytes(std::span<const std::uint8_t> bytes) {
    using nxf::core::Error;
    using R = nxf::core::Result<MidiMessage>;
    if (bytes.empty()) return R::failure(Error::invalidArgument("empty MIDI data"));
    const std::uint8_t status = bytes[0];
    if (status < 0x80) return R::failure(Error::invalidArgument("first byte is not a status byte"));
    if (status == 0xF0) {
        if (bytes.size() < 2 || bytes.back() != 0xF7) return R::failure(Error::invalidArgument("unterminated sysex"));
        return R::success(systemExclusive({bytes.begin(), bytes.end()}));
    }
    if (bytes.size() < expectedSize(status)) return R::failure(Error::invalidArgument("truncated MIDI message"));
    if (expectedSize(status) == 3 && bytes[2] >= 0x80) return R::failure(Error::invalidArgument("data byte out of range"));
    if (expectedSize(status) >= 2 && status < 0xF0 && bytes[1] >= 0x80) return R::failure(Error::invalidArgument("data byte out of range"));
    MidiMessage message{typeOf(status), status, expectedSize(status) > 1 ? bytes[1] : std::uint8_t{0},
                        expectedSize(status) > 2 ? bytes[2] : std::uint8_t{0}, expectedSize(status)};
    if (message.type_ == MidiMessageType::Unknown) return R::failure(Error::invalidArgument("unsupported MIDI status"));
    return R::success(message);
}

int MidiMessage::channel() const noexcept {
    if (type_ == MidiMessageType::SystemExclusive || size_ == 0) return 0;
    const auto status = bytes_[0];
    return status < 0xF0 ? static_cast<int>(status & 0x0F) + 1 : 0;
}
int MidiMessage::noteNumber() const noexcept {
    return (type_ == MidiMessageType::NoteOn || type_ == MidiMessageType::NoteOff ||
            type_ == MidiMessageType::PolyKeyPressure) ? static_cast<int>(bytes_[1]) : -1;
}
int MidiMessage::velocity() const noexcept {
    return (type_ == MidiMessageType::NoteOn || type_ == MidiMessageType::NoteOff) ? static_cast<int>(bytes_[2]) : -1;
}
int MidiMessage::controllerNumber() const noexcept {
    return type_ == MidiMessageType::ControlChange ? static_cast<int>(bytes_[1]) : -1;
}
int MidiMessage::controllerValue() const noexcept {
    return type_ == MidiMessageType::ControlChange ? static_cast<int>(bytes_[2]) : -1;
}
int MidiMessage::programNumber() const noexcept {
    return type_ == MidiMessageType::ProgramChange ? static_cast<int>(bytes_[1]) : -1;
}
int MidiMessage::pitchBend() const noexcept {
    return type_ == MidiMessageType::PitchBend
        ? static_cast<int>(bytes_[1]) | (static_cast<int>(bytes_[2]) << 7) : -1;
}

} // namespace nxf::midi
