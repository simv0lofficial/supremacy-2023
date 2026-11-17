#pragma once
#include "types.hpp"

namespace skeet {
struct GroundAcceleration {
    float amount;
    float wish_speed;
    float acceleration;
};

// Complete arithmetic helpers 0x4339E713 and 0x433AC9C0. Direction is supplied
// by their caller; these functions do not normalize it implicitly.
float air_acceleration_delta(MovementVector velocity, MovementVector direction, float wish_speed,
                             float acceleration, float interval_per_tick) noexcept;
GroundAcceleration ground_acceleration(const MovementPhysics& physics, const UserCommand& command,
                                       MovementVector velocity, MovementVector direction,
                                       float wish_speed) noexcept;

struct GroundAccelerationCapacity {
    float amount;
    bool ducking;
};
// Complete helper 0x433EE5FC, including prediction of the duck transition.
GroundAccelerationCapacity ground_acceleration_capacity(const MovementPhysics& physics,
                                                        const UserCommand& command,
                                                        MovementVector velocity,
                                                        MovementVector direction) noexcept;
} // namespace skeet
