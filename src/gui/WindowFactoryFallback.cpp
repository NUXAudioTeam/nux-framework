// No native window backend on this platform yet.
#include <nxf/gui/WindowFactory.h>

namespace nxf::gui {

std::string_view windowBackendName() noexcept { return "none"; }

std::unique_ptr<IWindow> createWindow(const WindowConfig&) { return nullptr; }

} // namespace nxf::gui
