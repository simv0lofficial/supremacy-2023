#pragma once
#include "skeet/hud/penetration_reticle.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/rage/ballistics.hpp"

namespace skeet::hud {
struct PenetrationReticleFrame {
    void* weapon; // Native frame+C.
    void* local_player; // Explicit player argument to phase4334B1DA.
    RageVector origin; // Frame+DC.
    RageVector direction; // Frame+1A8, already a direction, not Euler angles.
};
struct PenetrationReticleEnvironment {
    const rage::TargetBindings& targeting;
    const std::int32_t* maximum_clients; // Dereferenced native43469300.
    BallisticEngine& ballistics;
    std::ptrdiff_t target_tick; // Decoder2B554AD0.
    const std::int32_t* predicted_tick; // Native43469060; optional.
    std::ptrdiff_t collision_group; // DecoderE4E2D1A7.
    std::ptrdiff_t move_parent_handle; // Decoder49D93E62 minus18.
    void* point_contents; // Gameplay interface+28, IEngineTrace slot0.
    void* const* trace_filter_vtable; // Genuine external table43475B90, slots0..4.
};
// Complete4332AA97, including native43358962 eligibility and434012F7 geometry.
// Equal distances retain the earlier entity; unordered distances replace it.
void* closest_penetration_target(const rage::TargetBindings&, std::int32_t maximum_clients,
                                 const RageVector& origin, const RageVector& direction,
                                 float maximum_distance);
// Complete gameplay phase4334B1DA. Renderer4341CC34 consumes the same state.
void update_penetration_reticle(PenetrationReticleState&, const esp::ScreenOverlayConfig&,
                                const PenetrationReticleFrame&,
                                const PenetrationReticleEnvironment&);
}
