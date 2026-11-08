#pragma once

#include <nxf/graphics/GraphicsTypes.h>
#include <nxf/graphics/Image.h>
#include <nxf/graphics/Waveform.h>

#include <span>
#include <string_view>

namespace nxf::graphics {

/// The renderer contract every backend implements - the software
/// renderer in this module today, a GPU backend (Direct2D first)
/// behind this same interface tomorrow. Coordinates are floating
/// point pixels, origin top-left. All drawing is source-over alpha
/// blended and clipped to the target.
///
/// Text uses the module's built-in 5x7 bitmap font (ASCII, drawn
/// upper-case): deliberately simple and identical on every backend,
/// so measurements are exact. A full text engine (shaping, real
/// fonts) is a later graphics phase, not hidden inside this API.
class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual void fillRect(RectF rect, Color color) = 0;
    virtual void strokeRect(RectF rect, Color color, float thickness) = 0;
    virtual void drawLine(PointF from, PointF to, Color color, float thickness) = 0;
    virtual void fillCircle(PointF centre, float radius, Color color) = 0;
    virtual void drawText(std::string_view text, PointF position, int scale, Color color) = 0;
    virtual void drawImage(const Image& source, RectF sourceRect, RectF destinationRect) = 0;
    virtual void drawWaveform(std::span<const float> samples, RectF rect, Color color) = 0;
};

/// Width in pixels of text at a scale (glyphs are 5 px wide with a
/// 1 px gap, no trailing gap). Height at scale s is 7*s.
[[nodiscard]] int measureText(std::string_view text, int scale) noexcept;

} // namespace nxf::graphics
