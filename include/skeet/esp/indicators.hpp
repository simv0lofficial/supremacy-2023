#pragma once
#include "skeet/core.hpp"
#include "skeet/render_queue.hpp"
#include "skeet/script/bridge.hpp"
#include <span>
#include <string_view>

namespace skeet::ui {
class RendererBackend;
}
namespace skeet::esp {

struct IndicatorLayout {
    float scale = 1.0f;
    std::int32_t screen_height = 0;
    // Dimensions of the renderer's texture table, indexed by texture handle.
    // Native 0x433D7503 returns zero dimensions for an invalid handle.
    std::span<const Point> textures;
    // Production metrics use the actual renderer at the native query site;
    // standalone authored layouts supply the explicit dimension span above.
    const ui::RendererBackend* texture_source = nullptr;
};

// Native drawing branch 0x434004DE..0x434008B8. The caller routes an active
// Lua indicator hook before entering this branch (0x43400499..0x434004D9).
// cursor is the accumulated distance from the bottom edge of the screen.
void draw_indicator(ui::RenderQueue& queue, std::int32_t& cursor, std::string_view utf8,
                    std::uint32_t color, const IndicatorLayout& layout, std::int32_t texture = -1,
                    float fraction = -1.0f);

// Complete 0x4340047A routing: an installed Lua hook consumes the call even
// when its same-frame recursion guard suppresses dispatch.
inline void draw_indicator(script::Bridge& scripts, ui::RenderQueue& queue, std::int32_t& cursor,
                           std::string_view utf8, std::uint32_t color,
                           const IndicatorLayout& layout, std::int32_t texture = -1,
                           float fraction = -1.0f) {
    if (utf8.empty())
        return;
    if (script::has_indicator(scripts)) {
        script::dispatch_indicator(scripts, &queue, &cursor, utf8, color);
        return;
    }
    draw_indicator(queue, cursor, utf8, color, layout, texture, fraction);
}

} // namespace skeet::esp
