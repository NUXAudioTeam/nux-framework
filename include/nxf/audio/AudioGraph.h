#pragma once

#include <nxf/audio/AudioNode.h>
#include <nxf/core/Error.h>

#include <cstddef>
#include <memory>
#include <vector>

namespace nxf::audio {

/// A directed graph of audio nodes with preallocated routing.
///
/// Model: every node's inputs are summed (that sum *is* the mixer);
/// nodes with no inputs receive the graph's external input; the
/// graph's output is the sum of the sink nodes (nodes nothing feeds
/// into). prepare() topologically sorts the graph and allocates all
/// intermediate buffers - a cycle is an error there, never a hang in
/// the audio thread. process() allocates nothing and locks nothing.
///
/// Structural edits (add/connect) happen off the audio thread while
/// rendering is stopped. Node parameters may change at any time via
/// the nodes' own atomics.
class AudioGraph {
public:
    using NodeId = std::size_t;

    /// Takes shared ownership of the node (the caller keeps a handle
    /// to tweak its parameters). Returns the node's id.
    NodeId addNode(std::shared_ptr<IAudioNode> node);
    [[nodiscard]] nxf::core::Error connect(NodeId from, NodeId to);

    [[nodiscard]] nxf::core::Error prepare(double sampleRate, int channels, int maxFrames);
    [[nodiscard]] bool isPrepared() const noexcept { return prepared_; }

    /// Renders one block. Requires prepare() and matching channels;
    /// output.frames must not exceed the prepared maximum.
    [[nodiscard]] nxf::core::Error process(const ConstAudioBufferView& input,
                                           const AudioBufferView& output,
                                           std::uint64_t streamTimeFrames);

private:
    struct Entry {
        std::shared_ptr<IAudioNode> node;
        std::vector<NodeId> inputs;
        AudioBuffer inputBuffer;
        AudioBuffer outputBuffer;
    };
    [[nodiscard]] bool hasCycle() const;

    std::vector<Entry> entries_;
    std::vector<NodeId> order_;      // topological, built by prepare()
    double sampleRate_ = 0.0;
    int channels_ = 0;
    int maxFrames_ = 0;
    bool prepared_ = false;
};

} // namespace nxf::audio
