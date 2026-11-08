#pragma once

#include <nxf/plugin/PluginFormatAdapter.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace nxf::plugin {

struct PluginScanResult {
    std::vector<PluginDescriptor> descriptors;   // newly registered by this scan
    std::vector<std::string> errors;             // one entry per rejected candidate/duplicate
};

/// The plugin registry: owns the format adapters, scans folders,
/// and creates instances by descriptor. Scanning never throws and
/// never silently overwrites: a duplicate plugin id is an error,
/// the first registration wins, and the file that lost is named in
/// the errors. Control-thread API (not for the audio thread).
class PluginManager {
public:
    void registerAdapter(std::shared_ptr<IPluginFormatAdapter> adapter);
    [[nodiscard]] IPluginFormatAdapter* findAdapter(const std::string& format) const;

    [[nodiscard]] PluginScanResult scanDirectory(const std::string& directory);
    [[nodiscard]] const PluginDescriptor* findPlugin(const std::string& pluginId) const;
    [[nodiscard]] std::size_t pluginCount() const noexcept { return plugins_.size(); }

    [[nodiscard]] nxf::core::Result<std::unique_ptr<IPluginInstance>> createInstance(const PluginDescriptor& descriptor) const;

private:
    std::vector<std::shared_ptr<IPluginFormatAdapter>> adapters_;
    std::map<std::string, PluginDescriptor> plugins_;
};

} // namespace nxf::plugin
