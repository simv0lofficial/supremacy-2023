#pragma once
#include "skeet/core.hpp"
#include "skeet/render_queue.hpp"

namespace skeet::ui {
struct WindowRenderState {
    Rectangle bounds{};
    std::uint32_t background_texture = UINT32_MAX;
};

void draw_window_frame(RenderQueue& queue, const WindowRenderState& window);
void draw_window_accent(RenderQueue& queue, const WindowRenderState& window);
void draw_window_decorations(RenderQueue& queue, const WindowRenderState& window,
                             const Rectangle& active_tab);
}
