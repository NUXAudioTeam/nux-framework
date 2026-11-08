#include <nxf/platform/SystemInfo.h>
#include "TestSupport.h"
int main() {
    using namespace nxf::platform;
    const SystemInfo info = getSystemInfo();
    NXF_CHECK(!info.operatingSystem.empty());
#if defined(_WIN32)
    NXF_CHECK(info.operatingSystem == "Windows");
#elif defined(__linux__)
    NXF_CHECK(info.operatingSystem == "Linux");
#endif
    NXF_CHECK(!info.architecture.empty());
    NXF_CHECK(info.hardwareThreads >= 1);
    NXF_CHECK(info.totalPhysicalMemoryBytes > 0);
    return nxf::test::report("PlatformSystemInfoTests");
}
