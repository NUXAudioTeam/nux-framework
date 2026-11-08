#pragma once

#include <nxf/midi/MidiMessage.h>

#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace nxf::midi {

/// Turns a raw byte stream into messages. Handles the parts of the
/// wire format devices actually rely on: running status, messages
/// split across arbitrary feed() calls, realtime bytes (clock etc.)
/// interleaved anywhere - including inside another message - and
/// sysex spanning multiple feeds. A realtime byte never disturbs
/// the message being assembled.
class MidiStreamParser {
public:
    using Callback = std::function<void(const MidiMessage&)>;
    explicit MidiStreamParser(Callback callback) : callback_(std::move(callback)) {}
    void feed(std::span<const std::uint8_t> bytes);
    void feed(std::initializer_list<std::uint8_t> bytes) { feed(std::span<const std::uint8_t>{bytes.begin(), bytes.size()}); }
    void reset();

private:
    void emitCurrent();
    Callback callback_;
    std::uint8_t runningStatus_ = 0;
    std::array<std::uint8_t, 3> pending_{};
    std::size_t pendingCount_ = 0;
    std::size_t pendingExpected_ = 0;
    bool inSysex_ = false;
    std::vector<std::uint8_t> sysex_;
};

} // namespace nxf::midi
