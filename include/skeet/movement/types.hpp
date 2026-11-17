#pragma once
#include "skeet/core.hpp"

namespace skeet {
struct MovementVector {
    float x, y, z;
};
struct MovementAngles {
    float pitch, yaw, roll;
};

// Native movement angles can belong to either a dedicated vector or the
// command frame's three-float array. Reference views preserve intermediate
// writes across engine calls without aliasing unrelated C++ aggregate types.
struct MovementAngleView {
    float& pitch;
    float& yaw;
    float& roll;
    MovementAngleView(MovementAngles& value) noexcept
        : pitch(value.pitch), yaw(value.yaw), roll(value.roll) {}
    MovementAngleView(std::array<float, 3>& value) noexcept
        : pitch(value[0]), yaw(value[1]), roll(value[2]) {}
    MovementAngleView(const MovementAngleView&) = default;
    operator MovementAngles() const noexcept {
        return {pitch, yaw, roll};
    }
    MovementAngleView& operator=(MovementAngles value) noexcept {
        pitch = value.pitch;
        yaw = value.yaw;
        roll = value.roll;
        return *this;
    }
    MovementAngleView& operator=(const MovementAngleView& value) noexcept {
        return *this = static_cast<MovementAngles>(value);
    }
};
struct ConstMovementAngleView {
    const float& pitch;
    const float& yaw;
    const float& roll;
    ConstMovementAngleView(const MovementAngles& value) noexcept
        : pitch(value.pitch), yaw(value.yaw), roll(value.roll) {}
    ConstMovementAngleView(const std::array<float, 3>& value) noexcept
        : pitch(value[0]), yaw(value[1]), roll(value[2]) {}
    ConstMovementAngleView(MovementAngleView value) noexcept
        : pitch(value.pitch), yaw(value.yaw), roll(value.roll) {}
    operator MovementAngles() const noexcept {
        return {pitch, yaw, roll};
    }
};

inline constexpr std::uint32_t duck_button = 4;
inline constexpr std::uint32_t forward_button = 8;
inline constexpr std::uint32_t back_button = 16;
inline constexpr std::uint32_t move_left_button = 0x200;
inline constexpr std::uint32_t move_right_button = 0x400;
inline constexpr std::uint32_t speed_button = 0x20000;

// Decoded values supplied by the engine adapter. No encoded offsets, binary
// entrypoints, or guessed netvar offsets are required by the algorithms.
struct MovementPhysics {
    float interval_per_tick;
    float stop_speed;
    float friction;
    float accelerate;
    float player_speed_limit; // decoded player field, used by min helper
    float player_max_speed; // external player virtual method
    std::uint32_t player_flags;
    bool ducking;
    float duck_amount;
    float duck_speed;
    bool weapon_speed_enabled; // encoded cvar guard is not self-valued
    bool has_weapon; // external entity lookup succeeded
    float weapon_max_speed; // external weapon virtual method
    int weapon_mode_count; // external weapon virtual method
    int weapon_mode; // external weapon virtual method
};
} // namespace skeet
