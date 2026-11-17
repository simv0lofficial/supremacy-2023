#pragma once
#include "core.hpp"
#include "render_queue.hpp"

namespace skeet::ui {
Point measure_text(RenderQueue& queue, std::u16string_view content, std::uint32_t flags);
}
