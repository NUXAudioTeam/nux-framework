#pragma once

#include <nxf/gui/Window.h>

#include <memory>
#include <string_view>

namespace nxf::gui {

/// "win32" on Windows, "none" where no backend exists yet.
[[nodiscard]] std::string_view windowBackendName() noexcept;

/// Creates a window, or nullptr where there is no backend.
/// A null result is honest, never a fake window.
[[nodiscard]] std::unique_ptr<IWindow> createWindow(const WindowConfig& config);

} // namespace nxf::gui
