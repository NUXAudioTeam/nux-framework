#pragma once

#include <nxf/gui/Geometry.h>

#include <cstdint>

namespace nxf::gui {

enum class MouseEventType { Down, Up, Move, Wheel };
enum class MouseButton { None, Left, Middle, Right };

/// A mouse event. position is in root (window) coordinates;
/// localPosition is rewritten by the dispatcher for every component
/// the event is delivered to, so handlers always work locally.
struct MouseEvent {
    MouseEventType type = MouseEventType::Move;
    Point position{};
    Point localPosition{};
    MouseButton button = MouseButton::None;
    int wheelDelta = 0;
    bool shiftDown = false;
    bool controlDown = false;
    bool altDown = false;
};

/// A key event. code is a platform-neutral virtual code for special
/// keys (use the host's code for now; a full key map is Phase 8+
/// work) and character is the typed character, or 0 if none.
struct KeyEvent {
    std::uint32_t code = 0;
    char32_t character = 0;
    bool shiftDown = false;
    bool controlDown = false;
    bool altDown = false;
};

} // namespace nxf::gui
