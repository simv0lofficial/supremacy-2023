#pragma once
#include "pre_speed.hpp"
#include "pre_speed_probe.hpp"
#include "simulation_capture.hpp"
#include "simulation_environment.hpp"

namespace skeet {

struct LivePreSpeedInput {
    PreSpeedLimits limits;
    MovementVector velocity; // cached XY seed used by the initial turn calculation
    const MovementVector& forecast_velocity; // reread in full at the start of each trial
    const MovementVector& radial_origin;
    float previous_heading;
};
struct LivePreSpeedBindings {
    void* player;
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& capture;
    std::uint32_t forecast_position_a9ee6062;
    MovementCollisionEngine& collision;
};

// The same recovered circular search as choose_raw_pre_speed_heading, with
// live native reads at each trial and real mask/maximum/minimum getters at
// every hull trace. The radial origin remains a live reference throughout.
float choose_live_pre_speed_heading(const LivePreSpeedInput&, float movement_heading,
                                    const LivePreSpeedBindings&);

} // namespace skeet
