#pragma once
#include "skeet/anti_aim/freestanding.hpp"
#include "skeet/animation/bone_runtime.hpp"

namespace skeet::anti_aim {
struct BodyFreestandingContext {
    std::int32_t target_handle;
    RageVector target_origin;
    RageVector shot_position;
    EntityLookup entities;
};
struct BodyFreestandingMath {
    RageTrigonometry trigonometry;
    double (*atan2)(double, double);
};
struct BodyFreestandingBones {
    void* player;
    animation::BoneEntityState* state;
    const animation::BoneModel* model;
    const animation::BoneEngine* engine;
    float* eye_pitch; // decoded C1B66EE8 +4
    float* eye_yaw; // decoded C1B66EE8 +8
    float* pose_parameter_11; // decoded 1CAAF0A8 +2C
    bool hitbox_set_present;
    std::int32_t hitbox_count;
    std::int32_t first_hitbox_bone;
    RageVector first_hitbox_minimum, first_hitbox_maximum;
    std::span<const RageLocalHitbox> hitboxes;
};
// Per-call production projection. Hierarchy, selected set and descriptors are
// acquired from the current entity; the bone transaction uses the source live
// BoneRuntime rather than a detached BoneEntityState snapshot.
struct LiveBodyFreestandingBones {
    void* player;
    const animation::BoneRuntime& runtime;
    float* eye_pitch;
    float* eye_yaw;
    float* pose_parameter_11;
    bool hitbox_set_present;
    std::int32_t hitbox_count;
    std::int32_t first_hitbox_bone;
    RageVector first_hitbox_minimum, first_hitbox_maximum;
    std::span<const RageLocalHitbox> hitboxes;
    std::span<const std::uint32_t> hitbox_bones;
};
struct LiveBodyFreestandingSource {
    void* player;
    const animation::BoneRuntime& runtime;
    std::uint32_t eye_angles, pose_parameters;
};
struct LiveBodyFreestandingModel {
    bool hitbox_set_present;
    std::int32_t hitbox_count;
    std::array<RageLocalHitbox, 19> hitboxes;
    std::array<std::uint32_t, 19> hitbox_bones;
};
LiveBodyFreestandingModel capture_body_freestanding_model(const animation::BoneRuntime&,
                                                          const void* player);
LiveBodyFreestandingBones bind_body_freestanding_bones(const animation::BoneRuntime&, void* player,
                                                       const LiveBodyFreestandingModel&,
                                                       std::uint32_t eye_angles,
                                                       std::uint32_t pose_parameters) noexcept;
struct BodyFreestandingWorkspace {
    // Failed native bone builds leave their corresponding scratch center
    // untouched; preserve that caller-owned storage through all hypotheses.
    std::array<RageVector, 3> centers;
    std::array<Matrix3x4, animation::maximum_bones> matrices;
    FreestandingWorkspace freestanding;
};
// Complete native 433B7773. The command argument is unused by the native body.
bool solve_animation_body_freestanding(const RageVector& frame_origin_dc,
                                       const BodyFreestandingContext&, BodyFreestandingBones&,
                                       float maximum_yaw, float& result,
                                       const BodyFreestandingMath&, BodyFreestandingWorkspace&);
bool solve_animation_body_freestanding(const RageVector& frame_origin_dc,
                                       const BodyFreestandingContext&, LiveBodyFreestandingSource&,
                                       float maximum_yaw, float& result,
                                       const BodyFreestandingMath&, BodyFreestandingWorkspace&);
struct BodyFreestandingSettings {
    std::int32_t yaw_mode_2c;
    float maximum_angle_148;
};
// Complete 433375AC: the head must intersect, then a torso hitgroup 2/3
// must intersect at a strictly earlier fraction. Every clip is engine 43468C3C.
bool body_occludes_head(const RageRay&, const BodyFreestandingBones&, std::span<const Matrix3x4>,
                        const RageBoxClipper&);
bool body_occludes_head(const RageRay&, const LiveBodyFreestandingBones&,
                        std::span<const Matrix3x4>, const RageBoxClipper&);
// Complete 433EFAAA: three bone hypotheses, five support rays each, then
// strict maximum coverage selection. Unequal coverage is the success result.
bool solve_covered_body_freestanding(const BodyFreestandingSettings&,
                                     const RageVector& frame_origin_dc,
                                     const BodyFreestandingContext&, BodyFreestandingBones&,
                                     float command_pitch, float base_yaw, float maximum_yaw,
                                     float& result, const BodyFreestandingMath&,
                                     const RageBoxClipper&, BodyFreestandingWorkspace&);
bool solve_covered_body_freestanding(const BodyFreestandingSettings&,
                                     const RageVector& frame_origin_dc,
                                     const BodyFreestandingContext&, LiveBodyFreestandingSource&,
                                     float command_pitch, float base_yaw, float maximum_yaw,
                                     float& result, const BodyFreestandingMath&,
                                     const RageBoxClipper&, BodyFreestandingWorkspace&);
} // namespace skeet::anti_aim
