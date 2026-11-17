#pragma once
#include "animation_capture.hpp"
#include "update_timing.hpp"
#include "history_velocity_reconstruction.hpp"
#include "history_configuration.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/resolver_runtime/environment_entry.hpp"
#include <vector>

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
struct HistoryResolverInput {
    std::uint8_t previous_resolver_index; // captured before owner+A1 is cleared
    float maximum_yaw; // earlier433EAEEB result, default58 when state is absent
    float initial_speed_fraction; // earlier animation+11C sample
    std::uint32_t player_table_offset; // earlier networkable index *20
};
struct HistoryResolverScratch {
    // Enclosing4340C971 stack+0x328..0x330 (decompiler stack10B8). The original
    // both-point-contents-blocked arm consumes the incoming three words.
    RageVector probe_direction;
};
struct HistoryResolverFields {
    std::uint32_t recoil_pitch_66146802;
    std::uint32_t absolute_velocity_4e66bb34;
    std::uint32_t animation_override_00a5bd68;
    std::uint32_t pose_parameters_1caaf0a8;
    std::uint32_t studio_wrapper;
    std::uint32_t hitbox_set_c2b6fb10;
    std::uint32_t local_time_9f8e7f71;
};
struct HistoryResolverUpdateBindings {
    const EntityStateRuntime& entity;
    // Shared source-owned receiver configuration. The recovered decisions
    // read its live resolver cell and prediction-availability byte at use.
    const HistoryRecordingConfiguration& configuration;
    const std::uint8_t* player_table; // global4347BE08, stride20
    const std::int32_t& selected_entity; // global4346DB1C
    void* const* const& local_time_entity; // live outer pointer cell4346A3A4; nullable
    const float& current_time; // pointed4346A334
    float& animation_time; // pointed43467EB8; stepping intentionally changes it
    const std::int32_t& animation_frame; // pointed434678F8
    const std::uint8_t& shot_angles_active; // global43468D7C
    std::uint8_t* animation_reset_byte; // optional43467B24
    RageVector local_origin; // cached43475BD4..5BDC before the +64 offset
    // The source producer can grow this queue after the receiver is composed.
    // Each native traversal captures its own range at the actual gate below.
    const std::vector<HistoryEvaluation>& pending_shots;
    HistoryResolverFields fields;
    engine::NativeConVarReference recoil_scale_convar; // actual4346794C xorC429E1BF
    const HistoryVelocityReconstructionBindings& velocity; // actual cvars/classification
    const animation::AnimationUpdateEngine& animation;
    const resolver_runtime::EnvironmentEngine& environment;
};
struct HistoryResolverUpdateResult {
    float animation_publish_layer_weight; // native stack13AC, not an address
    float speed_fraction; // native stack1294, updated after each animation step
};
// Complete43411A37..43414F3C. Resolver policy, inline feedback and animation
// stepping execute here; only genuine engine/CRT operations are bound. The
// player and animation state are nonnull. Owner/history are the canonical
// PlayerState storage. Final field restoration precedes the publish worker.
HistoryResolverUpdateResult
update_history_resolver(client::PlayerState&, void* player, void* animation_state,
                        const HistoryAnimationSeed&, SimulationTickSpan, std::int32_t shot_tick,
                        std::uint32_t& update_flags, const HistoryResolverInput&,
                        HistoryResolverScratch&, const HistoryResolverUpdateBindings&);
} // namespace skeet::lagcomp
