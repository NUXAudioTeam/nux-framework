// Minimal VST3 test plug-in used only as a test fixture: the
// VST3 hosting test scans, instantiates, prepares, processes,
// changes parameters on and saves/restores state from this real
// .vst3 binary. It is built only when NXF_ENABLE_VST3=ON (it
// needs the Steinberg SDK) and is never shipped with NXF.
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "base/source/fstreamer.h"
#include "public.sdk/source/main/pluginfactory.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace {
constexpr ParamID kGainId = 0;
double gGain = 1.0;
const FUID kProcUid(0x1A2B3C4D, 0x11111111, 0x22222222, 0x33333333);
const FUID kCtrlUid(0x1A2B3C4D, 0x44444444, 0x55555555, 0x66666666);
}

class TestGainProc : public AudioEffect {
public:
    tresult PLUGIN_API initialize(FUnknown* context) override {
        if (AudioEffect::initialize(context) != kResultOk) return kResultFalse;
        setControllerClass(kCtrlUid);
        addAudioInput(STR16("In"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Out"), SpeakerArr::kStereo);
        return kResultOk;
    }
    tresult PLUGIN_API process(ProcessData& data) override {
        if (data.inputParameterChanges) {
            const int count = data.inputParameterChanges->getParameterCount();
            for (int i = 0; i < count; ++i) {
                auto* queue = data.inputParameterChanges->getParameterData(i);
                if (queue && queue->getParameterId() == kGainId) {
                    int32 index = 0; int32 offset = 0; ParamValue value = 0;
                    const int points = queue->getPointCount();
                    if (points > 0 && queue->getPoint(points - 1, offset, value) == kResultOk) {
                        (void)index; gGain = value;
                    }
                }
            }
        }
        if (data.numInputs == 0 || data.numOutputs == 0) return kResultOk;
        auto& in = data.inputs[0];
        auto& out = data.outputs[0];
        const int channels = std::min(in.numChannels, out.numChannels);
        for (int c = 0; c < channels; ++c) {
            for (int32 f = 0; f < data.numSamples; ++f)
                out.channelBuffers32[c][f] = static_cast<float>(in.channelBuffers32[c][f] * gGain);
        }
        return kResultOk;
    }
    tresult PLUGIN_API getState(IBStream* state) override {
        IBStreamer streamer(state, kLittleEndian);
        streamer.writeDouble(gGain);
        return kResultOk;
    }
    tresult PLUGIN_API setState(IBStream* state) override {
        IBStreamer streamer(state, kLittleEndian);
        double v = 1.0;
        if (!streamer.readDouble(v)) return kResultFalse;
        gGain = v;
        return kResultOk;
    }
    static FUnknown* createInstance(void*) { return (IAudioProcessor*)new TestGainProc(); }
};

class TestGainCtrl : public EditController {
public:
    tresult PLUGIN_API initialize(FUnknown* context) override {
        if (EditController::initialize(context) != kResultOk) return kResultFalse;
        parameters.addParameter(STR16("Gain"), nullptr, 0, 1.0, ParameterInfo::kCanAutomate, kGainId);
        return kResultOk;
    }
    static FUnknown* createInstance(void*) { return (IEditController*)new TestGainCtrl(); }
};

BEGIN_FACTORY_DEF("NUX Audio Team", "https://github.com/NUXAudioTeam", "mailto:NUXAudioTeam@users.noreply.github.com")
    DEF_CLASS2(INLINE_UID_FROM_FUID(kProcUid),
               PClassInfo::kManyInstances, kVstAudioEffectClass, "NXF Test Gain", Vst::kDistributable,
               Vst::PlugType::kFx, "1.0.0", kVstVersionString, TestGainProc::createInstance)
    DEF_CLASS2(INLINE_UID_FROM_FUID(kCtrlUid),
               PClassInfo::kManyInstances, kVstComponentControllerClass, "NXF Test Gain Ctrl", 0, "",
               "1.0.0", kVstVersionString, TestGainCtrl::createInstance)
END_FACTORY
