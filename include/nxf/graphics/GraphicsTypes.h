#pragma once

#include <cstdint>

namespace nxf::graphics {

struct PointF {
    float x = 0.0f;
    float y = 0.0f;
};

struct RectF {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    [[nodiscard]] float right() const noexcept { return x + width; }
    [[nodiscard]] float bottom() const noexcept { return y + height; }
};

/// An 8-bit-per-channel RGBA colour. fromRgb makes an opaque colour.
struct Color {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
    std::uint8_t alpha = 0;
    Color() = default;
    constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) : red(r), green(g), blue(b), alpha(a) {}
    [[nodiscard]] static constexpr Color fromRgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return {r, g, b, 255}; }
    [[nodiscard]] static constexpr Color transparent() { return {}; }
    [[nodiscard]] static constexpr Color black() { return fromRgb(0, 0, 0); }
    [[nodiscard]] static constexpr Color white() { return fromRgb(255, 255, 255); }
    bool operator==(const Color&) const = default;
};

} // namespace nxf::graphics
