#include <nxf/gui/Layout.h>

#include <vector>

namespace nxf::gui {
namespace {
std::vector<Component*> visibleChildren(Component& container)
{
    std::vector<Component*> result;
    for (const auto& child : container.children())
        if (child->isVisible()) result.push_back(child.get());
    return result;
}
Rectangle innerArea(const Component& container, Insets padding)
{
    const auto b = container.bounds();
    return {padding.left, padding.top,
            b.width - padding.left - padding.right,
            b.height - padding.top - padding.bottom};
}
} // namespace

void layoutVertical(Component& container, Insets padding, int spacing)
{
    const auto kids = visibleChildren(container);
    if (kids.empty()) return;
    const Rectangle area = innerArea(container, padding);
    int fixedTotal = spacing * (static_cast<int>(kids.size()) - 1);
    int flexible = 0;
    for (const Component* kid : kids) {
        if (kid->preferredSize().height > 0) fixedTotal += kid->preferredSize().height;
        else ++flexible;
    }
    const int flexibleHeight = flexible > 0 ? (area.height - fixedTotal) / flexible : 0;
    int y = area.y;
    for (Component* kid : kids) {
        const int height = kid->preferredSize().height > 0 ? kid->preferredSize().height : flexibleHeight;
        kid->setBounds({area.x, y, area.width, height});
        y += height + spacing;
    }
}

void layoutHorizontal(Component& container, Insets padding, int spacing)
{
    const auto kids = visibleChildren(container);
    if (kids.empty()) return;
    const Rectangle area = innerArea(container, padding);
    int fixedTotal = spacing * (static_cast<int>(kids.size()) - 1);
    int flexible = 0;
    for (const Component* kid : kids) {
        if (kid->preferredSize().width > 0) fixedTotal += kid->preferredSize().width;
        else ++flexible;
    }
    const int flexibleWidth = flexible > 0 ? (area.width - fixedTotal) / flexible : 0;
    int x = area.x;
    for (Component* kid : kids) {
        const int width = kid->preferredSize().width > 0 ? kid->preferredSize().width : flexibleWidth;
        kid->setBounds({x, area.y, width, area.height});
        x += width + spacing;
    }
}

void layoutGrid(Component& container, int columns, Insets padding, int spacing)
{
    const auto kids = visibleChildren(container);
    if (kids.empty() || columns <= 0) return;
    const Rectangle area = innerArea(container, padding);
    const int rows = (static_cast<int>(kids.size()) + columns - 1) / columns;
    const int cellWidth = (area.width - spacing * (columns - 1)) / columns;
    const int cellHeight = (area.height - spacing * (rows - 1)) / rows;
    for (std::size_t i = 0; i < kids.size(); ++i) {
        const int row = static_cast<int>(i) / columns;
        const int col = static_cast<int>(i) % columns;
        kids[i]->setBounds({area.x + col * (cellWidth + spacing), area.y + row * (cellHeight + spacing), cellWidth, cellHeight});
    }
}

} // namespace nxf::gui
