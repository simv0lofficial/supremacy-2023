#pragma once
#include "command_state.hpp"
#include "steering_math.hpp"

namespace skeet {

// Raw pre-speed guard path4337C573..4337C636, before ordinary final steering.
// The caller uses smoothing100 for this path. Only yaw can change here.
void apply_guarded_pre_speed_heading(float speed, float velocity_yaw, float previous_heading,
                                     float maximum_angle, MovementAngleView) noexcept;

// Complete feedback overshoot decision4337C3D0..4337C549 after the selected
// pre-speed heading has been committed. True goes directly to the ground-speed
// tail, retaining side_flip and the raw heading; false uses ordinary steering
// with smoothing0. The feedback heading alone is normalized on the true edge.
bool resolve_pre_speed_feedback(float previous_heading, float ideal_step, float turn_limit,
                                float side_magnitude, MovementFeedback&, UserCommand&,
                                ConstMovementAngleView) noexcept;

} // namespace skeet
