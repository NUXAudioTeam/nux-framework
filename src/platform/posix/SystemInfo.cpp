// POSIX backend for nxf_platform system information (Linux build
// verification; macOS shares this path). OS types stay in this file.
#include <nxf/platform/SystemInfo.h>

#include <sys/utsname.h>
#include <unistd.h>

namespace nxf::platform {

SystemInfo getSystemInfo()
{
    SystemInfo info;
    utsname name{};
    if (uname(&name) == 0) {
        info.operatingSystem = name.sysname;
        info.architecture = name.machine;
    }
    const long threads = sysconf(_SC_NPROCESSORS_ONLN);
    info.hardwareThreads = threads > 0 ? static_cast<std::uint32_t>(threads) : 1;

    const long pages = sysconf(_SC_PHYS_PAGES);
    const long pageSize = sysconf(_SC_PAGE_SIZE);
    if (pages > 0 && pageSize > 0)
        info.totalPhysicalMemoryBytes = static_cast<std::uint64_t>(pages) * static_cast<std::uint64_t>(pageSize);
    return info;
}

} // namespace nxf::platform
