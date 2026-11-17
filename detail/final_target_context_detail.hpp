#pragma once
#include "skeet/rage/final_target_context.hpp"

namespace skeet::rage::final_target_detail {
struct Producer {
    const FinalTargetContextBindings& bindings;
    client::CommandFrame& frame;
    PreambleState& state;
    std::array<Matrix3x4, 256> saved_bones;
    const void* selected_descriptor = nullptr;
};
void bind_teammate_policy(FinalHitchanceContext&, const Producer&);
bool teammate_hit(const Producer&, BallisticEntity);
bool select_studio_hitbox(FinalHitchanceContext&, Producer&, const TargetCandidate&,
                          const lagcomp::LagRecord&);
std::vector<RageScanHitbox> hitbox_geometry(const FinalHitchanceContext&, const Producer&,
                                            const HitboxMatrices&);
void apply_record(const FinalHitchanceContext&, const Producer&, lagcomp::EntityRecordScope&,
                  const lagcomp::LagRecord&);
void publish_failure(FinalHitchanceFailure&, const Producer&, std::uint32_t entity_index);
std::int32_t trace_direction(const FinalTargetContextBindings&, BallisticEngine&,
                             const FinalHitchanceSample&, const RageVector& direction,
                             BallisticQuery&);
std::int32_t trace_projected(const FinalTargetContextBindings&, BallisticEngine&,
                             const FinalHitchanceSample&, const RageVector& point, BallisticQuery&);
std::int32_t trace_teammate(const Producer&, FinalHitchanceContext&, const RageVector& direction,
                            BallisticQuery&);
// Shared failure predicate is also used by the prebound lower-level API.
bool requests_body_aim(const FinalHitchanceFailure&);
bool requests_body_aim(const FinalHitchanceFailure&, const client::CommandFrame*);
} // namespace skeet::rage::final_target_detail
