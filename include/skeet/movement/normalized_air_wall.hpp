#pragma once
#include "air_wall_steering.hpp"

namespace skeet {

struct NormalizedAirWallInput {
    bool enabled; // movement owner +0x158
    std::int32_t prediction_milliseconds; // +0x16C
    std::int32_t smoothing; // +0x68
    float maximum_angle, side_magnitude; // +0x1C8 / +0x1C4
    float acceleration_limit, angle_step; // +0x1D0 / +0x1D4
    float air_acceleration; // decoded cvar through owner +0x14
    bool pre_speed_effective;
    std::uint32_t player;
    MovementVector position, velocity;
    std::uint32_t player_flags;
    std::int32_t weapon_handle;
};

// Dynamic prediction heading used inside the normalized wall forecast. The
// simulated stamina/friction and live player duck state are distinct inputs.
float normalized_air_prediction_yaw(const NormalizedAirWallInput&, const MovementSimulation&,
                                    const UserCommand&, float previous_yaw,
                                    const MovementPhysics&) noexcept;

// Complete normalized-basis wall block 0x43373F63..0x433750E3, ending before
// the pre-speed dispatch. Returns the original velocity heading at ESP+0x10C;
// only movement_angles.yaw can change. The simulation constructor reads its
// stamina from the player snapshot, separately from the caller's ESP+0x14.
float steer_normalized_air_wall(const NormalizedAirWallInput&, const MovementFeedback&,
                                const client::Command&, MovementAngleView movement_angles,
                                const AirWallSteeringBindings&);

// Actual command path: constructor, each prediction tick and both trace-hull
// reads use the live engine bindings at their native call sites. Input carries
// only the cached velocity/selection/enable state in this path; owner tuning
// is read through bindings. The cvar cell is the actual owner+0x14 and is
// decoded separately for both angle terms.
float steer_live_normalized_air_wall(const NormalizedAirWallInput&, const MovementFeedback&,
                                     const client::Command&, MovementAngleView movement_angles,
                                     const AirWallSteeringLiveBindings&,
                                     const void* const& air_acceleration);

} // namespace skeet
