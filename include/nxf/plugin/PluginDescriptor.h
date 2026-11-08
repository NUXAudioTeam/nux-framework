#pragma once

#include <nxf/core/Result.h>
#include <nxf/project/Value.h>

#include <string>

namespace nxf::plugin {

/// Everything known about a plugin without loading it. Produced by
/// scanning, stored in caches, and used to decide what can be
/// instantiated. format names the adapter family ("nxf-test" today,
/// "vst3" when that adapter exists); factoryKey is only meaningful
/// to the adapter that owns the format.
struct PluginDescriptor {
    std::string id;
    std::string name;
    std::string manufacturer;
    std::string version;
    std::string category;      // "effect", "instrument", "analyzer"
    std::string format;
    std::string factoryKey;
    std::string path;
    int inputChannels = 0;
    int outputChannels = 0;
    bool acceptsMidi = false;
    bool producesMidi = false;

    [[nodiscard]] bool isEffect() const noexcept { return category == "effect"; }
    [[nodiscard]] bool isInstrument() const noexcept { return category == "instrument"; }

    [[nodiscard]] nxf::project::Value toValue() const;
    [[nodiscard]] static nxf::core::Result<PluginDescriptor> fromValue(const nxf::project::Value& value);

    bool operator==(const PluginDescriptor&) const = default;
};

} // namespace nxf::plugin
