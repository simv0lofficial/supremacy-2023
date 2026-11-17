#pragma once
#include "collision.hpp"
#include <array>

namespace skeet {
struct MovementEnvironmentBindings;

// Decoded player reads and actual virtual-method results for the native
// 0x4337D3C8 snapshot. Hash-only fields retain their decoder identity.
struct MovementSimulationPlayer {
    float speed_scale;
    float duck_speed;
    float duck_amount;
    float view_offset_z;
    float stamina;
    MovementVector hull_minimum, hull_maximum;
    std::uint8_t ducking;
    float fall_velocity;
    float surface_friction;
    std::uint8_t movement_type;
    std::uint32_t collision_mask;
    std::uint8_t field_f3c10e80_00;
};

// Constructor cvars, in native read order. Known stop/friction/acceleration
// are named; unresolved cvar names retain their encoding identities.
struct MovementSimulationRules {
    float gravity;
    float jump_impulse;
    float step_size;
    float stamina_recovery;
    float stamina_land;
    float stamina_jump;
    float stamina_maximum;
    float stop_speed;
    float friction;
    float accelerate;
    float cvar_dceaeac8;
    float interval_per_tick;
};

// The simulation is local source state. The player value is an opaque engine
// handle supplied by the adapter, not a host pointer to captured memory.
struct MovementSimulation {
    std::uint32_t player;
    MovementVector position, velocity;
    std::uint32_t player_flags, mode;
    float speed_scale, duck_speed, duck_amount, view_offset_z, stamina;
    MovementVector hull_minimum, hull_maximum;
    MovementVector contact_normal;
    float state_5c;
    MovementVector contact_position;
    MovementCollisionTrace trace;
    std::uint8_t ducking;
    std::array<std::byte, 3> padding_c1;
    float fall_velocity;
    float surface_friction;
    std::uint8_t movement_type;
    std::array<std::byte, 3> padding_cd;
    std::uint32_t collision_mask;
    MovementSimulationRules rules;
    float saved_vertical_velocity;
    std::uint8_t simulation_flags;
    std::array<std::byte, 3> padding_109;
    float maximum_speed;
    std::uint32_t buttons;
    MovementVector command_move;
};
static_assert(offsetof(MovementSimulation, trace) == 0x6c);
static_assert(offsetof(MovementSimulation, rules) == 0xd4);
static_assert(offsetof(MovementSimulation, simulation_flags) == 0x108);
static_assert(sizeof(MovementSimulation) == 0x120);

// Complete 0x433B0D5C..0x433B0FCF and 0x4337D3C8..0x4337D643.
// Fields untouched by the native constructors retain their previous bytes.
void initialize_movement_simulation(MovementSimulation& simulation, std::uint32_t player,
                                    MovementVector position, MovementVector velocity,
                                    std::uint32_t player_flags, std::uint32_t mode,
                                    const MovementSimulationPlayer& snapshot,
                                    const MovementSimulationRules& rules) noexcept;

// First complete stage of 0x433D7606: 0x433D7624..0x433D7868. The two speed
// inputs are the actual decoded player field and player virtual-method result.
void prepare_movement_simulation_command(MovementSimulation& simulation, std::uint32_t player,
                                         float player_speed_limit, float player_max_speed,
                                         const UserCommand& command) noexcept;
void prepare_live_movement_simulation_command(MovementSimulation&, void* player, const UserCommand&,
                                              const MovementEnvironmentBindings&);

// Closing state update 0x433DEFAB..0x433DF09F, before the enclosing trace-flag
// guard is restored. The cvar-presence test is independent of its stored value.
void finish_movement_simulation_tick(MovementSimulation& simulation,
                                     bool landing_stamina_cvar_present,
                                     bool advance_vertical = true) noexcept;

} // namespace skeet
