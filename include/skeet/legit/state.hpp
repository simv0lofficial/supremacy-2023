#pragma once
#include "tracking.hpp"
#include "skeet/client/commands.hpp"

namespace skeet::legit {
struct RuntimeState {
    ProfileState profile;
    std::int32_t last_command = 0; // owner808
    std::uint8_t new_shot = 0; // 80C
    Vector pending_step{}; // 864
    float pending_fraction = 0; // 870
    Vector remainder{}; // 874
    float block_until = 0; // 880
    std::int32_t target = 0, previous_target = 0; //884,888
    Vector recoil_velocity{}; //88C
    Vector filtered_motion{}; //8C0
    Vector raw_motion{}; //8CC
    float time_scale = 0; //8D8
    float manual_time = 0; //8DC
    Vector target_offset{}; //8E0
    float curve = 0; //8EC
    Vector current_view{}; //8F0
    std::uint8_t current_view_valid = 0; //8FC
    Vector previous_view{}; //900
    std::uint8_t previous_view_valid = 0; //90C
    std::array<TargetTrack, 64> targets{}; //910
};
// Complete433B2251: reset an active lock and reseed the native curvature.
void release_target(RuntimeState&, float interval, RandomBindings);
// Complete433C8F71, including record ages/masks/points and view-history validity.
void reset_runtime(RuntimeState&, float interval, RandomBindings);
// The full owner reloads the interval after target-release RNG callbacks.
void reset_runtime_live(RuntimeState&, const float* interval, RandomBindings);
// Complete433CC76E, the owner's secondary-interface fractional angle consumer.
void apply_pending_aim(RuntimeState&, client::Command&, float delta_time, float interval);
} // namespace skeet::legit
