#include <nxf/plugin/PluginManager.h>
#include <nxf/platform/FileSystem.h>

namespace nxf::plugin {

void PluginManager::registerAdapter(std::shared_ptr<IPluginFormatAdapter> adapter)
{
    if (adapter != nullptr) adapters_.push_back(std::move(adapter));
}

IPluginFormatAdapter* PluginManager::findAdapter(const std::string& format) const
{
    for (const auto& adapter : adapters_)
        if (adapter->formatName() == format) return adapter.get();
    return nullptr;
}

PluginScanResult PluginManager::scanDirectory(const std::string& directory)
{
    PluginScanResult result;
    const auto listing = nxf::platform::listDirectory(directory);
    if (!listing.hasValue()) {
        result.errors.push_back("cannot list plugin directory: " + directory);
        return result;
    }
    for (const std::string& name : listing.value()) {
        if (name.size() < 8 || name.substr(name.size() - 8) != ".nxfplug") continue;
        const std::string path = directory + "/" + name;
        bool scanned = false;
        for (const auto& adapter : adapters_) {
            auto descriptor = adapter->scanFile(path);
            if (!descriptor.hasValue()) {
                if (adapter->formatName() == std::string_view{"nxf-test"})
                    result.errors.push_back(name + ": " + std::string{descriptor.error().message()});
                continue;
            }
            scanned = true;
            PluginDescriptor found = descriptor.value();
            found.path = path;
            if (plugins_.contains(found.id)) {
                result.errors.push_back(name + ": duplicate plugin id '" + found.id + "' (first registration wins)");
            } else {
                plugins_[found.id] = found;
                result.descriptors.push_back(found);
            }
            break;
        }
        if (!scanned && adapters_.empty()) result.errors.push_back(name + ": no plugin format adapters registered");
    }
    return result;
}

const PluginDescriptor* PluginManager::findPlugin(const std::string& pluginId) const
{
    const auto it = plugins_.find(pluginId);
    return it == plugins_.end() ? nullptr : &it->second;
}

nxf::core::Result<std::unique_ptr<IPluginInstance>> PluginManager::createInstance(const PluginDescriptor& descriptor) const
{
    using R = nxf::core::Result<std::unique_ptr<IPluginInstance>>;
    IPluginFormatAdapter* adapter = findAdapter(descriptor.format);
    if (adapter == nullptr)
        return R::failure(nxf::core::Error::unavailable("no adapter registered for plugin format '" + descriptor.format + "'"));
    return adapter->createInstance(descriptor);
}

} // namespace nxf::plugin
