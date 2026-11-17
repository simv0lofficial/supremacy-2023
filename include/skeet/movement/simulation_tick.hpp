#pragma once
#include "simulation.hpp"

namespace skeet {

struct MovementSimulationEnvironment {
    MovementPhysics physics;
    bool player_field_a15c3105;
    bool jump_speed_cap_enabled;
    bool landing_stamina_cvar_present;
    std::uint8_t* engine_trace_flag;
};

// Native 0x433D7606. All player/cvar/weapon inputs are decoded engine values;
// collision calls use only the actual engine interfaces exposed in collision.hpp.
void simulate_movement_tick(MovementSimulation& state, MovementCollisionEngine& engine,
                            const MovementSimulationEnvironment& environment, std::uint32_t player,
                            const UserCommand& command, MovementAngles command_angles);

} // namespace skeet
