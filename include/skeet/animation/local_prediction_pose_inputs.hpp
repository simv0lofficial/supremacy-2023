#pragma once
#include "skeet/core.hpp"
#include "skeet/animation/state_copy.hpp"

namespace skeet::animation {
// Automatic local storage reserved by4331EE04. Production creates it inside
// the actual invocation without value-initialization. The explicit input API
// uses this layout only to reproduce selected stack bytes in fixtures.
struct LocalPredictionPoseInputs {
    std::array<std::byte, copied_animation_state_size> animation;
    std::array<Matrix3x4, 256> bones;
};
} //namespace skeet::animation
