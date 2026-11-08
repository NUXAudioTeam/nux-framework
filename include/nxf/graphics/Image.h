#pragma once

#include <nxf/graphics/GraphicsTypes.h>

#include <cstddef>
#include <vector>

namespace nxf::graphics {

/// A CPU-side RGBA image - the software renderer's target and the
/// interchange format for image drawing. Row-major, top-left origin.
class Image {
public:
    Image() = default;
    Image(int width, int height)
        : width_(width > 0 ? width : 0), height_(height > 0 ? height : 0),
          pixels_(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_)) {}

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    [[nodiscard]] bool empty() const noexcept { return pixels_.empty(); }

    [[nodiscard]] Color pixel(int x, int y) const noexcept {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return Color::transparent();
        return pixels_[index(x, y)];
    }
    void setPixel(int x, int y, Color color) noexcept {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
        pixels_[index(x, y)] = color;
    }
    void fill(Color color) { for (auto& p : pixels_) p = color; }

private:
    [[nodiscard]] std::size_t index(int x, int y) const noexcept {
        return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x);
    }
    int width_ = 0;
    int height_ = 0;
    std::vector<Color> pixels_;
};

} // namespace nxf::graphics
