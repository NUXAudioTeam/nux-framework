#pragma once

#include <nxf/audio/AudioNode.h>
#include <nxf/plugin/AudioPlugin.h>

#include <memory>
#include <utility>

namespace nxf::plugin {

/// Hosts a plugin instance inside an nxf_audio graph. The node is
/// the bridge between the plugin lifecycle and the graph's: graph
/// prepare() prepares the plugin, graph process() runs it (with no
/// MIDI events - feeding MIDI into the graph is engine work that
/// arrives with the DAW integration phase).
class PluginNode final : public nxf::audio::IAudioNode {
public:
    explicit PluginNode(std::unique_ptr<IPluginInstance> instance) : instance_(std::move(instance)) {}
    [[nodiscard]] IPluginInstance* instance() noexcept { return instance_.get(); }

    void prepare(double sampleRate, int, int maxFrames) override {
        if (instance_ != nullptr) {
            const int channels = instance_->descriptor().outputChannels > 0
                ? instance_->descriptor().outputChannels : 2;
            (void)instance_->prepare(sampleRate, maxFrames, channels);
        }
    }

    void process(const nxf::audio::ConstAudioBufferView& input, const nxf::audio::AudioBufferView& output,
                 double sampleRate, std::uint64_t streamTimeFrames) override {
        if (instance_ == nullptr) {
            output.clear();
            return;
        }
        instance_->process(input, output, {}, sampleRate, streamTimeFrames);
    }

private:
    std::unique_ptr<IPluginInstance> instance_;
};

} // namespace nxf::plugin
