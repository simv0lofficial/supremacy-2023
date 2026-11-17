#pragma once
#include "simulation_tick.hpp"
#include "speed_control.hpp"
#include "skeet/client/commands.hpp"

namespace skeet {
struct MovementCommandBindings;

struct WeaponSpeedPhaseSettings {
    std::uint32_t policy_flags; // 43467E40; weapon state replaces bits 1..3
};

struct WeaponSpeedPhasePlayer {
    std::uint32_t entity;
    MovementPhysics physics;
    MovementVector velocity;
};

struct WeaponSpeedPhaseFrame {
    WeaponSpeedPhasePlayer player;
    MovementVector position;
    std::uint32_t simulation_mode;
    MovementSimulationPlayer snapshot;
    MovementSimulationRules rules;
    MovementSimulationEnvironment simulation_environment;
    float current_time;
    float player_next_attack; // decoded DE530A8B field
    float weapon_next_attack; // decoded F71CE331 field
    float attack_guard_8815d240;
    std::int32_t weapon_state; // 43468488; -1 means no per-weapon override
    std::uint16_t weapon_definition;
    bool weapon_blocked; // decoded B8969256 weapon byte
};

enum class WeaponSpeedPhaseContinuation {
    player_movement, // 43372C76
    complete, // 4337CAA4
};

// Native 4337206A..43372C6A, after the preceding ground/weapon gates.
// Simulation and acceleration execute recovered source directly. Collision
// remains the actual engine boundary; frame members are decoded engine reads.
// The full copied command supplies simulation angles and the signed sequence
// number used by the two periodic regulators.
WeaponSpeedPhaseContinuation
process_weapon_speed_phase(const WeaponSpeedPhaseSettings&, const WeaponSpeedPhaseFrame&,
                           client::Command&, MovementAngles command_angles,
                           MovementAngleView movement_angles, std::uint32_t& scheduler_flags,
                           MovementCollisionEngine& collision);

// Live 4337206A..43372C6A. Player/weapon reads, constructor getters, cvars
// and acceleration inputs remain at their original branch sites. Command
// view angles remain a live reference through regulator engine calls.
WeaponSpeedPhaseContinuation process_live_weapon_speed_phase(void* player, void* weapon,
                                                             client::Command&,
                                                             MovementAngleView movement_angles,
                                                             const MovementCommandBindings&);

} // namespace skeet
