#include <nxf/gui/Geometry.h>
#include "TestSupport.h"
int main() {
    using namespace nxf::gui;
    const Rectangle r{10, 20, 100, 50};
    NXF_CHECK(r.right() == 110); NXF_CHECK(r.bottom() == 70);
    NXF_CHECK(r.contains(Point{10, 20}));
    NXF_CHECK(r.contains(Point{109, 69}));
    NXF_CHECK(!r.contains(Point{110, 70}));
    NXF_CHECK(!r.contains(Point{9, 30}));
    NXF_CHECK(r.translated(5, -5) == Rectangle({15, 15, 100, 50}));
    NXF_CHECK(Rectangle({0, 0, 10, 10}).intersects(Rectangle({5, 5, 10, 10})));
    NXF_CHECK(!Rectangle({0, 0, 10, 10}).intersects(Rectangle({10, 0, 10, 10})));
    NXF_CHECK(Rectangle({0, 0, 10, 10}).intersection(Rectangle({5, 5, 10, 10})) == Rectangle({5, 5, 5, 5}));
    NXF_CHECK(Rectangle({0, 0, 4, 4}).expanded(2) == Rectangle({-2, -2, 8, 8}));
    return nxf::test::report("GuiGeometryTests");
}
