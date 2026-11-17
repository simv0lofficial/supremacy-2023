#pragma once
#include "skeet/rage_safety.hpp"

namespace skeet::rage {
struct ScanSafeHitchanceInput {
    RageVector origin;
    RageSpreadBasis basis;
    float range;
    std::int32_t hitgroup;
    float minimum_hitgroup_multiplier;
    RageResolverHitboxes hypotheses;
};
// Native4335F8FA..4335FEB9 safe branch. Each sample has one normalization;
// its ray then spans the weapon range and must qualify in all three records.
RageHitchance evaluate_scan_safe_hitchance(const ScanSafeHitchanceInput&,
                                           std::span<const RageSpreadSample, 32>,
                                           const RageBoxClipper&);
}
