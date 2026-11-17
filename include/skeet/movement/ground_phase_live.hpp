#pragma once
#include "ground_phase.hpp"
#include "command_runtime.hpp"

namespace skeet {

GroundPhaseContinuation process_live_ground_phase(MovementCommandOwner&, void* player, void* weapon,
                                                  client::Command&, MovementAngleView,
                                                  const MovementCommandBindings&);

// Actual4337C899 tail. Each regulator obtains its own live reads; the second
// gate observes command mutations made by the first call.
void process_live_ground_speed_tail(const MovementCommandOwner&, void* player, client::Command&,
                                    MovementAngleView, const MovementCommandBindings&);

} // namespace skeet
