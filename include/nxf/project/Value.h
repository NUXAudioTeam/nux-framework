#pragma once

#include <nxf/core/Result.h>

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace nxf::project {

/// A dynamically-typed state value: the unit projects are made of.
/// Types are null, bool, int64, double, string, array and object
/// (string keys, kept sorted). Accessors never throw: asking for the
/// wrong type yields an empty/zero value, and the type predicates
/// are how you check first.
class Value {
public:
    using Array = std::vector<Value>;
    using Object = std::map<std::string, Value>;

    Value() = default;
    [[nodiscard]] static Value null() { return {}; }
    [[nodiscard]] static Value boolean(bool v) { Value x; x.data_ = v; return x; }
    [[nodiscard]] static Value integer(std::int64_t v) { Value x; x.data_ = v; return x; }
    [[nodiscard]] static Value real(double v) { Value x; x.data_ = v; return x; }
    [[nodiscard]] static Value string(std::string v) { Value x; x.data_ = std::move(v); return x; }
    [[nodiscard]] static Value array(Array v = {}) { Value x; x.data_ = std::move(v); return x; }
    [[nodiscard]] static Value object(Object v = {}) { Value x; x.data_ = std::move(v); return x; }

    [[nodiscard]] bool isNull() const noexcept { return std::holds_alternative<std::monostate>(data_); }
    [[nodiscard]] bool isBool() const noexcept { return std::holds_alternative<bool>(data_); }
    [[nodiscard]] bool isInt() const noexcept { return std::holds_alternative<std::int64_t>(data_); }
    [[nodiscard]] bool isReal() const noexcept { return std::holds_alternative<double>(data_); }
    [[nodiscard]] bool isString() const noexcept { return std::holds_alternative<std::string>(data_); }
    [[nodiscard]] bool isArray() const noexcept { return std::holds_alternative<Array>(data_); }
    [[nodiscard]] bool isObject() const noexcept { return std::holds_alternative<Object>(data_); }

    [[nodiscard]] bool asBool() const noexcept { return isBool() ? std::get<bool>(data_) : false; }
    [[nodiscard]] std::int64_t asInt() const noexcept { return isInt() ? std::get<std::int64_t>(data_) : 0; }
    [[nodiscard]] double asReal() const noexcept {
        if (isReal()) return std::get<double>(data_);
        if (isInt()) return static_cast<double>(std::get<std::int64_t>(data_));
        return 0.0;
    }
    [[nodiscard]] const std::string& asString() const noexcept {
        static const std::string empty;
        return isString() ? std::get<std::string>(data_) : empty;
    }
    [[nodiscard]] const Array* asArray() const noexcept { return isArray() ? &std::get<Array>(data_) : nullptr; }
    [[nodiscard]] const Object* asObject() const noexcept { return isObject() ? &std::get<Object>(data_) : nullptr; }

    /// Object access. set() is a no-op on non-objects.
    void set(const std::string& key, Value value) {
        if (isObject()) std::get<Object>(data_)[key] = std::move(value);
    }
    [[nodiscard]] const Value* get(const std::string& key) const noexcept {
        if (!isObject()) return nullptr;
        const auto& object = std::get<Object>(data_);
        const auto it = object.find(key);
        return it == object.end() ? nullptr : &it->second;
    }
    [[nodiscard]] Value* get(const std::string& key) noexcept {
        if (!isObject()) return nullptr;
        auto& object = std::get<Object>(data_);
        const auto it = object.find(key);
        return it == object.end() ? nullptr : &it->second;
    }
    /// Array access: nullptr out of range (or not an array).
    [[nodiscard]] const Value* at(std::size_t index) const noexcept {
        if (!isArray()) return nullptr;
        const auto& array = std::get<Array>(data_);
        return index < array.size() ? &array[index] : nullptr;
    }

    bool operator==(const Value&) const = default;

private:
    std::variant<std::monostate, bool, std::int64_t, double, std::string, Array, Object> data_;
};

/// Serialises in the NXF structured-text format. The grammar is
/// deliberately small and ours (no JSON dependency):
///   value  := null | true | false | number | "string" | [v, ...] | { key = value; ... }
/// Object keys are emitted sorted (std::map order), so the output
/// is canonical: two equal values always serialise identically,
/// which is what dirty tracking and tests compare.
[[nodiscard]] std::string serializeValue(const Value& value);

/// Parses the format above. Trailing content, truncation and bad
/// escapes are failures (a Result), never partial values.
[[nodiscard]] nxf::core::Result<Value> parseValue(const std::string& text);

} // namespace nxf::project
