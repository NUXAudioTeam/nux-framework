#include <nxf/audio/AudioBackendFactory.h>
#include "TestSupport.h"
int main() {
    auto backend = nxf::audio::createSystemBackend();
    NXF_CHECK(backend != nullptr);
    NXF_CHECK(!backend->name().empty());
#if !defined(_WIN32)
    NXF_CHECK(backend->name() == "offline");   // no WASAPI off Windows: honest fallback
#endif
    const auto devices = backend->enumerateDevices();   // must never crash, device or not
    NXF_CHECK(devices.hasValue() || !devices.error().message().empty());
    return nxf::test::report("AudioBackendTests");
}
