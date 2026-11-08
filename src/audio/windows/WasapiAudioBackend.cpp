// Windows WASAPI backend for nxf_audio (shared mode).
//
// All Windows/COM types stay inside this translation unit: nothing
// Windows-specific appears in an NXF public header. All COM work and
// the device calls happen on the stream's own worker thread; control
// calls from other threads only set flags and wait on events.
//
// Verification status (honest): compiled and linked by Windows CI.
// Opening a real device, duplex behaviour and latency are NOT
// verified without hardware - see docs/ROADMAP.md Phase 3 notes.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>
#include <wrl/client.h>

#include <nxf/audio/AudioBackendFactory.h>
#include <nxf/audio/AudioDevice.h>
#include <nxf/platform/Event.h>

#include <atomic>
#include <cstring>
#include <thread>
#include <vector>

namespace nxf::audio {
namespace {

using Microsoft::WRL::ComPtr;
using nxf::core::Error;

std::string toUtf8(const wchar_t* text)
{
    if (text == nullptr) return {};
    const int length = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (length <= 1) return {};
    std::string out(static_cast<std::size_t>(length - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), length, nullptr, nullptr);
    return out;
}

std::wstring toWide(const std::string& text)
{
    if (text.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (length <= 1) return {};
    std::wstring out(static_cast<std::size_t>(length - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, out.data(), length);
    return out;
}

struct ComScope {
    HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool initialised = SUCCEEDED(result);
    ~ComScope() { if (initialised) CoUninitialize(); }
    [[nodiscard]] bool ok() const { return SUCCEEDED(result) || result == RPC_E_CHANGED_MODE; }
};

Error hresultError(std::string_view what, HRESULT hr)
{
    return Error::unavailable(std::string{what} + " failed (HRESULT " + std::to_string(static_cast<long>(hr)) + ")");
}

enum class FormatKind { Float32, Int16, Int32, Unsupported };

FormatKind formatKindOf(const WAVEFORMATEX* format)
{
    WORD tag = format->wFormatTag;
    WORD bits = format->wBitsPerSample;
    if (tag == WAVE_FORMAT_EXTENSIBLE) {
        const auto* ext = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(format);
        // Sub-format GUIDs: KSDATAFORMAT_SUBTYPE_PCM has Data1 == 1,
        // KSDATAFORMAT_SUBTYPE_IEEE_FLOAT has Data1 == 3.
        if (ext->SubFormat.Data1 == 3) tag = WAVE_FORMAT_IEEE_FLOAT;
        else if (ext->SubFormat.Data1 == 1) tag = WAVE_FORMAT_PCM;
        bits = ext->Samples.wValidBitsPerSample != 0 ? ext->Samples.wValidBitsPerSample : format->wBitsPerSample;
    }
    if (tag == WAVE_FORMAT_IEEE_FLOAT && bits == 32) return FormatKind::Float32;
    if (tag == WAVE_FORMAT_PCM && bits == 16) return FormatKind::Int16;
    if (tag == WAVE_FORMAT_PCM && bits == 32) return FormatKind::Int32;
    return FormatKind::Unsupported;
}

float readSample(const BYTE* data, FormatKind kind)
{
    switch (kind) {
        case FormatKind::Float32: { float v; std::memcpy(&v, data, sizeof(v)); return v; }
        case FormatKind::Int16: { std::int16_t v; std::memcpy(&v, data, sizeof(v)); return static_cast<float>(v) / 32768.0f; }
        case FormatKind::Int32: { std::int32_t v; std::memcpy(&v, data, sizeof(v)); return static_cast<float>(v) / 2147483648.0f; }
        default: return 0.0f;
    }
}

void writeSample(BYTE* data, FormatKind kind, float value)
{
    switch (kind) {
        case FormatKind::Float32: std::memcpy(data, &value, sizeof(value)); break;
        case FormatKind::Int16: {
            const auto clipped = value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value);
            const auto v = static_cast<std::int16_t>(clipped * 32767.0f);
            std::memcpy(data, &v, sizeof(v)); break;
        }
        case FormatKind::Int32: {
            const auto clipped = value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value);
            const auto v = static_cast<std::int32_t>(clipped * 2147483647.0);
            std::memcpy(data, &v, sizeof(v)); break;
        }
        default: break;
    }
}

HRESULT openDevice(IMMDeviceEnumerator* enumerator, const std::string& id, EDataFlow flow, IMMDevice** out)
{
    if (id.empty()) return enumerator->GetDefaultAudioEndpoint(flow, eMultimedia, out);
    return enumerator->GetDevice(toWide(id).c_str(), out);
}

// ---------------------------------------------------------------------------

class WasapiStream final : public IAudioStream {
public:
    ~WasapiStream() override { close(); }

