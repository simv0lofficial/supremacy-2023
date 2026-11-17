#pragma once
#include "collision.hpp"

namespace skeet {

// Decoded snapshot used by the manual circle forecast in 0x4336F912.
// position is decoder A9EE6062; gravity and jump_impulse are the actual cvars.
// collision_mask is the player's virtual +0x260 result, without extra bits.
struct PreSpeedProbeInput {
    MovementVector position, velocity;
    std::uint32_t player_flags;
    float gravity, jump_impulse, interval_per_tick, horizon, turn_step;
    MovementVector hull_minimum, hull_maximum;
    std::uint32_t collision_mask, player;
};

struct PreSpeedProbeResult {
    bool blocked;
    MovementVector position, velocity;
    bool on_ground;
    // A blocked forward trace does not complete the current forecast tick.
    std::int32_t completed_ticks;
};

// 0x4337AED0..0x4337B722 / 0x4337B74B, stopping before the enclosing
// controller changes turn_step. The caller owns its enclosing trace-flag guard.
PreSpeedProbeResult forecast_pre_speed_circle(const PreSpeedProbeInput& input,
                                              MovementCollisionEngine& engine);

} // namespace skeet
