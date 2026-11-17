#pragma once
#include "simulation.hpp"

namespace skeet {

// Shared native ground-classification chain: full hull, then the four hull
// quadrants when required. The original fraction/end survive quadrant retries.
// This uses the actual engine-backed skip-player filter (4346EDC0 forwarding
// through434758B4), collision trace and surface-friction lookup boundaries.
bool categorize_movement_ground(MovementSimulation& state, MovementCollisionEngine& engine,
                                bool extend_grounded_drop, bool mark_hard_landing = false);

// Initial 0x433D7868..0x433D8565 pass. MOVETYPE_WALK retains its native shortcut.
void prepare_movement_simulation_ground(MovementSimulation& state, MovementCollisionEngine& engine);

} // namespace skeet
