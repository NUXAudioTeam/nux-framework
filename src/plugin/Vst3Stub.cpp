#include <nxf/plugin/PluginFormatAdapter.h>

// Stub used when the Steinberg VST3 SDK is not integrated
// (NXF_ENABLE_VST3=OFF, the default). It reports Unavailable
// honestly; the real adapter is src/plugin/vst3/Vst3Adapter.cpp.

namespace nxf::plugin {

bool isVst3SdkIntegrated() noexcept { return false; }

nxf::core::Result<PluginDescriptor> Vst3FormatAdapter::scanFile(const std::string&)
{
    return nxf::core::Result<PluginDescriptor>::failure(
        nxf::core::Error::unavailable("VST3 SDK is not integrated in this build (NXF_ENABLE_VST3=OFF)"));
}

nxf::core::Result<std::unique_ptr<IPluginInstance>> Vst3FormatAdapter::createInstance(const PluginDescriptor&)
{
    return nxf::core::Result<std::unique_ptr<IPluginInstance>>::failure(
        nxf::core::Error::unavailable("VST3 SDK is not integrated in this build (NXF_ENABLE_VST3=OFF)"));
}

} // namespace nxf::plugin
