#pragma once
#include "skeet/script/shutdown.hpp"
#include "skeet/core.hpp"
#include "skeet/render_queue.hpp"

namespace skeet::script {
// Complete native 0x4336918D, using the registered reference/string/color list.
// A callback receives entity_index and returns enabled, optional text.
void draw_flags(Bridge&, ReferenceStrings&, ui::RenderQueue&, std::int32_t entity_index,
                Point& cursor, std::int32_t line_height);
} // namespace skeet::script
