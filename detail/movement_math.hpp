#pragma once
#include "skeet/movement/types.hpp"
#include <bit>
#include <cmath>

namespace skeet::movement_detail {
constexpr float epsilon = 0x1p-23f;
constexpr float degrees_to_radians = 0.01745329238474369049072265625f;
constexpr float radians_to_degrees = 57.2957763671875f;
inline float negate(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000u);
}
inline float absolute(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) & 0x7fffffffu);
}
inline float dot(MovementVector a, MovementVector b) noexcept {
    return (a.y * b.y + a.x * b.x) + a.z * b.z;
}
inline float horizontal_length(MovementVector value) noexcept {
    return std::sqrt(value.x * value.x + value.y * value.y);
}
inline float command_length(const UserCommand& command) noexcept {
    return std::sqrt(
        (command.forward_move * command.forward_move + command.side_move * command.side_move) +
        command.up_move * command.up_move);
}
inline MovementVector normalize_horizontal(MovementVector value) noexcept {
    const float reciprocal = 1.0f / (horizontal_length(value) + epsilon);
    return {value.x * reciprocal, value.y * reciprocal, 0.0f};
}
inline float friction_speed(const MovementPhysics& physics, float speed) noexcept {
    const float control = maximum_number(physics.stop_speed, speed);
    const float drop = (control * physics.friction) * physics.interval_per_tick;
    return maximum_number(speed - drop, 0.0f);
}
inline MovementAngles horizontal_angles(MovementVector value, bool negative_pitch_zero) noexcept {
    if (value.x == 0.0f && value.y == 0.0f)
        return {90.0f, 0.0f, 0.0f};
    const float yaw =
        static_cast<float>(std::atan2(static_cast<double>(value.y), static_cast<double>(value.x)));
    const float pitch = static_cast<float>(std::atan2(
        negative_pitch_zero ? -0.0 : 0.0, static_cast<double>(horizontal_length(value))));
    return {pitch * radians_to_degrees, yaw * radians_to_degrees, 0.0f};
}
inline bool weapon_slowdown_exception(const MovementPhysics& physics) noexcept {
    return physics.weapon_mode_count > 0 && physics.weapon_mode > 1 &&
           physics.weapon_max_speed * 0.52f < 110.0f;
}
// SSE MAXSS/MINSS select the second operand on unordered comparisons.
inline float sse_max(float a, float b) noexcept {
    return a > b ? a : b;
}
inline float sse_min(float a, float b) noexcept {
    return a < b ? a : b;
}
inline float walking_acceleration(float acceleration, float speed, float limit) noexcept {
    speed = maximum_number(speed, 0.0f);
    if (speed > limit - 5.0f && speed - limit >= -5.0f) {
        float scale = (limit * 0.2f - (1.0f - speed * -0.2f)) - -1.0f;
        scale = sse_min(sse_max(scale, 0.0f), 1.0f);
        return scale * acceleration;
    }
    return acceleration;
}
inline void command_basis(MovementAngles angles, MovementVector& forward,
                          MovementVector& right) noexcept {
    const float pitch = angles.pitch * degrees_to_radians;
    const float yaw = angles.yaw * degrees_to_radians;
    const float roll = angles.roll * degrees_to_radians;
    const float sp = std::sin(pitch), cp = std::cos(pitch);
    const float sy = std::sin(yaw), cy = std::cos(yaw);
    const float sr = std::sin(roll), cr = std::cos(roll);
    forward = {cy * cp, sy * cp, negate(sp)};
    const float minus_cr = cr * -1.0f;
    const float sr_sp = sr * sp;
    right = {minus_cr * negate(sy) - sr_sp * cy, minus_cr * cy - sr_sp * sy, (sr * -1.0f) * cp};
}
}
