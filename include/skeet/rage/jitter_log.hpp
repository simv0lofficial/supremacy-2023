#pragma once
#include <cstdint>
namespace skeet {
struct ShotRecordView;
}
namespace skeet::client {
struct PlayerState;
class PlayerStateCache;
}
namespace skeet::misc {
class NormalLog;
}
namespace skeet::rage {
struct TargetCandidate;
struct AcquisitionEvaluation;
void log_jitter_fire(client::PlayerState&, misc::NormalLog&, const TargetCandidate&,
                     const AcquisitionEvaluation&, std::int32_t id, std::int32_t command,
                     bool feedback_pending);
void log_jitter_result(client::PlayerStateCache&, misc::NormalLog&, const ShotRecordView&, bool hit,
                       std::uint32_t reason, std::int32_t target = 0, std::int32_t hitgroup = 0,
                       std::int32_t damage = 0);
bool jitter_shot_used(client::PlayerStateCache&, const ShotRecordView&) noexcept;
}
