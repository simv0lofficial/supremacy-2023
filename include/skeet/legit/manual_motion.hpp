#pragma once
#include "state.hpp"
#include "skeet/lagcomp/interpolation.hpp"

namespace skeet::legit {
// 43399F05..4339A2A2: complete manual-motion projection/filter/flag update.
// time_scale is the owner's native frame-time*64 value established on entry.
void update_manual_motion(RuntimeState&, const client::Command&, const Vector& frame_angles,
                          float current_time, const lagcomp::RotationMath&);
} // namespace skeet::legit
