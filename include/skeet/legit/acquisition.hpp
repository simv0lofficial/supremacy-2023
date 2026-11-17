#pragma once
#include "tracking.hpp"
#include "trigger_trace.hpp"

namespace skeet::anti_aim {
class TargetTraceEntities;
}
namespace skeet::legit {
struct StudioOffsets;
struct AcquisitionHitbox {
    std::int32_t bone, hitgroup;
    Vector minimum, maximum;
    float radius;
};
// 4339A43D..4339A95C, after eligibility and live/historical bone preparation.
// The native owner inspects exactly the first seven hitboxes; missing/short
// studio sets take its clear-record path. Matrix order is the selected hitbox
// order produced by complete433899B3, rather than raw bone-array order.
void acquire_visible_target(TargetTrack&, const ProfileState&, std::span<const AcquisitionHitbox>,
                            std::span<const Matrix3x4>, const Vector& eye, const Vector& forward,
                            float interval, BallisticEngine&, const BallisticFilter&,
                            TriggerTraceBindings);
struct AcquisitionTraceContext {
    void* const* engine_filter_vtable; // genuine43475B90 slots0..4
    const anti_aim::TargetTraceEntities& entities;
};
// Full command-owner path. Every hitbox gets a new43470404 filter prefix,
// including the concrete433B4EB6 rejection body and genuine engine delegates.
void acquire_visible_target(TargetTrack&, const ProfileState&, std::span<const AcquisitionHitbox>,
                            std::span<const Matrix3x4>, const Vector& eye, const Vector& forward,
                            float interval, BallisticEngine&, const AcquisitionTraceContext&,
                            TriggerTraceBindings);
// Whole live acquisition: resolve the current studio set after bone setup,
// then read each descriptor at its actual iteration, after preceding smoke/
// TraceRay calls. No per-player descriptor snapshot spans the trace loop.
void acquire_visible_target_live(TargetTrack&, const ProfileState&, const void* target,
                                 const StudioOffsets&, std::span<const Matrix3x4>,
                                 const Vector& eye, const Vector& forward, float interval,
                                 BallisticEngine&, const AcquisitionTraceContext&,
                                 TriggerTraceBindings);
} // namespace skeet::legit
