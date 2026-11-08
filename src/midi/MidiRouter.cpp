#include <nxf/midi/MidiRouter.h>

namespace nxf::midi {

std::vector<MidiMessage> MidiRouter::route(const MidiMessage& message) const
{
    std::vector<MidiMessage> out;
    const int channel = message.channel();
    for (const auto& rule : routes_) {
        if (rule.inputChannel != 0 && channel != 0 && channel != rule.inputChannel) continue;
        const int outChannel = rule.outputChannel != 0 ? rule.outputChannel : channel;
        switch (message.type()) {
            case MidiMessageType::NoteOn:
            case MidiMessageType::NoteOff: {
                if (!rule.passNotes) break;
                const int note = message.noteNumber() + rule.transposeSemitones;
                if (note < 0 || note > 127) break;   // dropped, not wrapped
                out.push_back(message.type() == MidiMessageType::NoteOn && !message.isNoteOff()
                    ? MidiMessage::noteOn(outChannel, note, message.velocity())
                    : MidiMessage::noteOff(outChannel, note, message.velocity()));
                break;
            }
            case MidiMessageType::ControlChange:
                if (rule.passControlChange)
                    out.push_back(MidiMessage::controlChange(outChannel, message.controllerNumber(), message.controllerValue()));
                break;
            case MidiMessageType::PitchBend:
                if (rule.passPitchBend) out.push_back(MidiMessage::pitchBend(outChannel, message.pitchBend()));
                break;
            case MidiMessageType::ProgramChange:
                if (rule.passProgramChange) out.push_back(MidiMessage::programChange(outChannel, message.programNumber()));
                break;
            default:
                if (rule.passOther && channel == 0) out.push_back(message);
                else if (rule.passOther && channel != 0) out.push_back(message);
                break;
        }
    }
    return out;
}

} // namespace nxf::midi
