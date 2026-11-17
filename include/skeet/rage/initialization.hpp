#pragma once
#include "reset.hpp"
#include "skeet/engine/jobs.hpp"

namespace skeet::rage {
//4348055C..4348062D, after the owner's configuration registrations. The
// canonical AcquisitionStorage already supplies the80 allocated evaluations.
// Unlike lifecycle reset, construction initializes highlight hitbox IDs and
// each frame's sparse constructor, and preserves the seven target-mask words.
void initialize_rage_state(const RageResetBindings&) noexcept;
// Production constructor path: prefix stores precede the actual heap call;
// queue/frame initialization still runs when the evaluation allocation fails.
void initialize_rage_state(const RageResetBindings&, engine::JobMemory);
}
