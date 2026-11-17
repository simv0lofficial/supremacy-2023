#pragma once
#include "state.hpp"

namespace skeet::legit {
struct TargetSelectionInput {
    Vector eye_position;
    Vector reference_angles; // complete433D5ACC result, roll ignored
    std::int32_t maximum_clients;
    float time;
    const float* live_time = nullptr;
};
struct AimTarget {
    std::int32_t index = 0;
    Vector angles{};
    float field_of_view = 0;
    float distance = 0;
};
struct TargetSelectionBindings {
    const rage::TargetBindings& entities;
    const TargetTrackingBindings& tracking;
    // Native __libm_sse2_atan2, with its original double ABI.
    double (*arc_tangent2)(double, double);
    RandomBindings random;
};
// 4339AF3F..4339B70E: nearest angular candidate, concrete target prediction,
// previous-target eligibility/retention, and target-change random state. The
// caller has already admitted the aim hotkey/lock-time gates. A failed previous
// target sets the native .4-second block; every failed selection releases it.
AimTarget select_aim_target(RuntimeState&, const TargetSelectionInput&,
                            const TargetSelectionBindings&);
} // namespace skeet::legit
