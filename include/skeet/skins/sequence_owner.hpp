#pragma once
#include "skeet/skins/sequence_receive.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/events/feature_listeners.hpp"

namespace skeet::skins {
class NativeSequenceRuntime;
// The physical owner+8 observer registered at4348DDD4. Its borrowed runtime
// is the same producer used by the receive-proxy owner and sequence history.
struct SequenceEntityObserverReceiver : client::NativeEntityObserver {
    NativeSequenceRuntime* runtime = nullptr;
};
struct SequenceOwnerBindings {
    // Receive, raw-update and death-listener bindings borrow these same two
    // canonical objects; the viewmodel-lag producer borrows state.history.
    Settings& settings;
    SequenceRuntime& state;
    SequenceReceivers& receive;
    NativeSequenceRuntime& update;
    void* const& base_client;
    void* const& entity_list;
    void* const& string_tables;
    // Captured native initialization globals. Their loads are restored INT3
    // holes at4348E019/4348E021; original instruction bytes do not contain them.
    const std::uint32_t& viewmodel_class;
    const std::uint32_t& knife_class;
    engine::RecvVarProxy& viewmodel_model;
    events::DeathWeaponIconsListener& death_listener;
    events::EventRegistry& events;
};
// Canonical owner4348DDB6 and initializer43481F6A. Storage is an existing
// native-header allocation, as required by the primary lifecycle destructor.
// Constructor order is caller-provided; this does not reorder protected setup.
// The four knife descriptors are registered here, using the same Settings
// fields consumed by SkinsMenu. Do not also initialize those same links through
// Settings::initialize before constructing this native owner.
std::size_t sequence_owner_size() noexcept;
void construct_sequence_owner(void* storage, engine::OwnerRegistryBinding owners,
                              client::EntityObserverRegistry& entity_updates, ConfigRegistries&,
                              SequenceOwnerBindings);
// Complete43481F6A allocation/publication wrapper using the recovered native
// allocation header. Null allocation clears the same singleton proxy binding.
void* initialize_sequence_owner(engine::OwnerRegistryBinding owners,
                                client::EntityObserverRegistry& entity_updates, ConfigRegistries&,
                                SequenceOwnerBindings);
} //namespace skeet::skins
