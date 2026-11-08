#pragma once

#include <nxf/core/Error.h>
#include <nxf/core/Result.h>
#include <nxf/midi/MidiMessage.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace nxf::midi {

struct MidiDeviceInfo {
    std::string id;
    std::string name;
    bool isInput = false;
    bool isOutput = false;
};

/// Receives messages from an opened input. Called on the backend's
/// receiving thread - hand messages off through a MidiEventQueue if
/// the consumer is the audio thread.
class IMidiInputCallback {
public:
    virtual ~IMidiInputCallback() = default;
    virtual void onMidiMessage(const MidiMessage& message, std::uint64_t timestampFrames) = 0;
};

class IMidiInput {
public:
    virtual ~IMidiInput() = default;
    [[nodiscard]] virtual nxf::core::Error start() = 0;
    [[nodiscard]] virtual nxf::core::Error stop() = 0;
    virtual void close() = 0;
};

class IMidiOutput {
public:
    virtual ~IMidiOutput() = default;
    [[nodiscard]] virtual nxf::core::Error send(const MidiMessage& message) = 0;
    virtual void close() = 0;
};

/// A source of MIDI devices. Enumeration must be honest: no devices
/// means an empty list, not an invented one.
class IMidiBackend {
public:
    virtual ~IMidiBackend() = default;
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual nxf::core::Result<std::vector<MidiDeviceInfo>> enumerateDevices() = 0;
    [[nodiscard]] virtual nxf::core::Result<std::unique_ptr<IMidiInput>> openInput(const std::string& deviceId, IMidiInputCallback& callback) = 0;
    [[nodiscard]] virtual nxf::core::Result<std::unique_ptr<IMidiOutput>> openOutput(const std::string& deviceId) = 0;
};

} // namespace nxf::midi
