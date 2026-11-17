#pragma once
#include "types.hpp"

namespace skeet {
// Complete 0x43340ACB and 0x43388405. Callers retain the feature scheduler's
// original ground/weapon/hotkey gates. Fast walk passes target 134 and true;
// slow-motion callers pass their computed target and acceleration flag.
void regulate_movement_speed(const MovementPhysics& physics, UserCommand& command,
                             MovementVector velocity, MovementAngles command_angles,
                             MovementAngleView movement_angles, float target_speed,
                             bool clear_speed_on_acceleration) noexcept;
bool write_counter_movement(const MovementPhysics& physics, UserCommand& command,
                            MovementVector velocity, MovementAngleView movement_angles) noexcept;
} // namespace skeet
