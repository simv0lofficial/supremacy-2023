#pragma once
#include "skeet/render_queue.hpp"
#include "skeet/core.hpp"

namespace skeet::esp {

struct WeaponIcon {
    std::int32_t texture = -1, width = 0, height = 12;
};
struct WeaponIconProvider {
    void* context;
    // Native cache boundary 0x433591C2; normalized definition, actual
    // definition, requested raster height, output texture and dimensions.
    bool (*load)(void*, ui::RenderQueue&, std::uint16_t, std::uint16_t, std::int32_t, WeaponIcon&);
};
void draw_weapon_icon(ui::RenderQueue& queue, WeaponIconProvider provider, std::uint16_t definition,
                      Point position, std::uint32_t color, std::int32_t* width = nullptr);

} // namespace skeet::esp
