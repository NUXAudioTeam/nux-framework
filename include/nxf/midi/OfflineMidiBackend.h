#pragma once

#include <nxf/midi/MidiDevice.h>

#include <vector>

namespace nxf::midi {

/// Simulated MIDI hardware for tests and offline tools: one input
/// whose messages are injected with simulateIncoming(), one output
/// that records everything sent to it. Named offline so simulated
/// behaviour can never be reported as a real device.
class OfflineMidiBackend final : public IMidiBackend {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "offline-midi"; }
    [[nodiscard]] nxf::core::Result<std::vector<MidiDeviceInfo>> enumerateDevices() override;
    [[nodiscard]] nxf::core::Result<std::unique_ptr<IMidiInput>> openInput(const std::string& deviceId, IMidiInputCallback& callback) override;
    [[nodiscard]] nxf::core::Result<std::unique_ptr<IMidiOutput>> openOutput(const std::string& deviceId) override;

    /// Delivers a message to the currently open, started input.
    void simulateIncoming(const MidiMessage& message);
    [[nodiscard]] const std::vector<MidiMessage>& sentMessages() const noexcept { return sent_; }

private:
    friend class OfflineMidiInput;
    friend class OfflineMidiOutput;
    IMidiInputCallback* activeCallback_ = nullptr;
    bool inputActive_ = false;
    std::vector<MidiMessage> sent_;
};

} // namespace nxf::midi
