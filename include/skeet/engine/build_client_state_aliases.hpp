#pragma once
#include "native_data_image.hpp"

namespace skeet::engine {
struct BuildClientStateAliases {
    std::int32_t* choked_commands;
    std::int32_t* last_outgoing_command;
    std::int32_t* server_tick;
    std::int32_t* command_tick;
    float* real_time;
    std::int32_t* tick_count;
    std::int32_t* maximum_clients;
    std::int32_t* acknowledged_tick;
    std::int32_t* outgoing_tick;
};

// Run after initialize_build_game_state_bindings. Verify the actual engine
// build and its current-record/member operands before publishing the live
// client-state/global aliases. No pointed tick, command or time is copied.
// The acknowledged/outgoing cells name engine-owned counters, not commands.
// Also publishes434758D8 as the current record's delta-tick member, shared by
// SendNetMsg, incoming updates, prediction repair and local-player creation.
// Publishes43475A50 as the engine's pending event queue used by FireEvents.
BuildClientStateAliases initialize_build_client_state_aliases(NativeDataImage&);
} // namespace skeet::engine
