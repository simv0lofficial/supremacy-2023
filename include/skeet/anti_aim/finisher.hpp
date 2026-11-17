#pragma once
#include "skeet/anti_aim/angles.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/animation/bone_runtime.hpp"

namespace skeet::anti_aim {
struct FinisherState {
    AngleStageState& common;
    // Native full owner+1D8. Initialized by433F526C, then retained across ticks.
    std::span<std::byte, 0x348> animation_1d8;
    RageVector absolute_angles_1b8;
    std::span<Matrix3x4> matrices_520;
};
struct FinisherPlayer {
    std::uint32_t identity, flags_38ca5228;
    std::uint8_t field_043fa575, field_334b460c;
    float field_86a5f8b3_minus4;
};
struct FinisherBindings {
    void* player;
    void*& animation_state; // actual decoded3D52A1A7 slot
    float& scalar_a2a8247f;
    const float& simulation_time; //43476B40
    const std::uint8_t* const& build_bones_12c; //live owner+12C pointer, read after animationupdate
    RageVector& eye_angles; // decodedC1B66EE8+4
    const RageVector& final_angles; //4347A4F0
    const lagcomp::EntityNetvars& entity_offsets;
    const animation::AnimationAngleHistory& angle_history;
    const animation::AnimationUpdateTiming& timing;
    const animation::AnimationUpdateEngine& update;
    lagcomp::SetAbsoluteVector set_absolute_angles; // optional434688C8
    animation::BoneEntityState& bone_state;
    const animation::BoneModel& model;
    const animation::BoneEngine& bones;
    const RageTrigonometry& trigonometry;
};
struct LiveFinisherBindings {
    void* player;
    void*& animation_state;
    float& scalar_a2a8247f;
    const float& simulation_time;
    const std::uint8_t* const& build_bones_12c;
    RageVector& eye_angles;
    const RageVector& final_angles;
    const lagcomp::EntityNetvars& entity_offsets;
    const animation::AnimationAngleHistory& angle_history;
    const animation::AnimationUpdateTiming& timing;
    const animation::AnimationUpdateEngine& update;
    lagcomp::SetAbsoluteVector set_absolute_angles;
    const animation::BoneRuntime& bones;
};
// Complete phase6 owner433470E4, including saved-state reinitialization,
// temporary player animation/scalar/pose transaction and optional bone cache.
void finish_command_angles(bool enabled, std::int32_t body_yaw, FinisherState&,
                           const FinisherPlayer&, std::int32_t frame_command,
                           const client::Command&, std::uint8_t send_packet,
                           const FinisherBindings&);
void finish_command_angles(bool enabled, std::int32_t body_yaw, FinisherState&,
                           const FinisherPlayer&, std::int32_t frame_command,
                           const client::Command&, std::uint8_t send_packet,
                           const LiveFinisherBindings&);
} // namespace skeet::anti_aim
