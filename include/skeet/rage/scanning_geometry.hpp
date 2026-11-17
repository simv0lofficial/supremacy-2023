#pragma once
#include "skeet/rage_hitchance.hpp"

namespace skeet::rage {
struct ScanIntersection {
    std::uint32_t hitbox = 0;
    float fraction = 1.0f;
    bool found = false;
};
// Exact scanner4335F16C..4335F269 nearest-hitbox traversal. Head candidates
// keep searching rank<=2; the first closer non-head hit terminates traversal.
ScanIntersection intersect_scan_hitboxes(const RageRay&, std::span<const RageScanHitbox>,
                                         const RageBoxClipper&);
}
