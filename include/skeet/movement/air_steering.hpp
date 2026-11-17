#pragma once
#include "steering_math.hpp"

namespace skeet {

struct AirSteeringInput {
    float velocity_yaw;
    float ideal_step;
    float speed;
    float side_magnitude;
    std::int32_t smoothing;
    bool large_turn;
};

struct AirSteeringState {
    bool side_flip; // command feedback + 0
    float feedback_heading; // command feedback + 8
};

// Complete final air-steering chain 0x4337C54D..0x4337C883. Simulation and
// collision stages supply the inputs; all command/state mutations stay here.
void write_air_strafe_step(const AirSteeringInput& input, AirSteeringState& state,
                           UserCommand& command, MovementAngleView movement_angles) noexcept;

} // namespace skeet
