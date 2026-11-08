#include <nxf/core/Time.h>

#include <chrono>

namespace nxf::core {

std::uint64_t monotonicNanoseconds() noexcept
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
}

} // namespace nxf::core
