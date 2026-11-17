#pragma once
#include "simulation.hpp"
#include "types.hpp"

namespace skeet {
namespace client {
struct Command;
}
struct MovementEnvironmentBindings;
enum class MovementVelocityPath : std::uint32_t {
    skip_walk,
    ground,
    ground_stopped,
    air
};

// Native433DA2FD..433DACC9 /433DD368..433DD61A. The returned path marks
// the next collision stage; ground_stopped takes native433DD35C directly.
// physics contains actual decoded player/weapon values. The jump cap gate
// means cvar43467FA0^4B52A9BF has a self-valued integer (native false value).
MovementVelocityPath prepare_movement_simulation_velocity(MovementSimulation& state,
                                                          const MovementPhysics& physics,
                                                          const UserCommand& command,
                                                          MovementAngles angles,
                                                          bool jump_speed_cap_enabled) noexcept;

// Same native stage with its actual player/weapon reads performed only after
// their native jump and positive-additional-speed gates.
MovementVelocityPath prepare_live_movement_simulation_velocity(MovementSimulation&,
                                                               const UserCommand&, MovementAngles,
                                                               void* player,
                                                               const MovementEnvironmentBindings&);
MovementVelocityPath prepare_live_movement_simulation_velocity(MovementSimulation&,
                                                               const client::Command&, void* player,
                                                               const MovementEnvironmentBindings&);

// Initial grounded movement probe433DACC9..433DAEF4. The endpoint preserves
// position.z, the trace writes state.trace, and no slide/step response runs.
MovementVector trace_movement_simulation_ground(MovementSimulation& state,
                                                MovementCollisionEngine& engine);
void finish_movement_simulation_ground(MovementSimulation& state) noexcept; //433DD35C..433DD363
} // namespace skeet
