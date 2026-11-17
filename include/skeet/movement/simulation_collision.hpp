#pragma once
#include "simulation.hpp"

namespace skeet {

// Shared four-bump / five-plane movement solver in 0x433D7606. Ground movement
// can reuse the already completed trace when its destination equals the next
// move; airborne and raised-step movement pass no first destination.
void slide_movement_simulation(MovementSimulation& state, MovementCollisionEngine& engine,
                               const MovementVector* first_destination = nullptr);

} // namespace skeet
