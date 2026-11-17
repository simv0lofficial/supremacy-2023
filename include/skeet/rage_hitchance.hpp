#pragma once
#include "rage_geometry.hpp"

namespace skeet {
struct RageSpreadSample {
    float first;
    float second;
};
struct RageSpreadBasis {
    RageVector center;
    RageVector first;
    RageVector second;
};
struct RageScanHitbox {
    std::int32_t hitgroup;
    RageCapsule capsule;
    // Engine box-clipping inputs from native record +4 and +0x10.
    const void* engine_hitbox;
    Matrix3x4 transform;
};
struct RageBoxClipper {
    void* context;
    // Actual engine dependency at 0x43468C3C through 0x4339FA8E.
    // Required when a hitbox's capsule radius is not positive.
    int (*clip)(void*, const RageRay&, const RageScanHitbox&, RageTrace&);
};
struct RageHitchanceInput {
    RageVector origin;
    RageSpreadBasis basis;
    float distance;
    std::span<const RageScanHitbox> hitboxes;
    std::int32_t record_count;
    std::int32_t preferred_hitgroup;
    float minimum_hitgroup_multiplier;
};
struct RageHitchance {
    std::uint32_t hits;
    float fraction;
};
// Complete 32-sample loop 0x433600FE..0x433604AC. Input samples are the two
// consumed floats of each original 0x20-byte record; no RNG model is assumed.
// Capsule clipping is native recovered code. Box clipping stays an engine call.
RageHitchance evaluate_rage_hitchance(const RageHitchanceInput&,
                                      const std::array<RageSpreadSample, 32>&,
                                      const RageBoxClipper&);
} // namespace skeet
