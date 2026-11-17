#pragma once
#include "runtime.hpp"
#include "skeet/lagcomp/history.hpp"

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::legit {
struct TargetTrack {
    float elapsed = 0;
    std::uint32_t visible_hitgroups = 0;
    Vector point{};
    Vector velocity{};
    std::uint8_t velocity_valid = 0;
    std::array<std::uint8_t, 3> padding{};
    float interval = 0;
};
static_assert(sizeof(TargetTrack) == 0x28);
static_assert(offsetof(TargetTrack, velocity) == 0x14);
struct TargetTrackingBindings {
    std::span<const lagcomp::RecordRing, 64> histories;
    std::span<TargetTrack, 64> tracks;
    std::ptrdiff_t velocity_offset;
    std::int32_t clock_tick;
    float interval;
    // Full command owner binds the actual433B8A2B cache, including allocation
    // side effects. A supplied ring span remains the standalone verified API.
    client::PlayerStateCache* players = nullptr;
    const std::int32_t* live_clock_tick = nullptr; // [43475A60]+4, not43468D4C
    const float* live_interval = nullptr;
};
// Complete native4340C148, with cached history owners already present.
// Uses the entity networkable index, command-tick window and actual velocity
// netvar when no eligible historical velocities were accumulated.
Vector estimate_target_velocity(const ProfileState&, const TargetTrackingBindings&, void* entity);
// Complete4340B419, including its second networkable-index lookup and persistent
// velocity clamp, then the bounded 0.1..320 speed translation of the aim point.
void predict_target_point(const ProfileState&, const TargetTrackingBindings&, void* entity,
                          Vector& point);
} // namespace skeet::legit
