#pragma once
#include "skeet/rage/ballistics.hpp"

namespace skeet {
// Complete 433C0AE9 world-only penetration-to-point loop. minimum_damage is
// the native signed stack integer; the effective threshold is max(1,value-5).
// query.allow_penetration controls penetration independently of that threshold.
// The optional byte is the engine-state pointer at 434683D0, temporarily zeroed
// through 433AC5BD and restored on every return, including nested calls.
std::int32_t trace_point_penetration(BallisticEngine&, const BallisticVector& start,
                                     const BallisticVector& target, std::int32_t minimum_damage,
                                     BallisticQuery&, std::uint8_t* suppression_flag = nullptr);
} // namespace skeet
