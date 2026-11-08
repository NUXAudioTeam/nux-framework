#pragma once

#include <nxf/graphics/Renderer.h>

namespace nxf::graphics {

/// The reference renderer: draws into a CPU Image with exact,
/// testable pixels. It is the conformance target every future GPU
/// backend must match, and it is fast enough for UI prototyping and
/// headless rendering/screenshots.
class SoftwareRenderer final : public IRenderer {
public:
    explicit SoftwareRenderer(Image& target) : target_(target) {}

    void fillRect(RectF rect, Color color) override;
    void strokeRect(RectF rect, Color color, float thickness) override;
    void drawLine(PointF from, PointF to, Color color, float thickness) override;
    void fillCircle(PointF centre, float radius, Color color) override;
    void drawText(std::string_view text, PointF position, int scale, Color color) override;
    void drawImage(const Image& source, RectF sourceRect, RectF destinationRect) override;
    void drawWaveform(std::span<const float> samples, RectF rect, Color color) override;

private:
    void blendPixel(int x, int y, Color color) noexcept;
    Image& target_;
};

} // namespace nxf::graphics
