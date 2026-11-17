#pragma once
#include "rage_point_math.hpp"

namespace skeet::rage_point_detail {
struct SafePointBounds {
    std::array<RageCapsule, 3> last_hypotheses;
    PointBasis basis;
    std::vector<RageAimPoint> points;
};
RageCapsule transform_capsule(const RageLocalHitbox&, const Matrix3x4&) noexcept;
SafePointBounds build_safe_point_bounds(const RageVector& origin, const RageVector& center,
                                        const RageLocalHitbox&,
                                        std::span<const Matrix3x4> hypotheses);
std::vector<RageAimPoint> finish_safe_points(std::vector<RageAimPoint>, const RagePointGeneration&,
                                             const RageVector& target_origin,
                                             std::uint32_t hitbox_index, std::int32_t hitgroup);
} // namespace skeet::rage_point_detail
