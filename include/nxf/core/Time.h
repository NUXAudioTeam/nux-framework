#pragma once

#include <cstdint>

namespace nxf::core {

/// Monotonic time in nanoseconds (std::chrono::steady_clock based):
/// suitable for measuring intervals, never for wall-clock dates.
/// Thread-safe; may be called from any non-real-time thread freely,
/// and is cheap enough for profiling hooks around - not inside -
/// an audio block.
[[nodiscard]] std::uint64_t monotonicNanoseconds() noexcept;

[[nodiscard]] constexpr double nanosecondsToSeconds(std::uint64_t ns) noexcept {
    return static_cast<double>(ns) / 1'000'000'000.0;
}
/// Precondition: seconds >= 0.
[[nodiscard]] constexpr std::uint64_t secondsToNanoseconds(double seconds) noexcept {
    return static_cast<std::uint64_t>(seconds * 1'000'000'000.0);
}

} // namespace nxf::core
