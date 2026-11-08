#pragma once

#include <nxf/audio/AudioBuffer.h>
#include <nxf/core/Error.h>
#include <nxf/midi/MidiEventQueue.h>
#include <nxf/plugin/PluginDescriptor.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace nxf::plugin {

enum class PluginState { Created, Prepared };

/// One automatable parameter. Values cross the API boundary
/// normalised to 0..1 (the plugin world convention); the plain
/// range is metadata for UIs and tests.
struct PluginParameter {
    std::string id;
    std::string name;
    double minimum = 0.0;
    double maximum = 1.0;
    double defaultNormalized = 0.0;
};

/// A live plugin instance. Lifecycle: Created --prepare()-->
/// Prepared --suspend()--> Created. process() is only meaningful
/// while Prepared; calling it otherwise writes silence, so a
/// mis-sequenced host degrades quietly instead of crashing.
/// process() runs under audio-thread rules when hosted in a graph:
/// parameters are atomics in implementations, and saveState /
/// restoreState are non-real-time operations.
class IPluginInstance {
public:
    virtual ~IPluginInstance() = default;
    [[nodiscard]] virtual const PluginDescriptor& descriptor() const noexcept = 0;
    [[nodiscard]] virtual PluginState state() const noexcept = 0;
    [[nodiscard]] virtual nxf::core::Error prepare(double sampleRate, int maxFrames, int channels) = 0;
    virtual void suspend() = 0;
    virtual void reset() = 0;
    [[nodiscard]] virtual const std::vector<PluginParameter>& parameters() const noexcept = 0;
    [[nodiscard]] virtual bool setParameterNormalized(std::size_t index, double normalized) = 0;
    [[nodiscard]] virtual double parameterNormalized(std::size_t index) const noexcept = 0;
    virtual void process(const nxf::audio::ConstAudioBufferView& input,
                         const nxf::audio::AudioBufferView& output,
                         std::span<const nxf::midi::MidiEvent> midi,
                         double sampleRate, std::uint64_t streamTimeFrames) = 0;
    /// Opaque state chunk (the plugin's own format); restoring a
    /// chunk the plugin cannot parse is an Error, never a guess.
    [[nodiscard]] virtual std::vector<std::uint8_t> saveState() const = 0;
    [[nodiscard]] virtual nxf::core::Error restoreState(std::span<const std::uint8_t> state) = 0;
};

} // namespace nxf::plugin
