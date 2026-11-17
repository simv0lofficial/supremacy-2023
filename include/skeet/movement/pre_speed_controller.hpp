#pragma once
#include "pre_speed.hpp"
#include "pre_speed_probe.hpp"

namespace skeet {

struct RawPreSpeedInput {
    PreSpeedLimits limits;
    PreSpeedProbeInput forecast;
    // The radial fallback uses the actual player virtual+28 origin, whereas
    // the circular forecast uses the separately decoded A9EE6062 position.
    MovementVector radial_origin;
    float player_speed_limit;
    float previous_heading;
};

// Complete raw-basis pre-speed search4337A8ED..4337C3D0, before feedback
// overshoot handling. The caller has established requested/effective pre-speed.
// This owns both native trace-flag scopes and the positive/negative searches;
// each trial executes the concrete manual forecast rather than the full tick.
float choose_raw_pre_speed_heading(const RawPreSpeedInput&, float movement_heading,
                                   MovementCollisionEngine&, std::uint8_t* engine_trace_flag);

} // namespace skeet
