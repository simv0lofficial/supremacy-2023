#pragma once
#include "records.hpp"

namespace skeet::lagcomp {
struct SimulationTickSpan {
    std::int32_t first, last, count;
};
// Complete4340D1B4..4340D24A. Differences wrap as32-bit integers, clamp0..16,
// and move the beginning forward when a long simulation gap was clamped.
SimulationTickSpan simulation_tick_span(float previous_time, float simulation_time,
                                        float interval) noexcept;

struct SameTickHistoryState {
    std::int32_t simulation_override_receive_tick; // owner+1E4
    std::int32_t simulation_override_tick; // +1E0
    std::int32_t velocity_receive_tick; // +1DC
    Vector velocity; // +1C8
};
// Complete nonlocal correction4340D0DF..4340D16A, before the independently
// recovered player display-cache capture. The history/player simulation fields
// are written together; velocity has its own receive-tick equality gate.
struct SameTickRestoreWrites {
    bool simulation = false, velocity = false;
};
SameTickRestoreWrites restore_same_tick_history_fields(const SameTickHistoryState&,
                                                       std::int32_t receive_tick, float interval,
                                                       float& history_simulation_time,
                                                       float& player_simulation_time,
                                                       Vector& player_velocity) noexcept;

struct UpdateDiscontinuity {
    std::int32_t saved_receive_tick_1a4;
    std::int32_t receive_tick;
    std::uint32_t entity_effects_38ca5228;
    std::uint8_t field_043fa575;
    std::uint8_t field_334b460c;
    std::int32_t velocity_receive_tick_1dc;
    std::int32_t next_receive_tick;
};
// Complete4340D2B2..4340D357; engine[14] receive ticks are independently
// sampled at4340D296 and4340D32B, so the two input ticks remain distinct.
std::uint32_t initial_history_update_flags(const UpdateDiscontinuity&) noexcept;

bool history_weapon_is_firearm(std::uint16_t identifier) noexcept; // complete433D24BB
struct HistoryWeaponShot {
    bool available;
    float time;
    std::uint16_t identifier;
};
// Complete4340D357..4340D44C after the actual entity-list/weapon property
// lookup. Returns the new shot tick or-1. No-update and failed weapon lookup
// retain exactly the native flag bits rather than clearing an unrelated state.
std::int32_t update_history_shot_flags(std::uint32_t& owner_flags, std::uint32_t& update_flags,
                                       SimulationTickSpan, HistoryWeaponShot,
                                       float interval) noexcept;

struct CollisionTransition {
    std::int32_t received_updates_1d4;
    float saved_collision_height_11c;
    float field_12c;
    float field_148;
};
// Numeric write branch4340D494..4340D4F2. Caller invokes player[77] only when
// received_updates>1 and span.count!=0, as in4340D44C..4340D494.
void update_history_collision_transition(CollisionTransition&, SimulationTickSpan,
                                         float current_collision_height, float origin_z,
                                         float previous_time, float interval) noexcept;
// Complete4340D548..4340D577, comparing the13 captured seven-dword layers.
bool history_animation_layers_changed(const std::array<AnimationLayerState, 13>& current,
                                      const std::array<AnimationLayerState, 13>& previous) noexcept;
} // namespace skeet::lagcomp
