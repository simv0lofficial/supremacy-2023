#pragma once
#include "skeet/rage/targeting_pipeline.hpp"

namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet::rage::targeting_detail {
// Entered at433551BA after the concrete readiness result admits angles.
TargetFiringResult aim_and_commit_target(TargetShotChoice, FiringState&, FiringBranchState&,
                                         client::Command&, RageShotState&, RageShotCommitState&,
                                         const FiringReadinessBindings&,
                                         const FiringPostAngleBindings&);
TargetFiringResult aim_and_commit_target(TargetShotChoice, FiringState&, FiringBranchState&,
                                         client::Command&, RageShotState&, RageShotCommitState&,
                                         const FiringReadinessBindings&,
                                         const FiringPostAngleBindings&,
                                         const animation::BoneRuntime&);
}
