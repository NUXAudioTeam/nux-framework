#include <nxf/graphics/SoftwareRenderer.h>
#include <nxf/graphics/Waveform.h>
#include "TestSupport.h"
#include <cmath>
#include <vector>
int main() {
    using namespace nxf::graphics;
    NXF_CHECK(measureText("HELLO", 1) == 29);        // 5 glyphs * 6 px, no trailing gap
    NXF_CHECK(measureText("", 1) == 0);
    NXF_CHECK(measureText("AB", 2) == 22);
    Image text(40, 12); SoftwareRenderer textRenderer(text);
    textRenderer.drawText("A1", {1, 1}, 1, Color::fromRgb(255, 255, 255));
    int lit = 0;
    for (int y = 0; y < text.height(); ++y) for (int x = 0; x < text.width(); ++x)
        if (text.pixel(x, y).alpha > 0) ++lit;
    NXF_CHECK(lit > 10);                              // glyphs really drew pixels

    Image source(2, 2); source.fill(Color::fromRgb(10, 20, 30));
    Image dest(8, 8); SoftwareRenderer destRenderer(dest);
    destRenderer.drawImage(source, {0, 0, 2, 2}, {2, 2, 4, 4});
    NXF_CHECK(dest.pixel(2, 2).red == 10);
    NXF_CHECK(dest.pixel(5, 5).blue == 30);
    NXF_CHECK(dest.pixel(0, 0).alpha == 0);

    const std::vector<float> samples{0.0f, 0.5f, -0.5f, 1.0f, -1.0f, 0.25f};
    const auto peaks = computeWaveformPeaks(samples, 3);
    NXF_CHECK(peaks.size() == 3);
    // 6 samples over 3 columns: [0, .5] [-.5, 1] [-1, .25]
    NXF_CHECK(peaks[0].minimum == 0.0f);  NXF_CHECK(peaks[0].maximum == 0.5f);
    NXF_CHECK(peaks[1].minimum == -0.5f); NXF_CHECK(peaks[1].maximum == 1.0f);
    NXF_CHECK(peaks[2].minimum == -1.0f); NXF_CHECK(peaks[2].maximum == 0.25f);
    Image wave(30, 20); SoftwareRenderer waveRenderer(wave);
    waveRenderer.drawWaveform(samples, {0, 0, 30, 20}, Color::fromRgb(0, 255, 255));
    bool columnDrawn = false;
    for (int y = 0; y < wave.height(); ++y) columnDrawn = columnDrawn || wave.pixel(15, y).alpha > 0;
    NXF_CHECK(columnDrawn);
    return nxf::test::report("GraphicsTextImageTests");
}
