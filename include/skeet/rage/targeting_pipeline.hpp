#pragma once
#include "firing.hpp"

namespace skeet::rage {
struct TargetFiringTiming {
    std::int32_t field_b4;
    std::int32_t field_13c;
};
enum class TargetFiringStatus {
    empty_batch, //4335619C; no target bookkeeping
    rejected_batch, //4335617E; set context100 and retain sorted[0]
    retained_target, //4335615E; selected target bookkeeping
    readiness_stopped, //4335615A; selected target bookkeeping
    post_angles_stopped, //43356146; player-cache cleanup then bookkeeping
    attack_button_only, //43356146; Revolver commit gate
    recorded //43355EA9; publish accepted shot
};
struct TargetFiringResult {
    TargetFiringStatus status;
    TargetShotChoice choice;
};
// 43354C61..43355EA9, entered with an admitted selection. Keeping this
// concrete tail separate lets acquisition invoke the full final-consumer
// engine reads lazily for each attempted primary or alternate point.
TargetFiringResult fire_target_choice(TargetShotChoice, const TargetFiringTiming&, FiringState&,
                                      FiringBranchState&, FiringSharedState&, client::Command&,
                                      RageShotState&, RageShotCommitState&,
                                      const FiringReadinessBindings&,
                                      const FiringPostAngleBindings&);
// Post-scan43354AD8..43355EA9 composition. All bindings refer to the same
// current command/weapon snapshot; evaluation context_flags references the
// supplied commit.context_flags. Timing entries follow the sorted batch.
// This is the recovered selection/firing chain, not the enclosing scheduler's
// target scan, command entry, quickstop or later shot-feedback owner.
TargetFiringResult select_and_fire_target(TargetBatch&, std::span<const FinalTargetEvaluation>,
                                          std::span<const TargetFiringTiming>, FiringState&,
                                          FiringBranchState&, FiringSharedState&, client::Command&,
                                          RageShotState&, RageShotCommitState&,
                                          const FiringReadinessBindings&,
                                          const FiringPostAngleBindings&, float chance_floor,
                                          float point_penalty);
}
