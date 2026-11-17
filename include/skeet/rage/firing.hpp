#pragma once
#include "targeting_commit.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/animation/bones.hpp"
#include "skeet/animation/bone_source_reference.hpp"
#include "skeet/engine/build_eye_adjustment_bindings.hpp"

namespace skeet::animation {
struct BoneRuntime;
}

namespace skeet::rage {
// Typed projection of the scheduler's weapon owner. Offset names deliberately
// retain fields whose meaning is not established by their consumers.
struct FiringState {
    std::uint32_t flags;
    std::int32_t weapon_id, marker, previous_tick, current_tick;
    std::int32_t field_fc, field_15c, field_1d4;
    float field_198;
    MovementAngles reference_angles; // +20
    RageVector recoil, origin, bone_origin; // +9C,+DC,+10C
    float field_84, field_a8, field_164; // +E4 is origin[2], not another field
    float special_threshold, angle_step; // +1E4,+1E8
    std::uint8_t* send_packet;
};
struct FiringTargetData {
    const TargetCandidate& candidate;
    std::int32_t field_b4, field_13c;
};
struct FiringBranchState {
    bool field_22, alternate_path, field_33, field_83, timing_ready;
    bool field_5b = false;
};
struct FiringSharedState {
private:
    std::int32_t local_marker_;
    RageVector local_point_;
    std::int32_t local_delay_;

public:
    struct Live {};
    std::int32_t& selected_marker;
    RageVector& selected_point;
    std::int32_t& last_delay_marker;
    FiringSharedState(std::int32_t marker = 0, RageVector point = {},
                      std::int32_t delay = 0) noexcept
        : local_marker_(marker), local_point_(point), local_delay_(delay),
          selected_marker(local_marker_), selected_point(local_point_),
          last_delay_marker(local_delay_) {}
    FiringSharedState(Live, std::int32_t& marker, RageVector& point, std::int32_t& delay) noexcept
        : selected_marker(marker), selected_point(point), last_delay_marker(delay) {}
    FiringSharedState(const FiringSharedState&) = delete;
    FiringSharedState& operator=(const FiringSharedState&) = delete;
};
struct FiringReadinessBindings {
    client::PlayerStateCache& players;
    void* engine_client;
    const void* timing_convar;
    float tick_interval, current_time;
    float& prediction_time;
    std::int32_t latency_milliseconds;
    float correction_time;
    float receive_interval;
    bool enabled, mode_enabled, send_mode;
    std::int32_t delay_option;
    std::uint32_t scheduler_flags;
    std::int32_t committed_marker, command_marker;
    float revolver_gate;
};
enum class FiringReadiness {
    angles,
    retained_target,
    stopped
};
// Complete43354C61..433551BA. Cache lookup is the recovered433B8A2B;
// engine-client slot14 supplies time on the history-age branch.
FiringReadiness evaluate_firing_readiness(FiringState&, const FiringTargetData&, FiringBranchState&,
                                          FiringSharedState&, client::Command&,
                                          const FiringReadinessBindings&);
std::int32_t firing_mode(bool mode_enabled, bool send_mode) noexcept; //4331FF20

// Both copies43355497..43355615 /43355C4D..43355DC8. Returns true when
// the angular distance was at least the configured limit and angles changed.
bool limit_firing_angle_step(MovementAngles&, const MovementAngles& reference,
                             float maximum_step) noexcept;
struct FiringPlayerOffsets {
    std::ptrdiff_t flags, first_byte, second_byte, animation_state;
    std::ptrdiff_t studio_header, hitbox_set, pose_parameters, animation_guard;
};
class FiringPoseParameter {
    using Decoded = float (*)(const void*, std::uint32_t, float, float&);
    std::variant<Decoded, std::reference_wrapper<const engine::EyePoseParameterRuntime>> source_;

public:
    FiringPoseParameter(Decoded entry) noexcept : source_(entry) {}
    FiringPoseParameter(const engine::EyePoseParameterRuntime& source) noexcept
        : source_(std::cref(source)) {}
    float operator()(const void* wrapper, std::uint32_t index, float angle,
                     float& normalized) const {
        if (auto* decoded = std::get_if<Decoded>(&source_))
            return (*decoded)(wrapper, index, angle, normalized);
        return std::get<std::reference_wrapper<const engine::EyePoseParameterRuntime>>(source_)
            .get()
            .convert(wrapper, index, angle, normalized);
    }
};
struct FiringPostAngleBindings {
    void* player;
    FiringPlayerOffsets offsets;
    void* input; // actual CInput; GetUserCmd slot8(slot=0,sequence=number-1)
    animation::BoneSourceReference<animation::BoneEntityState> bone_state;
    animation::BoneSourceReference<const animation::BoneModel> bone_model;
    animation::BoneSourceReference<const animation::BoneEngine> bone_engine;
    RageTrigonometry trigonometry;
    TargetAngleMath angle_math;
    // Genuine resolved engine target43467CF0. The output reference receives
    // the normalized pose value; the returned adjusted angle is unused here.
    FiringPoseParameter convert_pose_parameter;
    bool compensate_recoil, enabled, send_mode;
    bool recoil_option; // native43467144, also copied into stack branch33
    std::uint32_t scheduler_flags;
    float prediction_time;
};
// Complete433553BB..43355DEF after target_aim_angles. Reuses the concrete
// bone transaction, corrects eye height from the selected studio bone, then
// rebuilds angles and repeats the limiter. Only actual engine getters/poses
// are dependencies. False takes native43356146 before accepted-shot commit.
bool evaluate_firing_post_angles(FiringState&, const TargetCandidate&, FiringBranchState&,
                                 client::Command&, const FiringPostAngleBindings&);
// Production43388720 entry. Every build reads this player's current engine
// model/pose/entity fields through the mandatory raw runtime. The semantic
// bone_state/model/engine fields above belong only to the separate fixture API.
bool evaluate_firing_post_angles(FiringState&, const TargetCandidate&, FiringBranchState&,
                                 client::Command&, const FiringPostAngleBindings&,
                                 const animation::BoneRuntime&);
// Complete433D6CD1 and433DF6A6 substeps used by that transaction.
bool firing_animation_ready(const void* animation_state, std::uint8_t player_guard) noexcept;
float correct_firing_eye_height(const Matrix3x4&, float height) noexcept;
} // namespace skeet::rage
