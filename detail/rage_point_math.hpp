#pragma once
#include "skeet/rage_points.hpp"
#include <cmath>

namespace skeet::rage_point_detail {
void scale_points(std::vector<RageAimPoint>&, const RageVector&, const RagePointGeneration&);
constexpr float epsilon = 0x1p-23f;
inline RageVector subtract(const RageVector& a, const RageVector& b) noexcept {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}
inline float length(const RageVector& a) noexcept {
    return std::sqrt((a[0] * a[0] + a[1] * a[1]) + a[2] * a[2]);
}
inline RageVector normalize(const RageVector& a, float magnitude) noexcept {
    const float inverse = 1.0f / (magnitude + epsilon);
    return {inverse * a[0], inverse * a[1], inverse * a[2]};
}
inline RageVector midpoint(const RageCapsule& capsule) noexcept {
    return {(capsule.first[0] + capsule.second[0]) * 0.5f,
            (capsule.second[1] + capsule.first[1]) * 0.5f,
            (capsule.second[2] + capsule.first[2]) * 0.5f};
}
inline RageVector interpolate(const RageVector& from, const RageVector& to,
                              float fraction) noexcept {
    return {(to[0] - from[0]) * fraction + from[0], (to[1] - from[1]) * fraction + from[1],
            (to[2] - from[2]) * fraction + from[2]};
}
inline RageVector support(const RageCapsule& capsule, const RageVector& direction,
                          bool xyz_length = false) noexcept {
    const auto delta = subtract(capsule.second, capsule.first);
    const float projection =
        (delta[1] * direction[1] + delta[0] * direction[0]) + delta[2] * direction[2];
    const auto& endpoint = projection >= 0.0f ? capsule.second : capsule.first;
    const float magnitude =
        xyz_length ? length(direction)
                   : std::sqrt((direction[1] * direction[1] + direction[2] * direction[2]) +
                               direction[0] * direction[0]);
    const float scale = capsule.radius / magnitude;
    return {scale * direction[0] + endpoint[0], scale * direction[1] + endpoint[1],
            scale * direction[2] + endpoint[2]};
}
struct PointBasis {
    RageVector upper;
    RageVector side;
};
inline PointBasis basis_from_axis(const RageVector& axis) noexcept {
    if (axis[0] == 0.0f && axis[1] == 0.0f)
        return {{-axis[2], 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};
    const RageVector side_delta{axis[1] - axis[2] * 0.0f, axis[2] * 0.0f - axis[0],
                                axis[0] * 0.0f - axis[1] * 0.0f};
    const auto side = normalize(side_delta, length(side_delta));
    const RageVector upper_delta{side[1] * axis[2] - side[2] * axis[1],
                                 side[2] * axis[0] - side[0] * axis[2],
                                 side[0] * axis[1] - side[1] * axis[0]};
    return {normalize(upper_delta, length(upper_delta)), side};
}
} // namespace skeet::rage_point_detail
