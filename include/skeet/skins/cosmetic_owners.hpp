#pragma once
#include "skeet/skins/native_entity_cosmetics.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/engine/owner_registry.hpp"

namespace skeet::skins {
class GloveOwner;
class MaskOwner;
struct CosmeticOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78/7C/80
    client::EntityObserverRegistry& entities; //43475BA8/BAC, actual constructor order
};
struct CosmeticOwnerBindings {
    Settings& settings;
    NativeEntityCosmetics& cosmetics;
    void* const& engine_client; //4346A514, glove primary release tests virtual26
    void* const& local_player; //43475808, read after that connected test
};

// Complete native constructors43490739 and4347EB9D. Both append the primary
// owner and BA8 receiver before publishing their actual tables/configuration.
// They register their own six/four links in the SAME canonical Settings object.
// Those links must be fresh: do not first register them through Settings::initialize.
// Placement storage must be valid and nonnull; primary slot0 expects its
// original43387448 allocation header immediately before that storage.
std::size_t glove_owner_size() noexcept;
std::size_t mask_owner_size() noexcept;
GloveOwner& construct_glove_owner(void* storage, const CosmeticOwnerRegistries&, ConfigRegistries&,
                                  CosmeticOwnerBindings);
MaskOwner& construct_mask_owner(void* storage, const CosmeticOwnerRegistries&, ConfigRegistries&,
                                CosmeticOwnerBindings);

// Only the genuine43387448 allocator boundary: these return its nullable
// result and do not construct or choose a protected factory continuation.
// Native factories43484AC4 (0x8C) and43480644 (0x1E4) tail to the constructors
// on success; their INT1 null continuations remain unrecovered. Source storage
// uses its own size because configuration/state are canonical borrowed objects.
// Pass valid allocated storage to construct_* at the actual startup boundary.
void* allocate_glove_owner_storage(engine::JobMemory);
void* allocate_mask_owner_storage(engine::JobMemory);
NativeEntityCosmetics& glove_owner_cosmetics(GloveOwner&) noexcept;
NativeEntityCosmetics& mask_owner_cosmetics(MaskOwner&) noexcept;
} //namespace skeet::skins
