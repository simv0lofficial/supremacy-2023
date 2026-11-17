#pragma once
#include "acceleration.hpp"

namespace skeet {
struct MovementCommandBindings;

// Live433EE5FC. Duck-transition reads precede weapon lookups; acceleration
// and final interval are read at their native later stages.
GroundAccelerationCapacity live_movement_acceleration_capacity(void* player, const UserCommand&,
                                                               MovementVector velocity,
                                                               MovementVector direction,
                                                               const MovementCommandBindings&);

// Complete43340ACB with actual command-angle references and stage-local
// engine reads. Both braking and acceleration reach the live capacity path.
void regulate_live_movement_speed(void* player, UserCommand&, MovementVector velocity,
                                  ConstMovementAngleView command_angles,
                                  MovementAngleView movement_angles, float target_speed,
                                  bool clear_speed_on_acceleration, const MovementCommandBindings&);
} // namespace skeet
