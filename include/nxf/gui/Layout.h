#pragma once

#include <nxf/gui/Component.h>

namespace nxf::gui {

/// Layouts position a container's visible children inside its
/// bounds. A child with a positive preferred size keeps it along
/// the layout axis; children without one share what is left.
/// Layout is a pure function of bounds and preferences - calling it
/// again after a resize is the intended workflow.
void layoutVertical(Component& container, Insets padding, int spacing);
void layoutHorizontal(Component& container, Insets padding, int spacing);
/// Equal cells, row-major. Rows are derived from the child count.
void layoutGrid(Component& container, int columns, Insets padding, int spacing);

} // namespace nxf::gui
