#include <nxf/project/ProjectDocument.h>

#include <sstream>

namespace nxf::project {
namespace {

std::vector<std::string> splitPath(const std::string& path)
{
    std::vector<std::string> parts;
    std::string current;
    for (const char c : path) {
        if (c == '.') {
            parts.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(current);
    return parts;
}

bool isIndex(const std::string& part)
{
    if (part.empty()) return false;
    for (const char c : part) if (c < '0' || c > '9') return false;
    return true;
}

const Value* getAt(const Value& root, const std::vector<std::string>& parts, std::size_t at)
{
    if (at == parts.size()) return &root;
    if (root.isObject()) return root.get(parts[at]) != nullptr ? getAt(*root.get(parts[at]), parts, at + 1) : nullptr;
    if (root.isArray() && isIndex(parts[at])) {
        const auto* element = root.at(static_cast<std::size_t>(std::stoul(parts[at])));
        return element != nullptr ? getAt(*element, parts, at + 1) : nullptr;
    }
    return nullptr;
}

bool setAt(Value& node, const std::vector<std::string>& parts, std::size_t at, Value newValue)
{
    if (at == parts.size()) {
        node = std::move(newValue);
        return true;
    }
    const std::string& part = parts[at];
    if (node.isObject()) {
        Value* child = node.get(part);
        if (child == nullptr) {
            node.set(part, isIndex(parts[at + 1 < parts.size() ? at + 1 : at]) && at + 1 < parts.size() ? Value::array() : Value::object());
            child = node.get(part);
        }
        return setAt(*child, parts, at + 1, std::move(newValue));
    }
    if (node.isArray() && isIndex(part)) {
        auto* array = const_cast<Value::Array*>(node.asArray());
        const auto index = static_cast<std::size_t>(std::stoul(part));
        if (index > array->size()) return false;
        if (index == array->size())
            array->push_back(at + 1 < parts.size() && isIndex(parts[at + 1]) ? Value::array() : Value::object());
        return setAt((*array)[index], parts, at + 1, std::move(newValue));
    }
    return false;
}

} // namespace

ProjectDocument::ProjectDocument(std::string name)
{
    root_ = Value::object();
    root_.set("formatVersion", Value::integer(currentFormatVersion));
    root_.set("name", Value::string(std::move(name)));
    savedCanonical_ = serializeValue(root_);
}

ProjectDocument::ProjectDocument(Value root) : root_(std::move(root))
{
    savedCanonical_ = serializeValue(root_);
}

std::string ProjectDocument::name() const
{
    const auto* value = root_.get("name");
    return value != nullptr ? value->asString() : std::string{};
}

void ProjectDocument::setName(const std::string& name)
{
    root_.set("name", Value::string(name));
}

const Value* ProjectDocument::value(const std::string& path) const
{
    if (path.empty()) return nullptr;
    const auto parts = splitPath(path);
    for (const auto& part : parts) if (part.empty()) return nullptr;
    return getAt(root_, parts, 0);
}

bool ProjectDocument::setValue(const std::string& path, Value newValue)
{
    if (path.empty() || !root_.isObject()) return false;
    const auto parts = splitPath(path);
    for (const auto& part : parts) if (part.empty()) return false;
    return setAt(root_, parts, 0, std::move(newValue));
}

} // namespace nxf::project
