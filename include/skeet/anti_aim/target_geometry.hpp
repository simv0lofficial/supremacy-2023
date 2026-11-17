#pragma once
#include "skeet/rage_hitboxes.hpp"

namespace skeet::anti_aim {
struct TargetPoints {
    std::array<RageVector, 6> points{};
    std::array<std::int32_t, 6> hitgroups{};
};
struct TargetGeometryOffsets {
    std::ptrdiff_t studio_header;
    std::ptrdiff_t hitbox_set;
};
struct TargetStudioHitbox {
    std::int32_t bone, hitgroup;
    RageVector minimum, maximum;
    std::int32_t name_index;
    RageVector rotation;
    float radius;
    std::array<std::int32_t, 4> unused;
};
static_assert(sizeof(TargetStudioHitbox) == 0x44);

// Native433F2CC1 selects hitboxes0/3/6/11/12, independently of the generic
// RAGE point generator. The supplied matrices are indexed by hitbox, not bone.
TargetPoints build_target_points(const RageVector& target_origin, float view_offset_z,
                                 const RageVector& shoot_origin, std::span<const RageLocalHitbox>,
                                 const HitboxMatrices&, const RageTrigonometry&);

// Actual entity slot77 and the decoded studio-header/hitbox-set fields. The
// remaining snapshots correspond to globals4347C9E0 and4347C650 respectively.
TargetPoints target_points_from_entity(void* entity, const TargetGeometryOffsets&,
                                       const RageVector& target_origin,
                                       const RageVector& shoot_origin, const HitboxMatrices&,
                                       const RageTrigonometry&);
} // namespace skeet::anti_aim
