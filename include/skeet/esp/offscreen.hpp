#pragma once
#include "config.hpp"
#include "projection.hpp"
#include "skeet/render_queue.hpp"

namespace skeet::esp {

// Native geometry/animation block 0x43409F44..0x4340A18B. The caller has passed
// the local-player, enabled, screen visibility and angular-FOV gates.
void draw_out_of_fov_arrow(ui::RenderQueue& queue, Point screen, std::int32_t size,
                           std::int32_t distance_percent, std::uint32_t color, float yaw_delta,
                           float animation_time);
struct OffscreenInput {
    Point screen;
    Vector3 origin, target;
    // Globals 0x43479CD8/DC, in degrees.
    float pitch, yaw, animation_time;
    float camera_fov; //live4346D548, after the actual view/FOV update
};
struct LocalPlayerPredicate {
    void* context;
    bool (*evaluate)(void*);
};
// Full checkbox/predicate/FOV/draw block 0x43409D3D..0x4340A195. The enclosing
// owner has already rejected missing/local targets and on-screen players.
void draw_offscreen_indicator(ui::RenderQueue& queue, const OverlayConfig& config,
                              const OffscreenInput& input, LocalPlayerPredicate predicate);
// Continuation after the native checkbox and IsPlayer gates. The live engine
// owner reads screen/camera inputs after that SDK call, then enters here.
void draw_selected_offscreen_indicator(ui::RenderQueue&, const OverlayConfig&,
                                       const OffscreenInput&);

} // namespace skeet::esp
