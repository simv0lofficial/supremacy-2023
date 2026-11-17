#pragma once
#include "command_runtime.hpp"
#include "ground_phase.hpp"
#include "simulation_environment.hpp"
#include "weapon_speed_phase.hpp"
#include "player_air_gate.hpp"
#include "simulation_capture.hpp"
#include "skeet/client/command_orchestrator.hpp"
#include "skeet/prediction/history.hpp"
#include "skeet/rage/ballistics.hpp"

namespace skeet {

struct MovementCommandFields {
    std::uint32_t velocity_36375c23;
    std::uint32_t movement_type_7501cb91;
    std::uint32_t player_state_ac7f2913;
    std::uint32_t stamina_2f33c681;
    std::uint32_t duck_time_04b798c6;
    std::uint32_t weapon_simulation_time_0176a381;
    std::uint32_t player_next_attack_de530a8b;
    std::uint32_t weapon_next_attack_f71ce331;
    std::uint32_t attack_guard_8815d240;
    std::uint32_t weapon_blocked_b8969256;
    std::uint32_t weapon_definition_5c9f0fbb;
    std::uint32_t simulation_mode_3d10b94c;
    std::uint32_t origin_a9ee6062;
    std::uint32_t shot_counter_8cfc633a;
};

// Bind the actual globals/engine objects. None of the stage producers is an
// internal callback; the scheduler calls each one at its recovered read site.
struct MovementCommandBindings {
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& simulation;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* const& engine_client;
    void* const& surface_properties;
    InputBitmap input;
    MovementCommandFields fields;
    const client::CommandBudgetBindings& budget;
    const HotkeyBinding& duck_peek_hotkey;
    const std::uint8_t& infinite_duck;
    const HotkeyBinding& slow_motion_hotkey;
    const HotkeyBinding& command_mark_hotkey; //43467F44/48
    BunnyHopGuard bunny_hop_guard;
    engine::ConVarBinding duck_delay; //43467D7C xor4A285142
    engine::NativeConVarReference pre_speed_guard; //43468EA4 xorFCD2DA7A
    const float& current_time; //*43467EB8
    const float& engine_time; //*4346A334
    const std::int32_t& last_acknowledged; //*43475B2C
    const std::int32_t& history_command; //4346DB1C
    const std::int32_t& history_available; //4346DB2C
    const std::int32_t& weapon_state; //43468488
    const std::uint32_t& weapon_policy; //43467E40
    const std::uint8_t& periodic_gate_4346a740;
    const std::int32_t& periodic_gate_43467f60;
    const std::int32_t& periodic_gate_434685b8;
    client::EntityCommandOwner* const& history_owner; //actual43475898 owner
    std::span<std::uint32_t, 150> marked_commands;
    std::uint32_t& scheduler_flags;
};

std::uint32_t movement_command_player_flags(void*, const MovementCommandBindings&) noexcept;
std::uint8_t movement_command_player_type(void*, const MovementCommandBindings&) noexcept;
MovementVector movement_command_velocity(void*, const MovementCommandBindings&) noexcept;
MovementVector movement_command_position(void*);
const MovementVector& movement_command_position_ref(void*);
std::uint32_t movement_command_trace_mask(void*);
PlayerAirGateBindings capture_player_air_gate_bindings(const MovementCommandBindings&) noexcept;
bool movement_command_weapon_blocked(void*, const MovementCommandBindings&) noexcept;
float movement_command_player_attack(void*, const MovementCommandBindings&) noexcept;
float movement_command_weapon_attack(void*, const MovementCommandBindings&) noexcept;
float movement_command_attack_guard(void*, const MovementCommandBindings&) noexcept;
std::uint16_t movement_command_weapon_definition(void*, const MovementCommandBindings&) noexcept;
float movement_command_convar_float(const void*) noexcept;

struct MovementCommandFriction {
    float stop_speed, friction, interval;
};
// Native friction loops read stop, friction and interval anew each iteration.
MovementCommandFriction capture_movement_command_friction(const MovementCommandBindings&) noexcept;

// Duck prefix reads are gated before the weapon virtual6C4 call. The native
// button write happens before that call, and its cvar read happens afterwards.
void process_live_duck_peek(MovementCommandOwner&, void* player, void* weapon, UserCommand&,
                            std::uint8_t& send_packet, const MovementCommandBindings&);

struct MovementCommandHull {
    MovementVector minimum, maximum;
    std::uint32_t mask;
};
// Origin probe: player260 -> player00C/collideable008 -> player00C/004.
// Maximum/minimum pointer aliases are copied after both collideable calls.
MovementCommandHull capture_quick_peek_origin_hull(void* player);
// Distance clamp: player130 -> player134; copy both vectors before player260.
MovementCommandHull capture_quick_peek_limit_hull(void* player);
QuickPeekHistory capture_quick_peek_history(void*, std::int32_t command,
                                            const MovementCommandBindings&);
bool movement_command_quick_peek_shot(const QuickPeekState&, void* player,
                                      const MovementCommandBindings&) noexcept;

// Only player450 plus its following3EB71843 read. Friction/weapon values are
// separate stage reads; this must be called again after a simulation getter.
MovementPhysics capture_movement_command_speed(void*, const MovementCommandBindings&);
GroundAcceleration movement_command_ground_acceleration(void* player, const UserCommand&,
                                                        MovementVector velocity,
                                                        MovementVector direction, float wish_speed,
                                                        const MovementCommandBindings&);
bool write_live_movement_counter(void* player, UserCommand&, MovementVector velocity,
                                 MovementAngleView, const MovementCommandBindings&);

} // namespace skeet
