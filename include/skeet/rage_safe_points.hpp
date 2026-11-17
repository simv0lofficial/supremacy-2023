#pragma once
#include "rage_points.hpp"

namespace skeet {
// Complete non-head branch of 0x433FD9B7 and caller 0x4335E9D5..0x4335EED9.
// hitbox_index is 1..18. Matrices are either the three record hypotheses or
// nine additional hypotheses, in native traversal order; they transform the
// original local endpoints directly. center is the prepared hitbox midpoint.
// target_origin is candidate +0x120, used when averaging the retained bounds.
// The caller applies the safe-hitbox mask before invoking this function.
std::vector<RageAimPoint>
generate_rage_body_safe_points(const RagePointGeneration&, const RageVector& center,
                               const RageVector& target_origin, std::uint32_t hitbox_index,
                               const RageLocalHitbox&, std::span<const Matrix3x4> hypotheses);

struct RageHeadOcclusion {
    // Model hitbox 6 resolved through the entity's model/hitbox-set interface at
    // 0x433FE9E0..0x433FEA3D, followed by the same three record matrices.
    RageLocalHitbox hitbox;
    std::array<Matrix3x4, 3> matrices;
};
// Head branch of 0x433FD9B7, including native clipping and the 15-angle search.
// A null occlusion pointer specifically denotes the native additional-records
// pointer being present: 0x433FE9C0 returns the angular bounds in that case.
// Otherwise model hitbox 6 and its matrices are required engine/model inputs.
std::vector<RageAimPoint>
generate_rage_head_safe_points(const RagePointGeneration&, const RageVector& center,
                               const RageVector& target_origin, const RageLocalHitbox& head,
                               std::span<const Matrix3x4, 3> hypotheses, const RageHeadOcclusion*,
                               const RageTrigonometry&);
} // namespace skeet
