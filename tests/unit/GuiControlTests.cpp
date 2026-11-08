#include <nxf/gui/Controls.h>
#include <nxf/gui/Component.h>
#include "TestSupport.h"
namespace {
nxf::gui::MouseEvent clickAt(int x, int y, nxf::gui::MouseEventType type) {
    nxf::gui::MouseEvent e; e.type = type; e.position = {x, y}; e.button = nxf::gui::MouseButton::Left; return e;
}
}
int main() {
    using namespace nxf::gui;
    Component root; root.setBounds({0, 0, 300, 200});
    auto buttonOwner = std::make_unique<Button>("OK"); auto* button = buttonOwner.get();
    button->setBounds({10, 10, 80, 30});
    int clicks = 0; button->onClick = [&] { ++clicks; };
    root.addChild(std::move(buttonOwner));
    root.dispatchMouseEvent(clickAt(20, 20, MouseEventType::Down));
    NXF_CHECK(clicks == 0);                                   // click fires on release
    root.dispatchMouseEvent(clickAt(20, 20, MouseEventType::Up));
    NXF_CHECK(clicks == 1);
    button->setEnabled(false);
    root.dispatchMouseEvent(clickAt(20, 20, MouseEventType::Down));
    root.dispatchMouseEvent(clickAt(20, 20, MouseEventType::Up));
    NXF_CHECK(clicks == 1);                                   // disabled: no click
    button->setEnabled(true);

    auto toggleOwner = std::make_unique<ToggleButton>("Mute"); auto* toggle = toggleOwner.get();
    toggle->setBounds({10, 60, 80, 30});
    root.addChild(std::move(toggleOwner));
    root.dispatchMouseEvent(clickAt(20, 70, MouseEventType::Down));
    NXF_CHECK(toggle->isOn());
    root.dispatchMouseEvent(clickAt(20, 70, MouseEventType::Down));
    NXF_CHECK(!toggle->isOn());

    auto sliderOwner = std::make_unique<Slider>(); auto* slider = sliderOwner.get();
    slider->setBounds({10, 110, 100, 20});
    slider->setRange(0.0, 10.0); slider->setValue(0.0);
    root.addChild(std::move(sliderOwner));
    root.dispatchMouseEvent(clickAt(60, 115, MouseEventType::Down));   // middle of the track
    NXF_CHECK(slider->value() > 4.5 && slider->value() < 5.5);
    slider->setValue(99.0);
    NXF_CHECK(slider->value() == 10.0);                                // clamped
    slider->setValue(-5.0);
    NXF_CHECK(slider->value() == 0.0);

    root.dispatchMouseEvent(clickAt(20, 20, MouseEventType::Down));
    NXF_CHECK(root.focusedComponent() == button);                      // click focuses a focusable control
    KeyEvent key; key.code = static_cast<std::uint32_t>('A'); key.character = 'A';
    NXF_CHECK(!root.dispatchKeyEvent(key));                            // button does not consume plain keys
    return nxf::test::report("GuiControlTests");
}
