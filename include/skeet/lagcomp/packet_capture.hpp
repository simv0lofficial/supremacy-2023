#pragma once
#include "update_capture.hpp"

namespace skeet::lagcomp {
LagTimeline history_timeline(const client::PlayerState&) noexcept;
struct HistoryPitchOverride {
    bool enabled; // per-player table+4
    std::int32_t pitch; // per-player table+C
};
//4340E58B..E6A1 for an enemy. The resolver branch requires a nonzero
// simulation span; the final pitch override also runs for zero-span enemies.
// The enclosing non-enemy path skips this entire publication block.
void publish_history_eye_angles(client::PlayerState&, void* player, bool resolver_branch,
                                HistoryPitchOverride, const EntityStateRuntime&);
// Complete4340E6A1..E7BF. A changed set of 91 layer words publishes one of
// six packet samples. The engine time query occurs after the first six words
// of the new sample have been written, before receive tick and reference yaw.
void capture_history_packet(client::PlayerState&, bool layers_changed, void* player,
                            const EntityStateRuntime&, std::uint32_t field_e2fbe695_offset);
} // namespace skeet::lagcomp
