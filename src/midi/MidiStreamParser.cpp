#include <nxf/midi/MidiStreamParser.h>

namespace nxf::midi {
namespace {
bool isRealtime(std::uint8_t b) { return b >= 0xF8; }
std::size_t dataBytesFor(std::uint8_t status) {
    if (status < 0xF0) {
        const auto high = status & 0xF0;
        return (high == 0xC0 || high == 0xD0) ? 1 : 2;
    }
    switch (status) {
        case 0xF1: case 0xF3: return 1;
        case 0xF2: return 2;
        default: return 0;
    }
}
} // namespace

void MidiStreamParser::reset()
{
    runningStatus_ = 0;
    pendingCount_ = 0;
    pendingExpected_ = 0;
    inSysex_ = false;
    sysex_.clear();
}

void MidiStreamParser::emitCurrent()
{
    if (pendingExpected_ == 0) return;
    std::vector<std::uint8_t> bytes{pending_[0]};
    for (std::size_t i = 1; i < pendingCount_; ++i) bytes.push_back(pending_[i]);
    pendingCount_ = 0;   // runningStatus_ survives: the next data byte re-arms from it
    pendingExpected_ = 0;
    if (const auto message = MidiMessage::fromBytes(bytes); message.hasValue()) callback_(message.value());
}

void MidiStreamParser::feed(std::span<const std::uint8_t> bytes)
{
    for (const std::uint8_t byte : bytes) {
        if (isRealtime(byte)) {
            if (const auto message = MidiMessage::fromBytes({byte}); message.hasValue()) callback_(message.value());
            continue;
        }
        if (inSysex_) {
            sysex_.push_back(byte);
            if (byte == 0xF7) {
                inSysex_ = false;
                runningStatus_ = 0;
                if (const auto message = MidiMessage::fromBytes(sysex_); message.hasValue()) callback_(message.value());
                sysex_.clear();
            }
            continue;
        }
        if (byte >= 0x80) {
            if (byte == 0xF0) {
                inSysex_ = true;
                runningStatus_ = 0;
                pendingCount_ = 0;
                pendingExpected_ = 0;
                sysex_ = {byte};
                continue;
            }
            runningStatus_ = byte < 0xF0 ? byte : 0;
            pending_[0] = byte;
            pendingCount_ = 1;
            pendingExpected_ = 1 + dataBytesFor(byte);
            if (dataBytesFor(byte) == 0) emitCurrent();
            continue;
        }
        // data byte
        if (pendingCount_ == 0) {
            if (runningStatus_ == 0) continue;   // stray data byte: ignore
            pending_[0] = runningStatus_;
            pendingCount_ = 1;
            pendingExpected_ = 1 + dataBytesFor(runningStatus_);
        }
        if (pendingCount_ < 3) pending_[pendingCount_] = byte;
        ++pendingCount_;
        if (pendingCount_ >= pendingExpected_) emitCurrent();
    }
}

} // namespace nxf::midi
