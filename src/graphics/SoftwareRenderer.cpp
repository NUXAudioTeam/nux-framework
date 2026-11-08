#include <nxf/graphics/SoftwareRenderer.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

namespace nxf::graphics {
namespace {

using Glyph = std::array<std::string_view, 7>;
// A compact hand-drawn 5x7 font. Lower-case is drawn as upper-case;
// anything unlisted draws blank (and still advances).
const Glyph& glyphFor(char raw)
{
    static constexpr Glyph blank{"     ", "     ", "     ", "     ", "     ", "     ", "     "};
    char c = raw;
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    switch (c) {
        case 'A': { static constexpr Glyph g{" ### ", "#   #", "#   #", "#####", "#   #", "#   #", "#   #"}; return g; }
        case 'B': { static constexpr Glyph g{"#### ", "#   #", "#   #", "#### ", "#   #", "#   #", "#### "}; return g; }
        case 'C': { static constexpr Glyph g{" ### ", "#   #", "#    ", "#    ", "#    ", "#   #", " ### "}; return g; }
        case 'D': { static constexpr Glyph g{"#### ", "#   #", "#   #", "#   #", "#   #", "#   #", "#### "}; return g; }
        case 'E': { static constexpr Glyph g{"#####", "#    ", "#    ", "#### ", "#    ", "#    ", "#####"}; return g; }
        case 'F': { static constexpr Glyph g{"#####", "#    ", "#    ", "#### ", "#    ", "#    ", "#    "}; return g; }
        case 'G': { static constexpr Glyph g{" ### ", "#   #", "#    ", "#  ##", "#   #", "#   #", " ### "}; return g; }
        case 'H': { static constexpr Glyph g{"#   #", "#   #", "#   #", "#####", "#   #", "#   #", "#   #"}; return g; }
        case 'I': { static constexpr Glyph g{" ### ", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", " ### "}; return g; }
        case 'J': { static constexpr Glyph g{"#####", "    #", "    #", "    #", "    #", "#   #", " ### "}; return g; }
        case 'K': { static constexpr Glyph g{"#   #", "#  # ", "# #  ", "##   ", "# #  ", "#  # ", "#   #"}; return g; }
        case 'L': { static constexpr Glyph g{"#    ", "#    ", "#    ", "#    ", "#    ", "#    ", "#####"}; return g; }
        case 'M': { static constexpr Glyph g{"#   #", "## ##", "# # #", "# # #", "#   #", "#   #", "#   #"}; return g; }
        case 'N': { static constexpr Glyph g{"#   #", "##  #", "# # #", "#  ##", "#   #", "#   #", "#   #"}; return g; }
        case 'O': { static constexpr Glyph g{" ### ", "#   #", "#   #", "#   #", "#   #", "#   #", " ### "}; return g; }
        case 'P': { static constexpr Glyph g{"#### ", "#   #", "#   #", "#### ", "#    ", "#    ", "#    "}; return g; }
        case 'Q': { static constexpr Glyph g{" ### ", "#   #", "#   #", "#   #", "# # #", "#  # ", " ## #"}; return g; }
        case 'R': { static constexpr Glyph g{"#### ", "#   #", "#   #", "#### ", "# #  ", "#  # ", "#   #"}; return g; }
        case 'S': { static constexpr Glyph g{" ####", "#    ", "#    ", " ### ", "    #", "    #", "#### "}; return g; }
        case 'T': { static constexpr Glyph g{"#####", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "  #  "}; return g; }
        case 'U': { static constexpr Glyph g{"#   #", "#   #", "#   #", "#   #", "#   #", "#   #", " ### "}; return g; }
        case 'V': { static constexpr Glyph g{"#   #", "#   #", "#   #", "#   #", "#   #", " # # ", "  #  "}; return g; }
        case 'W': { static constexpr Glyph g{"#   #", "#   #", "#   #", "# # #", "# # #", "## ##", "#   #"}; return g; }
        case 'X': { static constexpr Glyph g{"#   #", " # # ", "  #  ", "  #  ", "  #  ", " # # ", "#   #"}; return g; }
        case 'Y': { static constexpr Glyph g{"#   #", " # # ", "  #  ", "  #  ", "  #  ", "  #  ", "  #  "}; return g; }
        case 'Z': { static constexpr Glyph g{"#####", "    #", "   # ", "  #  ", " #   ", "#    ", "#####"}; return g; }
        case '0': { static constexpr Glyph g{" ### ", "#   #", "#  ##", "# # #", "##  #", "#   #", " ### "}; return g; }
        case '1': { static constexpr Glyph g{"  #  ", " ##  ", "# #  ", "  #  ", "  #  ", "  #  ", "#####"}; return g; }
        case '2': { static constexpr Glyph g{" ### ", "#   #", "    #", "   # ", "  #  ", " #   ", "#####"}; return g; }
        case '3': { static constexpr Glyph g{"#####", "   # ", "  #  ", "    #", "    #", "#   #", " ### "}; return g; }
        case '4': { static constexpr Glyph g{"   # ", "  ## ", " # # ", "#  # ", "#####", "   # ", "   # "}; return g; }
        case '5': { static constexpr Glyph g{"#####", "#    ", "#### ", "    #", "    #", "#   #", " ### "}; return g; }
        case '6': { static constexpr Glyph g{" ### ", "#    ", "#    ", "#### ", "#   #", "#   #", " ### "}; return g; }
        case '7': { static constexpr Glyph g{"#####", "    #", "   # ", "   # ", "  #  ", "  #  ", "  #  "}; return g; }
        case '8': { static constexpr Glyph g{" ### ", "#   #", "#   #", " ### ", "#   #", "#   #", " ### "}; return g; }
        case '9': { static constexpr Glyph g{" ### ", "#   #", "#   #", " ####", "    #", "    #", " ### "}; return g; }
        case '.': { static constexpr Glyph g{"     ", "     ", "     ", "     ", "     ", " ##  ", " ##  "}; return g; }
        case ',': { static constexpr Glyph g{"     ", "     ", "     ", "     ", " ##  ", " ##  ", " #   "}; return g; }
        case '-': { static constexpr Glyph g{"     ", "     ", "     ", "#####", "     ", "     ", "     "}; return g; }
        case '+': { static constexpr Glyph g{"     ", "  #  ", "  #  ", "#####", "  #  ", "  #  ", "     "}; return g; }
        case ':': { static constexpr Glyph g{"     ", " ##  ", " ##  ", "     ", " ##  ", " ##  ", "     "}; return g; }
        case '/': { static constexpr Glyph g{"    #", "    #", "   # ", "  #  ", " #   ", "#    ", "#    "}; return g; }
        case '%': { static constexpr Glyph g{"##   #", "##  # ", "    # ", "   #  ", "  #   ", " #  ##", "#   ##"}; return g; }
        case '(': { static constexpr Glyph g{"   ##", "  #  ", " #   ", " #   ", " #   ", "  #  ", "   ##"}; return g; }
        case ')': { static constexpr Glyph g{"##   ", "  #  ", "   # ", "   # ", "   # ", "  #  ", "##   "}; return g; }
        case '!': { static constexpr Glyph g{"  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "     ", "  #  "}; return g; }
        case '?': { static constexpr Glyph g{" ### ", "#   #", "    #", "   # ", "  #  ", "     ", "  #  "}; return g; }
        case '_': { static constexpr Glyph g{"     ", "     ", "     ", "     ", "     ", "     ", "#####"}; return g; }
        case '=': { static constexpr Glyph g{"     ", "     ", "#####", "     ", "#####", "     ", "     "}; return g; }
        case '#': { static constexpr Glyph g{" # # ", " # # ", "#####", " # # ", "#####", " # # ", " # # "}; return g; }
        case '*': { static constexpr Glyph g{"     ", " # # ", "  #  ", "#####", "  #  ", " # # ", "     "}; return g; }
        default: return blank;
    }
}

int roundInt(float value) { return static_cast<int>(std::lround(value)); }

} // namespace

int measureText(std::string_view text, int scale) noexcept
{
    if (text.empty() || scale <= 0) return 0;
    return (static_cast<int>(text.size()) * 6 - 1) * scale;
}

void SoftwareRenderer::blendPixel(int x, int y, Color color) noexcept
{
    if (x < 0 || y < 0 || x >= target_.width() || y >= target_.height() || color.alpha == 0) return;
    if (color.alpha == 255) {
        target_.setPixel(x, y, color);
        return;
    }
    const Color dst = target_.pixel(x, y);
    const float srcA = static_cast<float>(color.alpha) / 255.0f;
    const float dstA = static_cast<float>(dst.alpha) / 255.0f;
    const float outA = srcA + dstA * (1.0f - srcA);
    auto channel = [&](std::uint8_t s, std::uint8_t d) {
        if (outA <= 0.0f) return std::uint8_t{0};
        const float value = (static_cast<float>(s) * srcA + static_cast<float>(d) * dstA * (1.0f - srcA)) / outA;
        return static_cast<std::uint8_t>(std::clamp(value, 0.0f, 255.0f));
    };
    target_.setPixel(x, y, {channel(color.red, dst.red), channel(color.green, dst.green),
                            channel(color.blue, dst.blue),
                            static_cast<std::uint8_t>(std::clamp(outA * 255.0f, 0.0f, 255.0f))});
}

void SoftwareRenderer::fillRect(RectF rect, Color color)
{
    const int x0 = roundInt(rect.x);
    const int y0 = roundInt(rect.y);
    const int x1 = roundInt(rect.right());
    const int y1 = roundInt(rect.bottom());
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x) blendPixel(x, y, color);
}

void SoftwareRenderer::strokeRect(RectF rect, Color color, float thickness)
{
    const float t = thickness < 1.0f ? 1.0f : thickness;
    fillRect({rect.x, rect.y, rect.width, t}, color);
    fillRect({rect.x, rect.bottom() - t, rect.width, t}, color);
    fillRect({rect.x, rect.y, t, rect.height}, color);
    fillRect({rect.right() - t, rect.y, t, rect.height}, color);
}

void SoftwareRenderer::drawLine(PointF from, PointF to, Color color, float thickness)
{
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;
    const int steps = (std::max)(1, static_cast<int>(std::ceil((std::max)(std::fabs(dx), std::fabs(dy)))));
    const int radius = (std::max)(0, static_cast<int>(thickness / 2.0f));
    for (int i = 0; i <= steps; ++i) {
        const float fraction = static_cast<float>(i) / static_cast<float>(steps);
        const int cx = roundInt(from.x + dx * fraction);
        const int cy = roundInt(from.y + dy * fraction);
        for (int y = cy - radius; y <= cy + radius; ++y)
            for (int x = cx - radius; x <= cx + radius; ++x) blendPixel(x, y, color);
    }
}

void SoftwareRenderer::fillCircle(PointF centre, float radius, Color color)
{
    const int r = roundInt(radius);
    const int cx = roundInt(centre.x);
    const int cy = roundInt(centre.y);
    for (int y = cy - r; y <= cy + r; ++y)
        for (int x = cx - r; x <= cx + r; ++x) {
            const float dx = static_cast<float>(x) - centre.x;
            const float dy = static_cast<float>(y) - centre.y;
            if (dx * dx + dy * dy <= radius * radius) blendPixel(x, y, color);
        }
}

void SoftwareRenderer::drawText(std::string_view text, PointF position, int scale, Color color)
{
    if (scale <= 0) return;
    int cursorX = roundInt(position.x);
    const int baseY = roundInt(position.y);
    for (const char c : text) {
        const Glyph& glyph = glyphFor(c);
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 5; ++col)
                if (glyph[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)] == '#')
                    fillRect({static_cast<float>(cursorX + col * scale), static_cast<float>(baseY + row * scale),
                              static_cast<float>(scale), static_cast<float>(scale)}, color);
        cursorX += 6 * scale;
    }
}

void SoftwareRenderer::drawImage(const Image& source, RectF sourceRect, RectF destinationRect)
{
    if (source.empty() || destinationRect.width <= 0 || destinationRect.height <= 0) return;
    const int x0 = roundInt(destinationRect.x);
    const int y0 = roundInt(destinationRect.y);
    const int x1 = roundInt(destinationRect.right());
    const int y1 = roundInt(destinationRect.bottom());
    for (int y = y0; y < y1; ++y) {
        const float v = static_cast<float>(y - y0) / (std::max)(1.0f, destinationRect.height);
        const int sy = static_cast<int>(sourceRect.y + v * sourceRect.height);
        for (int x = x0; x < x1; ++x) {
            const float u = static_cast<float>(x - x0) / (std::max)(1.0f, destinationRect.width);
            const int sx = static_cast<int>(sourceRect.x + u * sourceRect.width);
            blendPixel(x, y, source.pixel(sx, sy));
        }
    }
}

void SoftwareRenderer::drawWaveform(std::span<const float> samples, RectF rect, Color color)
{
    if (samples.empty() || rect.width <= 0 || rect.height <= 0) return;
    const int columns = (std::max)(1, roundInt(rect.width));
    const auto peaks = computeWaveformPeaks(samples, columns);
    const float centreY = rect.y + rect.height / 2.0f;
    for (int column = 0; column < columns; ++column) {
        const auto& peak = peaks[static_cast<std::size_t>(column)];
        const float top = centreY - peak.maximum * rect.height / 2.0f;
        const float bottom = centreY - peak.minimum * rect.height / 2.0f;
        const int x = roundInt(rect.x) + column;
        const int y0 = roundInt((std::min)(top, bottom));
        const int y1 = roundInt((std::max)(top, bottom));
        for (int y = y0; y <= y1; ++y) blendPixel(x, y, color);
    }
}

} // namespace nxf::graphics
