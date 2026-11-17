#pragma once
#include "rage_geometry.hpp"

namespace skeet {
struct RageShotCandidate {
    std::uint32_t flags;
    RageVector aim_point;
};
struct RageShotState {
    std::uint32_t flags;
    std::int32_t weapon_id;
    float special_gate_threshold;
    RageVector selected_aim_point;
    std::int32_t marker;
};
struct RageShotCommitState {
    std::uint32_t context_flags;
    std::uint32_t command_flags;
    std::int32_t last_committed_marker;
    std::uint8_t send_packet;
};
enum class RageShotCommit {
    attack_button_only,
    recorded
};
// Native accepted-shot commit 0x43355DEF..0x43355EA9. The attack button is
// written before the weapon-64 gate. Later shot history/entity notifications
// after 0x43355EA9 are separate work, not represented as completed here.
RageShotCommit commit_accepted_rage_shot(RageShotState&, RageShotCommitState&, UserCommand&,
                                         const RageShotCandidate&, float special_gate_value,
                                         bool alternate_path, bool send_mode) noexcept;
} // namespace skeet