    Error open(const AudioStreamConfig& config, IAudioCallback& callback) override
    {
        if (state_ == StreamState::Running) return Error::unavailable("stop the stream before reopening");
        if (const auto valid = validateAudioStreamConfig(config); !valid) return valid;
        config_ = config;
        callback_ = &callback;
        streamTime_.store(0);
        state_ = StreamState::Open;
        return Error::success();
    }

    Error start() override
    {
        if (state_ != StreamState::Open || callback_ == nullptr) return Error::unavailable("stream is not open");
        keepRunning_.store(true);
        initFinished_.reset();
        initError_ = Error::success();
        worker_ = std::thread{[this] { run(); }};
        initFinished_.wait();
        if (!initError_.isSuccess()) {
            if (worker_.joinable()) worker_.join();
            state_ = StreamState::Open;
            return initError_;
        }
        state_ = StreamState::Running;
        return Error::success();
    }

    Error stop() override
    {
        keepRunning_.store(false);
        if (worker_.joinable()) worker_.join();
        if (state_ == StreamState::Running) state_ = StreamState::Open;
        return Error::success();
    }

    void close() override
    {
        (void)stop();
        callback_ = nullptr;
        state_ = StreamState::Closed;
    }

    [[nodiscard]] StreamState state() const noexcept override { return state_; }
    [[nodiscard]] double sampleRate() const noexcept override { return config_.sampleRate; }
    [[nodiscard]] std::uint64_t streamTimeFrames() const noexcept override { return streamTime_.load(); }

private:
    struct ClientSide {
        ComPtr<IAudioClient> client;
        ComPtr<IAudioRenderClient> render;
        ComPtr<IAudioCaptureClient> capture;
        WAVEFORMATEX* mixFormat = nullptr;    // freed with CoTaskMemFree in teardown
        FormatKind kind = FormatKind::Unsupported;
        UINT32 bufferFrames = 0;
        ~ClientSide() { if (mixFormat != nullptr) CoTaskMemFree(mixFormat); }
    };

