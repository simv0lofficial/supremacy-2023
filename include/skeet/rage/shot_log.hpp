#pragma once
#include <cstdint>

namespace skeet {
struct ShotRecordView;
namespace client { struct Command; struct CommandFrame; }
namespace misc { class NormalLog; }
namespace rage {
struct ShotLogSettings;
struct TargetCandidate;
struct AcquisitionEvaluation;

void log_shot_fire(const ShotLogSettings&, misc::NormalLog&, const client::CommandFrame&,
                   const client::Command&, const TargetCandidate&, const AcquisitionEvaluation&,
                   std::int32_t id, bool pending);
void log_shot_result(const ShotLogSettings&, misc::NormalLog&, const ShotRecordView&, bool hit,
                     std::uint32_t reason, std::int32_t target = 0, std::int32_t group = 0,
                     std::int32_t damage = 0);
}
}
