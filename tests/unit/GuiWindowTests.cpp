#include <nxf/gui/WindowFactory.h>
#include "TestSupport.h"
int main() {
    const auto backend = nxf::gui::windowBackendName();
    NXF_CHECK(!backend.empty());
#if defined(_WIN32)
    NXF_CHECK(backend == "win32");
#else
    NXF_CHECK(backend == "none");
    NXF_CHECK(nxf::gui::createWindow({}) == nullptr);   // honest: no backend, no fake window
#endif
    return nxf::test::report("GuiWindowTests");
}
