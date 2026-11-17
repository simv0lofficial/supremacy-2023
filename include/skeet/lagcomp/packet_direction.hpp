#pragma once
#include "history.hpp"

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
struct HistoryPacketDirection {
    Vector direction; // owner16C/170/174
    float time; // owner19C
};
// Complete4340EC1A..F047 once the native enemy/mode branch is selected.
// Uses all six packet slots, and can redirect the newest record's horizontal
// velocity while retaining its speed. Returns whether direction/time changed.
bool update_history_packet_direction(HistoryPacketDirection&, const LagTimeline&, RecordRing&,
                                     std::uint32_t update_flags, std::int32_t last_simulation_tick,
                                     float interval) noexcept;
bool update_history_packet_direction(client::PlayerState&, std::uint32_t update_flags,
                                     std::int32_t last_simulation_tick, float interval) noexcept;
} // namespace skeet::lagcomp
