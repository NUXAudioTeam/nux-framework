// Real VST3 hosting adapter, compiled only when
// NXF_ENABLE_VST3=ON. All Steinberg SDK types stay inside this
// translation unit; the public NXF headers remain SDK-free.

#include <nxf/plugin/PluginFormatAdapter.h>

#include <algorithm>
#include <atomic>
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"

namespace nxf::plugin {
namespace {

using namespace Steinberg;
namespace vst = Steinberg::Vst;
namespace hosting = VST3::Hosting;
using nxf::core::Error;

[[nodiscard]] std::string utf8FromUtf16(const char16_t* text)
{
    std::string out;
    if (text == nullptr)
        return out;
    for (const char16_t* p = text; *p != 0; ++p) {
        const std::uint32_t cp = static_cast<std::uint32_t>(*p);
        if (cp < 0x80) {
            out.push_back(static_cast<char>(cp));
        } else if (cp < 0x800) {
            out.push_back(static_cast<char>(0xC0u | (cp >> 6)));
            out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
        } else {
            out.push_back(static_cast<char>(0xE0u | (cp >> 12)));
            out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu)));
            out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
        }
    }
    return out;
}

[[nodiscard]] bool hasVst3Suffix(const std::string& path)
{
    if (path.size() < 5)
        return false;
    std::string tail = path.substr(path.size() - 5);
    for (auto& ch : tail)
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return tail == ".vst3";
}

// One host application object for the whole process, exactly as
// the SDK samples do (audiohost uses the same pattern). It is
// intentionally leaked: plug-ins may hold it past instance death.
[[nodiscard]] FUnknown* hostContext()
{
    static FUnknown* context = new vst::HostApplication();
    return context;
}

struct LoadedClass {
    hosting::Module::Ptr module;
    hosting::ClassInfo info;
};

nxf::core::Result<LoadedClass> loadClass(const std::string& path)
{
    std::string ignoredError;
    auto module = hosting::Module::create(path, ignoredError);
    if (!module)
        return nxf::core::Result<LoadedClass>::failure(
            Error::unavailable("VST3 module could not be loaded"));
    const auto factory = module->getFactory();
    for (const auto& info : factory.classInfos()) {
        if (info.category() != kVstAudioEffectClass)
            continue;
        return nxf::core::Result<LoadedClass>::success(LoadedClass{module, info});
    }
    return nxf::core::Result<LoadedClass>::failure(
        Error::unavailable("no VST3 audio-effect class in module"));
}

[[nodiscard]] std::string categoryOf(const hosting::ClassInfo& info)
{
    const auto sub = info.subCategoriesString();
    if (sub.find("Instrument") != std::string::npos)
        return "instrument";
    if (sub.find("Analyzer") != std::string::npos || sub.find("Meter") != std::string::npos)
        return "analyzer";
    return "effect";
}

void probeBusCounts(const LoadedClass& loaded, int& inputs, int& outputs)
{
    inputs = 0;
    outputs = 0;
    auto component = loaded.module->getFactory().createInstance<vst::IComponent>(loaded.info.ID());
    if (!component)
        return;
    if (component->initialize(hostContext()) != kResultOk)
        return;
    const auto countChannels = [&component](vst::BusDirections direction) {
        int total = 0;
        const auto buses = component->getBusCount(vst::kAudio, direction);
        for (int32 b = 0; b < buses; ++b) {
            vst::BusInfo busInfo {};
            if (component->getBusInfo(vst::kAudio, direction, b, busInfo) == kResultOk)
                total = std::max(total, static_cast<int>(busInfo.channelCount));
        }
        return total;
    };
    inputs = countChannels(vst::kInput);
    outputs = countChannels(vst::kOutput);
    (void)component->terminate();
}

class Vst3Instance final : public IPluginInstance {
public:
    Vst3Instance(LoadedClass loaded, PluginDescriptor descriptor)
        : loaded_(std::move(loaded)), descriptor_(std::move(descriptor))
    {
    }

    ~Vst3Instance() override
    {
        suspend();
        if (connected_) {
            auto componentPoint = FUnknownPtr<vst::IConnectionPoint>(component_.get());
            auto controllerPoint = FUnknownPtr<vst::IConnectionPoint>(controller_.get());
            if (componentPoint && controllerPoint) {
                (void)componentPoint->disconnect(controllerPoint);
                (void)controllerPoint->disconnect(componentPoint);
            }
            connected_ = false;
        }
        if (controller_ && !singleComponent_)
            (void)controller_->terminate();
        if (component_)
            (void)component_->terminate();
    }

