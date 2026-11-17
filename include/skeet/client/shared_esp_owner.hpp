#pragma once
#include "command_empty_tick.hpp"
#include "command_owner_dispatch.hpp"
#include "entity_observer_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/network/shared_esp.hpp"

namespace skeet::client {
class NativeStartupSources;

struct SharedEspOwnerRegistries {
    engine::OwnerRegistryBinding primary; //43475B78,433676A0
    CommandPhaseRegistry& commands; //434757A0,43345EBC
    engine::OwnerRegistryBinding messages; //43475B94,4336F73C
    EntityObserverRegistry& entities; //43475BA8,4333BF32
    CommandEmptyTickRegistry& empty_ticks; //43475978,433F10DB
    events::EventRegistry& events;
};
struct SharedEspConstructorGlobals {
    void* const& engine_client; //4346A514, recorded MOV43485E86
    const std::uint32_t& encoded_send_gate_a; //43469068 xor7B3564BB
    const std::uint32_t& encoded_send_gate_b; //43468D88 xor584C3C72
    const std::uint32_t& encoded_game_type; //43467A8C xorAC9299CC
    const std::uint32_t& encoded_field_b0; //43467A4C xorFA2DD713
    const std::uint32_t& encoded_field_b4; //43468EA8 xorAC34F836
    // Actual loaded-engine CALL locations. The constructor consumes each
    // nonnull slot before reading its relative target, exactly as recorded.
    const std::byte*& message_constructor_call; //4346D668
    const std::byte*& receiver_constructor_call; //4346D66C
};
struct SharedEspOwnerBindings {
    RejectedPlayerUpdateOwner& state;
    esp::SharedEspConfig& settings;
    SharedEspCommandOwner& command;
    network::SharedEspRuntime& incoming;
    RejectedPlayerUpdateOwner*& active; //canonical434758D0 cell
    const SharedEspConstructorGlobals& globals;
};

std::size_t shared_esp_owner_size() noexcept;
// Whole43485DC9. Registrations retain native primary -> command -> network
// message -> BA8 -> empty-tick order. Only the two owner-local descriptors
// (activation7174D684, other-team6834D5DA) are registered here. The enabled
// and restrict-updates descriptors remain the same separate static globals.
// Placement construction preserves fields not assigned by the native body.
void* construct_shared_esp_owner(void* storage, const SharedEspOwnerRegistries&, ConfigRegistries&,
                                 SharedEspOwnerBindings);
// Whole4348DA78: allocate0x2B8 with43387448's zeroed/aligned heap protocol,
// construct if nonnull, then publish state/null at the recorded4348DA91 MOV.
// Registration in NativeStartupSources does not assert VM invocation order.
void* create_shared_esp_owner(const SharedEspOwnerRegistries&, ConfigRegistries&,
                              SharedEspOwnerBindings);
// Whole433801D6 ->433837D0. Clears the active cell before the null test and
// releases this allocation; registered receiver ranges have their own cleanup.
void destroy_shared_esp_owner(void* storage, RejectedPlayerUpdateOwner*& active);
// Primary slot7=433C0954 clears only geometry.last_live_tick for indices1..64.
void reset_shared_esp_owner(void* storage);

struct SharedEspInitialization {
    const SharedEspOwnerRegistries& registries;
    ConfigRegistries& configuration;
    SharedEspOwnerBindings bindings;
};
void bind_shared_esp_initializer(NativeStartupSources&, SharedEspInitialization&);
} // namespace skeet::client
