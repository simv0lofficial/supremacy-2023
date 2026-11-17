#pragma once
#include "skeet/resolver_runtime/hypotheses.hpp"
#include "skeet/core.hpp"
#include <cmath>
#include <span>

namespace skeet::resolver_runtime {
inline bool valid_hypothesis_input(const HypothesisInput& value) noexcept {
    for (float field : std::array{
             value.speed_fraction, value.feet_yaw, value.movement_yaw, value.movement_yaw_target,
             value.duck_amount, value.running_fraction, value.ducking_fraction, value.minimum_yaw,
             value.maximum_yaw, value.eye_yaw, value.animation_fraction, value.velocity_x,
             value.velocity_y, value.velocity_z, value.lower_body_yaw, value.interval})
        if (!std::isfinite(field))
            return false;
    return value.interval > 0.0f && value.minimum_yaw < value.maximum_yaw;
}
inline bool valid_hypothesis(const Hypothesis& value) noexcept {
    return std::isfinite(value.yaw) && std::isfinite(value.pose_yaw) &&
           std::isfinite(value.body_pose_parameter) && std::isfinite(value.move_pose_parameter);
}
inline bool complete_hitbox_matrices(std::int32_t count,
                                     std::span<const Matrix3x4> matrices) noexcept {
    if (count != 19 || matrices.size() < 19)
        return false;
    for (const auto& matrix : matrices.first(19)) {
        for (const auto& row : matrix.rows)
            for (float value : row)
                if (!std::isfinite(value))
                    return false;
        const auto& a = matrix.rows;
        const float determinant = a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
                                  a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
                                  a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);
        if (!std::isfinite(determinant) || std::fabs(determinant) < 1e-6f)
            return false;
    }
    return true;
}
}
