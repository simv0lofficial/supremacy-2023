#pragma once
#include "raw_air_solver.hpp"
#include "normalized_air_wall.hpp"
#include "normalized_air_final.hpp"

namespace skeet {

struct NormalizedAirStrafeSettings {
    RawAirStrafeSettings steering;
    float angle_step; //fullowner+1D4
};
struct NormalizedAirStrafeFrame {
    NormalizedAirWallInput collision; //velocity is the prepared73984 seed
    MovementVector forecast_position; //live A9EE6062
    std::uint32_t forecast_player_flags; //live E2FBE695
    float stamina;
    float engine_time;
};
struct NormalizedAirStrafeBindings {
    const AirWallSteeringBindings& wall;
    const NormalizedAirFinalBindings& final;
};

// Complete normalized solver43373CBC..43378DF9 after scout/velocity gates.
// Unlike the raw branch, the normalized guarded pre-speed and Z-Hop paths
// retain the configured smoothing for their distinct dynamic final solver.
bool process_normalized_air_strafe(const NormalizedAirStrafeSettings&, const AirStrafeSelection&,
                                   InputBitmap, const NormalizedAirStrafeFrame&,
                                   const MovementFeedbackHistory&, MovementFeedback&,
                                   client::Command&, MovementAngleView,
                                   const NormalizedAirStrafeBindings&);

} // namespace skeet
