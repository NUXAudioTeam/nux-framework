#include <nxf/gui/Layout.h>
#include "TestSupport.h"
int main() {
    using namespace nxf::gui;
    Component root; root.setBounds({0, 0, 200, 100});
    Component* a = root.addChild(std::make_unique<Component>());
    Component* b = root.addChild(std::make_unique<Component>());
    a->setPreferredSize({50, 20}); b->setPreferredSize({50, 30});
    layoutVertical(root, Insets{5, 5, 5, 5}, 10);
    NXF_CHECK(a->bounds() == Rectangle({5, 5, 190, 20}));
    NXF_CHECK(b->bounds() == Rectangle({5, 35, 190, 30}));
    layoutHorizontal(root, Insets{0, 0, 0, 0}, 0);
    NXF_CHECK(a->bounds() == Rectangle({0, 0, 50, 100}));
    NXF_CHECK(b->bounds() == Rectangle({50, 0, 50, 100}));   // preferred width is 50
    Component grid; grid.setBounds({0, 0, 100, 100});
    Component* cells[4];
    for (auto*& c : cells) c = grid.addChild(std::make_unique<Component>());
    layoutGrid(grid, 2, Insets{0, 0, 0, 0}, 0);
    NXF_CHECK(cells[0]->bounds() == Rectangle({0, 0, 50, 50}));
    NXF_CHECK(cells[1]->bounds() == Rectangle({50, 0, 50, 50}));
    NXF_CHECK(cells[2]->bounds() == Rectangle({0, 50, 50, 50}));
    NXF_CHECK(cells[3]->bounds() == Rectangle({50, 50, 50, 50}));
    return nxf::test::report("GuiLayoutTests");
}
