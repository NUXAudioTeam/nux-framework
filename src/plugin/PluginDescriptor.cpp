#include <nxf/plugin/PluginDescriptor.h>

namespace nxf::plugin {

using nxf::project::Value;

Value PluginDescriptor::toValue() const
{
    Value root = Value::object();
    root.set("id", Value::string(id));
    root.set("name", Value::string(name));
    root.set("manufacturer", Value::string(manufacturer));
    root.set("version", Value::string(version));
    root.set("category", Value::string(category));
    root.set("format", Value::string(format));
    root.set("behavior", Value::string(factoryKey));
    root.set("path", Value::string(path));
    root.set("inputChannels", Value::integer(inputChannels));
    root.set("outputChannels", Value::integer(outputChannels));
    root.set("acceptsMidi", Value::boolean(acceptsMidi));
    root.set("producesMidi", Value::boolean(producesMidi));
    return root;
}

nxf::core::Result<PluginDescriptor> PluginDescriptor::fromValue(const Value& value)
{
    using nxf::core::Error;
    using R = nxf::core::Result<PluginDescriptor>;
    if (!value.isObject()) return R::failure(Error::invalidArgument("plugin descriptor is not an object"));
    PluginDescriptor d;
    auto text = [&](const char* key) { const Value* v = value.get(key); return v != nullptr ? v->asString() : std::string{}; };
    auto number = [&](const char* key) { const Value* v = value.get(key); return v != nullptr ? static_cast<int>(v->asInt()) : 0; };
    auto flag = [&](const char* key) { const Value* v = value.get(key); return v != nullptr && v->asBool(); };
    d.id = text("id");
    d.name = text("name");
    d.manufacturer = text("manufacturer");
    d.version = text("version");
    d.category = text("category");
    d.format = text("format");
    d.factoryKey = text("behavior");
    d.path = text("path");
    d.inputChannels = number("inputChannels");
    d.outputChannels = number("outputChannels");
    d.acceptsMidi = flag("acceptsMidi");
    d.producesMidi = flag("producesMidi");
    if (d.id.empty() || d.name.empty()) return R::failure(Error::invalidArgument("plugin descriptor needs an id and a name"));
    return R::success(d);
}

} // namespace nxf::plugin
