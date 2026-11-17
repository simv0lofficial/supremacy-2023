#pragma once
#include "simulation_collision.hpp"

namespace skeet {

// Complete grounded movement chain 0x433DACC9..0x433DD363: initial probe,
// sliding, raised-step comparison, ground retention and completion flag.
void move_movement_simulation_ground(MovementSimulation& state, MovementCollisionEngine& engine);

} // namespace skeet
