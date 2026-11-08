#pragma once

#include <nxf/core/Result.h>
#include <nxf/plugin/AudioPlugin.h>

#include <memory>
#include <string>
#include <string_view>

namespace nxf::plugin {

/// One plugin format family. This is the seam the Steinberg VST3
/// SDK will plug into later: SDK types live behind an adapter
/// implementation and never enter these public headers.
class IPluginFormatAdapter {
public:
    virtual ~IPluginFormatAdapter() = default;
    [[nodiscard]] virtual std::string_view formatName() const noexcept = 0;
    /// Reads a candidate file's descriptor without instantiating it.
    [[nodiscard]] virtual nxf::core::Result<PluginDescriptor> scanFile(const std::string& path) = 0;
    [[nodiscard]] virtual nxf::core::Result<std::unique_ptr<IPluginInstance>> createInstance(const PluginDescriptor& descriptor) = 0;
};

/// True when this build integrated the Steinberg VST3 SDK
/// (NXF_ENABLE_VST3=ON). The default build returns false and the
/// VST3 adapter reports Unavailable.
[[nodiscard]] bool isVst3SdkIntegrated() noexcept;

/// The framework's own test-plugin format (.nxfplug structured-text
/// descriptors with "gain" and "sine" behaviours). It exists so
/// hosting, scanning, parameters and state can be verified end to
/// end without any third-party SDK - the same role JUCE test
/// plugins played for NUX DAW, in our own format.
class NativeTestPluginFormatAdapter final : public IPluginFormatAdapter {
public:
    [[nodiscard]] std::string_view formatName() const noexcept override { return "nxf-test"; }
    [[nodiscard]] nxf::core::Result<PluginDescriptor> scanFile(const std::string& path) override;
    [[nodiscard]] nxf::core::Result<std::unique_ptr<IPluginInstance>> createInstance(const PluginDescriptor& descriptor) override;
};

/// The VST3 seam. With NXF_ENABLE_VST3=OFF (the default) the
/// adapter reports Unavailable and does not pretend to host
/// anything. With it ON, the adapter hosts real VST3 plug-ins
/// through the Steinberg SDK (MIT-licensed since SDK 3.8 - see
/// docs/DEPENDENCIES_AND_LICENSING.md). SDK types stay behind the
/// adapter implementation either way.
class Vst3FormatAdapter final : public IPluginFormatAdapter {
public:
    [[nodiscard]] std::string_view formatName() const noexcept override { return "vst3"; }
    [[nodiscard]] nxf::core::Result<PluginDescriptor> scanFile(const std::string& path) override;
    [[nodiscard]] nxf::core::Result<std::unique_ptr<IPluginInstance>> createInstance(const PluginDescriptor& descriptor) override;
};

} // namespace nxf::plugin
