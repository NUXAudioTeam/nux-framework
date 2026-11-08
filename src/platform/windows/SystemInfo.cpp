// Windows backend for nxf_platform system information.
// Windows SDK types stay inside this file; the public header is pure NXF.
#include <nxf/platform/SystemInfo.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace nxf::platform {

SystemInfo getSystemInfo()
{
    SystemInfo info;
    info.operatingSystem = "Windows";

    SYSTEM_INFO native{};
    GetNativeSystemInfo(&native);
    switch (native.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: info.architecture = "x86_64"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: info.architecture = "arm64"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: info.architecture = "x86"; break;
        case PROCESSOR_ARCHITECTURE_ARM: info.architecture = "arm"; break;
        default: info.architecture = "unknown"; break;
    }
    info.hardwareThreads = native.dwNumberOfProcessors;

    MEMORYSTATUSEX memory{};
    memory.dwLength = sizeof(memory);
    if (GlobalMemoryStatusEx(&memory) != 0)
        info.totalPhysicalMemoryBytes = static_cast<std::uint64_t>(memory.ullTotalPhys);
    return info;
}

} // namespace nxf::platform
