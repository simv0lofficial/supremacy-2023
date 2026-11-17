#pragma once
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include <cstddef>
#include <cstdint>

namespace skeet::prediction {
struct EngineEntityUpdateBindings {
    void* const& engine_client; //4346A514
    void* const& entity_list; //4346A890
    void* const& pending_events; //43475A50, engine event queue, first node+0C
    void* const& base_client; //434683E0, ClientClass list virtual8
    std::uint32_t* const& replacement_property; //43469060, optional live pointer
    std::ptrdiff_t entity_property; //decoded2B554AD0
};
//Semantic view of the43472388 receiver. Its+4 engine interface and+8/+C
//filter identities are initialized by the enclosing434849D3 owner. Engine
//virtual59 dispatches FireEvents after clearing delays for these class IDs.
struct EngineEntityUpdateOwner {
    explicit EngineEntityUpdateOwner(const EngineEntityUpdateBindings& input) noexcept
        : bindings(input) {}
    void* source_interface;
    std::uint32_t first_filter_identity, second_filter_identity;
    EngineEntityUpdateBindings bindings;
};
// Registration borrows the physical receiver embedded in the same complete
// owner as the semantic view. Publication remains at434849F2 in that owner.
void register_engine_entity_update_owner(client::EntityObserverRegistry&,
                                         client::NativeEntityObserverReceiver&,
                                         EngineEntityUpdateOwner&);
//Complete4337D273. Native slot3's player/delta arguments are not consumed.
void update_prediction_entities(EngineEntityUpdateOwner&);
std::size_t engine_entity_update_owner_size() noexcept;
//Whole434849D3, reached by factoryJMP4347E364. Native-aligned storage comes
//from43387448; lifecycle slot0 releases it through the matching header.
//Registration order is primary then BA8; the caller supplies cross-owner order.
EngineEntityUpdateOwner& construct_engine_entity_update_owner(void* native_aligned_storage,
                                                              engine::OwnerRegistryBinding primary,
                                                              client::EntityObserverRegistry&,
                                                              const EngineEntityUpdateBindings&);
} //namespace skeet::prediction
