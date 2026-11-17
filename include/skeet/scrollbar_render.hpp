#pragma once
#include "gui_layout.hpp"
#include "render_queue.hpp"

namespace skeet::ui {
void draw_up_arrow(RenderQueue& queue, Point point, std::uint32_t color);
void draw_down_arrow(RenderQueue& queue, Point point, std::uint32_t color);
void draw_scrollbar(RenderQueue& queue, const Scrollbar& scroll, Point origin, float scale,
                    std::uint32_t alpha = 255);
}