    [[nodiscard]] Error initialize()
    {
        auto& factory = loaded_.module->getFactory();
        factory.setHostContext(hostContext());
        component_ = factory.createInstance<vst::IComponent>(loaded_.info.ID());
        if (!component_)
            return Error::unavailable("VST3 component creation failed");
        if (component_->initialize(hostContext()) != kResultOk)
            return Error::unavailable("VST3 component initialize failed");
        processor_ = FUnknownPtr<vst::IAudioProcessor>(component_.get());
        if (!processor_)
            return Error::unavailable("VST3 component is not an audio processor");

        // Controller: the component itself for single-component
        // plug-ins, otherwise the separate controller class the
        // component names (same order as the SDK's PlugProvider).
        controller_ = FUnknownPtr<vst::IEditController>(component_.get());
        if (controller_) {
            singleComponent_ = true;
        } else {
            TUID controllerId {};
            if (component_->getControllerClassId(controllerId) == kResultOk)
                controller_ = factory.createInstance<vst::IEditController>(VST3::UID::fromTUID(controllerId));
            if (controller_)
                (void)controller_->initialize(hostContext());
        }
        if (component_ && controller_ && !singleComponent_) {
            auto componentPoint = FUnknownPtr<vst::IConnectionPoint>(component_.get());
            auto controllerPoint = FUnknownPtr<vst::IConnectionPoint>(controller_.get());
            if (componentPoint && controllerPoint
                && componentPoint->connect(controllerPoint) == kResultOk
                && controllerPoint->connect(componentPoint) == kResultOk) {
                connected_ = true;
            }
            MemoryStream stream;
            if (component_->getState(&stream) == kResultOk) {
                stream.seek(0, IBStream::kIBSeekSet, nullptr);
                (void)controller_->setComponentState(&stream);
            }
        }
        readParameters();
        changes_.setMaxParameters(static_cast<int32>(std::max<std::size_t>(parameterIds_.size(), 1)));
        changes_.clearQueue();
        return Error::success();
    }

    [[nodiscard]] const PluginDescriptor& descriptor() const noexcept override { return descriptor_; }
    [[nodiscard]] PluginState state() const noexcept override
    {
        return prepared_ ? PluginState::Prepared : PluginState::Created;
    }

    [[nodiscard]] Error prepare(double sampleRate, int maxFrames, int /*channels*/) override
    {
        if (sampleRate <= 0.0 || maxFrames <= 0)
            return Error::invalidArgument("prepare: sample rate and frames must be positive");
        if (prepared_)
            suspend();

        inputChannels_ = busChannelCount(vst::kInput);
        outputChannels_ = busChannelCount(vst::kOutput);

        std::vector<vst::SpeakerArrangement> inArrangements;
        std::vector<vst::SpeakerArrangement> outArrangements;
        fillArrangements(vst::kInput, inArrangements);
        fillArrangements(vst::kOutput, outArrangements);
        if (!inArrangements.empty() || !outArrangements.empty()) {
            // Re-declaring the plug-in's own current arrangement.
            // Some plug-ins refuse this; that is not fatal because
            // processing below uses the live bus counts.
            (void)processor_->setBusArrangements(inArrangements.data(),
                                                 static_cast<int32>(inArrangements.size()),
                                                 outArrangements.data(),
                                                 static_cast<int32>(outArrangements.size()));
        }
        activateMainBuses();

        vst::ProcessSetup setup {};
        setup.processMode = vst::kRealtime;
        setup.symbolicSampleSize = vst::kSample32;
        setup.maxSamplesPerBlock = maxFrames;
        setup.sampleRate = sampleRate;
        if (processor_->setupProcessing(setup) != kResultOk)
            return Error::unavailable("VST3 setupProcessing failed");
        if (component_->setActive(true) != kResultOk)
            return Error::unavailable("VST3 setActive failed");

        maxFrames_ = maxFrames;
        inputPlanes_.assign(static_cast<std::size_t>(inputChannels_),
                            std::vector<float>(static_cast<std::size_t>(maxFrames_), 0.0f));
        outputPlanes_.assign(static_cast<std::size_t>(outputChannels_),
                             std::vector<float>(static_cast<std::size_t>(maxFrames_), 0.0f));
        inputPointers_.clear();
        outputPointers_.clear();
        for (auto& plane : inputPlanes_)
            inputPointers_.push_back(plane.data());
        for (auto& plane : outputPlanes_)
            outputPointers_.push_back(plane.data());
        inputBus_ = {};
        inputBus_.numChannels = inputChannels_;
        inputBus_.channelBuffers32 = inputPointers_.data();
        inputBus_.silenceFlags = 0;
        outputBus_ = {};
        outputBus_.numChannels = outputChannels_;
        outputBus_.channelBuffers32 = outputPointers_.data();
        outputBus_.silenceFlags = 0;

        // setProcessing is advisory: perfectly valid plug-ins
        // (including ones built straight on the SDK's AudioEffect
        // base class) answer kNotImplemented here, so the result
        // is intentionally ignored - same tolerance the SDK
        // validator shows.
        (void)processor_->setProcessing(true);
        prepared_ = true;
        return Error::success();
    }

