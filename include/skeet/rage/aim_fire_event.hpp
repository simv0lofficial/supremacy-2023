#pragma once
#include <cstdint>

namespace skeet::script {
struct Bridge;
}
namespace skeet::client {
struct CommandFrame;
class PlayerStateCache;
}

namespace skeet::rage {
struct TargetCandidate;
struct AcquisitionEvaluation;

// Complete4340C8C4 ->4332A6B4 ->4336A85F. The native ECX frame argument
// is not read. Candidate point fields and B4 tick refer to the same scan slot.
// The canonical player-cache lookup still precedes callback lookup, even when
// no aim_fire callback is registered. An absent Lua state skips that lookup.
// Predefined event/table names read the Bridge's live native-name rows.
int dispatch_aim_fire_event(script::Bridge&, client::PlayerStateCache&, const client::CommandFrame&,
                            const TargetCandidate&, const AcquisitionEvaluation&,
                            std::int32_t shot_index);
} // namespace skeet::rage
