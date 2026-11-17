#pragma once
#include "skeet/rage_hitboxes.hpp"

namespace skeet::animation {
using BoneQuaternion = std::array<float, 4>;
inline constexpr unsigned maximum_bones = 256;

struct BoneModel {
    // Engine/model readiness checks at 4338875F..433887C5.
    bool renderable_model;
    std::int32_t model_index;
    bool studio_header;
    bool virtual_model_required;
    bool virtual_model;
    std::span<const std::uint32_t> flags;
    std::span<const std::int32_t> parents;
    std::span<const std::uint32_t> hitbox_bones;
};
struct BoneEntityState {
    std::uint8_t interpolation_flags;
    std::uint32_t animation_frame;
    std::uint32_t effects;
    std::uint32_t setup_flags;
    std::uint32_t ik_mask;
    std::uint32_t entity_flags;
    std::array<RageVector, 2> angles;
    std::array<RageVector, 2> origins;
    // Layer 12 +20, touched only when the native layer count exceeds 12.
    float last_layer_weight;
    std::int32_t layer_count;
    // Native optional owner/IK dependencies resolved through entity handles.
    std::uint32_t* owner_setup_flags = nullptr;
    std::uintptr_t* ik_owner_slot = nullptr;
    std::uintptr_t ik_owner = 0;
    std::uint32_t readable_bones;
    std::uint32_t writable_bones;
};
struct BoneRequest {
    RageVector angles;
    RageVector origin;
    float time;
    std::uint32_t mask;
    std::int32_t starting_bone;
    std::uint32_t options;
    bool attachments;
};
struct BoneEngine {
    void* context;
    // Genuine engine virtual +338 called through 433E533F. This supplies local
    // poses, rather than replacing any recovered matrix-building routine.
    void (*accumulate_pose)(void*, BoneEntityState&, const BoneModel&,
                            std::span<RageVector, maximum_bones>,
                            std::span<BoneQuaternion, maximum_bones>, float, std::uint32_t);
    // Genuine engine target 43468F80; optional exactly as in 43341E47.
    void (*update_attachments)(void*, BoneEntityState&, std::span<Matrix3x4>,
                               std::uint32_t) = nullptr;
};

// Complete 433E4ADF; the first column deliberately uses double arithmetic.
Matrix3x4 bone_local_matrix(const RageVector&, const BoneQuaternion&) noexcept;
// Complete 433CB715. Valid studio arrays have at most 256 bones, parent-before-
// child order, an acyclic parent chain, and matching pose/output spans. Unmasked
// output matrices retain their previous contents, including skipped parents.
void compose_bone_hierarchy(const BoneModel&, const Matrix3x4& root, std::span<const RageVector>,
                            std::span<const BoneQuaternion>, std::int32_t starting_bone,
                            std::uint32_t mask, std::span<Matrix3x4> output) noexcept;
// Complete native 43388720 transaction, including owner/IK/layer state restore.
// accumulate_pose and CRT trig bindings are required engine dependencies.
bool build_bone_matrices(BoneEntityState&, const BoneModel&, const BoneRequest&,
                         std::span<Matrix3x4>, const BoneEngine&, const RageTrigonometry&);
// 433899B3 selects at most 19 matrices through model hitbox bone indices.
std::int32_t select_hitbox_bone_matrices(const BoneModel&, std::span<const Matrix3x4>,
                                         std::span<Matrix3x4>) noexcept;
// Complete 433FF63C: zero time, yaw-only angles, 80300 mask, all bones, then
// hitbox matrix selection. Nonpositive count is the resolver failure condition.
std::int32_t build_resolver_hitbox_matrices(BoneEntityState&, const BoneModel&,
                                            const RageVector& entity_origin, float yaw,
                                            std::span<Matrix3x4> hitboxes, const BoneEngine&,
                                            const RageTrigonometry&);
} // namespace skeet::animation
