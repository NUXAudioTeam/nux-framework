// Windows WinMM MIDI backend for nxf_midi.
//
// Verification status (honest): compiled/linked by Windows CI.
// Real MIDI hardware has NOT been tested - no device was opened.
// WinMM types stay inside this file. Short messages only: sysex
// output returns Unavailable rather than pretending (long-message
// support is future work, recorded in the roadmap notes).
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>

#include <nxf/midi/MidiBackendFactory.h>
#include <nxf/midi/MidiDevice.h>

#include <string>

namespace nxf::midi {
namespace {

using nxf::core::Error;
template <typename T>
using R = nxf::core::Result<T>;

std::string toUtf8(const wchar_t* text)
{
    if (text == nullptr) return {};
    const int length = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (length <= 1) return {};
    std::string out(static_cast<std::size_t>(length - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), length, nullptr, nullptr);
    return out;
}

bool parseIndex(const std::string& id, const char* prefix, UINT& index)
{
    const std::string head{prefix};
    if (id.rfind(head, 0) != 0) return false;
    try {
        index = static_cast<UINT>(std::stoul(id.substr(head.size())));
        return true;
    } catch (...) {
        return false;
    }
}

DWORD packShortMessage(const MidiMessage& message)
{
    DWORD packed = message.byte(0);
    if (message.size() > 1) packed |= static_cast<DWORD>(message.byte(1)) << 8;
    if (message.size() > 2) packed |= static_cast<DWORD>(message.byte(2)) << 16;
    return packed;
}

class WinMmInput final : public IMidiInput {
public:
    WinMmInput(HMIDIIN handle, IMidiInputCallback& callback) : handle_(handle), callback_(callback) {}
    ~WinMmInput() override { close(); }
    void adopt(HMIDIIN handle) { handle_ = handle; }   // used by the backend right after midiInOpen

    static void CALLBACK onMidiIn(HMIDIIN, UINT message, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2)
    {
        if (message != MIM_DATA) return;
        auto* self = reinterpret_cast<WinMmInput*>(instance);
        const auto packed = static_cast<std::uint32_t>(param1);
        const std::uint8_t status = static_cast<std::uint8_t>(packed & 0xFF);
        const std::size_t expected = (status & 0xF0) == 0xC0 || (status & 0xF0) == 0xD0 ? 2 : (status >= 0xF8 ? 1 : 3);
        std::array<std::uint8_t, 3> bytes{status,
            static_cast<std::uint8_t>((packed >> 8) & 0xFF), static_cast<std::uint8_t>((packed >> 16) & 0xFF)};
        if (const auto parsed = MidiMessage::fromBytes(std::span<const std::uint8_t>{bytes.data(), expected}); parsed.hasValue())
            self->callback_.onMidiMessage(parsed.value(), static_cast<std::uint64_t>(param2));
    }

    Error start() override {
        return midiInStart(handle_) == MMSYSERR_NOERROR ? Error::success() : Error::unavailable("midiInStart failed");
    }
    Error stop() override {
        return midiInStop(handle_) == MMSYSERR_NOERROR ? Error::success() : Error::unavailable("midiInStop failed");
    }
    void close() override {
        if (handle_ != nullptr) {
            midiInStop(handle_);
            midiInClose(handle_);
            handle_ = nullptr;
        }
    }

private:
    HMIDIIN handle_ = nullptr;
    IMidiInputCallback& callback_;
};

class WinMmOutput final : public IMidiOutput {
public:
    explicit WinMmOutput(HMIDIOUT handle) : handle_(handle) {}
    ~WinMmOutput() override { close(); }
    Error send(const MidiMessage& message) override {
        if (message.type() == MidiMessageType::SystemExclusive)
            return Error::unavailable("sysex output is not supported by this backend yet");
        return midiOutShortMsg(handle_, packShortMessage(message)) == MMSYSERR_NOERROR
            ? Error::success() : Error::unavailable("midiOutShortMsg failed");
    }
    void close() override {
        if (handle_ != nullptr) {
            midiOutClose(handle_);
            handle_ = nullptr;
        }
    }
private:
    HMIDIOUT handle_ = nullptr;
};

class WinMmBackend final : public IMidiBackend {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "winmm"; }

    R<std::vector<MidiDeviceInfo>> enumerateDevices() override
    {
        std::vector<MidiDeviceInfo> devices;
        const UINT inputs = midiInGetNumDevs();
        for (UINT i = 0; i < inputs; ++i) {
            MIDIINCAPSW caps{};
            if (midiInGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
                devices.push_back({"in:" + std::to_string(i), toUtf8(caps.szPname), true, false});
        }
        const UINT outputs = midiOutGetNumDevs();
        for (UINT i = 0; i < outputs; ++i) {
            MIDIOUTCAPSW caps{};
            if (midiOutGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
                devices.push_back({"out:" + std::to_string(i), toUtf8(caps.szPname), false, true});
        }
        return R<std::vector<MidiDeviceInfo>>::success(std::move(devices));
    }

    R<std::unique_ptr<IMidiInput>> openInput(const std::string& deviceId, IMidiInputCallback& callback) override
    {
        UINT index = 0;
        if (!parseIndex(deviceId, "in:", index))
            return R<std::unique_ptr<IMidiInput>>::failure(Error::invalidArgument("unknown WinMM input id"));
        // The callback receives the WinMmInput as its instance, but the
        // handle must exist first: open with a trampoline via a holder.
        auto input = std::make_unique<WinMmInput>(nullptr, callback);
        HMIDIIN handle = nullptr;
        const MMRESULT result = midiInOpen(&handle, index,
            reinterpret_cast<DWORD_PTR>(&WinMmInput::onMidiIn),
            reinterpret_cast<DWORD_PTR>(input.get()), CALLBACK_FUNCTION);
        if (result != MMSYSERR_NOERROR)
            return R<std::unique_ptr<IMidiInput>>::failure(Error::unavailable("midiInOpen failed"));
        input->adopt(handle);
        return R<std::unique_ptr<IMidiInput>>::success(std::move(input));
    }

    R<std::unique_ptr<IMidiOutput>> openOutput(const std::string& deviceId) override
    {
        UINT index = 0;
        if (!parseIndex(deviceId, "out:", index))
            return R<std::unique_ptr<IMidiOutput>>::failure(Error::invalidArgument("unknown WinMM output id"));
        HMIDIOUT handle = nullptr;
        if (midiOutOpen(&handle, index, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
            return R<std::unique_ptr<IMidiOutput>>::failure(Error::unavailable("midiOutOpen failed"));
        return R<std::unique_ptr<IMidiOutput>>::success(std::unique_ptr<IMidiOutput>{std::make_unique<WinMmOutput>(handle)});
    }
};

} // namespace

} // namespace nxf::midi

// The trampoline above needs the object to own its handle after open.
namespace nxf::midi {
std::unique_ptr<IMidiBackend> createSystemMidiBackend()
{
    return std::make_unique<WinMmBackend>();
}
} // namespace nxf::midi