    Error setupSide(ClientSide& side, EDataFlow flow, const std::string& deviceId, int channels)
    {
        ComPtr<IMMDeviceEnumerator> enumerator;
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(enumerator.GetAddressOf()));
        if (FAILED(hr)) return hresultError("create device enumerator", hr);
        ComPtr<IMMDevice> device;
        hr = openDevice(enumerator.Get(), deviceId, flow, device.GetAddressOf());
        if (FAILED(hr)) return hresultError(flow == eRender ? "open output device" : "open input device", hr);
        hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                              reinterpret_cast<void**>(side.client.GetAddressOf()));
        if (FAILED(hr)) return hresultError("activate audio client", hr);
        hr = side.client->GetMixFormat(&side.mixFormat);
        if (FAILED(hr)) return hresultError("get mix format", hr);
        side.kind = formatKindOf(side.mixFormat);
        if (side.kind == FormatKind::Unsupported) return Error::unavailable("unsupported device sample format");
        if (side.mixFormat->nChannels < channels) return Error::unavailable("device has fewer channels than requested");
        if (static_cast<double>(side.mixFormat->nSamplesPerSec) != config_.sampleRate)
            return Error::unavailable("device does not run at the requested sample rate in shared mode");
        hr = side.client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 200000, 0, side.mixFormat, nullptr);
        if (FAILED(hr)) return hresultError("initialise audio client", hr);
        hr = side.client->GetBufferSize(&side.bufferFrames);
        if (FAILED(hr)) return hresultError("get buffer size", hr);
        if (flow == eRender) {
            hr = side.client->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void**>(side.render.GetAddressOf()));
        } else {
            hr = side.client->GetService(__uuidof(IAudioCaptureClient), reinterpret_cast<void**>(side.capture.GetAddressOf()));
        }
        if (FAILED(hr)) return hresultError("get audio service", hr);
        return Error::success();
    }

    void run()
    {
        ComScope com;
        ClientSide output, input;
        AudioBuffer inputBlock, outputBlock;
        std::vector<float> inputRing;
        std::size_t ringWrite = 0, ringRead = 0, ringUsed = 0;
        UINT32 ringCapacityFrames = 0;

        auto fail = [&](Error error) {
            initError_ = error;
            initFinished_.set();
        };

        if (!com.ok()) { fail(hresultError("initialise COM", com.result)); return; }
        if (config_.outputChannels > 0) {
            if (const auto e = setupSide(output, eRender, config_.outputDeviceId, config_.outputChannels); !e) { fail(e); return; }
        }
        if (config_.inputChannels > 0) {
            if (const auto e = setupSide(input, eCapture, config_.inputDeviceId, config_.inputChannels); !e) { fail(e); return; }
        }

        const UINT32 maxFrames = output.bufferFrames != 0 ? output.bufferFrames : input.bufferFrames;
        inputBlock = AudioBuffer{config_.inputChannels, static_cast<int>(maxFrames)};
        outputBlock = AudioBuffer{config_.outputChannels, static_cast<int>(maxFrames)};
        if (config_.inputChannels > 0 && config_.outputChannels > 0) {
            ringCapacityFrames = maxFrames * 4;
            inputRing.assign(static_cast<std::size_t>(ringCapacityFrames) * static_cast<std::size_t>(config_.inputChannels), 0.0f);
        }

        if (output.client != nullptr && FAILED(output.client->Start())) { fail(Error::unavailable("output client start failed")); return; }
        if (input.client != nullptr && FAILED(input.client->Start())) { fail(Error::unavailable("input client start failed")); return; }
        initFinished_.set();

        const int bytesPerFrameOut = output.mixFormat != nullptr ? output.mixFormat->nBlockAlign : 0;
        const int bytesPerFrameIn = input.mixFormat != nullptr ? input.mixFormat->nBlockAlign : 0;

        while (keepRunning_.load()) {
            bool worked = false;

            // Drain capture into the ring (duplex) or straight to the callback (input-only).
            if (input.capture != nullptr) {
                UINT32 packetFrames = 0;
                while (SUCCEEDED(input.capture->GetNextPacketSize(&packetFrames)) && packetFrames > 0) {
                    BYTE* data = nullptr; DWORD flags = 0;
                    if (FAILED(input.capture->GetBuffer(&data, &packetFrames, &flags, nullptr, nullptr))) break;
                    const bool silent = (flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0;
                    if (config_.outputChannels > 0) {
                        for (UINT32 frame = 0; frame < packetFrames; ++frame) {
                            if (ringUsed >= ringCapacityFrames) break;  // overflow: drop newest, never block
                            for (int ch = 0; ch < config_.inputChannels; ++ch) {
                                const float v = silent ? 0.0f : readSample(data + (frame * static_cast<UINT32>(input.mixFormat->nChannels) + static_cast<UINT32>(ch)) * (bytesPerFrameIn / input.mixFormat->nChannels), input.kind);
                                inputRing[(ringWrite * static_cast<std::size_t>(config_.inputChannels)) + static_cast<std::size_t>(ch)] = v;
                            }
                            ringWrite = (ringWrite + 1) % ringCapacityFrames;
                            ++ringUsed;
                        }
                    } else {
                        for (UINT32 frame = 0; frame < packetFrames; ++frame)
                            for (int ch = 0; ch < config_.inputChannels; ++ch)
                                inputBlock.setSample(ch, static_cast<int>(frame),
                                    silent ? 0.0f : readSample(data + (frame * static_cast<UINT32>(input.mixFormat->nChannels) + static_cast<UINT32>(ch)) * (bytesPerFrameIn / input.mixFormat->nChannels), input.kind));
                        ConstAudioBufferView inView{inputBlock.view().samples, config_.inputChannels, static_cast<int>(packetFrames)};
                        AudioBufferView outView{outputBlock.view().samples, 0, static_cast<int>(packetFrames)};
                        callback_->processAudio(inView, outView, config_.sampleRate, streamTime_.load());
                        streamTime_ += packetFrames;
                    }
                    input.capture->ReleaseBuffer(packetFrames);
                    worked = true;
                }
            }

            if (output.render != nullptr) {
                UINT32 padding = 0;
                if (SUCCEEDED(output.client->GetCurrentPadding(&padding)) && output.bufferFrames > padding) {
                    const UINT32 frames = output.bufferFrames - padding;
                    // Pull this block's input out of the ring (silence on underrun).
                    for (UINT32 frame = 0; frame < frames; ++frame) {
                        const bool have = ringUsed > 0;
                        for (int ch = 0; ch < config_.inputChannels; ++ch)
                            inputBlock.setSample(ch, static_cast<int>(frame),
                                have ? inputRing[(ringRead * static_cast<std::size_t>(config_.inputChannels)) + static_cast<std::size_t>(ch)] : 0.0f);
                        if (have) { ringRead = (ringRead + 1) % ringCapacityFrames; --ringUsed; }
                    }
                    ConstAudioBufferView inView{inputBlock.view().samples, config_.inputChannels, static_cast<int>(frames)};
                    AudioBufferView outView{outputBlock.view().samples, config_.outputChannels, static_cast<int>(frames)};
                    callback_->processAudio(inView, outView, config_.sampleRate, streamTime_.load());

                    BYTE* data = nullptr;
                    if (SUCCEEDED(output.render->GetBuffer(frames, &data))) {
                        for (UINT32 frame = 0; frame < frames; ++frame)
                            for (UINT32 ch = 0; ch < output.mixFormat->nChannels; ++ch) {
                                const float v = static_cast<int>(ch) < config_.outputChannels
                                    ? outputBlock.sample(static_cast<int>(ch), static_cast<int>(frame)) : 0.0f;
                                writeSample(data + frame * static_cast<UINT32>(bytesPerFrameOut) + ch * static_cast<UINT32>(bytesPerFrameOut / output.mixFormat->nChannels), output.kind, v);
                            }
                        output.render->ReleaseBuffer(frames, 0);
                        streamTime_ += frames;
                        worked = true;
                    }
                }
            }

            if (!worked) std::this_thread::sleep_for(std::chrono::milliseconds{3});
        }

        if (output.client != nullptr) output.client->Stop();
        if (input.client != nullptr) input.client->Stop();
    }

    AudioStreamConfig config_;
    IAudioCallback* callback_ = nullptr;
    StreamState state_ = StreamState::Closed;
    std::thread worker_;
    std::atomic<bool> keepRunning_{false};
    std::atomic<std::uint64_t> streamTime_{0};
    nxf::platform::Event initFinished_;
    Error initError_ = Error::success();
};

