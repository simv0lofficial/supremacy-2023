#pragma once
#include "skeet/prediction/entity_update.hpp"
#include "skeet/lagcomp/history_owner.hpp"

namespace skeet::client {
class NativeStartupSources;

// Both factories append to the same source lifecycle/entity vectors. All
// referenced contexts and the factory binding record retain stable addresses
// through initialization and the registered owners' terminal cleanup.
struct HistoryRuntimeFactoryBindings {
    std::shared_ptr<engine::NativeDataImage> data;
    engine::OwnerRegistryBinding lifecycle; //43475B78
    EntityObserverRegistry& entities; //43475BA8
    prediction::EngineEntityUpdateBindings prediction;
    lagcomp::HistoryOwnerBindings history;
    lagcomp::HistoryOwnerRuntime*& history_active; //43475AC0
};

inline constexpr std::uint32_t prediction_entity_update_factory = 0x4347E356u;
inline constexpr std::uint32_t lagcomp_history_factory = 0x43484F66u;

//4347E356 allocates native0x18 storage and tail-calls434849D3. The returned
// source allocation owns its physical primary/entity receivers, with no
// singleton publication. The unresolved native INT1 failure edge4347E369 is
// an explicit std::bad_alloc failure in this source entry.
void* create_prediction_entity_update_owner(HistoryRuntimeFactoryBindings&);

//43484F66 allocates native0x60 storage, constructs4347E5DE on success and
// publishes owner/null only afterward. The canonical43475AC0 cell is also
// cleared by the existing433B5B46 source destructor before allocation release.
lagcomp::HistoryOwnerRuntime* create_lagcomp_history_owner(HistoryRuntimeFactoryBindings&);

// Bind concrete source bodies without invoking either constructor or choosing
// a cross-owner order. initialize_runtime_owners supplies the recovered order.
void bind_history_runtime_factories(NativeStartupSources&, HistoryRuntimeFactoryBindings&);
} //namespace skeet::client
