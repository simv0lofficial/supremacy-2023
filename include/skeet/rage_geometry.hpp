#pragma once
#include "core.hpp"

namespace skeet {
using RageVector = std::array<float, 3>;

struct RageRay {
    RageVector start;
    RageVector delta;
};
struct RageCapsule {
    RageVector first;
    RageVector second;
    float radius;
};
struct CapsuleIntersection {
    float fraction = 1.0f;
    RageVector point{};
    RageVector normal{};
    bool start_solid = false;
};
// 0x43329A78 initialization followed by complete 0x43418772 geometry.
// Retains the native near-stationary capsule arithmetic and rounding order.
CapsuleIntersection intersect_rage_capsule(const RageRay&, const RageCapsule&) noexcept;

struct RageTrace {
    float fraction = 1.0f;
    RageVector point{};
    RageVector normal{};
    bool start_solid = false;
};
// Complete native 0x433F8E09: scales the delta by the current trace fraction,
// applies capsule intersection, and updates only when its fraction is below 1.
int clip_rage_ray_to_capsule(const RageRay&, const RageCapsule&, RageTrace&) noexcept;
} // namespace skeet
