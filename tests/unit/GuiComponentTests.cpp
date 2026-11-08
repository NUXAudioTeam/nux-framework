#include <nxf/gui/Component.h>
#include "TestSupport.h"
namespace {
struct Probe final : nxf::gui::Component {
    int downs = 0; int ups = 0; bool handleDown = true;
    nxf::gui::Point lastLocal{-1, -1};
    bool onMouseDown(const nxf::gui::MouseEvent& e) override { ++downs; lastLocal = e.localPosition; return handleDown; }
    bool onMouseUp(const nxf::gui::MouseEvent& e) override { ++ups; lastLocal = e.localPosition; return true; }
};
}
int main() {
    using namespace nxf::gui;
    Component root; root.setBounds({0, 0, 400, 300});
    auto childOwner = std::make_unique<Probe>(); auto* child = childOwner.get();
    child->setBounds({50, 40, 100, 80});
    root.addChild(std::move(childOwner));
    auto grandOwner = std::make_unique<Probe>(); auto* grand = grandOwner.get();
    grand->setBounds({10, 10, 30, 30});
    child->addChild(std::move(grandOwner));

    NXF_CHECK(root.hitTest({65, 55}) == grand);        // deepest child wins
    NXF_CHECK(root.hitTest({55, 45}) == child);
    NXF_CHECK(root.hitTest({5, 5}) == &root);
    NXF_CHECK(root.hitTest({999, 999}) == nullptr);
    NXF_CHECK(grand->globalBounds() == Rectangle({60, 50, 30, 30}));

    MouseEvent down; down.type = MouseEventType::Down; down.position = {65, 55}; down.button = MouseButton::Left;
    NXF_CHECK(root.dispatchMouseEvent(down));
    NXF_CHECK(grand->downs == 1);
    NXF_CHECK(grand->lastLocal == Point({5, 5}));      // converted to target-local coords
    NXF_CHECK(child->downs == 0);                      // handled: no bubbling
    grand->handleDown = false; child->handleDown = false;
    NXF_CHECK(!root.dispatchMouseEvent(down));         // nobody handled it
    NXF_CHECK(grand->downs == 2); NXF_CHECK(child->downs == 1);   // but it bubbled to the parent
    grand->setVisible(false);
    NXF_CHECK(root.hitTest({65, 55}) == child);        // invisible: not hit
    grand->setVisible(true); child->setEnabled(false);
    NXF_CHECK(root.hitTest({65, 55}) == &root);        // disabled subtree: not hit
    child->setEnabled(true);
    NXF_CHECK(child->removeChild(grand) == true);
    NXF_CHECK(child->children().empty());
    return nxf::test::report("GuiComponentTests");
}
