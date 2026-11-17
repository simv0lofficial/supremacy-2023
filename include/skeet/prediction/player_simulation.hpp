#pragma once
#include "skeet/client/commands.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::prediction {
struct ObserverRegistry;
// Inline player+decoded5DF1A4BC command context. Slot3 observers receive this
// base; slots0/1 receive &command. sequence_number at+68 is distinct from the
// embedded command.number at context+8.
struct PlayerCommandContext {
    std::uint8_t needs_processing;
    std::array<std::byte, 3> padding_01;
    client::Command command;
    std::int32_t sequence_number;
};
static_assert(sizeof(PlayerCommandContext) == 0x6C);
static_assert(offsetof(PlayerCommandContext, command) == 4);
static_assert(offsetof(PlayerCommandContext, sequence_number) == 0x68);

struct PlayerSimulationOffsets {
    std::ptrdiff_t command_context; // 5DF1A4BC
    std::ptrdiff_t simulation_enabled; // D070684C, byte
    std::ptrdiff_t rejection_byte; // 9D920E35
    std::ptrdiff_t simulation_tick; // A249061A, dword
    std::ptrdiff_t tick_base; // 3D10B94C
    std::ptrdiff_t attack_guard; // 53E7C7D2, byte
    std::ptrdiff_t recoil_angles; // 1BAE1168
    std::ptrdiff_t scaled_recoil_pitch; // 66146802
};
struct PlayerSimulationBindings {
    void* engine_client; // 4346A514
    void** original_player_table; // player hook metadata[-2]+8
    const std::int32_t* simulation_tick; // *43468D4C
    engine::NativeConVarReference recoil_scale_convar; //live4346794C XOR C429E1BF
    ObserverRegistry& observers; // one actual43475AB0 registry
    PlayerSimulationOffsets offsets;
};
// Complete player PhysicsSimulate hook433EB3F4..433EB6F1. ECX is a nonnull
// player and the original engine method has no stack arguments (slot155).
// Sentinel contexts notify slot3 forward and skip the original method. Other
// accepted contexts run capture forward, tick adjustment forward, original,
// then finish reverse. Recoil scaling belongs only to the rejected gate path.
void simulate_player_command(void* player, const PlayerSimulationBindings&);
// Production entry: resolve the current receiver's installed[-2] metadata at
// each actual original155 call, after any preceding prediction observers.
// The explicit-table fixture entry above retains its separate contract.
void simulate_hooked_player_command(void* player, const PlayerSimulationBindings&);
} // namespace skeet::prediction
