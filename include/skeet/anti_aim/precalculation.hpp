#pragma once
#include "skeet/anti_aim/config.hpp"
#include "skeet/anti_aim/state.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/movement/simulation_tick.hpp"
#include "skeet/anti_aim/movement_capture.hpp"

namespace skeet::anti_aim {
struct PrecalculationState {
    State& common;
    float feet_yaw_194;
    float micro_move_19c = 1.01f;
    float previous_duck_1a4;
};
struct PrecalculationPlayer {
    std::uint32_t flags_e2fbe695;
    bool has_animation_state;
    float feet_yaw_80;
    bool guard_d720aea6; //433DF22C: shared-player field, false when absent
    std::uint8_t movement_type_7501cb91;
    MovementVector velocity_36375c23;
    float duck_3309c0cf, rate_fb7e3404;
    bool field_6b1a59b5;
};
// Shared native micro-movement block4339F5EF..4339F727, also consumed by
// generated-command owner43320B61. Previous duck publication remains separate.
void apply_micro_movement(PrecalculationState&, const PrecalculationPlayer&, float interval,
                          client::Command&) noexcept;
struct PrecalculationLiveFields {
    std::uint32_t duck_3309c0cf, rate_fb7e3404, field_6b1a59b5, flags_e2fbe695;
    std::int32_t* const* choked;
    float* const* interval;
};
struct PrecalculationBindings {
    void* player; // actual origin getter virtual10
    std::uint32_t player_value;
    const MovementSimulationPlayer& movement_player;
    const MovementSimulationRules& movement_rules;
    const MovementSimulationEnvironment& movement_environment;
    MovementCollisionEngine& collision;
    MovementSimulation& workspace;
    const client::ScriptCommandEnvironment& roll_guard;
};
struct LivePrecalculationBindings {
    void* player;
    const MovementCaptureInput& movement;
    const MovementEnvironmentBindings& environment;
    MovementCollisionEngine& collision;
    MovementSimulation& workspace;
    const client::ScriptCommandEnvironment& roll_guard;
    const PrecalculationLiveFields& fields;
};
// Complete phase2 owner4339F3BF. Choked count is *434757DC, independently
// of the later phase4 frame count. The caller retains the native scratch state.
void precalculate_command_angles(const GlobalConfig&, const Config&, PrecalculationState&,
                                 const PrecalculationPlayer&, std::int32_t choked_commands,
                                 client::Command&, const std::array<float, 3>& incoming_angles,
                                 std::uint8_t& send_packet, const PrecalculationBindings&);
// Raw command owner always supplies live constructor/tick/field bindings.
// The decoded entry above is a separate semantic API, never a runtime fallback.
void precalculate_live_command_angles(const GlobalConfig&, const Config&, PrecalculationState&,
                                      const PrecalculationPlayer&, std::int32_t choked_commands,
                                      client::Command&, const std::array<float, 3>& incoming_angles,
                                      std::uint8_t& send_packet, const LivePrecalculationBindings&);
} // namespace skeet::anti_aim
