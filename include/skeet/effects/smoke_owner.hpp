#pragma once
#include "world.hpp"
#include "skeet/network/update_observers.hpp"

namespace skeet::effects {
struct SmokeOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding updates; //43475988
};
struct SmokeOwnerBindings {
    WorldRuntime& runtime;
    const SmokeRuntimeBindings& engine;
    WorldRuntime*& active; //canonical4347585C slot
};
std::size_t smoke_owner_size() noexcept;
// Whole ctor43480340..4348037E. One canonical623BD4F2 descriptor; primary
// 43470980 and update4347005C are published after both actual registrations.
void construct_smoke_owner(void* storage, const SmokeOwnerRegistries&, ConfigRegistries&,
                           SmokeOwnerBindings);
} // namespace skeet::effects