    void suspend() override
    {
        if (!prepared_)
            return;
        if (processor_)
            (void)processor_->setProcessing(false);
        if (component_)
            (void)component_->setActive(false);
        prepared_ = false;
    }

    void reset() override
    {
        if (!prepared_ || !processor_)
            return;
        (void)processor_->setProcessing(false);
        events_.clear();
        (void)processor_->setProcessing(true);
    }

    [[nodiscard]] const std::vector<PluginParameter>& parameters() const noexcept override
    {
        return parameters_;
    }

    [[nodiscard]] bool setParameterNormalized(std::size_t index, double value) override
    {
        if (index >= parameterValues_.size())
            return false;
        const double clamped = std::clamp(value, 0.0, 1.0);
        parameterValues_[index].store(clamped);
        parameterDirty_[index].store(true);
        if (controller_)
            (void)controller_->setParamNormalized(parameterIds_[index], clamped);
        return true;
    }

    [[nodiscard]] double parameterNormalized(std::size_t index) const noexcept override
    {
        if (index >= parameterValues_.size())
            return 0.0;
        return parameterValues_[index].load();
    }

    void process(const nxf::audio::ConstAudioBufferView& input,
                 const nxf::audio::AudioBufferView& output,
                 std::span<const nxf::midi::MidiEvent> midi,
                 double /*sampleRate*/, std::uint64_t /*streamTimeFrames*/) override
    {
        if (!prepared_ || !processor_) {
            output.clear();
            return;
        }
        const int frames = std::min(output.frames, std::max(0, maxFrames_));
        if (frames <= 0 || output.channels <= 0) {
            output.clear();
            return;
        }

        for (int c = 0; c < inputChannels_; ++c) {
            auto* plane = inputPlanes_[static_cast<std::size_t>(c)].data();
            if (input.samples != nullptr && c < input.channels) {
                for (int f = 0; f < frames; ++f)
                    plane[f] = input.sample(c, f);
            } else {
                std::fill_n(plane, static_cast<std::size_t>(frames), 0.0f);
            }
        }

        events_.clear();
        for (const auto& midiEvent : midi) {
            const auto& message = midiEvent.message;
            vst::Event event {};
            if (message.isNoteOn()) {
                event.type = vst::Event::kNoteOnEvent;
                event.noteOn.channel = static_cast<int16>(std::max(0, message.channel() - 1));
                event.noteOn.pitch = static_cast<int16>(message.noteNumber());
                event.noteOn.velocity = static_cast<float>(message.velocity()) / 127.0f;
                event.noteOn.noteId = -1;
            } else if (message.isNoteOff()) {
                event.type = vst::Event::kNoteOffEvent;
                event.noteOff.channel = static_cast<int16>(std::max(0, message.channel() - 1));
                event.noteOff.pitch = static_cast<int16>(message.noteNumber());
                event.noteOff.velocity = static_cast<float>(message.velocity()) / 127.0f;
                event.noteOff.noteId = -1;
            } else {
                continue;
            }
            const auto offset = std::min<std::uint64_t>(midiEvent.timestampFrames,
                                                        static_cast<std::uint64_t>(frames - 1));
            event.sampleOffset = static_cast<int32>(offset);
            (void)events_.addEvent(event);
        }

        changes_.clearQueue();
        for (std::size_t i = 0; i < parameterIds_.size(); ++i) {
            if (parameterDirty_[i].exchange(false)) {
                int32 index = 0;
                if (auto* queue = changes_.addParameterData(parameterIds_[i], index))
                    (void)queue->addPoint(0, parameterValues_[i].load(), index);
            }
        }

        vst::ProcessData data {};
        data.processMode = vst::kRealtime;
        data.symbolicSampleSize = vst::kSample32;
        data.numSamples = frames;
        data.numInputs = component_->getBusCount(vst::kAudio, vst::kInput) > 0 ? 1 : 0;
        data.numOutputs = component_->getBusCount(vst::kAudio, vst::kOutput) > 0 ? 1 : 0;
        data.inputs = data.numInputs > 0 ? &inputBus_ : nullptr;
        data.outputs = data.numOutputs > 0 ? &outputBus_ : nullptr;
        data.inputParameterChanges = &changes_;
        data.inputEvents = &events_;
        data.processContext = nullptr;
        (void)processor_->process(data);

        output.clear();
        for (int c = 0; c < output.channels && c < outputChannels_; ++c) {
            const auto* plane = outputPlanes_[static_cast<std::size_t>(c)].data();
            for (int f = 0; f < frames; ++f)
                output.setSample(c, f, plane[f]);
        }
    }

