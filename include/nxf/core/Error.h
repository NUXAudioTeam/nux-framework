#pragma once

#include <string_view>

namespace nxf::core {

/// Machine-readable failure categories for NXF APIs.
/// Expected failures are reported as values (Error / Result), never as
/// exceptions across a module boundary - see docs/API_DESIGN.md.
enum class ErrorCode {
    None = 0,
    InvalidArgument,
    OutOfRange,
    OutOfMemory,
    Unavailable,
    Internal
};

/// A failure value: a code plus a human-readable message.
/// The message is a non-owning view; pass string literals or strings
/// that outlive the Error. An Error with code None means success.
/// Thread-safety: immutable after construction, safe to share.
class Error {
public:
    constexpr Error() = default;
    constexpr Error(ErrorCode code, std::string_view message) : code_(code), message_(message) {}

    [[nodiscard]] static constexpr Error success() { return {}; }
    [[nodiscard]] static constexpr Error invalidArgument(std::string_view m) { return {ErrorCode::InvalidArgument, m}; }
    [[nodiscard]] static constexpr Error outOfRange(std::string_view m) { return {ErrorCode::OutOfRange, m}; }
    [[nodiscard]] static constexpr Error outOfMemory(std::string_view m) { return {ErrorCode::OutOfMemory, m}; }
    [[nodiscard]] static constexpr Error unavailable(std::string_view m) { return {ErrorCode::Unavailable, m}; }
    [[nodiscard]] static constexpr Error internal(std::string_view m) { return {ErrorCode::Internal, m}; }

    [[nodiscard]] constexpr ErrorCode code() const noexcept { return code_; }
    [[nodiscard]] constexpr std::string_view message() const noexcept { return message_; }
    [[nodiscard]] constexpr bool isSuccess() const noexcept { return code_ == ErrorCode::None; }
    explicit constexpr operator bool() const noexcept { return isSuccess(); }

private:
    ErrorCode code_ = ErrorCode::None;
    std::string_view message_{};
};

} // namespace nxf::core