// ---------------------------------------------------------------------------

class WasapiBackend final : public IAudioBackend {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "wasapi"; }

    nxf::core::Result<std::vector<AudioDeviceInfo>> enumerateDevices() override
    {
        ComScope com;
        if (!com.ok()) return nxf::core::Result<std::vector<AudioDeviceInfo>>::failure(hresultError("initialise COM", com.result));
        ComPtr<IMMDeviceEnumerator> enumerator;
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(enumerator.GetAddressOf()));
        if (FAILED(hr)) return nxf::core::Result<std::vector<AudioDeviceInfo>>::failure(hresultError("create device enumerator", hr));

        auto defaultId = [&](EDataFlow flow) {
            ComPtr<IMMDevice> device; LPWSTR rawId = nullptr;
            if (FAILED(enumerator->GetDefaultAudioEndpoint(flow, eMultimedia, device.GetAddressOf()))) return std::string{};
            if (FAILED(device->GetId(&rawId))) return std::string{};
            std::string id = toUtf8(rawId);
            CoTaskMemFree(rawId);
            return id;
        };
        const std::string defaultOut = defaultId(eRender);
        const std::string defaultIn = defaultId(eCapture);

        ComPtr<IMMDeviceCollection> collection;
        hr = enumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, collection.GetAddressOf());
        if (FAILED(hr)) return nxf::core::Result<std::vector<AudioDeviceInfo>>::failure(hresultError("enumerate endpoints", hr));

        std::vector<AudioDeviceInfo> devices;
        UINT count = 0;
        collection->GetCount(&count);
        for (UINT i = 0; i < count; ++i) {
            ComPtr<IMMDevice> device;
            if (FAILED(collection->Item(i, device.GetAddressOf()))) continue;
            AudioDeviceInfo info;
            LPWSTR rawId = nullptr;
            if (SUCCEEDED(device->GetId(&rawId))) { info.id = toUtf8(rawId); CoTaskMemFree(rawId); }

            ComPtr<IPropertyStore> properties;
            if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, properties.GetAddressOf()))) {
                PROPVARIANT nameValue;
                PropVariantInit(&nameValue);
                if (SUCCEEDED(properties->GetValue(PKEY_Device_FriendlyName, &nameValue)) && nameValue.vt == VT_LPWSTR)
                    info.name = toUtf8(nameValue.pwszVal);
                PropVariantClear(&nameValue);
            }

            ComPtr<IMMEndpoint> endpoint;
            EDataFlow flow = eRender;
            if (SUCCEEDED(device.As(&endpoint))) endpoint->GetDataFlow(&flow);

            ComPtr<IAudioClient> client;
            WAVEFORMATEX* mix = nullptr;
            if (SUCCEEDED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(client.GetAddressOf())))
                && SUCCEEDED(client->GetMixFormat(&mix)) && mix != nullptr) {
                if (flow == eRender) info.maxOutputChannels = mix->nChannels; else info.maxInputChannels = mix->nChannels;
                info.defaultSampleRate = static_cast<double>(mix->nSamplesPerSec);
                CoTaskMemFree(mix);
            }
            info.isDefaultOutput = flow == eRender && info.id == defaultOut;
            info.isDefaultInput = flow == eCapture && info.id == defaultIn;
            devices.push_back(std::move(info));
        }
        return nxf::core::Result<std::vector<AudioDeviceInfo>>::success(std::move(devices));
    }

    nxf::core::Result<std::unique_ptr<IAudioStream>> createStream() override
    {
        return nxf::core::Result<std::unique_ptr<IAudioStream>>::success(
            std::unique_ptr<IAudioStream>{std::make_unique<WasapiStream>()});
    }
};

} // namespace

std::unique_ptr<IAudioBackend> createSystemBackend()
{
    return std::make_unique<WasapiBackend>();
}

} // namespace nxf::audio
