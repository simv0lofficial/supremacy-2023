#pragma once
#include "air_strafe.hpp"
#include "air_wall_steering.hpp"
#include "pre_speed.hpp"
#include "z_hop.hpp"

namespace skeet {

struct RawAirStrafeSettings {
    PreSpeedLimits limits;
    float side_magnitude;
    float feedback_turn_limit; //full movement owner+1D0
    std::int32_t smoothing;
    ZHopSettings z_hop;
};
struct RawAirStrafeFrame {
    AirWallSteeringInput collision;
    MovementVector forecast_position; //decoded A9EE6062
    std::uint32_t forecast_player_flags; //live E2FBE695, distinct from a simulated wall seed
    float previous_heading;
    float engine_time; //4346A334, distinct from simulation clock43467EB8
};

// Complete raw-basis airborne solver43379B3B..4337C899 after its scout/ground
// gates. All wall prediction, pre-speed forecasts, Z-Hop and final feedback are
// concrete source routines. False is an input-heading early edge to the same
// ground-speed tail; true means this solver reached a steering output.
bool process_raw_air_strafe(const RawAirStrafeSettings&, const AirStrafeSelection&, InputBitmap,
                            const RawAirStrafeFrame&, MovementFeedback&, client::Command&,
                            MovementAngleView movement_angles, const AirWallSteeringBindings&);

} // namespace skeet
