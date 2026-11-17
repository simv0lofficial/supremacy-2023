#pragma once
#include "simulation.hpp"

namespace skeet {
struct MovementEnvironmentBindings;
// Exact 433D8583..433DA2FD. The boolean is the decoded player byte with
// identity A15C3105, read only while a duck transition is in progress.
// Ground contact is recategorized by the shared native trace algorithm.
// The following DA2FD stage owns the amount<=0 flag-four clear.
void update_movement_simulation_duck(MovementSimulation&, MovementCollisionEngine&,
                                     bool field_a15c3105);
void update_live_movement_simulation_duck(MovementSimulation&, MovementCollisionEngine&,
                                          void* player, const MovementEnvironmentBindings&);
} // namespace skeet
