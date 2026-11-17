#pragma once
#include "preamble.hpp"
#include "targeting_pipeline.hpp"
#include "acquisition.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "single_target.hpp"
#include "skeet/movement/simulation_tick.hpp"
#include "skeet/movement/simulation_capture.hpp"

namespace skeet {
struct RageStopBindings;
}
namespace skeet::animation {
struct BoneRuntime;
}

namespace skeet::rage {
struct PostFiringBindings;
enum class PreambleTailEntry;
// Full main continuation contract shared with the command owner. The binding
// owns views of native globals and concrete helper contexts, not phase calls.
PreambleTailEntry continue_after_target_firing(PreambleState&, PreambleResult&, AcquisitionStorage&,
                                               TargetBatch&, const TargetFiringResult&,
                                               void* local_player, const PostFiringBindings&);
class PredictionMaskOwner;
//433C8BD6 loads the canonical singleton at use time. RAGE is constructed
// before the mask owner, so spans captured during startup would be premature.
struct PostFiringPredictionMasks {
    PredictionMaskOwner* const& owner; //43475A58; initialized before frame dispatch
};
bool post_firing_prediction_allowed(const PostFiringPredictionMasks&) noexcept; //433C8BD6
// First prefer the remembered low16-bit entity, then all prepared descriptors.
// Greater E8 ballistic count wins; ties keep the earlier descriptor.
std::size_t select_post_firing_candidate(const PreambleState&, const AcquisitionStorage&,
                                         std::int32_t remembered_handle) noexcept;
struct PostFiringOffsets {
    std::ptrdiff_t network_origin; //A9EE6062
    std::ptrdiff_t player_next_attack; //DE530A8B
    std::ptrdiff_t scope_guard_cacaeecc;
    std::ptrdiff_t scope_guard_c07d1150;
};
struct PostFiringMovementBindings {
    const GlobalSettings& settings;
    InputBitmap input;
    std::uint32_t& context_flags;
    std::uint32_t& scheduler_flags;
    std::uint32_t weapon_flags;
    std::span<const std::uint32_t, 2> target_mask; // canonicalPreambleState.target_mask
    const TargetBindings& targeting;
    PostFiringOffsets offsets;
    BallisticEngine& ballistics;
    BallisticEntity source;
    std::ptrdiff_t weapon_item;
    void* const* trace_filter_vtable;
    const anti_aim::TargetTraceEntities& trace_entities;
    PostFiringPredictionMasks prediction;
    const float& current_time;
    const std::int32_t& remembered_handle;
};
// Complete43356EED..433576F0: auto-scope, both native quick-stop publications,
// target damage probes, between-shot gate and final frame-bit400 restoration.
// scope_ground_gate is native AL at56EED; restore_firing_bit is saved SP+D8.
void finish_post_firing_movement(client::CommandFrame&, client::Command&,
                                 const TargetCandidate* leading,
                                 const AcquisitionEvaluation* leading_evaluation,
                                 bool scope_ground_gate, bool restore_firing_bit,
                                 const PostFiringMovementBindings&);
struct FinalTargetContextBindings;
struct ShotDiagnosticBindings;
struct ShotModelPublicationBindings;
struct PendingShotPublicationBindings;
struct PostFiringBindings {
    const PreambleBindings& preamble;
    const PostFiringMovementBindings& movement;
    client::PlayerStateCache& players;
    const client::CommandAngleBindings& command_angles;
    const FinalTargetContextBindings& final_target;
    script::Bridge& scripts;
    const ShotDiagnosticBindings& diagnostics;
    const PendingShotPublicationBindings& pending_shots;
    const ShotModelPublicationBindings& shot_models;
    std::int32_t& shot_index; //native gameplay+168
    const std::int32_t& entity_limit; //actual pointed43469300, exclusive upper bound
    const float& tick_correction; //43469254
    const TargetCandidate* prior_leading = nullptr; //SP+E0, initialized0 at4334FB52
    const RageStopBindings& stop;
    const MovementCaptureInput& simulation_capture;
    MovementCollisionEngine& collision;
    const TargetPredictionBindings& prediction;
    ScanContext& scanner;
    const animation::BoneRuntime& bone_runtime;
};
} // namespace skeet::rage
