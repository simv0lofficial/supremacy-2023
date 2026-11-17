#pragma once
#include "entity_state.hpp"
#include "history.hpp"
#include "update_timing.hpp"
#include "skeet/client/player_state.hpp"

namespace skeet::lagcomp {
struct HistoryPreludeOffsets {
    std::uint32_t life_state_9d920e35;
    std::uint32_t suppressed_cde290d2;
    std::uint32_t previous_time_1599541f;
    std::uint32_t weapon_definition_5c9f0fbb;
};
// Complete433255B6 for a valid player. Updates the live timestamp even when
// the life-state/weapon gates retain the previous cached definition.
void capture_history_display_metadata(client::PlayerState&, void* player, const EntityStateRuntime&,
                                      const HistoryPreludeOffsets&);
// Complete4340D000..4340D06F. The actual collideable virtuals supply height;
// the captured origin precedes those calls, as in the native writer.
void cache_history_entity_fields(client::PlayerState&, void* player, const EntityStateRuntime&);
struct CachedHistoryFrame {
    bool ready = false;
    std::int32_t receive_tick = 0;
    void* animation_state = nullptr;
    float animation_speed_fraction = 0;
    SimulationTickSpan ticks{};
};
// Complete per-player4340D000..4340D24A, including the dead/suppressed clear
// exit4341695A..43416991. Caller already passed player/info/dormancy gates.
// This works with the shared0x1700-byte player cache, its single RecordRing,
// and the existing global record pool rather than another player-state owner.
CachedHistoryFrame prepare_cached_history_frame(client::PlayerState&, RecordPool&, void* player,
                                                bool is_local_player, const EntityStateRuntime&,
                                                const HistoryPreludeOffsets&);
} // namespace skeet::lagcomp
