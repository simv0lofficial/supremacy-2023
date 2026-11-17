#pragma once
#include "skeet/esp/other_config.hpp"
#include "skeet/render_queue.hpp"

namespace skeet::hud {
struct PenetrationReticleState {
    std::uint32_t color = 0; // full owner +28, render interface +18
    bool ready = false; // full owner +2C, render interface +1C
};
struct PenetrationReticleBindings {
    void* entity_list = nullptr;
    std::uint32_t life_state, active_weapon, weapon_definition;
};
// Complete 4341CC34. The gameplay owner publishes ready/color; rendering
// retains them and queries the genuine entity-list handle virtual.
void draw_penetration_reticle(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                              const PenetrationReticleState& state, void* local_player,
                              const PenetrationReticleBindings& bindings, Point screen_size);
} // namespace skeet::hud
