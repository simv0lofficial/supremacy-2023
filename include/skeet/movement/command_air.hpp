#pragma once
#include "command_runtime.hpp"

namespace skeet {

// Main4336F912 air branches after the player gate, ending before sharedC899.
// This owns selection/scout/velocity ordering and calls only concrete live
// wall, pre-speed and normalized-final routines.
void process_live_movement_air(MovementCommandOwner&, void* player, void* weapon, client::Command&,
                               MovementAngleView, const MovementFeedbackHistory&, MovementFeedback&,
                               const MovementCommandBindings&);

} // namespace skeet