    [[nodiscard]] std::vector<std::uint8_t> saveState() const override
    {
        if (!component_)
            return {};
        MemoryStream stream;
        if (component_->getState(&stream) != kResultOk)
            return {};
        const auto size = static_cast<std::size_t>(stream.getSize());
        const auto* data = reinterpret_cast<const std::uint8_t*>(stream.getData());
        if (data == nullptr || size == 0)
            return {};
        return {data, data + size};
    }

    [[nodiscard]] Error restoreState(std::span<const std::uint8_t> state) override
    {
        if (!component_)
            return Error::unavailable("no VST3 component");
        std::vector<std::uint8_t> copy(state.begin(), state.end());
        MemoryStream stream(copy.data(), static_cast<TSize>(copy.size()));
        stream.seek(0, IBStream::kIBSeekSet, nullptr);
        if (component_->setState(&stream) != kResultOk)
            return Error::invalidArgument("VST3 setState failed");
        if (controller_ && !singleComponent_) {
            MemoryStream controllerStream(copy.data(), static_cast<TSize>(copy.size()));
            controllerStream.seek(0, IBStream::kIBSeekSet, nullptr);
            (void)controller_->setComponentState(&controllerStream);
        }
        return Error::success();
    }

private:
    void readParameters()
    {
        parameters_.clear();
        parameterIds_.clear();
        parameterValues_.clear();
        parameterDirty_.clear();
        if (!controller_)
            return;
        const auto count = controller_->getParameterCount();
        for (int32 i = 0; i < count; ++i) {
            vst::ParameterInfo info {};
            if (controller_->getParameterInfo(i, info) != kResultOk)
                continue;
            PluginParameter parameter;
            parameter.id = std::to_string(info.id);
            parameter.name = utf8FromUtf16(info.title);
            parameter.minimum = 0.0;
            parameter.maximum = 1.0;
            parameter.defaultNormalized = info.defaultNormalizedValue;
            parameters_.push_back(std::move(parameter));
            parameterIds_.push_back(info.id);
            parameterValues_.emplace_back(info.defaultNormalizedValue);
            parameterDirty_.emplace_back(false);
        }
    }

    [[nodiscard]] int busChannelCount(vst::BusDirections direction) const
    {
        vst::BusInfo info {};
        if (component_ && component_->getBusInfo(vst::kAudio, direction, 0, info) == kResultOk)
            return std::max<int32>(info.channelCount, 0);
        return 0;
    }

    void fillArrangements(vst::BusDirections direction,
                          std::vector<vst::SpeakerArrangement>& out) const
    {
        out.clear();
        if (!component_ || !processor_)
            return;
        const auto buses = component_->getBusCount(vst::kAudio, direction);
        for (int32 b = 0; b < buses; ++b) {
            vst::SpeakerArrangement arrangement = vst::SpeakerArr::kStereo;
            (void)processor_->getBusArrangement(direction, b, arrangement);
            out.push_back(arrangement);
        }
    }

