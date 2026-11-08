#pragma once

#include <string_view>

namespace nxf::core {

/// Framework major version. Kept in step with the CMake project version;
/// VersionTests fail if the two drift apart.
/// Thread-safety: all functions in this header are pure and thread-safe.
[[nodiscard]] constexpr int versionMajor() noexcept { return 0; }
[[nodiscard]] constexpr int versionMinor() noexcept { return 1; }
[[nodiscard]] constexpr int versionPatch() noexcept { return 0; }

/// Human-readable framework name, e.g. for an application's about box.
[[nodiscard]] std::string_view frameworkName() noexcept;

/// Dotted version string, e.g. "0.1.0".
[[nodiscard]] std::string_view versionString() noexcept;

} // namespace nxf::core
