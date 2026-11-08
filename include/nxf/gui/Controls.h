#pragma once

#include <nxf/gui/Component.h>

#include <functional>
#include <string>

namespace nxf::gui {

/// Non-interactive text display.
class Label final : public Component {
public:
    explicit Label(std::string text = {}) : text_(std::move(text)) {}
    void setText(std::string text) { text_ = std::move(text); }
    [[nodiscard]] const std::string& text() const noexcept { return text_; }
private:
    std::string text_;
};

/// A push button. The click fires on mouse-up inside the button
/// after a press on it (the usual platform behaviour), and never
/// while disabled (a disabled subtree is not even hit-tested).
class Button : public Component {
public:
    explicit Button(std::string text = {}) : text_(std::move(text)) { setFocusable(true); }
    void setText(std::string text) { text_ = std::move(text); }
    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    [[nodiscard]] bool isPressed() const noexcept { return pressed_; }
    void triggerClick() { if (isEnabled() && onClick) onClick(); }

    std::function<void()> onClick;

    bool onMouseDown(const MouseEvent&) override { pressed_ = true; return true; }
    bool onMouseUp(const MouseEvent& event) override {
        const bool wasPressed = pressed_;
        pressed_ = false;
        if (wasPressed && bounds().contains(event.localPosition + bounds().origin())) {
            if (onClick) onClick();
            return true;
        }
        return wasPressed;
    }

private:
    std::string text_;
    bool pressed_ = false;
};

/// A two-state button. Toggles on mouse-down.
class ToggleButton final : public Button {
public:
    explicit ToggleButton(std::string text = {}) : Button(std::move(text)) {}
    [[nodiscard]] bool isOn() const noexcept { return on_; }
    void setOn(bool on) { on_ = on; }
    std::function<void(bool)> onToggle;
    bool onMouseDown(const MouseEvent& event) override {
        on_ = !on_;
        if (onToggle) onToggle(on_);
        return Button::onMouseDown(event);
    }
private:
    bool on_ = false;
};

/// A horizontal value slider. Dragging (down or move while pressed)
/// maps the local x position onto [minimum, maximum]; setValue
/// clamps. Value changes are reported through onValueChanged.
class Slider final : public Component {
public:
    Slider() { setFocusable(true); }
    void setRange(double minimum, double maximum) {
        minimum_ = minimum;
        maximum_ = maximum < minimum ? minimum : maximum;
        setValue(value_);
    }
    void setValue(double value) {
        const double clamped = value < minimum_ ? minimum_ : (value > maximum_ ? maximum_ : value);
        if (clamped != value_) {
            value_ = clamped;
            if (onValueChanged) onValueChanged(value_);
        }
    }
    [[nodiscard]] double value() const noexcept { return value_; }
    [[nodiscard]] double minimum() const noexcept { return minimum_; }
    [[nodiscard]] double maximum() const noexcept { return maximum_; }

    std::function<void(double)> onValueChanged;

    bool onMouseDown(const MouseEvent& event) override {
        dragging_ = true;
        applyPosition(event.localPosition.x);
        return true;
    }
    bool onMouseMove(const MouseEvent& event) override {
        if (!dragging_) return false;
        applyPosition(event.localPosition.x);
        return true;
    }
    bool onMouseUp(const MouseEvent&) override {
        const bool was = dragging_;
        dragging_ = false;
        return was;
    }

private:
    void applyPosition(int localX) {
        const int width = bounds().width;
        if (width <= 0) return;
        double fraction = static_cast<double>(localX) / static_cast<double>(width);
        fraction = fraction < 0.0 ? 0.0 : (fraction > 1.0 ? 1.0 : fraction);
        setValue(minimum_ + fraction * (maximum_ - minimum_));
    }
    double minimum_ = 0.0;
    double maximum_ = 1.0;
    double value_ = 0.0;
    bool dragging_ = false;
};

} // namespace nxf::gui
