#include <nxf/audio/AudioGraph.h>

#include <algorithm>

namespace nxf::audio {
namespace {
void addInto(const ConstAudioBufferView& source, const AudioBufferView& destination)
{
    for (int frame = 0; frame < destination.frames; ++frame)
        for (int ch = 0; ch < destination.channels; ++ch)
            destination.setSample(ch, frame, destination.sample(ch, frame) + (ch < source.channels ? source.sample(ch, frame) : 0.0f));
}
} // namespace

AudioGraph::NodeId AudioGraph::addNode(std::shared_ptr<IAudioNode> node)
{
    entries_.push_back(Entry{std::move(node), {}, {}, {}});
    prepared_ = false;
    return entries_.size() - 1;
}

nxf::core::Error AudioGraph::connect(NodeId from, NodeId to)
{
    if (from >= entries_.size() || to >= entries_.size())
        return nxf::core::Error::invalidArgument("connect: unknown node id");
    auto& inputs = entries_[to].inputs;
    if (std::find(inputs.begin(), inputs.end(), from) == inputs.end()) inputs.push_back(from);
    prepared_ = false;
    return nxf::core::Error::success();
}

bool AudioGraph::hasCycle() const
{
    // Kahn's algorithm: if not every node can be emitted, what
    // remains is part of (or downstream of) a cycle.
    std::vector<std::size_t> indegree(entries_.size(), 0);
    std::vector<std::vector<NodeId>> outgoing(entries_.size());
    for (NodeId to = 0; to < entries_.size(); ++to)
        for (const NodeId from : entries_[to].inputs) {
            outgoing[from].push_back(to);
            ++indegree[to];
        }
    std::vector<NodeId> ready;
    for (NodeId i = 0; i < entries_.size(); ++i) if (indegree[i] == 0) ready.push_back(i);
    std::size_t emitted = 0;
    while (!ready.empty()) {
        const NodeId node = ready.back(); ready.pop_back();
        ++emitted;
        for (const NodeId next : outgoing[node]) if (--indegree[next] == 0) ready.push_back(next);
    }
    return emitted != entries_.size();
}

nxf::core::Error AudioGraph::prepare(double sampleRate, int channels, int maxFrames)
{
    if (sampleRate <= 0.0 || channels <= 0 || maxFrames <= 0)
        return nxf::core::Error::invalidArgument("prepare: sample rate, channels and max frames must be positive");
    if (entries_.empty()) return nxf::core::Error::invalidArgument("prepare: the graph has no nodes");
    if (hasCycle()) return nxf::core::Error::invalidArgument("prepare: the graph contains a cycle");

    // Topological order (the order_ vector left by Kahn above is a
    // valid execution order: every node after all of its inputs).
    order_.clear();
    {
        std::vector<std::size_t> indegree(entries_.size(), 0);
        std::vector<std::vector<NodeId>> outgoing(entries_.size());
        for (NodeId to = 0; to < entries_.size(); ++to)
            for (const NodeId from : entries_[to].inputs) {
                outgoing[from].push_back(to);
                ++indegree[to];
            }
        std::vector<NodeId> ready;
        for (NodeId i = 0; i < entries_.size(); ++i) if (indegree[i] == 0) ready.push_back(i);
        while (!ready.empty()) {
            const NodeId node = ready.back(); ready.pop_back();
            order_.push_back(node);
            for (const NodeId next : outgoing[node]) if (--indegree[next] == 0) ready.push_back(next);
        }
    }

    sampleRate_ = sampleRate;
    channels_ = channels;
    maxFrames_ = maxFrames;
    for (auto& entry : entries_) {
        entry.inputBuffer = AudioBuffer{channels, maxFrames};
        entry.outputBuffer = AudioBuffer{channels, maxFrames};
        entry.node->prepare(sampleRate, channels, maxFrames);
    }
    prepared_ = true;
    return nxf::core::Error::success();
}

nxf::core::Error AudioGraph::process(const ConstAudioBufferView& input, const AudioBufferView& output, std::uint64_t streamTimeFrames)
{
    if (!prepared_) return nxf::core::Error::unavailable("graph is not prepared");
    if (output.frames > maxFrames_ || output.channels != channels_)
        return nxf::core::Error::invalidArgument("process: block does not match the prepared format");

    std::vector<char> isSink(entries_.size(), 1);
    for (const auto& entry : entries_)
        for (const NodeId from : entry.inputs) isSink[from] = 0;

    for (const NodeId id : order_) {
        auto& entry = entries_[id];
        AudioBufferView nodeInput{entry.inputBuffer.view().samples, channels_, output.frames};
        nodeInput.clear();
        if (entry.inputs.empty()) {
            addInto(input, nodeInput);   // roots receive the external input
        } else {
            for (const NodeId from : entry.inputs) {
                const auto& source = entries_[from];
                ConstAudioBufferView sourceView{source.outputBuffer.constView().samples, channels_, output.frames};
                addInto(sourceView, nodeInput);
            }
        }
        AudioBufferView nodeOutput{entry.outputBuffer.view().samples, channels_, output.frames};
        nodeOutput.clear();
        ConstAudioBufferView nodeInputConst{nodeInput.samples, channels_, output.frames};
        entry.node->process(nodeInputConst, nodeOutput, sampleRate_, streamTimeFrames);
    }

    output.clear();
    AudioBufferView outView{output.samples, output.channels, output.frames};
    for (NodeId id = 0; id < entries_.size(); ++id) {
        if (isSink[id] == 0) continue;
        const auto& entry = entries_[id];
        ConstAudioBufferView sinkView{entry.outputBuffer.constView().samples, channels_, output.frames};
        addInto(sinkView, outView);
    }
    return nxf::core::Error::success();
}

} // namespace nxf::audio
