#pragma once
#include "clan_tag.hpp"
#include "config.hpp"
#include "network_console_handlers.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/engine/owner_registry.hpp"

namespace skeet::misc {
// Complete4336E573, the outgoing-message endpoint of owner4347E092. changed
// is the same live byte temporarily set around ClanTagRuntime::set_tag.
void process_clan_tag_message(ClanTagRuntime&, const client::ScriptCommandEnvironment&,
                              NetworkMessageCall&);

// Native4347E092: primary lifecycle owner, four-entry43475BA8 receiver at+8,
// outgoing43475B94 message endpoint at+10, then descriptor9F6C4676 and state.
// Runtime and configuration are canonical external storage, shared with the
// menu and the same latency producer. Storage must be native43387448 heap
// storage; the lifecycle slot0 uses the original433837D0 allocation header.
std::size_t clan_tag_owner_size() noexcept;
void construct_clan_tag_owner(void* storage, engine::OwnerRegistryBinding owners,
                              client::EntityObserverRegistry& entity_updates,
                              engine::OwnerRegistryBinding messages, ConfigRegistries&,
                              ClanTagConfig&, ClanTagRuntime&,
                              const client::ScriptCommandEnvironment&);
} //namespace skeet::misc
