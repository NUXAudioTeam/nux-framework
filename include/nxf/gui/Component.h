#pragma once

#include <nxf/gui/Events.h>
#include <nxf/gui/Geometry.h>

#include <memory>
#include <string>
#include <vector>

namespace nxf::gui {

/// One element of the GUI tree. A component owns its children, has
/// bounds relative to its parent, and receives mouse events that
/// the root dispatches: the deepest visible, enabled component
/// under the pointer is offered the event first (in its own local
/// coordinates); if it does not handle it, the event bubbles up.
///
/// All GUI work happens on the UI thread. Painting itself belongs
/// to the graphics layer (Phase 8): components will draw through a
/// renderer interface there, so this class deliberately has no
/// paint API yet - inventing one before the renderer exists would
/// be the wrong contract.
class Component {
public:
    Component() = default;
    virtual ~Component();
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    void setName(std::string name) { name_ = std::move(name); }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    void setBounds(Rectangle bounds);
    [[nodiscard]] Rectangle bounds() const noexcept { return bounds_; }
    [[nodiscard]] Rectangle globalBounds() const;
    void setPreferredSize(Size size) noexcept { preferredSize_ = size; }
    [[nodiscard]] Size preferredSize() const noexcept { return preferredSize_; }

    void setVisible(bool visible) noexcept { visible_ = visible; }
    [[nodiscard]] bool isVisible() const noexcept { return visible_; }
    void setEnabled(bool enabled) noexcept { enabled_ = enabled; }
    [[nodiscard]] bool isEnabled() const noexcept { return enabled_; }
    void setFocusable(bool focusable) noexcept { focusable_ = focusable; }
    [[nodiscard]] bool isFocusable() const noexcept { return focusable_; }

    /// Takes ownership; returns the child for convenient setup.
    Component* addChild(std::unique_ptr<Component> child);
    /// Removes and destroys a direct child. False if not a child.
    bool removeChild(Component* child);
    [[nodiscard]] const std::vector<std::unique_ptr<Component>>& children() const noexcept { return children_; }
    [[nodiscard]] Component* parent() const noexcept { return parent_; }

    /// Deepest hittable component at a root-space point (or this /
    /// nullptr). Invisible or disabled subtrees are never hit.
    [[nodiscard]] Component* hitTest(Point rootPoint);
    /// Offers the event to the hit component, bubbling on refusal.
    /// Also moves keyboard focus to a focusable hit component.
    bool dispatchMouseEvent(const MouseEvent& event);
    /// Delivers a key event to the focused component (root calls).
    bool dispatchKeyEvent(const KeyEvent& event);
    [[nodiscard]] Component* focusedComponent() const noexcept { return focused_; }
    void requestFocus();
    void clearFocus() noexcept { focused_ = nullptr; }

    virtual bool onMouseDown(const MouseEvent&) { return false; }
    virtual bool onMouseUp(const MouseEvent&) { return false; }
    virtual bool onMouseMove(const MouseEvent&) { return false; }
    virtual bool onMouseWheel(const MouseEvent&) { return false; }
    virtual bool onKeyEvent(const KeyEvent&) { return false; }

private:
    [[nodiscard]] Component* root();
    [[nodiscard]] Point toLocal(Point rootPoint) const { return rootPoint - globalBounds().origin(); }

    std::string name_;
    Rectangle bounds_{};
    Size preferredSize_{};
    bool visible_ = true;
    bool enabled_ = true;
    bool focusable_ = false;
    Component* parent_ = nullptr;
    std::vector<std::unique_ptr<Component>> children_;
    Component* focused_ = nullptr;   // only meaningful on the root
};

} // namespace nxf::gui
