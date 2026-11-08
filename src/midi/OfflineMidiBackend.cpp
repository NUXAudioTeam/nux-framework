#include <nxf/midi/OfflineMidiBackend.h>

namespace nxf::midi {
namespace {
using nxf::core::Error;
template <typename T>
using R = nxf::core::Result<T>;
} // namespace

class OfflineMidiInput final : public IMidiInput {
public:
    OfflineMidiInput(OfflineMidiBackend& backend, IMidiInputCallback& callback) : backend_(backend), callback_(callback) {}
    ~OfflineMidiInput() override { close(); }
    Error start() override {
        backend_.activeCallback_ = &callback_;
        backend_.inputActive_ = true;
        return Error::success();
    }
    Error stop() override {
        backend_.inputActive_ = false;
        return Error::success();
    }
    void close() override {
        (void)stop();
        if (backend_.activeCallback_ == &callback_) backend_.activeCallback_ = nullptr;
    }
private:
    OfflineMidiBackend& backend_;
    IMidiInputCallback& callback_;
};

class OfflineMidiOutput final : public IMidiOutput {
public:
    explicit OfflineMidiOutput(OfflineMidiBackend& backend) : backend_(backend) {}
    Error send(const MidiMessage& message) override {
        backend_.sent_.push_back(message);
        return Error::success();
    }
    void close() override {}
private:
    OfflineMidiBackend& backend_;
};

R<std::vector<MidiDeviceInfo>> OfflineMidiBackend::enumerateDevices()
{
    return R<std::vector<MidiDeviceInfo>>::success({
        MidiDeviceInfo{"offline-in", "NXF Offline MIDI Input (simulated)", true, false},
        MidiDeviceInfo{"offline-out", "NXF Offline MIDI Output (simulated)", false, true}});
}

R<std::unique_ptr<IMidiInput>> OfflineMidiBackend::openInput(const std::string& deviceId, IMidiInputCallback& callback)
{
    if (deviceId != "offline-in") return R<std::unique_ptr<IMidiInput>>::failure(Error::unavailable("unknown offline MIDI input"));
    return R<std::unique_ptr<IMidiInput>>::success(std::unique_ptr<IMidiInput>{std::make_unique<OfflineMidiInput>(*this, callback)});
}

R<std::unique_ptr<IMidiOutput>> OfflineMidiBackend::openOutput(const std::string& deviceId)
{
    if (deviceId != "offline-out") return R<std::unique_ptr<IMidiOutput>>::failure(Error::unavailable("unknown offline MIDI output"));
    return R<std::unique_ptr<IMidiOutput>>::success(std::unique_ptr<IMidiOutput>{std::make_unique<OfflineMidiOutput>(*this)});
}

void OfflineMidiBackend::simulateIncoming(const MidiMessage& message)
{
    if (inputActive_ && activeCallback_ != nullptr) activeCallback_->onMidiMessage(message, 0);
}

} // namespace nxf::midi
