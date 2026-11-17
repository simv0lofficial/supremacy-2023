#pragma once
#include "skeet/esp/effects_config.hpp"
#include "skeet/rage/target_relationships.hpp"
#include "skeet/engine/hooks.hpp"
#include <array>
#include <vector>

namespace skeet::effects {
enum class HarmlessMolotovHook {
    creation,
    primary,
    secondary,
    renderable
};
class HarmlessMolotovRuntime;
struct ParticleColorBackup {
    float* address;
    std::array<std::uint32_t, 3> values;
};
struct ParticleAlphaBackup {
    float* address;
    std::uint32_t value;
};
struct HarmlessMolotovState {
    explicit HarmlessMolotovState(bool initialize_storage = true);
    void initialize_native_storage(); //43484B67..43484CDD, after config registration
    void release_native_storage() noexcept; //whole43326010: alpha, color, hashes
    std::array<float, 3> color{}, hsv{};
    float alpha = 1;
    std::vector<std::uint32_t> definition_hashes;
    std::vector<ParticleColorBackup> colors;
    std::vector<ParticleAlphaBackup> alphas;
};
struct HarmlessMolotovBindings {
    void* const& engine; //live4346A514,actual slots82/93
    void* const& entity_list; //live4346A890
    void* const* local_player; //43475808
    std::uint32_t owner_handle; //8D1BD8D0
    const std::uint32_t& encoded_friendly_fire; //live43467C3C XOR211963A6
    const std::uint32_t& encoded_grenade_damage_reduction; //live434685C8 XORD5F0540C
    rage::TargetRelationshipBindings relationships;
    engine::VtableHookRegistry& hook_registry;
    std::array<engine::VtableHookMetadata*, 3> entity_hooks; //43475908/434758F4/434757F4
    HarmlessMolotovRuntime*& active; //one canonical43475828 slot, shared with factory
};
// Whole433C8A72 and433CEAAB. Particle traversal, HSV interpolation, backup
// and restoration execute in source; only real engine virtuals are bound.
class HarmlessMolotovRuntime {
public:
    const esp::EffectsSettings& settings;
    HarmlessMolotovState& state;
    const HarmlessMolotovBindings& bindings;
    void apply_collection(void* collection); //complete433AD7B0, recursive children
    std::uint32_t render(void* receiver, std::uintptr_t first, std::uintptr_t second);
    std::uint32_t created(void* receiver, const char* name, void* created);
    std::uint32_t destroy_primary(void* receiver, std::uintptr_t flags); //433E7C16
    std::uint32_t destroy_secondary(void* receiver, std::uintptr_t flags); //4340B2C6
    // The RTTI producer supplies descriptor43475AC4 (creation110) plus the
    // canonical three entity descriptors above. No second table is allocated.
    void bind_tables(engine::VtableHookMetadata& creation);
    // Independent descriptor preparation must not publish the feature owner.
    void bind_table(HarmlessMolotovHook, engine::VtableHookMetadata&);
    static void bind_static_table(HarmlessMolotovHook, engine::VtableHookMetadata&);
    void publish() noexcept; //canonical43475828 publication, separate from RTTI preparation
    void unbind() noexcept;
};
void unbind_harmless_molotov_runtime() noexcept;
} // namespace skeet::effects
