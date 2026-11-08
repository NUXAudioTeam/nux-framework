#include <nxf/midi/MidiDevice.h>
#include <nxf/midi/MidiBackendFactory.h>
#include <nxf/midi/OfflineMidiBackend.h>
#include "TestSupport.h"
#include <vector>
namespace {
struct Collector final : nxf::midi::IMidiInputCallback {
    std::vector<nxf::midi::MidiMessage> messages;
    void onMidiMessage(const nxf::midi::MidiMessage& m, std::uint64_t) override { messages.push_back(m); }
};
}
int main() {
    using namespace nxf::midi;
    OfflineMidiBackend backend;
    NXF_CHECK(backend.name() == "offline-midi");
    const auto devices = backend.enumerateDevices();
    NXF_CHECK(devices.hasValue()); NXF_CHECK(devices.value().size() == 2);
    Collector collector;
    auto input = std::move(backend.openInput("offline-in", collector).value());
    NXF_CHECK(input->start().isSuccess());
    backend.simulateIncoming(MidiMessage::noteOn(3, 65, 90));
    NXF_CHECK(collector.messages.size() == 1);
    NXF_CHECK(collector.messages[0].channel() == 3);
    NXF_CHECK(input->stop().isSuccess()); input->close();
    backend.simulateIncoming(MidiMessage::noteOn(3, 65, 90));     // closed: nothing new arrives
    NXF_CHECK(collector.messages.size() == 1);
    auto output = std::move(backend.openOutput("offline-out").value());
    NXF_CHECK(output->send(MidiMessage::noteOn(1, 60, 100)).isSuccess());
    NXF_CHECK(backend.sentMessages().size() == 1);
    NXF_CHECK(backend.sentMessages()[0].noteNumber() == 60);
    NXF_CHECK(!backend.openInput("no-such-device", collector).hasValue());

    auto system = createSystemMidiBackend();
    NXF_CHECK(system != nullptr);
#if !defined(_WIN32)
    NXF_CHECK(system->name() == "offline-midi");
#endif
    const auto sysDevices = system->enumerateDevices();
    NXF_CHECK(sysDevices.hasValue() || !sysDevices.error().message().empty());
    return nxf::test::report("MidiDeviceTests");
}
