#pragma once

namespace nxf::gui {

/// Integer geometry for the GUI layer. Rectangles are half-open:
/// [x, right()) x [y, bottom()) - a 100-wide rect at x=10 contains
/// columns 10..109. Half-open bounds make adjacency exact, which
/// the layout and hit tests rely on.

struct Point {
    int x = 0;
    int y = 0;
    bool operator==(const Point&) const = default;
    Point operator+(Point other) const { return {x + other.x, y + other.y}; }
    Point operator-(Point other) const { return {x - other.x, y - other.y}; }
};

struct Size {
    int width = 0;
    int height = 0;
    bool operator==(const Size&) const = default;
};

struct Rectangle {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    bool operator==(const Rectangle&) const = default;
    [[nodiscard]] int right() const noexcept { return x + width; }
    [[nodiscard]] int bottom() const noexcept { return y + height; }
    [[nodiscard]] Point origin() const noexcept { return {x, y}; }
    [[nodiscard]] Size size() const noexcept { return {width, height}; }
    [[nodiscard]] bool contains(Point p) const noexcept {
        return p.x >= x && p.x < right() && p.y >= y && p.y < bottom();
    }
    [[nodiscard]] Rectangle translated(int dx, int dy) const noexcept { return {x + dx, y + dy, width, height}; }
    [[nodiscard]] Rectangle expanded(int amount) const noexcept {
        return {x - amount, y - amount, width + amount * 2, height + amount * 2};
    }
    [[nodiscard]] bool intersects(Rectangle other) const noexcept {
        return x < other.right() && other.x < right() && y < other.bottom() && other.y < bottom();
    }
    [[nodiscard]] Rectangle intersection(Rectangle other) const noexcept {
        const int nx = x > other.x ? x : other.x;
        const int ny = y > other.y ? y : other.y;
        const int nr = right() < other.right() ? right() : other.right();
        const int nb = bottom() < other.bottom() ? bottom() : other.bottom();
        if (nr <= nx || nb <= ny) return {nx, ny, 0, 0};
        return {nx, ny, nr - nx, nb - ny};
    }
};

struct Insets {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
};

} // namespace nxf::gui
