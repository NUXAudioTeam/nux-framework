#include <nxf/platform/Timing.h>

#include <chrono>
#include <thread>

namespace nxf::platform {

void sleepMilliseconds(std::uint32_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds{milliseconds});
}

void yieldThread() noexcept
{
    std::this_thread::yield();
}

} // namespace nxf::platform