    void activateMainBuses()
    {
        if (!component_)
            return;
        for (const auto direction : {vst::kInput, vst::kOutput}) {
            const auto buses = component_->getBusCount(vst::kAudio, direction);
            for (int32 b = 0; b < buses; ++b)
                (void)component_->activateBus(vst::kAudio, direction, b, b == 0);
        }
    }

    LoadedClass loaded_;
    PluginDescriptor descriptor_;
    IPtr<vst::IComponent> component_;
    IPtr<vst::IEditController> controller_;
    IPtr<vst::IAudioProcessor> processor_;
    bool connected_ = false;
    bool singleComponent_ = false;
    bool prepared_ = false;
    int inputChannels_ = 0;
    int outputChannels_ = 0;
    int maxFrames_ = 0;
    std::vector<std::vector<float>> inputPlanes_;
    std::vector<std::vector<float>> outputPlanes_;
    std::vector<float*> inputPointers_;
    std::vector<float*> outputPointers_;
    vst::AudioBusBuffers inputBus_ {};
    vst::AudioBusBuffers outputBus_ {};
    std::vector<PluginParameter> parameters_;
    std::vector<vst::ParamID> parameterIds_;
    // std::deque: elements never relocate, so the atomics
    // stay put while the list grows during readParameters().
    std::deque<std::atomic<double>> parameterValues_;
    std::deque<std::atomic<bool>> parameterDirty_;
    vst::EventList events_ {2048};
    vst::ParameterChanges changes_;
};

} // namespace

bool isVst3SdkIntegrated() noexcept { return true; }

nxf::core::Result<PluginDescriptor> Vst3FormatAdapter::scanFile(const std::string& path)
{
    if (!hasVst3Suffix(path))
        return nxf::core::Result<PluginDescriptor>::failure(
            Error::invalidArgument("not a .vst3 file"));
    auto loaded = loadClass(path);
    if (!loaded.hasValue())
        return nxf::core::Result<PluginDescriptor>::failure(loaded.error());

    int inputs = 0;
    int outputs = 0;
    probeBusCounts(loaded.value(), inputs, outputs);

    PluginDescriptor descriptor;
    descriptor.format = "vst3";
    descriptor.factoryKey = loaded.value().info.ID().toString();
    descriptor.id = "vst3." + descriptor.factoryKey;
    descriptor.name = loaded.value().info.name();
    descriptor.manufacturer = loaded.value().info.vendor();
    descriptor.version = loaded.value().info.version();
    descriptor.category = categoryOf(loaded.value().info);
    descriptor.path = path;
    descriptor.inputChannels = inputs;
    descriptor.outputChannels = outputs;
    descriptor.acceptsMidi = true;
    descriptor.producesMidi = false;
    return nxf::core::Result<PluginDescriptor>::success(std::move(descriptor));
}

nxf::core::Result<std::unique_ptr<IPluginInstance>> Vst3FormatAdapter::createInstance(const PluginDescriptor& descriptor)
{
    using R = nxf::core::Result<std::unique_ptr<IPluginInstance>>;
    if (descriptor.format != "vst3")
        return R::failure(Error::invalidArgument("descriptor is not VST3"));

    std::string ignoredError;
    auto module = hosting::Module::create(descriptor.path, ignoredError);
    if (!module)
        return R::failure(Error::unavailable("VST3 module could not be loaded"));
    const auto factory = module->getFactory();
    hosting::ClassInfo match;
    bool found = false;
    for (const auto& info : factory.classInfos()) {
        if (info.category() == kVstAudioEffectClass && info.ID().toString() == descriptor.factoryKey) {
            match = info;
            found = true;
            break;
        }
    }
    if (!found)
        return R::failure(Error::unavailable("VST3 class not found in module"));

    auto instance = std::make_unique<Vst3Instance>(LoadedClass{module, match}, descriptor);
    const auto status = instance->initialize();
    if (!status.isSuccess())
        return R::failure(status);
    return R::success(std::unique_ptr<IPluginInstance>{std::move(instance)});
}

} // namespace nxf::plugin
