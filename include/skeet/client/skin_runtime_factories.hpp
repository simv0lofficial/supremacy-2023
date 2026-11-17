#pragma once
#include "skeet/skins/agent_owner.hpp"
#include "skeet/skins/cosmetic_owners.hpp"
#include "skeet/skins/inventory_owner.hpp"

namespace skeet::client {
class NativeStartupSources;

struct SkinRuntimeFactoryBindings {
    ConfigRegistries& configuration;
    engine::OwnerRegistryBinding lifecycle; //43475B78
    CommandPhaseRegistry& commands; //434757A0
    EntityObserverRegistry& entity_updates; //43475BA8
    skins::Settings& settings;
    skins::InventoryOwnerBindings inventory;
    skins::CosmeticOwnerBindings cosmetics;
    skins::AgentOwnerBindings agent;
};

inline constexpr std::uint32_t skin_inventory_factory = 0x4347E81Au;
inline constexpr std::uint32_t skin_glove_factory = 0x43484AC4u;
inline constexpr std::uint32_t skin_mask_factory = 0x43480644u;
inline constexpr std::uint32_t skin_agent_factory = 0x4348574Eu;
inline constexpr std::size_t skin_inventory_native_size = 0x43C;
inline constexpr std::size_t skin_glove_native_size = 0x8C;
inline constexpr std::size_t skin_mask_native_size = 0x1E4;
inline constexpr std::size_t skin_agent_native_size = 0x64;

// The four native factories allocate with43387448 and tail to their complete
// constructors. They publish no singleton: their physical owners enter the
// existing registries. Source metadata may require a larger representation.
// Native OOM branches4347E82F/43484AD9/43480659/43485761 enter unresolved
// INT1 paths. Source policy throws bad_alloc there without claiming recovery.
skins::InventoryLifecycleOwner* create_skin_inventory_owner(const SkinRuntimeFactoryBindings&);
skins::GloveOwner* create_skin_glove_owner(const SkinRuntimeFactoryBindings&);
skins::MaskOwner* create_skin_mask_owner(const SkinRuntimeFactoryBindings&);
void* create_skin_agent_owner(const SkinRuntimeFactoryBindings&);

// Bind only these four identities; the live startup operand order chooses
// their construction order. The separate43481F6A sequence factory is unchanged.
void bind_skin_runtime_factories(NativeStartupSources&, SkinRuntimeFactoryBindings&);
} //namespace skeet::client
