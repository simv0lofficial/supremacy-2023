#pragma once
#include "skeet/anti_aim/body_yaw.hpp"

namespace skeet::lagcomp {
struct AnimationYawMotion {
    anti_aim::AnimationYawInput bounds;
    float horizontal_speed_ec;
    float vertical_speed_f0;
};

// Complete 433CAFE0. The three scalar arguments are native XMM0/1/2;
// current_yaw is the incoming value at EDX. The result is its final write.
float advance_animation_yaw(const AnimationYawMotion&, float target_yaw, float stationary_yaw,
                            float elapsed, float current_yaw, bool strict_bounds = false) noexcept;
} // namespace skeet::lagcomp
