#pragma once
#include "skeet/client/command_frame.hpp"
#include "simulation_environment.hpp"

namespace skeet {

struct RageStopBindings {
    const MovementEnvironmentBindings& environment;
};

//433F0C03: actual weapon virtual6E8. A zero result selects250 before the
// crouch factor and one-unit margin; NaN and signed-zero retain native tests.
float rage_stop_speed(void* weapon);

//4336961E: actual player virtual450 precedes the current decoded3EB71843
// property read. Both values are read anew on every call.
float rage_move_speed(void* player, const RageStopBindings&);

//433CE0FE: the frame receiver is unused. Only simulated on-ground bit1
// invokes the real counter-movement arithmetic with current engine reads.
// The simulation is inspected without changing any of its bytes.
void prepare_rage_stop_command(const client::CommandFrame&, void* player, client::Command&,
                               const MovementSimulation&, const RageStopBindings&);

} // namespace skeet
