#pragma once
#include "command_frame.hpp"
#include "skeet/animation/bones.hpp"
#include "skeet/lagcomp/entity_state.hpp"

namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet::engine {
class EyePoseParameterRuntime;
}
namespace skeet::client {
struct CommandEyeAdjustmentOffsets {
    lagcomp::EntityNetvars common;
    std::ptrdiff_t view_offset; // C246315E, vector
    std::ptrdiff_t overridden_property; // 2B554AD0, uint32
    std::ptrdiff_t animation_guard; // E2FBE695, byte
    std::ptrdiff_t weapon_identifier; // 5C9F0FBB, uint16
    std::ptrdiff_t studio_header; // 32E4DAF1, studio wrapper pointer
    std::ptrdiff_t hitbox_set; // C2B6FB10, int32
    std::ptrdiff_t player_pitch_adjustment; // 5AE32976, float
    std::ptrdiff_t bone_origin; // A9EE6062, vector
};
struct CommandEyeAdjustmentContext {
    CommandEyeAdjustmentOffsets offsets;
    const std::uint8_t* enabled; // 43467224
    const std::uint8_t* alternate_enabled; // 4346A740
    const std::uint32_t* pitch_enabled; // 43467F60
    const std::int32_t* selected_command; // 434759CC
    const std::array<float, 3>* selected_point; // 434759D0
    const std::uint32_t* replacement_property; // optional 43469060
    std::span<const std::uint32_t, 3> weapon_classification; // 43474580
    animation::BoneEntityState& bone_state;
    const animation::BoneModel& bone_model;
    const animation::BoneEngine& bone_engine;
    const RageTrigonometry& trigonometry;
    // Genuine CRT __libm_sse2_atan2 boundary 77509EE0.
    double (*atan2)(double y, double x);
    // Genuine resolved engine entry 43467CF0, as in FiringPostAngleBindings.
    // 4331F306 supplies studio wrapper, index 12, angle, normalized output.
    float (*convert_pose_parameter)(const void* studio_wrapper, std::uint32_t index, float angle,
                                    float& normalized);
};
// Production uses the actual live-player bone builder and the verified engine
// pose ABI, without manufacturing semantic BoneEntityState/BoneModel copies.
// The ordinary geometry body and its read/build/write order are shared with
// the explicit standalone context above.
struct CommandEyeAdjustmentSourceContext {
    CommandEyeAdjustmentOffsets offsets;
    const std::uint8_t* enabled;
    const std::uint8_t* alternate_enabled;
    const std::uint32_t* pitch_enabled;
    const std::int32_t* selected_command;
    const std::array<float, 3>* selected_point;
    const std::uint32_t* const& replacement_property;
    std::span<const std::uint32_t, 3> weapon_classification;
    const animation::BoneRuntime& bones;
    const RageTrigonometry& trigonometry;
    double (*atan2)(double, double);
    const engine::EyePoseParameterRuntime& pose;
};

// Complete 433B5BC4, including its 433F8EEA bone wrapper. Uses the player's
// actual live matrix cache and restores pose parameter 12 on every exit after
// capture. Command is the unused native stack argument, retained in this API.
void adjust_command_eye(CommandFrame&, void* player, Command&, const CommandEyeAdjustmentContext&);
void adjust_command_eye(CommandFrame&, void* player, Command&,
                        const CommandEyeAdjustmentSourceContext&);
} // namespace skeet::client
