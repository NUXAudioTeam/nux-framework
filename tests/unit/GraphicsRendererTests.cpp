#include <nxf/graphics/SoftwareRenderer.h>
#include "TestSupport.h"
#include <cmath>
namespace {
bool eq(nxf::graphics::Color a, nxf::graphics::Color b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue && a.alpha == b.alpha;
}
}
int main() {
    using namespace nxf::graphics;
    Image image(20, 20);
    NXF_CHECK(eq(image.pixel(5, 5), Color::transparent()));
    SoftwareRenderer renderer(image);
    renderer.fillRect({2, 3, 10, 5}, Color::fromRgb(255, 0, 0));
    NXF_CHECK(eq(image.pixel(2, 3), Color::fromRgb(255, 0, 0)));
    NXF_CHECK(eq(image.pixel(11, 7), Color::fromRgb(255, 0, 0)));
    NXF_CHECK(eq(image.pixel(12, 7), Color::transparent()));
    NXF_CHECK(eq(image.pixel(2, 8), Color::transparent()));
    // 50% white over opaque black = mid grey
    Image blend(4, 4); blend.fill(Color::fromRgb(0, 0, 0));
    SoftwareRenderer blendRenderer(blend);
    blendRenderer.fillRect({0, 0, 4, 4}, Color{255, 255, 255, 128});
    const auto mid = blend.pixel(1, 1);
    NXF_CHECK(mid.red >= 126 && mid.red <= 129);
    NXF_CHECK(mid.alpha == 255);
    // diagonal line reaches both ends
    Image lineImage(16, 16); SoftwareRenderer lineRenderer(lineImage);
    lineRenderer.drawLine({0, 0}, {15, 15}, Color::fromRgb(0, 255, 0), 1.0f);
    NXF_CHECK(eq(lineImage.pixel(0, 0), Color::fromRgb(0, 255, 0)));
    NXF_CHECK(eq(lineImage.pixel(15, 15), Color::fromRgb(0, 255, 0)));
    NXF_CHECK(eq(lineImage.pixel(0, 15), Color::transparent()));
    // circle: centre filled, corner empty
    Image circleImage(21, 21); SoftwareRenderer circleRenderer(circleImage);
    circleRenderer.fillCircle({10, 10}, 5.0f, Color::fromRgb(0, 0, 255));
    NXF_CHECK(eq(circleImage.pixel(10, 10), Color::fromRgb(0, 0, 255)));
    NXF_CHECK(eq(circleImage.pixel(0, 0), Color::transparent()));
    // stroked rect: border only
    Image rectImage(10, 10); SoftwareRenderer rectRenderer(rectImage);
    rectRenderer.strokeRect({1, 1, 8, 8}, Color::fromRgb(255, 255, 0), 1.0f);
    NXF_CHECK(eq(rectImage.pixel(1, 1), Color::fromRgb(255, 255, 0)));
    NXF_CHECK(eq(rectImage.pixel(5, 5), Color::transparent()));
    return nxf::test::report("GraphicsRendererTests");
}
