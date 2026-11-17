#pragma once
#include "harmless_molotov.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/network/update_observers.hpp"

namespace skeet::effects {
struct HarmlessMolotovOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78, primary first
    engine::OwnerRegistryBinding updates; //43475988, owner+8 second
};
struct HarmlessMolotovOwnerBindings {
    esp::EffectsSettings& settings;
    const HarmlessMolotovBindings& effects;
};
// Complete ordinary ctor43484AEA..43484CEC. Storage carries the canonical
// 43387448 alignment header; primary slot0 frees it through lifecycle.memory.
// Only the three C82CBD3B/F9200DEF/921CAA0A descriptors are initialized here.
std::size_t harmless_molotov_owner_size() noexcept;
void construct_harmless_molotov_owner(void* storage, const HarmlessMolotovOwnerRegistries&,
                                      ConfigRegistries&, HarmlessMolotovOwnerBindings);
} // namespace skeet::effects
