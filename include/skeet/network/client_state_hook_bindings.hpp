#pragma once
#include "skeet/network/client_state_hooks.hpp"
#include "skeet/network/client_state_cvars.hpp"
#include "skeet/client/startup.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/config_storage/presets.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/misc/logs.hpp"

namespace skeet::network {
// Actual43476900 chain. A matched node is temporarily linked into *target
// around original slot32, then its CURRENT target/saved fields are reread.
struct ClientStateScopedPatch {
    std::uint32_t field_00;
    ClientStateScopedPatch* saved;
    ClientStateScopedPatch** target;
    std::uint32_t field_0c;
    std::int32_t identifier;
    ClientStateScopedPatch* next;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(ClientStateScopedPatch, target) == 8);
static_assert(offsetof(ClientStateScopedPatch, identifier) == 0x10);
static_assert(offsetof(ClientStateScopedPatch, next) == 0x14);
#endif
struct ClientStateHooksBindings {
    void* const& engine_client; //4346A514
    client::ChannelHookState& channel; //canonical43475B08/0C/14
    ClientStateServerCvars& server_cvars;
    ClientStateScopedPatch*& scoped_patches; //43476900
    engine::NativeConVarReference convar_43469238; //live encoded43469238^F7E8922F
    client::StartupConVars variables; //same decoded live slots as startup
    float* const& interval; //4346A338
    std::int32_t& transition_status; //4347C9EC
    float& interpolation_floor; //43469254
    std::span<client::StartupAcknowledgement, 150> acknowledgements;
    client::CommandResetState command_reset;
    engine::OwnerRegistry& owners; //actual source-factory lifecycle objects
    const std::uint8_t& allow_untrusted; //434693EC
    void** const& shared_player; //4346A3A4
    std::ptrdiff_t valve_matchmaking_bf567619;
    ConfigPresetsManager& presets; //canonicalactive_hash4347C9F8
    misc::ErrorLog& errors;
};
} //namespace skeet::network
