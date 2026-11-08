#pragma once

#include <nxf/gui/Component.h>

#include <functional>
#include <string>

namespace nxf::gui {

struct WindowConfig {
    std::string title = "NXF";
    int width = 960;
    int height = 600;
};

/// A top-level window hosting a root Component. Windows are UI
/// thread objects. The backend creates the native window and
/// forwards native mouse/key/resize events into the component
/// tree; actual painting arrives with the graphics layer
/// (Phase 8) - the paint callback is the seam it will use.
class IWindow {
public:
    virtual ~IWindow() = default;
    virtual void show() = 0;
    virtual void hide() = 0;
    [[nodiscard]] virtual bool isVisible() const = 0;
    virtual void setTitle(const std::string& title) = 0;
    virtual void setRootComponent(Component* root) = 0;   // not owned
    /// Blocks in the platform message loop until the window closes.
    virtual void runEventLoop() = 0;
    /// Called when the window needs repainting (Phase 8 seam).
    std::function<void()> onPaint;
};

} // namespace nxf::gui
