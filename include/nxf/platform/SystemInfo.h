#pragma once

#include <cstdint>
#include <string>

namespace nxf::platform {

/// A snapshot of the machine NXF is running on, for about boxes,
/// diagnostics and bug reports. Values come from the platform
/// backend (Windows API / POSIX), never guessed.
struct SystemInfo {
    std::string operatingSystem;          // "Windows", "Linux", "macOS", ...
    std::string architecture;             // "x86_64", "arm64", ...
    std::uint32_t hardwareThreads = 0;    // logical processors reported by the OS
    std::uint64_t totalPhysicalMemoryBytes = 0;
};

/// Any thread. The result is a fresh snapshot on every call.
[[nodiscard]] SystemInfo getSystemInfo();

} // namespace nxf::platform
