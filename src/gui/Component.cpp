#include <nxf/gui/Component.h>

#include <algorithm>

namespace nxf::gui {

Component::~Component()
{
    for (auto& child : children_) child->parent_ = nullptr;
}

void Component::setBounds(Rectangle bounds)
{
    bounds_ = bounds;
}

Rectangle Component::globalBounds() const
{
    Rectangle result = bounds_;
    for (const Component* p = parent_; p != nullptr; p = p->parent_) {
        result.x += p->bounds_.x;
        result.y += p->bounds_.y;
    }
    return result;
}

Component* Component::addChild(std::unique_ptr<Component> child)
{
    if (child == nullptr) return nullptr;
    child->parent_ = this;
    Component* raw = child.get();
    children_.push_back(std::move(child));
    return raw;
}

bool Component::removeChild(Component* child)
{
    const auto it = std::find_if(children_.begin(), children_.end(),
                                 [child](const std::unique_ptr<Component>& c) { return c.get() == child; });
    if (it == children_.end()) return false;
    (*it)->parent_ = nullptr;
    children_.erase(it);
    return true;
}

Component* Component::root()
{
    Component* node = this;
    while (node->parent_ != nullptr) node = node->parent_;
    return node;
}

Component* Component::hitTest(Point rootPoint)
{
    if (!visible_ || !enabled_ || !globalBounds().contains(rootPoint)) return nullptr;
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if (Component* hit = (*it)->hitTest(rootPoint)) return hit;
    }
    return this;
}

void Component::requestFocus()
{
    if (focusable_ && enabled_ && visible_) root()->focused_ = this;
}

bool Component::dispatchMouseEvent(const MouseEvent& event)
{
    Component* target = hitTest(event.position);
    if (target == nullptr) return false;
    if (event.type == MouseEventType::Down && target->focusable_) target->requestFocus();

    for (Component* node = target; node != nullptr; node = node->parent_) {
        MouseEvent local = event;
        local.localPosition = node->toLocal(event.position);
        bool handled = false;
        switch (event.type) {
            case MouseEventType::Down: handled = node->onMouseDown(local); break;
            case MouseEventType::Up: handled = node->onMouseUp(local); break;
            case MouseEventType::Move: handled = node->onMouseMove(local); break;
            case MouseEventType::Wheel: handled = node->onMouseWheel(local); break;
        }
        if (handled) return true;
    }
    return false;
}

bool Component::dispatchKeyEvent(const KeyEvent& event)
{
    Component* target = focused_ != nullptr ? focused_ : this;
    for (Component* node = target; node != nullptr; node = node->parent_)
        if (node->onKeyEvent(event)) return true;
    return false;
}

} // namespace nxf::gui
