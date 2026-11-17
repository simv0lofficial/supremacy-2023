#pragma once
#include <cstdint>
#include "skeet/anti_aim/proximity.hpp"
#include "skeet/anti_aim/yaw.hpp"
#include "skeet/client/commands.hpp"

namespace skeet::anti_aim {
// Complete native command-yaw tail 4332EBBC..4332EF25. Native owner offsets
// are absolute (the phase-4 scheduler receives the secondary owner + 8).
struct CommandYawState {
    std::uint32_t flags_130;
    float cached_yaw_18c;
};
void finalize_command_yaw(float& command_yaw, float frame_yaw_24, CommandYawState&) noexcept;
// Cached-only continuation4332EE97..4332EF25: preserve an uncached command's
// current word while computing the normalized value stored in the cache.
void apply_cached_command_yaw(float& command_yaw, CommandYawState&) noexcept;
struct AngleCycleState {
    std::uint32_t flags_130, phase_190;
    float saved_yaw_198;
};
// Native phase-4 prefix4332D1E0..4332D1F4 and cycle block4332D2FF..4332D3A4.
// cached_command is the already recovered player/proximity flag before D2FF.
void update_command_flags(std::uint32_t frame_flags, std::uint32_t& flags_130) noexcept;
void begin_angle_cycle(bool cached_command, std::int32_t choked_commands, std::int32_t spin_offset,
                       AngleCycleState&) noexcept;
struct ActionGateFrame {
    std::uint32_t flags, previous_buttons_44;
    std::int32_t choke_limit_fc, choked_15c, weapon_id_d8;
    std::uint8_t state_104, weapon_kind_124;
    float pitch_20, yaw_24;
};
struct ActionGatePlayer {
    bool weapon_state_e1df7e9b;
    float weapon_time_3b93667b;
    std::int32_t tick_base_3d10b94c, movement_type_8b5586f6;
    bool field_7fcb7381, has_weapon_3d52a1a7;
    ProximityEntity entity;
};
struct ActionGateGlobals {
    std::int32_t mode_43475b2c, choked_434757dc;
    float interval_per_tick;
    std::uint32_t command_flags;
    std::int32_t target_handle;
    EntityLookup entities;
};
struct ActionGateSettings {
    bool enabled;
    std::int32_t pitch_mode, yaw_base;
};
struct ActionGateState {
    std::uint32_t flags_130;
    std::int32_t sequence_158;
};
enum class ActionGatePath : std::uint32_t {
    stop,
    angles,
    cached_yaw
};
// Native main action gates4332D3D2..4332D70D. Returns the actual next native
// continuation; packet/buttons/global state writes occur before the stop path.
ActionGatePath evaluate_action_gates(const ActionGateSettings&, const ActionGateFrame&,
                                     const ActionGatePlayer&, ActionGateGlobals&, ActionGateState&,
                                     client::Command&, std::uint8_t& send_packet, ProximityEngine&);
} // namespace skeet::anti_aim
