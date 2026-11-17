#pragma once
#include "skeet/lagcomp/records.hpp"

namespace skeet::lagcomp {
// 4341143D..434115E3, before record/ground-dependent speed constraints.
// native starts with260 unless its preceding weapon query supplies maximum_speed.
// blend is owner+5A8; interpolated_factor is the preceding stack+D0 result.
void adjust_capture_velocity(Vector&, float blend, float maximum_speed,
                             float interpolated_factor) noexcept;
} // namespace skeet::lagcomp
