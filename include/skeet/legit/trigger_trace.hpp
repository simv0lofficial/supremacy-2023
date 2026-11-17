#pragma once
#include "runtime.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/rage/targeting_commit.hpp"

namespace skeet::legit {
struct TriggerTraceInput {
    BallisticEntity source;
    Vector origin;
    RageSpreadBasis basis;
    rage::SpreadParameters spread;
    std::uint32_t weapon_flags_7c;
    std::span<const float, 64> target_times;
    const TargetTrack* live_targets = nullptr; // full owner uses actual0x28-byte records
};
struct TriggerTraceBindings {
    void* context;
    // Actual entity networkable virtual slot 10. Invoked only when a hit with
    // nonzero reaction time reaches the predicate at native 4331FBDB.
    std::int32_t (*entity_index)(void*, BallisticEntity);
    // The optional engine smoke function reached through native 433E7C6B.
    bool (*through_smoke)(void*, const Vector&, const Vector&);
};
// 4339C260..4339C404: full 128-seed trigger hitchance traversal, using the
// native seed permutation, actual spread generator, direction trace, damage,
// hitgroup and reaction-time predicate, including both early stop rules.
bool evaluate_trigger_samples(BallisticEngine&, const ProfileState&, const TriggerTraceInput&,
                              const BallisticFilter&, BallisticQuery&, const rage::SpreadTables&,
                              const rage::SpreadRandom&, const rage::ShotgunPattern&,
                              const RageTrigonometry&, TriggerTraceBindings);
// 4339C194..4339C404: initial center trace/smoke check and the exact zero-hit-
// chance fast path, followed by the complete sample traversal above.
bool trace_trigger(BallisticEngine&, const ProfileState&, const TriggerTraceInput&,
                   const BallisticFilter&, BallisticQuery&, const rage::SpreadTables&,
                   const rage::SpreadRandom&, const rage::ShotgunPattern&, const RageTrigonometry&,
                   TriggerTraceBindings);
} // namespace skeet::legit
