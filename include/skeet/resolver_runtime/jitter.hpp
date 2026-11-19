#pragma once
#include <array>
#include <cstdint>

namespace skeet::lagcomp {
struct LagRecordHeader;
struct RecordRing;
}

namespace skeet::resolver_runtime {
struct JitterPoseEstimate {
    float anchor = 0, error = 0, margin = 0;
    std::uint8_t attempt = 0xff;
    bool valid() const noexcept { return attempt < 3; }
};
JitterPoseEstimate estimate_jitter_pose(float eye_yaw, float animation_yaw,
                                        float maximum_yaw) noexcept;
enum class JitterDecision : std::uint8_t {
    not_applied,
    animation_yaw,
    shot_feedback,
    environment,
    movement,
    shared,
    forced_yaw,
    shot_record,
    correction_disabled,
    ambiguous_pose,
    unchanged_pose,
    history_pose
};
const char* jitter_decision_name(JitterDecision) noexcept;
struct JitterRecordEvidence {
    std::int32_t tick = -1;
    float observed_yaw = 0;
    JitterPoseEstimate estimate;
    JitterDecision decision = JitterDecision::not_applied;
    std::int32_t receive_tick = -1;
    bool history_checked = false;
    bool applied() const noexcept {
        return estimate.valid() &&
               (decision == JitterDecision::animation_yaw || decision == JitterDecision::history_pose);
    }
    bool applied_to(const lagcomp::LagRecordHeader&) const noexcept;
};
struct JitterLoggedShot {
    std::int32_t id = -1, command = -1, tick = -1;
    bool pending = false;
    bool resolved = false;
};
// Source-owned metadata; the recovered native player and record layouts stay
// unchanged. A duplicate capture gets its own evidence until it enters the ring.
struct JitterTracking {
    JitterRecordEvidence current;
    JitterRecordEvidence fallback;
    std::array<JitterRecordEvidence, 32> records{};
    std::array<JitterLoggedShot, 32> shots{};
    std::uint32_t next_record = 0, next_shot = 0;
    void remember(const JitterRecordEvidence&) noexcept;
    const JitterRecordEvidence* find(const lagcomp::LagRecordHeader&) const noexcept;
};
void correct_jitter_history(lagcomp::RecordRing&, JitterTracking&) noexcept;
} // namespace skeet::resolver_runtime
