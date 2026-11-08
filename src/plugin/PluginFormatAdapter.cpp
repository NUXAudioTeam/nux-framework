#include <nxf/plugin/PluginFormatAdapter.h>
#include <nxf/platform/FileSystem.h>
#include <nxf/project/Value.h>

#include <atomic>
#include <cmath>

namespace nxf::plugin {
namespace {

using nxf::core::Error;
using nxf::project::Value;

class TestPluginBase : public IPluginInstance {
public:
    explicit TestPluginBase(PluginDescriptor descriptor) : descriptor_(std::move(descriptor)) {}
    [[nodiscard]] const PluginDescriptor& descriptor() const noexcept override { return descriptor_; }
    [[nodiscard]] PluginState state() const noexcept override { return state_; }

    [[nodiscard]] Error prepare(double sampleRate, int maxFrames, int channels) override {
        if (sampleRate <= 0.0 || maxFrames <= 0 || channels <= 0)
            return Error::invalidArgument("prepare: sample rate, frames and channels must be positive");
        sampleRate_ = sampleRate;
        state_ = PluginState::Prepared;
        return Error::success();
    }
    void suspend() override { state_ = PluginState::Created; }

protected:
    PluginDescriptor descriptor_;
    PluginState state_ = PluginState::Created;
    double sampleRate_ = 0.0;
};

// ---------------------------------------------------------------------------

class GainPlugin final : public TestPluginBase {
public:
    explicit GainPlugin(PluginDescriptor descriptor) : TestPluginBase(std::move(descriptor)) {
        parameters_.push_back(PluginParameter{"gain", "Gain", 0.0, 2.0, 0.5});
    }
    [[nodiscard]] const std::vector<PluginParameter>& parameters() const noexcept override { return parameters_; }
    [[nodiscard]] bool setParameterNormalized(std::size_t index, double normalized) override {
        if (index >= parameters_.size()) return false;
        gainNormalized_.store(normalized < 0.0 ? 0.0 : (normalized > 1.0 ? 1.0 : normalized));
        return true;
    }
    [[nodiscard]] double parameterNormalized(std::size_t index) const noexcept override {
        return index < parameters_.size() ? gainNormalized_.load() : 0.0;
    }
    void reset() override {}
    void process(const nxf::audio::ConstAudioBufferView& input, const nxf::audio::AudioBufferView& output,
                 std::span<const nxf::midi::MidiEvent>, double, std::uint64_t) override {
        if (state_ != PluginState::Prepared) {
            output.clear();
            return;
        }
        const auto gain = static_cast<float>(parameters_[0].minimum
            + gainNormalized_.load(std::memory_order_relaxed) * (parameters_[0].maximum - parameters_[0].minimum));
        for (int frame = 0; frame < output.frames; ++frame)
            for (int ch = 0; ch < output.channels; ++ch)
                output.setSample(ch, frame, (ch < input.channels ? input.sample(ch, frame) : 0.0f) * gain);
    }
    [[nodiscard]] std::vector<std::uint8_t> saveState() const override {
        Value state = Value::object();
        state.set("gain", Value::real(gainNormalized_.load()));
        const std::string text = nxf::project::serializeValue(state);
        return {text.begin(), text.end()};
    }
    [[nodiscard]] Error restoreState(std::span<const std::uint8_t> state) override {
        const std::string text{reinterpret_cast<const char*>(state.data()), state.size()};
        auto parsed = nxf::project::parseValue(text);
        if (!parsed.hasValue()) return Error::invalidArgument("unparseable plugin state");
        const Value* gain = parsed.value().get("gain");
        if (gain == nullptr || !gain->isReal() && !gain->isInt()) return Error::invalidArgument("plugin state has no gain");
        return setParameterNormalized(0, gain->asReal()) ? Error::success() : Error::internal("could not restore gain");
    }

private:
    std::vector<PluginParameter> parameters_;
    std::atomic<double> gainNormalized_{0.5};
};

// ---------------------------------------------------------------------------

class SinePlugin final : public TestPluginBase {
public:
    explicit SinePlugin(PluginDescriptor descriptor) : TestPluginBase(std::move(descriptor)) {
        parameters_.push_back(PluginParameter{"frequency", "Frequency", 50.0, 1000.0, (440.0 - 50.0) / 950.0});
        parameters_.push_back(PluginParameter{"amplitude", "Amplitude", 0.0, 1.0, 0.8});
    }
    [[nodiscard]] const std::vector<PluginParameter>& parameters() const noexcept override { return parameters_; }
    [[nodiscard]] bool setParameterNormalized(std::size_t index, double normalized) override {
        if (index >= values_.size()) return false;
        values_[index].store(normalized < 0.0 ? 0.0 : (normalized > 1.0 ? 1.0 : normalized));
        return true;
    }
    [[nodiscard]] double parameterNormalized(std::size_t index) const noexcept override {
        return index < values_.size() ? values_[index].load() : 0.0;
    }
    void reset() override { phase_ = 0.0; }
    void process(const nxf::audio::ConstAudioBufferView&, const nxf::audio::AudioBufferView& output,
                 std::span<const nxf::midi::MidiEvent> midi, double sampleRate, std::uint64_t) override {
        if (state_ != PluginState::Prepared) {
            output.clear();
            return;
        }
        double frequency = parameters_[0].minimum + values_[0].load(std::memory_order_relaxed) * (parameters_[0].maximum - parameters_[0].minimum);
        for (const auto& event : midi) {
            if (event.message.isNoteOn()) {
                const double semitonesFromA4 = static_cast<double>(event.message.noteNumber() - 69);
                frequency = 440.0 * std::pow(2.0, semitonesFromA4 / 12.0);
            }
        }
        const float amplitude = static_cast<float>(values_[1].load(std::memory_order_relaxed));
        constexpr double twoPi = 6.28318530717958647692;
        const double increment = twoPi * frequency / sampleRate;
        for (int frame = 0; frame < output.frames; ++frame) {
            const float value = amplitude * static_cast<float>(std::sin(phase_));
            phase_ += increment;
            if (phase_ >= twoPi) phase_ -= twoPi;
            for (int ch = 0; ch < output.channels; ++ch) output.setSample(ch, frame, value);
        }
    }
    [[nodiscard]] std::vector<std::uint8_t> saveState() const override {
        Value state = Value::object();
        state.set("frequency", Value::real(values_[0].load()));
        state.set("amplitude", Value::real(values_[1].load()));
        const std::string text = nxf::project::serializeValue(state);
        return {text.begin(), text.end()};
    }
    [[nodiscard]] Error restoreState(std::span<const std::uint8_t> state) override {
        const std::string text{reinterpret_cast<const char*>(state.data()), state.size()};
        auto parsed = nxf::project::parseValue(text);
        if (!parsed.hasValue()) return Error::invalidArgument("unparseable plugin state");
        const Value* frequency = parsed.value().get("frequency");
        const Value* amplitude = parsed.value().get("amplitude");
        if (frequency == nullptr || amplitude == nullptr) return Error::invalidArgument("plugin state is incomplete");
        setParameterNormalized(0, frequency->asReal());
        setParameterNormalized(1, amplitude->asReal());
        return Error::success();
    }

private:
    std::vector<PluginParameter> parameters_;
    std::array<std::atomic<double>, 2> values_{0.4105, 0.8};
    double phase_ = 0.0;
};

} // namespace

nxf::core::Result<PluginDescriptor> NativeTestPluginFormatAdapter::scanFile(const std::string& path)
{
    using R = nxf::core::Result<PluginDescriptor>;
    const auto content = nxf::platform::readTextFile(path);
    if (!content.hasValue()) return R::failure(content.error());
    auto parsed = nxf::project::parseValue(content.value());
    if (!parsed.hasValue()) return R::failure(parsed.error());
    auto descriptor = PluginDescriptor::fromValue(parsed.value());
    if (!descriptor.hasValue()) return descriptor;
    PluginDescriptor found = descriptor.value();
    found.format = "nxf-test";
    found.path = path;
    return R::success(found);
}

nxf::core::Result<std::unique_ptr<IPluginInstance>> NativeTestPluginFormatAdapter::createInstance(const PluginDescriptor& descriptor)
{
    using R = nxf::core::Result<std::unique_ptr<IPluginInstance>>;
    if (descriptor.factoryKey == "gain")
        return R::success(std::unique_ptr<IPluginInstance>{std::make_unique<GainPlugin>(descriptor)});
    if (descriptor.factoryKey == "sine")
        return R::success(std::unique_ptr<IPluginInstance>{std::make_unique<SinePlugin>(descriptor)});
    return R::failure(Error::unavailable("unknown nxf-test plugin behaviour '" + descriptor.factoryKey + "'"));
}

nxf::core::Result<PluginDescriptor> Vst3FormatAdapter::scanFile(const std::string&)
{
    return nxf::core::Result<PluginDescriptor>::failure(
        Error::unavailable("VST3 SDK is not integrated (licensing decision pending)"));
}

nxf::core::Result<std::unique_ptr<IPluginInstance>> Vst3FormatAdapter::createInstance(const PluginDescriptor&)
{
    return nxf::core::Result<std::unique_ptr<IPluginInstance>>::failure(
        Error::unavailable("VST3 SDK is not integrated (licensing decision pending)"));
}

} // namespace nxf::plugin
