#pragma once

#include <nxf/core/Error.h>

#include <optional>
#include <utility>

namespace nxf::core {

/// The result of an operation that either produces a value or fails
/// with an Error. Construct with Result::success(v) / ::failure(e).
/// Not for use inside an audio callback (it may own a heap value,
/// exactly like the value it wraps); the callback rules are in
/// docs/REALTIME_AUDIO_RULES.md. value()/error() have the precondition
/// hasValue() / !hasValue() respectively.
template <typename T>
class [[nodiscard]] Result {
public:
    [[nodiscard]] static Result success(T value) { return Result{std::move(value)}; }
    [[nodiscard]] static Result failure(Error error) { return Result{error}; }

    [[nodiscard]] bool hasValue() const noexcept { return value_.has_value(); }
    [[nodiscard]] const T& value() const { return value_.value(); }
    [[nodiscard]] T& value() { return value_.value(); }
    [[nodiscard]] const Error& error() const noexcept { return error_; }

private:
    explicit Result(T value) : value_(std::move(value)) {}
    explicit Result(Error error) : error_(error) {}
    std::optional<T> value_;
    Error error_{};
};

} // namespace nxf::core
