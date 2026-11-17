#pragma once
#include "bone_runtime.hpp"
#include "entity_update.hpp"
#include "state_copy.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/anti_aim/movement_capture.hpp"
#include "skeet/movement/simulation_environment.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/client/input_commands.hpp"
#include "local_prediction_pose_inputs.hpp"

namespace skeet::animation {
struct LocalPredictionPoseOffsets {
    std::uint32_t velocity_36375c23, duck_3309c0cf, player_flags_e2fbe695;
    std::uint32_t command_f58e6118, origin_4e66bb34, origin_2a671b42, body_yaw_a2a8247f;
};
struct LocalPredictionPoseState {
    float& timestamp; //4347C640
    HitboxMatrices& hitboxes; //4347C650
    RageVector& origin; //4347C9E0
};
class LocalPredictionCommandRing {
    const client::Command* commands_;
    void* const* input_;

public:
    explicit LocalPredictionCommandRing(std::span<const client::Command, 150> commands) noexcept
        : commands_(commands.data()), input_(nullptr) {}
    explicit LocalPredictionCommandRing(void* const& input) noexcept
        : commands_(nullptr), input_(&input) {}
    const client::Command& operator[](std::uint32_t number) const noexcept {
        return input_ ? client::input_command_slot(*input_, number) : commands_[number % 150u];
    }
};
struct LocalPredictionPoseBindings {
    const lagcomp::EntityStateRuntime& entity;
    LocalPredictionPoseOffsets offsets;
    const lagcomp::TickTimingSamples& latency;
    const anti_aim::MovementCaptureInput& movement_capture;
    const MovementEnvironmentBindings& movement_environment;
    MovementCollisionEngine& collision;
    LocalPredictionCommandRing commands;
    void* const& animation_update; //optional434693F0
    const std::int32_t* const& animation_frame;
    float* const& current_time; //43467EB8
    const float* const& interval; //4346A338
    std::uint8_t* const& animation_reset; //optional43467B24
    std::uint8_t& animation_update_flag; //43475BD3
    void* const& allocator; //43467204; actualFree virtual5
    const BoneRuntime& bones;
    lagcomp::SetAbsoluteVector const& set_absolute_angles; //live optional434688C8
    LocalPredictionPoseState output;
    const float* const& publication_clock; //4346A334
};
// Complete4331EE04. The original is gated by send_packet or a full choke
// budget; this entry runs after the concrete restricted-player command owner.
// Movement, animation update and model-pose publication remain source.
void publish_local_prediction_pose(client::CommandFrame&, void* player, const client::Command&,
                                   std::uint8_t send_packet, const LocalPredictionPoseBindings&);
// Explicit native-frame input seam for differential fixtures. Production uses
// the automatic workspace owned by the entry above, not persistent seed data.
void publish_local_prediction_pose_with_inputs(client::CommandFrame&, void* player,
                                               const client::Command&, std::uint8_t send_packet,
                                               const LocalPredictionPoseBindings&,
                                               const LocalPredictionPoseInputs&);
} // namespace skeet::animation
