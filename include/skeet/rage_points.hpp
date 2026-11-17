#pragma once
#include "rage_hitboxes.hpp"

namespace skeet {
struct RageAimPoint {
    RageVector position;
    float scale;
    std::int32_t hitgroup;
    std::uint32_t flags;
};
struct RagePointGeneration {
    RageVector origin;
    std::uint32_t regular_mask;
    std::uint32_t multipoint_mask;
    float point_scale;
    float spread;
    std::int32_t accuracy_percent;
    bool fixed_scale;
};
// Regular points from 0x4335D68B..0x4335E91A and the native hitbox traversal.
// Both spans contain the 19 model records used by this scanner. Hitboxes must
// already be transformed by prepare_rage_scan_hitboxes. Resolver-intersection
// candidates are generated separately by 0x433FD9B7 and are not part of this API.
std::vector<RageAimPoint> generate_rage_regular_points(const RagePointGeneration&,
                                                       std::span<const RageLocalHitbox, 19>,
                                                       std::span<const RageScanHitbox, 19>);
// Same native body for one index of the traversal. The scanner uses this to
// insert that hitbox's resolver-intersection candidates before the next index.
// An index outside the actual model count contributes no regular points.
std::vector<RageAimPoint> generate_rage_regular_hitbox_points(const RagePointGeneration&,
                                                              std::uint32_t hitbox_index,
                                                              std::span<const RageLocalHitbox>,
                                                              std::span<const RageScanHitbox>);
} // namespace skeet
