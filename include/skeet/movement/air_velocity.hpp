#pragma once
#include "simulation.hpp"

namespace skeet {

struct AirVelocityPreparation {
    MovementVector velocity;
    float stamina;
    bool continue_solver;
};

// Complete normalized-basis solver stage43373984..43373CBC. The native
// airborne path applies its first half-gravity step here; a grounded jump
// also recovers stamina, applies the speed cap and duck/stamina jump scaling.
// Grounded no-jump and low-speed paths continue at the ground-speed tail.
AirVelocityPreparation prepare_air_strafe_velocity(MovementVector velocity, float stamina,
                                                   const MovementPhysics&,
                                                   const MovementSimulationRules&,
                                                   bool movement_keys, bool bunny_hop_enabled,
                                                   bool jump_speed_cap_enabled,
                                                   float side_magnitude, UserCommand&) noexcept;

} // namespace skeet
