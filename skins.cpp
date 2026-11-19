// Skin Changer Subsystem Implementation
#include "skeet/skins/agent_owner.hpp"
#include "skeet/network/data_update_observers.hpp"
#include <array>
#include <cstring>
#include <new>
#include <optional>
#include "skeet/skins/agent_receive.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/engine/interface_call.hpp"
#include <bit>
#include "skeet/skins/agents.hpp"
#include "skeet/skins/apply.hpp"
#include <algorithm>
#include "skeet/skins/command.hpp"
#include "skeet/skins/widget_setters.hpp"
#include "skeet/menu/skins_menu.hpp"
#include "skeet/menu/legit_menu.hpp"
#include "skeet/menu/rage_weapon_selector.hpp"
#include "skeet/menu/rage_menu.hpp"
#include "skeet/menu/root_menu.hpp"
#include <stdexcept>
#include "skeet/skins/config.hpp"
#include "skeet/skins/cosmetic_owners.hpp"
#include "skeet/skins/masks.hpp"
#include "skeet/skins/gloves.hpp"
#include "skeet/skins/inventory.hpp"
#include "skeet/skins/inventory_owner.hpp"
#include "skeet/skins/knife_precache.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/skins/model_resources.hpp"
#include "skeet/events/game_events.hpp"
#include "skeet/skins/models.hpp"
#include "skeet/skins/native_agents.hpp"
#include "skeet/skins/native_catalog.hpp"
#include <cstdlib>
#include "skeet/skins/native_entity_cosmetics.hpp"
#include "skeet/skins/native_inventory.hpp"
#include "skeet/skins/native_schema.hpp"
#include "skeet/config_storage/owner.hpp"
#include <windows.h>
#include <winternl.h>
#include "skeet/skins/native_sequence_update.hpp"
#include "skeet/misc/viewmodel_lag_owner.hpp"
#include <xmmintrin.h>
#include "skeet/skins/refresh.hpp"
#include <limits>
#include "skeet/skins/runtime_owner.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/engine/build_skin_receive_patches.hpp"
#include <map>
#include <string>
#include "skeet/skins/sequence_owner.hpp"
#include "skeet/skins/sequence_receive.hpp"
#include "skeet/skins/sequences.hpp"
#include "skeet/skins/stattrak.hpp"


#include "util/address.h"

// ============================================================================
// Subcomponent: agent_owner.cpp
// ============================================================================

namespace skeet::skins {
using skeet::field;
using skeet::read;
using skeet::write;
namespace {
inline std::uint32_t entity_index(std::uint32_t handle) noexcept {
    return (handle == 0xFFFFFFFF || handle == invalid_glove_handle) ? 0x1FFF : handle & 0xFFFF;
}
}
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define AGENT_MEMBER __thiscall
#else
#define AGENT_MEMBER
#endif
using OwnerEntry = void(AGENT_MEMBER*)(void*);
struct AgentOwner;
void AGENT_MEMBER destroy(void*);
void AGENT_MEMBER agent_unchanged(void*) {} //433CCE8E, all remaining primary slots
const std::array<OwnerEntry, 9> agent_lifecycle_table{destroy,   agent_unchanged, agent_unchanged,
                                                agent_unchanged, agent_unchanged, agent_unchanged,
                                                agent_unchanged, agent_unchanged, agent_unchanged};

struct AgentOwner {
    const OwnerEntry* vtable = nullptr;
    AgentOwnerBindings bindings;
    engine::JobMemory memory;
    std::optional<NativeAgentRuntime> runtime;
    std::array<std::int32_t, 2> constructor_cells_5c_60;
    AgentOwner(engine::OwnerRegistryBinding owners, ConfigRegistries& registry,
               AgentOwnerBindings input)
        : bindings(input), memory(owners.memory) {
        owners.append(this); //4347DAB2, original primary pointer is still zero
        const auto& capture = bindings.receiver.capture_globals();
        const auto& receive = bindings.receiver.receive_bindings();
        const NativeAgentBindings context{receive.engine_client,   bindings.entity_list,
                                          capture.original_model,  capture.original_handle,
                                          bindings.support_models, bindings.offsets};
        // NativeAgentRuntime's mandatory constructor append is deliberately
        // delayed until after the primary owner, preserving 4347DAB7 ordering.
        runtime.emplace(bindings.settings, bindings.catalog, context, bindings.models,
                        bindings.tables, bindings.observers);
        vtable = agent_lifecycle_table.data();
        bindings.observers.publish(*runtime); //4347DACA, after primary4347DAC2
        auto& settings = bindings.settings;
        initialize_boolean_config(registry.descriptors, settings.agent_t, 0x191A5A61);
        initialize_boolean_config(registry.descriptors, settings.agent_ct, 0x6CAF4832);
        initialize_integer_config(registry.descriptors, settings.agent_model_t, 0x3EB3F20B, 0);
        initialize_integer_config(registry.descriptors, settings.agent_model_ct, 0x8191D9BC, 0);
        constructor_cells_5c_60 = {-1, -1};
        // These two owner-local words are not the receive proxy's global
        // capture cells. The original constructor does not reset those cells.
    }
};
void AGENT_MEMBER destroy(void* object) { //433B1A7A ->433837D0
    if (!object)
        return;
    auto* owner = static_cast<AgentOwner*>(object);
    const auto memory = owner->memory;
    owner->~AgentOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
#undef AGENT_MEMBER
}
std::size_t agent_owner_size() noexcept {
    return sizeof(AgentOwner);
}
void construct_agent_owner(void* storage, engine::OwnerRegistryBinding owners,
                           ConfigRegistries& registry, AgentOwnerBindings bindings) {
    ::new (storage) AgentOwner(owners, registry, bindings);
}
void* allocate_agent_owner_storage(engine::JobMemory memory) {
    const auto size = sizeof(AgentOwner);
    auto* allocation = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!allocation)
        return nullptr;
    const auto base = reinterpret_cast<std::uintptr_t>(allocation);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - base);
    const auto allocation_size = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void bind_agent_owner_receiver(void* object, client::EntityUpdatePatch& patch) noexcept {
    bind_agent_model_receiver(static_cast<AgentOwner*>(object)->bindings.receiver, patch);
}
} //namespace skeet::skins

// ============================================================================
// Subcomponent: agent_receive.cpp
// ============================================================================

namespace skeet::skins {
namespace {
AgentModelReceiver* active_receiver = nullptr;
void agent_model_proxy(const engine::RecvProxyData* data, void* entity, void* output) {
    active_receiver->receive(data, entity, output);
}
// Exact initialization metadata in ctx.h: class/table/property followed by
// replacement4335CCD8 and the43476A8C node. Original callback is node+8.
constexpr AgentModelReceiveDescriptor descriptor{0x0F0EDE49, 0x8C24DA3D, 0x0BA094FB,
                                                 0x4335CCD8, 0x43476A8C, agent_model_proxy};
}
void AgentModelReceiver::receive(const engine::RecvProxyData* data, void* entity, void* output) {
    const auto* handle = engine::invoke<const std::uint32_t*>(entity, 2);
    const auto index =
        *handle == 0xFFFFFFFFu ? 0x1FFFu : std::uint32_t(static_cast<std::uint16_t>(*handle));
    if (index == std::uint32_t(engine::invoke<std::int32_t>(bindings_.engine_client, 12))) {
        globals_.original_handle = *handle; //Native rereads the word after GetLocalPlayer.
        std::memcpy(&globals_.original_model, reinterpret_cast<const std::byte*>(data) + 8,
                    sizeof(globals_.original_model));
    }
    bindings_.original(data, entity, output);
}
const AgentModelReceiveDescriptor& agent_model_receive_descriptor() noexcept {
    return descriptor;
}
void bind_agent_model_receiver(AgentModelReceiver& receiver,
                               client::EntityUpdatePatch& patch) noexcept {
    active_receiver = &receiver;
    receiver.bindings_.original =
        reinterpret_cast<engine::RecvVarProxy>(std::uintptr_t(std::uint32_t(patch.after)));
    patch.before = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(descriptor.replacement));
}
} //namespace skeet::skins

// ============================================================================
// Subcomponent: agents.cpp
// ============================================================================

namespace skeet::skins {
void apply_agent_model(const Settings& settings, const AgentModelState& state, AgentEntity* local,
                       AgentEngine& engine, ModelInfo& models, StringTables& tables) {
    if (state.original_model == -1 || !local)
        return;
    if (engine.demo_playback() || engine.hltv())
        return;
    bool active = false;
    std::uint32_t selected = 0;
    if (local->team == 2) {
        active = settings.agent_t.value != 0;
        selected = std::uint32_t(settings.agent_model_t.value);
    } else if (local->team == 3) {
        active = settings.agent_ct.value != 0;
        selected = std::uint32_t(settings.agent_model_ct.value);
    }
    if (active && !local->dormant) {
        const auto paths = engine.agent_paths(local->team);
        if (selected >= paths.size())
            return;
        const auto path = paths[selected];
        const auto index = resolve_model_index(path, models, tables);
        if (index == -1)
            return;
        if (auto support = engine.support_models(path))
            for (auto model : *support)
                resolve_model_index(model, models, tables);
        auto* target = local;
        if (local->dead) {
            if (local->ragdoll_handle == 0xFFFFFFFF)
                return;
            target = engine.ragdoll(local->ragdoll_handle);
            if (!target)
                return;
        }
        target->model_index = std::bit_cast<std::int16_t>(std::uint16_t(index));
        return;
    }
    if (local->handle == state.original_handle)
        local->model_index = std::bit_cast<std::int16_t>(std::uint16_t(state.original_model));
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: apply.cpp
// ============================================================================

namespace skeet::skins {
float quality_to_wear(std::int32_t quality) noexcept {
    const float scaled = static_cast<float>(quality) * 0.01f;
    return std::max(1.0f - scaled, 0.00001f);
}
bool apply_attributes(const WeaponSkin& record, FallbackAttributes& item, std::uint32_t account,
                      std::string_view name, std::span<const CounterClass> classes,
                      CounterRandom& rng, Refresh& refresh) {
    item.item_id_high = -1;
    item.item_id_low = -1;
    if (account)
        item.account_id = account;
    const auto old_paint = item.paintkit;
    item.paintkit = record.paintkit;
    item.seed = record.seed;
    item.wear = quality_to_wear(record.quality);
    if (old_paint == record.paintkit)
        return false;
    if (record.stattrak) {
        auto count = -1;
        for (const auto& entry : classes) {
            if (entry.catalog_index == -1)
                break;
            if (entry.valid && entry.catalog_index <= 0x133 && entry.name == name) {
                count = entry.counter;
                break;
            }
        }
        if (count == -1)
            count = rng.random_integer(1, 10000);
        item.stattrak = count;
    }
    refresh.schedule_full_update(refresh.current_time() + 0.25f);
    return true;
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: command.cpp
// ============================================================================

namespace skeet::skins {
namespace {
constexpr std::array<std::uint8_t, 65> weapon_groups{
    0, 9, 1, 1, 1, 0, 0, 3, 3, 7,  3, 6, 3, 3, 5, 0, 3, 2, 0, 2, 0, 0,
    0, 2, 2, 4, 2, 4, 5, 4, 1, 11, 1, 2, 2, 4, 1, 0, 6, 3, 8, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 3, 1, 0, 1, 10};
}
std::int32_t weapon_rage_group(std::int32_t definition, const ui::RageWeaponSelector& selector,
                               std::span<const std::uint32_t, 3> classification) noexcept {
    const auto id = static_cast<std::uint32_t>(definition);
    if (id >= weapon_groups.size() || !(classification[id >> 5] & (1u << (id & 31))))
        return -1;
    const auto group = weapon_groups[id];
    const auto* enabled = selector.enabled_groups[group];
    return enabled && *enabled ? group : 0;
}
void sync_rage_weapon_group(std::int32_t definition, ui::RageWeaponSelector& selector,
                            std::span<const std::uint32_t, 3> classification) {
    const auto group = weapon_rage_group(definition, selector, classification);
    if (group >= 0)
        selector.set_value(static_cast<std::uint32_t>(group));
}
void sync_legit_weapon_kind(std::int32_t kind, ui::LegitWeaponSelector& selector) {
    set_combo_value_atomic(selector, static_cast<std::uint32_t>(kind));
}
void CommandRuntime::phase4(const client::CommandFrame& frame, void* local_player) {
    refresh_.process_deferred_hud_refresh();
    const auto definition = frame.weapon_id_d8;
    if (static_cast<std::int32_t>(previous_weapon) == definition)
        return;
    previous_weapon = std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(definition));
    if (!local_player)
        throw std::logic_error("SKINS phase4 requires its live local player");
    std::int32_t team;
    const auto team_offset = std::ptrdiff_t(*offsets_.pointers.at(0x434688B0) ^
                                            offsets_.keys.at(0x4346A518) ^ 0x8B5586F6u);
    std::memcpy(&team, static_cast<const std::byte*>(local_player) + team_offset, sizeof(team));
    auto* menu = menu_;
    if (static_cast<std::uint32_t>(definition) <= 1000u) {
        menu->skins_menu().select_weapon(definition, team);
        menu = menu_;
    }
    const auto kind = std::bit_cast<std::int8_t>(frame.weapon_kind_124);
    if (static_cast<std::uint32_t>(std::int32_t(kind) - 1) <= 5u)
        sync_legit_weapon_kind(kind, menu->legit_menu().weapon_selector());
    sync_rage_weapon_group(frame.weapon_id_d8, menu_->rage_weapon_menu().selector(),
                           classification_);
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: config.cpp
// ============================================================================

namespace skeet::skins {
namespace {
void preserved_integer(ConfigLink& registry, IntegerConfig& d, std::uint32_t hash) {
    d.hash = hash;
    d.type = ConfigType::integer;
    d.serialized_size = 4;
    append_config_link(registry, d);
}
}
void Settings::initialize_shared(ConfigRegistries& registries) {
    // 43492E21 only appends the image-initialized 4346A488 descriptor.
    append_config_link(registries.descriptors, filter_by_weapon);
}
void Settings::initialize(ConfigRegistries& registries) {
    auto& r = registries.descriptors;
    initialize_boolean_config(r, mask_t, 0xDE88B6DC);
    preserved_integer(r, mask_model_t, 0x588FC522);
    initialize_boolean_config(r, mask_ct, 0xF6D4D395);
    preserved_integer(r, mask_model_ct, 0x82097AD3);
    initialize_boolean_config(r, glove_t, 0x3A8EF907);
    initialize_boolean_config(r, glove_ct, 0xE5249CA0);
    initialize_integer_config(r, glove_model_t, 0x7439E8B5, 0x13A3);
    initialize_integer_config(r, glove_model_ct, 0x5101537E, 0x13A3);
    preserved_integer(r, glove_paint_t, 0xE13F61E0);
    preserved_integer(r, glove_paint_ct, 0x7CFD0011);
    initialize_boolean_config(r, agent_t, 0x191A5A61);
    initialize_boolean_config(r, agent_ct, 0x6CAF4832);
    initialize_integer_config(r, agent_model_t, 0x3EB3F20B, 0);
    initialize_integer_config(r, agent_model_ct, 0x8191D9BC, 0);
    initialize_boolean_config(r, knife_t, 0xD347BC87);
    initialize_boolean_config(r, knife_ct, 0xD4F2A320);
    preserved_integer(r, knife_model_t, 0xE0BDA969);
    preserved_integer(r, knife_model_ct, 0xB3C4B6CA);
    initialize_boolean_config(r, filter_by_weapon, 0x9AB5333C);
    weapons.hash = 0xB6218FEC;
    weapons.type = ConfigType::integer_array;
    weapons.serialized_size = 0x410;
    append_config_link(r, weapons);
    for (auto& record : weapons.value) {
        record.enabled = record.stattrak = 0;
        record.quality = 100;
        record.seed = record.paintkit = 0;
    }
}
std::int32_t weapon_config_slot(std::int32_t item, std::int32_t team) noexcept {
    if (item <= 0)
        return -1;
    if (item <= 64)
        return item;
    if (std::uint32_t(item - 500) > 25)
        return -1;
    return team == 2 ? 59 : 42;
}
bool synchronize_enabled(const std::array<WeaponSkin, 65>& settings,
                         std::array<std::uint32_t, 32>& enabled) noexcept {
    bool changed = false;
    for (std::uint32_t id = 1; id <= 64; ++id) {
        const auto mask = std::uint32_t(1) << (id & 31);
        const bool old = (enabled[id >> 5] & mask) != 0;
        // Native compares the stored byte with a normalized bit, then bts/btr.
        if (std::uint8_t(old) != settings[id].enabled) {
            if (settings[id].enabled)
                enabled[id >> 5] |= mask;
            else
                enabled[id >> 5] &= ~mask;
            changed = true;
        }
    }
    return changed;
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: cosmetic_owners.cpp
// ============================================================================

namespace skeet::skins {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define COSMETIC_MEMBER __thiscall
#else
#define COSMETIC_MEMBER
#endif
using OwnerEntry = void(COSMETIC_MEMBER*)(void*);
void COSMETIC_MEMBER cosmetics_unchanged(void*) {} //433CCE8E
void COSMETIC_MEMBER destroy_gloves(void*);
void COSMETIC_MEMBER disable_gloves(void*);
void COSMETIC_MEMBER release_gloves(void*);
void COSMETIC_MEMBER destroy_mask(void*);
void COSMETIC_MEMBER release_mask(void*);
const std::array<OwnerEntry, 9> glove_table{//434707AC
                                            destroy_gloves, cosmetics_unchanged, cosmetics_unchanged,
                                            cosmetics_unchanged, disable_gloves, cosmetics_unchanged,
                                            cosmetics_unchanged, cosmetics_unchanged, release_gloves};
const std::array<OwnerEntry, 9> mask_table{//43472574
                                           destroy_mask, cosmetics_unchanged, cosmetics_unchanged,
                                           cosmetics_unchanged, release_mask, cosmetics_unchanged,
                                           release_mask, cosmetics_unchanged, release_mask};
void register_integer(ConfigLink& registry, IntegerConfig& value, std::uint32_t hash) {
    value.hash = hash;
    value.serialized_size = 4;
    value.type = ConfigType::integer;
    append_config_link(registry, value);
}
void* allocate_owner(engine::JobMemory memory, std::size_t size) { //43387448
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    const auto length = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &length, 4);
    return reinterpret_cast<void*>(header + 16);
}
void free_owner(engine::JobMemory memory, void* object) { //433837D0
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}
class GloveOwner {
public:
    const OwnerEntry* vtable = nullptr;
    CosmeticOwnerBindings bindings;
    engine::JobMemory memory;
    client::NativeEntityObserverReceiver entity_receiver{};
    GloveOwner(const CosmeticOwnerRegistries& registries, ConfigRegistries& config,
               CosmeticOwnerBindings input)
        : bindings(input), memory(registries.lifecycle.memory) {
        registries.lifecycle.append(this); //433676A0
        registries.entities.append_gloves(entity_receiver, bindings.cosmetics); //4349074A /4333BF32
        vtable = glove_table.data(); //43490752
        registries.entities.publish_gloves(entity_receiver); //4349075A
        auto& settings = bindings.settings;
        initialize_boolean_config(config.descriptors, settings.glove_t, 0x3A8EF907);
        initialize_boolean_config(config.descriptors, settings.glove_ct, 0xE5249CA0);
        initialize_integer_config(config.descriptors, settings.glove_model_t, 0x7439E8B5, 0x13A3);
        initialize_integer_config(config.descriptors, settings.glove_model_ct, 0x5101537E, 0x13A3);
        register_integer(config.descriptors, settings.glove_paint_t, 0xE13F61E0);
        register_integer(config.descriptors, settings.glove_paint_ct, 0x7CFD0011);
        bindings.cosmetics.initialize_glove_owner(); //handle=-1, captured4346841C
    }
};
class MaskOwner {
public:
    const OwnerEntry* vtable = nullptr;
    CosmeticOwnerBindings bindings;
    engine::JobMemory memory;
    client::NativeEntityObserverReceiver entity_receiver{};
    NativeMaskResources resources;
    MaskOwner(const CosmeticOwnerRegistries& registries, ConfigRegistries& config,
              CosmeticOwnerBindings input)
        : bindings(input), memory(registries.lifecycle.memory) {
        registries.lifecycle.append(this); //433676A0
        registries.entities.append_mask(entity_receiver, bindings.cosmetics); //4347EBBE /4333BF32
        vtable = mask_table.data(); //4347EBC6
        registries.entities.publish_mask(entity_receiver); //4347EBCE
        auto& settings = bindings.settings;
        initialize_boolean_config(config.descriptors, settings.mask_t, 0xDE88B6DC);
        register_integer(config.descriptors, settings.mask_model_t, 0x588FC522);
        initialize_boolean_config(config.descriptors, settings.mask_ct, 0xF6D4D395);
        register_integer(config.descriptors, settings.mask_model_ct, 0x82097AD3);
        bindings.cosmetics.initialize_mask_owner(resources); //handle=-1, class=0
        misc::construct_native_text(resources.attachment, "facemask", memory);
        constexpr char directory[] = "models/player/holiday/facemasks/";
        misc::construct_native_text(resources.directory, directory, memory);
        for (auto& text : resources.filenames)
            misc::construct_native_text(text, "", memory);
        //Native4347ED32 resolves its class after the24 text objects exist and
        //before assigning the22 separator-delimited filename ranges.
        bindings.cosmetics.resolve_mask_owner_class();
        const auto paths = mask_model_paths();
        for (std::size_t index = 0; index < paths.size(); ++index) {
            const auto filename = paths[index].substr(sizeof(directory) - 1);
            //All22 native names exceed the empty-vector growth minimum8;
            //43320011/434015F3/433F4815 therefore allocate exactly length+1.
            misc::construct_native_text_range(resources.filenames[index], filename.data(),
                                              filename.data() + filename.size(), memory);
        }
    }
};
namespace {
void COSMETIC_MEMBER release_gloves(void* object) { //4331EB65
    auto& owner = *static_cast<GloveOwner*>(object);
    if (owner.bindings.cosmetics.glove_handle() == 0xFFFFFFFF)
        return;
    auto* player = engine::invoke<bool>(owner.bindings.engine_client, 26)
                       ? owner.bindings.local_player
                       : nullptr;
    owner.bindings.cosmetics.release_glove_owner(player);
}
void COSMETIC_MEMBER disable_gloves(void* object) { //4332CAC3
    auto& owner = *static_cast<GloveOwner*>(object);
    owner.bindings.settings.glove_ct.value = 0;
    owner.bindings.settings.glove_t.value = 0;
    release_gloves(object);
}
void COSMETIC_MEMBER destroy_gloves(void* object) { //433B1A7A ->433837D0
    if (!object)
        return;
    auto* owner = static_cast<GloveOwner*>(object);
    const auto memory = owner->memory;
    //The native destructor only frees storage. Entity teardown is its real
    //primary slot4/8 operation, rather than an added destructor side effect.
    owner->~GloveOwner();
    free_owner(memory, object);
}
void COSMETIC_MEMBER release_mask(void* object) { //43386F30, all three actual slots
    static_cast<MaskOwner*>(object)->bindings.cosmetics.release_mask_owner();
}
void COSMETIC_MEMBER destroy_mask(void* object) { //4336E6A4 ->4335BD60 ->433837D0
    if (!object)
        return;
    auto* owner = static_cast<MaskOwner*>(object);
    const auto memory = owner->memory;
    for (std::size_t index = owner->resources.filenames.size(); index != 0; --index)
        misc::destroy_native_text(owner->resources.filenames[index - 1], memory);
    misc::destroy_native_text(owner->resources.directory, memory);
    misc::destroy_native_text(owner->resources.attachment, memory);
    owner->~MaskOwner();
    free_owner(memory, object);
}
#undef COSMETIC_MEMBER
}
std::size_t glove_owner_size() noexcept {
    return sizeof(GloveOwner);
}
std::size_t mask_owner_size() noexcept {
    return sizeof(MaskOwner);
}
GloveOwner& construct_glove_owner(void* storage, const CosmeticOwnerRegistries& registries,
                                  ConfigRegistries& config, CosmeticOwnerBindings bindings) {
    return *::new (storage) GloveOwner(registries, config, bindings);
}
MaskOwner& construct_mask_owner(void* storage, const CosmeticOwnerRegistries& registries,
                                ConfigRegistries& config, CosmeticOwnerBindings bindings) {
    return *::new (storage) MaskOwner(registries, config, bindings);
}
void* allocate_glove_owner_storage(engine::JobMemory memory) {
    return allocate_owner(memory, sizeof(GloveOwner));
}
void* allocate_mask_owner_storage(engine::JobMemory memory) {
    return allocate_owner(memory, sizeof(MaskOwner));
}
NativeEntityCosmetics& glove_owner_cosmetics(GloveOwner& owner) noexcept {
    return owner.bindings.cosmetics;
}
NativeEntityCosmetics& mask_owner_cosmetics(MaskOwner& owner) noexcept {
    return owner.bindings.cosmetics;
}
} //namespace skeet::skins

// ============================================================================
// Subcomponent: gloves.cpp
// ============================================================================

namespace skeet::skins {
namespace {
constexpr float glove_wear = std::bit_cast<float>(std::uint32_t(0x3727C5AC));
/* duplicate entity_index removed */
GloveEntity* resolve(GloveEngine& engine, std::uint32_t handle) {
    return handle == invalid_glove_handle ? nullptr : engine.resolve(handle);
}
// 0x43364942. The two registration operations are resolved engine functions;
// the factory, model initialization, index restoration and teardown are retained.
GloveEntity* create_glove(GloveFactory& factory, const char* model, GloveEngine& engine,
                          ModelInfo& models) {
    const auto old_highest = engine.highest_entity_index();
    auto* created = factory.create(0x7FE, 0x7FF);
    if (!created)
        return nullptr;
    if (engine.entity_registration_available()) {
        auto* entity = created->unknown()->base_entity();
        engine.register_entity(entity_index(entity->handle));
        engine.highest_entity_index() = old_highest;
        if (models.model_index(model) != -1 && engine.initialize_model(*entity, model, false))
            entity->creation_index = std::int32_t(entity_index(entity->handle));
        else {
            auto handle = invalid_glove_handle;
            engine.unregister_entity(handle, *entity);
        }
    }
    created->notify_transmission(0);
    return created->unknown()->base_entity();
}
// 0x433E320A: fields 43467F9C/43468C1C,4346A8A8,43468950,
// 4346908C and 4346A358; model selection crosses the real MDL cache calls.
void set_glove(GloveEntity& entity, std::int32_t paint, std::uint32_t definition,
               std::int32_t model, GloveEngine& engine) {
    entity.item_id_low = -1;
    entity.item_id_high = -1;
    entity.paintkit = paint;
    entity.wear = glove_wear;
    entity.definition = std::uint16_t(definition);
    entity.account_id = engine.local_account_id();
    engine.begin_model_cache();
    engine.set_model_index(entity, model);
    engine.end_model_cache();
}
// 0x433E2266 only updates attributes already present; it does not append one.
void set_attribute(GloveEntity& entity, std::uint32_t definition, float value,
                   GloveEngine& engine) {
    for (auto& attribute : entity.attributes) {
        if (attribute.definition != definition)
            continue;
        if (attribute.value == value && attribute.initial_value == value)
            return;
        attribute.value = value;
        attribute.initial_value = value;
        if (entity.attribute_cache) {
            engine.release_attribute_cache(entity.attribute_cache, true);
            entity.attribute_cache = nullptr;
        }
        return;
    }
}
bool matches(const GloveEntity& entity, std::int32_t paint, std::uint32_t definition,
             std::int32_t model) {
    return entity.paintkit == paint && std::uint32_t(entity.definition) == definition &&
           entity.model_index == model;
}
void detach(GloveEntity& entity, GloveEntity& local, GloveEngine& engine) {
    engine.equip(entity, local, false);
    engine.unequip(entity, local);
}
void destroy(GloveEntity* entity, bool recreating) {
    if (!entity || !entity->networkable)
        return;
    if (recreating)
        entity->networkable->mark_destroyed_on_recreate();
    entity->networkable->notify_transmission(1);
    entity->networkable->release();
}
// 0x43342DD7: select the player's indexed view model, following a valid live
// observer target in mode four. The original routine also uses this recursion.
GloveEntity* view_model(GloveEntity& player, std::int32_t index, GloveEngine& engine) {
    auto* result = resolve(engine, player.view_models[std::size_t(index)]);
    if (player.observer_mode != 4)
        return result;
    auto* target = resolve(engine, player.observer_target);
    if (target && target != &player && !target->dead && engine.health(*target) > 0 &&
        !engine.network_dormant(*target) && !target->observer_excluded)
        result = view_model(*target, index, engine);
    return result;
}
void teardown(GloveOwnerState& state, GloveEntity* local, GloveEngine& engine) {
    if (state.handle == invalid_glove_handle)
        return;
    if (local && local->wearable_handle != invalid_glove_handle &&
        local->wearable_handle == state.handle) {
        if (auto* worn = resolve(engine, local->wearable_handle)) {
            detach(*worn, *local, engine);
            local->wearable_change_counter = 0;
        }
        local->wearable_handle = invalid_glove_handle;
    }
    destroy(resolve(engine, state.handle), false);
    state.handle = invalid_glove_handle;
}
}
void apply_gloves(const Settings& settings, GloveOwnerState& state, GloveEntity* local,
                  GloveEngine& engine, SchemaEngine& schema, ModelInfo& models) {
    if (!local || engine.demo_playback() || engine.hltv()) {
        teardown(state, local, engine);
        return;
    }
    const bool ct = local->team == 3;
    if (!(ct ? settings.glove_ct.value : settings.glove_t.value) || local->dormant) {
        teardown(state, local, engine);
        return;
    }
    if (local->dead)
        return;
    const auto definition =
        std::uint32_t(ct ? settings.glove_model_ct.value : settings.glove_model_t.value);
    if (!schema.acquire_schema())
        return;
    const auto item = schema.definition(definition);
    if (!item || !item->world_model || !item->view_model)
        return;
    const auto world = models.model_index(item->world_model);
    const auto view = models.model_index(item->view_model);
    if (world == -1 || view == -1 || !state.factory)
        return;
    const auto paint = ct ? settings.glove_paint_ct.value : settings.glove_paint_t.value;
    if (resolve(engine, local->wearable_handle) && !resolve(engine, state.handle)) {
        if (auto* worn = resolve(engine, local->wearable_handle)) {
            if (!matches(*worn, paint, definition, world))
                set_glove(*worn, paint, definition, world, engine);
            set_attribute(*worn, 6, float(paint), engine);
            set_attribute(*worn, 8, glove_wear, engine);
            return;
        }
    }
    auto* owned = resolve(engine, state.handle);
    if (owned) {
        if (!owned->networkable || owned->networkable->client_class() != state.factory)
            return;
        if (matches(*owned, paint, definition, world) && local->wearable_handle == state.handle) {
            if (local->transmission_state != 1)
                engine.equip(*owned, *local, true);
            return;
        }
        if (local->wearable_handle != invalid_glove_handle) {
            if (auto* current = resolve(engine, local->wearable_handle)) {
                detach(*current, *local, engine);
                local->wearable_change_counter = 0;
            }
            local->wearable_handle = invalid_glove_handle;
        }
        destroy(resolve(engine, state.handle), true);
    }
    state.handle = invalid_glove_handle;
    auto* created = create_glove(*state.factory, item->world_model, engine, models);
    if (!created)
        return;
    state.handle = created->handle;
    auto& override_local = engine.local_override();
    auto* old_override = override_local;
    const bool set_override = override_local == nullptr;
    if (set_override)
        override_local = local;
    created->flags |= 0x80;
    created->quality = 11;
    set_glove(*created, paint, definition, world, engine);
    const auto old_model_update = local->model_update_handle;
    local->model_update_handle = invalid_glove_handle;
    engine.set_model_instance(*created, engine.model_instance(*created));
    local->model_update_handle = old_model_update;
    if (auto* current = resolve(engine, local->wearable_handle))
        detach(*current, *local, engine);
    local->wearable_handle = invalid_glove_handle;
    engine.attach(*created, *local);
    engine.equip(*created, *local, true);
    created->owner_handle = local->handle;
    engine.post_equip(*created);
    if (auto* weapon = resolve(engine, local->active_weapon_handle)) {
        if (auto* model = view_model(*local, weapon->view_model_index, engine)) {
            engine.invalidate_bone_cache(*model);
            engine.invalidate_physics(*model, 0x20);
        }
    }
    created->initialized = true;
    if (created->networkable)
        created->networkable->on_data_changed(0);
    local->wearable_change_counter = 0;
    if (set_override)
        override_local = old_override;
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: inventory.cpp
// ============================================================================

namespace skeet::skins {
bool inventory_owner_matches(const WeaponOwnership& owner, std::uint32_t tag, bool valid,
                             PlayerIdentity local, std::int32_t index) noexcept {
    if (valid && (local.low || local.high) && owner.original_owner.low == local.low &&
        owner.original_owner.high == local.high)
        return true;
    if (owner.original_owner.low || owner.zero_marker || tag)
        return false;
    const auto owner_index = owner.owner_handle == 0xFFFFFFFF
                                 ? 0x1FFFU
                                 : std::uint32_t(std::uint16_t(owner.owner_handle));
    return owner_index == std::uint32_t(index);
}
void apply_inventory(const WeaponSkinsConfig& settings, std::span<const InventoryEntry> entries,
                     InventoryEngine& engine) {
    if (!engine.local_player_exists())
        return;
    if (std::none_of(settings.value.begin(), settings.value.end(),
                     [](const auto& r) { return r.enabled != 0; }))
        return;
    if (engine.local_player_dead())
        return;
    for (const auto& entry : entries) {
        auto* item = engine.resolve(entry.handle);
        if (!item || !item->is_weapon)
            continue;
        PlayerIdentity identity;
        const bool valid = engine.local_identity(identity);
        if (!inventory_owner_matches(item->ownership, entry.tag, valid, identity,
                                     engine.local_player_index()))
            continue;
        const auto slot = weapon_config_slot(item->definition, engine.local_team());
        if (slot < 0 || !settings.value[slot].enabled)
            continue;
        PlayerIdentity account;
        engine.local_identity(account);
        apply_attributes(settings.value[slot], item->attributes, account.low, item->entity_class,
                         engine.counter_classes(), engine, engine);
    }
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: inventory_owner.cpp
// ============================================================================

namespace skeet::skins {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define INVENTORY_MEMBER __thiscall
#else
#define INVENTORY_MEMBER
#endif
using OwnerEntry = void(INVENTORY_MEMBER*)(void*);
void INVENTORY_MEMBER destroy_inventory(void*);
void INVENTORY_MEMBER unchanged_inventory(void*) {} //433CCE8E RET
// Native4346F358: no extra initialization/reset/update operation is present.
const std::array<OwnerEntry, 9> inventory_lifecycle_table{
    destroy_inventory,   unchanged_inventory, unchanged_inventory,
    unchanged_inventory, unchanged_inventory, unchanged_inventory,
    unchanged_inventory, unchanged_inventory, unchanged_inventory};
}

class InventoryLifecycleOwner {
public:
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    InventoryOwnerBindings bindings;
    void* resolver = nullptr;
    client::NativeCommandPhaseReceiver command_receiver{};
    client::NativeEntityObserverReceiver entity_receiver{};
    CommandRuntime command;
    std::optional<NativeInventoryRuntime> inventory;
    events::StatTrakDeath stattrak;
    events::StatTrakDeathListener death_listener;

    InventoryLifecycleOwner(engine::OwnerRegistryBinding lifecycle,
                            client::CommandPhaseRegistry& commands,
                            client::EntityObserverRegistry& updates, ConfigRegistries& registry,
                            InventoryOwnerBindings shared)
        : memory(lifecycle.memory), bindings(shared),
          command(shared.menu, shared.refresh, shared.offsets, shared.classification),
          stattrak{shared.weapons}, death_listener{stattrak, shared.death, shared.death_offsets} {
        lifecycle.append(this); //434902E7 /433676A0; primary table still null
        commands.append(command_receiver, &command); //434902EF /43345EBC
        // The runtime retains the owner's later resolver capture cell.
        inventory.emplace(bindings.weapons,
                          NativeInventoryBindings{bindings.engine_client, resolver,
                                                  bindings.counter_rows, bindings.counters,
                                                  bindings.offsets},
                          bindings.random, bindings.refresh);
        updates.append_inventory(entity_receiver, *inventory); //434902F7 /4333BF32
        vtable = agent_lifecycle_table.data(); //434902FC
        commands.publish(command_receiver, client::CommandOwnerId::owner_4346f1f4); //43490305
        updates.publish_inventory(entity_receiver); //4349030E
        resolver = bindings.entity_list; //43490315..31A
        command.previous_weapon = 0; //4349031F, signed word nativeowner+18
        auto& weapons = bindings.weapons;
        weapons.hash = 0xB6218FECu;
        weapons.serialized_size = 0x410;
        weapons.type = ConfigType::integer_array;
        append_config_link(registry.descriptors, weapons); //43490337 /43385AE4
        for (auto& value : weapons.value) { //43490344..35A; reserved+2 is untouched
            value.enabled = value.stattrak = 0;
            value.quality = 100;
            value.seed = value.paintkit = 0;
        }
        death_listener.bind(bindings.events); //43490360..36E /433F9037 /3461B584
    }
    InventoryLifecycleOwner(const InventoryLifecycleOwner&) = delete;
    InventoryLifecycleOwner& operator=(const InventoryLifecycleOwner&) = delete;
};

namespace {
void INVENTORY_MEMBER destroy_inventory(void* object) { //433B1A7A ->433837D0
    if (!object)
        return;
    auto* owner = static_cast<InventoryLifecycleOwner*>(object);
    const auto memory = owner->memory;
    owner->~InventoryLifecycleOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
    // The original primary destructor does not unlink any borrowed registry.
}
}

std::size_t inventory_owner_size() noexcept {
    return sizeof(InventoryLifecycleOwner);
}
InventoryLifecycleOwner& construct_inventory_owner(void* storage,
                                                   engine::OwnerRegistryBinding lifecycle,
                                                   client::CommandPhaseRegistry& commands,
                                                   client::EntityObserverRegistry& updates,
                                                   ConfigRegistries& registry,
                                                   InventoryOwnerBindings bindings) {
    return *::new (storage)
        InventoryLifecycleOwner(lifecycle, commands, updates, registry, bindings);
}
#undef INVENTORY_MEMBER
} //namespace skeet::skins

// ============================================================================
// Subcomponent: knife_precache.cpp
// ============================================================================

namespace skeet::skins {
namespace {
void* model_precache(void* container) {
    const auto count = engine::invoke<std::int32_t>(container, 5);
    for (std::int32_t index = 0; index < count; ++index) {
        auto* table = engine::invoke<void*>(container, 4, index);
        if (!table)
            continue;
        const auto* name = engine::invoke<const char*>(table, 1);
        if (name && engine::netvar_name_hash(name) == 0x9FE6B136U)
            return table;
    }
    return nullptr;
}
std::int32_t add(std::int32_t left, std::uint32_t right) {
    return std::bit_cast<std::int32_t>(std::uint32_t(left) + right);
}
}
void refresh_knife_precache(SequenceRuntime& state, void* string_tables) {
    auto* table = model_precache(string_tables);
    if (!table)
        return;
    const auto count = engine::invoke<std::int32_t>(table, 3);
    for (std::int32_t index = 0; index < count; index = add(index, 1)) {
        const auto* path = engine::invoke<const char*>(table, 9, index);
        if (!path || std::strlen(path) <= 19)
            continue;
        std::uint32_t prefix;
        std::memcpy(&prefix, path + 15, sizeof(prefix));
        if (prefix != 0x6E6B5F76U)
            continue;
        const auto hash = engine::netvar_name_hash(path);
        const auto definitions = knife_models();
        for (std::size_t knife = 0; knife < definitions.size(); ++knife) {
            if (definitions[knife].view_model_hash != hash)
                continue;
            const auto extra = definitions[knife].native_word_10;
            state.models[knife].entity_model = index;
            state.models[knife].view_model = add(index, 1U + extra);
            state.models[knife].world_model = add(index, 2U + extra);
            index = add(index, 2);
            break;
        }
    }
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: masks.cpp
// ============================================================================

namespace skeet::skins {
namespace {
constexpr std::array<std::string_view, 22> paths{{
    "models/player/holiday/facemasks/facemask_hoxton.mdl",
    "models/player/holiday/facemasks/porcelain_doll.mdl",
    "models/player/holiday/facemasks/facemask_skull.mdl",
    "models/player/holiday/facemasks/facemask_samurai.mdl",
    "models/player/holiday/facemasks/evil_clown.mdl",
    "models/player/holiday/facemasks/facemask_wolf.mdl",
    "models/player/holiday/facemasks/facemask_sheep_model.mdl",
    "models/player/holiday/facemasks/facemask_bunny_gold.mdl",
    "models/player/holiday/facemasks/facemask_anaglyph.mdl",
    "models/player/holiday/facemasks/facemask_porcelain_doll_kabuki.mdl",
    "models/player/holiday/facemasks/facemask_dallas.mdl",
    "models/player/holiday/facemasks/facemask_pumpkin.mdl",
    "models/player/holiday/facemasks/facemask_sheep_bloody.mdl",
    "models/player/holiday/facemasks/facemask_devil_plastic.mdl",
    "models/player/holiday/facemasks/facemask_boar.mdl",
    "models/player/holiday/facemasks/facemask_chains.mdl",
    "models/player/holiday/facemasks/facemask_tiki.mdl",
    "models/player/holiday/facemasks/facemask_bunny.mdl",
    "models/player/holiday/facemasks/facemask_sheep_gold.mdl",
    "models/player/holiday/facemasks/facemask_zombie_fortune_plastic.mdl",
    "models/player/holiday/facemasks/facemask_chicken.mdl",
    "models/player/holiday/facemasks/facemask_skull_gold.mdl",
}};
MaskEntity* resolve(MaskEngine& engine, std::uint32_t handle) {
    return handle == invalid_glove_handle ? nullptr : engine.resolve(handle);
}
/* duplicate entity_index removed */
// 0x43364942, using the existing real factory/networkable/model primitives.
MaskEntity* create_mask(GloveFactory& factory, const char* model, MaskEngine& engine,
                        ModelInfo& models) {
    const auto old_highest = engine.highest_entity_index();
    auto* created = factory.create(0x7FE, 0x7FF);
    if (!created)
        return nullptr;
    if (engine.entity_registration_available()) {
        auto* entity = static_cast<MaskEntity*>(created->unknown()->base_entity());
        engine.register_entity(entity_index(entity->handle));
        engine.highest_entity_index() = old_highest;
        if (models.model_index(model) != -1 && engine.initialize_model(*entity, model, false))
            entity->creation_index = std::int32_t(entity_index(entity->handle));
        else {
            auto handle = invalid_glove_handle;
            engine.unregister_entity(handle, *entity);
        }
    }
    created->notify_transmission(0);
    return static_cast<MaskEntity*>(created->unknown()->base_entity());
}
void zero_transform(MaskEntity& mask, std::array<float, 3>& value, std::int32_t flags,
                    MaskEngine& engine) {
    if (value[0] != 0.0f || value[1] != 0.0f || value[2] != 0.0f) {
        engine.invalidate_transform(mask, flags);
        value = {0.0f, 0.0f, 0.0f};
    }
}
void teardown(MaskOwnerState& state, MaskEngine& engine) {
    if (auto* entity = resolve(engine, state.handle)) {
        if (auto* networkable = entity->networkable) {
            networkable->notify_transmission(1);
            networkable->release();
        }
    }
    state.handle = invalid_glove_handle;
}
}
std::span<const std::string_view, 22> mask_model_paths() noexcept {
    return paths;
}
void apply_mask(const Settings& settings, MaskOwnerState& state, MaskEntity* local,
                MaskEngine& engine, ModelInfo& models, StringTables& tables) {
    // Native 0x4340884C gates even teardown on availability of SetParent.
    if (!engine.parent_function_available())
        return;
    if (!local || engine.demo_playback() || engine.hltv()) {
        teardown(state, engine);
        return;
    }
    const bool ct = local->team == 3;
    if (!(ct ? settings.mask_ct.value : settings.mask_t.value)) {
        teardown(state, engine);
        return;
    }
    // Unlike gloves, dormancy and death preserve an existing mask entity.
    if (local->dormant || !engine.render_model(*local) || !local->model_header ||
        !*local->model_header || local->dead)
        return;
    const auto attachment = engine.attachment(*local, "facemask");
    if (attachment == -1)
        return;
    const auto selected = ct ? settings.mask_model_ct.value : settings.mask_model_t.value;
    if (std::uint32_t(selected) >= paths.size())
        return;
    const auto path = paths[std::size_t(selected)];
    const auto model = resolve_model_index(path, models, tables);
    if (model == -1)
        return;
    auto* entity = resolve(engine, state.handle);
    if (entity) {
        if (!entity->networkable || entity->networkable->client_class() != state.factory)
            return;
        if (entity->model_index == model && resolve(engine, entity->parent_handle) == local &&
            entity->model_update_handle == local->handle && entity->owner_handle == local->handle)
            return;
    } else {
        state.handle = invalid_glove_handle;
        entity = create_mask(*state.factory, path.data(), engine, models);
        if (!entity)
            return;
        state.handle = entity->handle;
    }
    engine.begin_model_cache();
    engine.set_model_index(*entity, model);
    engine.end_model_cache();
    zero_transform(*entity, entity->local_origin, 1, engine);
    zero_transform(*entity, entity->local_angles, 2, engine);
    engine.set_parent(*entity, *local, attachment);
    zero_transform(*entity, entity->local_origin, 1, engine);
    zero_transform(*entity, entity->local_angles, 2, engine);
    entity->quality = 0;
    entity->flag_43468bd4 = false;
    engine.entity_boolean_3b8(*entity, false);
    entity->bits_43467044 |= 4;
    entity->flag_43467964 = false;
    entity->flags_43469224 &= 0xFFFBFFFF;
    entity->model_update_handle = local->handle;
    entity->owner_handle = local->handle;
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: model_resources.cpp
// ============================================================================

namespace skeet::skins {
std::int32_t resolve_model_index(std::string_view path, ModelInfo& models, StringTables& tables) {
    if (path.empty() || path.front() == '\0')
        return -1;
    auto index = models.model_index(path);
    if (index != -1)
        return index;
    StringTable* precache = nullptr;
    const auto count = tables.count();
    for (std::int32_t i = 0; i < count; ++i) {
        auto* table = tables.at(i);
        if (!table)
            continue;
        const auto* name = table->name();
        if (name && events::event_hash(name) == 0x9FE6B136) {
            precache = table;
            break;
        }
    }
    if (precache && models.load_model(path))
        index = precache->add_string(0, path, -1, nullptr);
    return index;
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: models.cpp
// ============================================================================

namespace skeet::skins {
namespace {
constexpr std::array<KnifeModel, 19> models{{
    {500, 0x4A39E8B9, 1, 1, 0}, {505, 0x785D931C, 1, 1, 0}, {506, 0x8D5BF97D, 1, 1, 0},
    {507, 0x606AF709, 1, 1, 0}, {508, 0x697E90CE, 1, 1, 0}, {509, 0x50E1EA52, 1, 1, 0},
    {515, 0x5EC93922, 3, 2, 0}, {512, 0xE37F744C, 2, 1, 0}, {516, 0x3BCCCF3B, 1, 1, 1},
    {514, 0x913A7A98, 1, 1, 0}, {519, 0xFB1B5F45, 2, 2, 0}, {520, 0x97A409DE, 1, 1, 0},
    {522, 0xC93B4D19, 2, 1, 0}, {523, 0x33AF4B65, 2, 1, 0}, {503, 0x8DBBB37E, 2, 1, 0},
    {517, 0xB733813F, 2, 2, 0}, {518, 0x5A3FA2DF, 2, 2, 0}, {521, 0x7A2E3B99, 2, 2, 0},
    {525, 0x77EAAA08, 2, 2, 0},
}};
}
std::span<const KnifeModel, 19> knife_models() noexcept {
    return models;
}
const KnifeModel* knife_model(std::int32_t selection) noexcept {
    return std::uint32_t(selection) < models.size() ? &models[std::size_t(selection)] : nullptr;
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: native_agents.cpp
// ============================================================================

namespace skeet::skins {
namespace {
/* duplicate field removed; using skeet::field */
void set_model(void* object, std::ptrdiff_t offset, std::int32_t model) {
    const auto value = std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(model));
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}
}
NativeAgentRuntime::NativeAgentRuntime(const Settings& settings, NativeCatalog& catalog,
                                       NativeAgentBindings bindings, ModelInfo& models,
                                       StringTables& tables,
                                       network::DataUpdateObserverRegistry& observers)
    : settings_(settings), catalog_(catalog), bindings_(bindings), models_(models),
      tables_(tables) {
    observers.append(*this); // 4347DAB7: secondary receiver registered through 4337EFD9.
}
void NativeAgentRuntime::process(void* player, std::int32_t update) {
    (void)update;
    if (bindings_.original_model == -1 || !player)
        return;
    if (engine::invoke<bool>(bindings_.engine_client, 82))
        return;
    if (engine::invoke<bool>(bindings_.engine_client, 93))
        return;
    const auto& globals = bindings_.offsets;
    const auto offset = [&](std::uint32_t pointer, std::uint32_t mask, std::uint32_t key) {
        return std::ptrdiff_t(*globals.pointers.at(pointer) ^ globals.keys.at(mask) ^ key);
    };
    const auto team = field<std::int32_t>(player, offset(0x434688B0, 0x4346A518, 0x8B5586F6));
    bool enabled = false;
    std::uint32_t selected = 0;
    if (team == 2) {
        enabled = settings_.agent_t.value != 0;
        selected = static_cast<std::uint32_t>(settings_.agent_model_t.value);
    } else if (team == 3) {
        enabled = settings_.agent_ct.value != 0;
        selected = static_cast<std::uint32_t>(settings_.agent_model_ct.value);
    }
    if (enabled && field<std::uint8_t>(player, offset(0x43468360, 0x4346891C, 0x40C98EF6)) == 0) {
        const auto paths = catalog_.agent_paths(team);
        if (selected >= paths.size())
            return;
        const auto& path = paths[selected];
        const auto index = resolve_model_index(path, models_, tables_);
        if (index == -1)
            return;
        const auto* support = bindings_.support_models(const_cast<char*>(path.c_str()));
        if (support) {
            const auto* first = field<const char*>(support, 8);
            resolve_model_index(first ? std::string_view(first) : std::string_view{}, models_,
                                tables_);
            const auto* second = field<const char*>(support, 0xC);
            resolve_model_index(second ? std::string_view(second) : std::string_view{}, models_,
                                tables_);
        }
        if (field<std::uint8_t>(player, offset(0x43468E78, 0x434684E4, 0x9D920E35)) == 0) {
            set_model(player, offset(0x43468978, 0x43468F7C, 0x47877DEB), index);
            return;
        }
        const auto handle =
            field<std::uint32_t>(player, offset(0x434691A0, 0x434670F0, 0xD5D91363));
        if (handle == 0xFFFFFFFFu)
            return;
        auto* ragdoll = engine::invoke<void*>(bindings_.entity_list, 4, handle);
        if (ragdoll)
            set_model(ragdoll, offset(0x43468978, 0x43468F7C, 0x47877DEB), index);
        return;
    }
    if (bindings_.original_model == -1)
        return;
    const auto* handle = engine::invoke<const std::uint32_t*>(player, 2);
    if (*handle == bindings_.original_handle)
        set_model(player, offset(0x43468978, 0x43468F7C, 0x47877DEB), bindings_.original_model);
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: native_catalog.cpp
// ============================================================================

namespace skeet::skins {
namespace {
// Native one-use buffers 0x434672A0 / 0x43467640, before their clearing pass.
constexpr std::string_view native_t =
    "Jumpsuit A\1tm_jumpsuit_varianta\2Jumpsuit B\1tm_jumpsuit_variantb\2"
    "Jumpsuit C\1tm_jumpsuit_variantc\2Pirate\1tm_pirate\2Pirate A\1tm_pirate_varianta\2"
    "Pirate B\1tm_pirate_variantb\2Pirate C\1tm_pirate_variantc\2Pirate D\1tm_pirate_variantd\2"
    "Professional\1tm_professional\2Professional 1\1tm_professional_var1\2"
    "Professional 2\1tm_professional_var2\2Professional 3\1tm_professional_var3\2Professional 4\1tm_professional_var4\2";
constexpr std::string_view native_ct =
    "FBI\1ctm_fbi\2FBI (Variant A)\1ctm_fbi_varianta\2FBI (Variant C)\1ctm_fbi_variantc\2"
    "FBI (Variant D)\1ctm_fbi_variantd\2FBI (Variant E)\1ctm_fbi_variante\2"
    "GIGN\1ctm_gign\2GIGN (Variant A)\1ctm_gign_varianta\2GIGN (Variant B)\1ctm_gign_variantb\2"
    "GIGN (Variant C)\1ctm_gign_variantc\2GIGN (Variant D)\1ctm_gign_variantd\2"
    "GSG-9\1ctm_gsg9\2GSG-9 (Variant A)\1ctm_gsg9_varianta\2GSG-9 (Variant B)\1ctm_gsg9_variantb\2"
    "GSG-9 (Variant C)\1ctm_gsg9_variantc\2GSG-9 (Variant D)\1ctm_gsg9_variantd\2"
    "SWAT\1ctm_swat\2SWAT (Variant A)\1ctm_swat_varianta\2SWAT (Variant B)\1ctm_swat_variantb\2"
    "SWAT (Variant C)\1ctm_swat_variantc\2SWAT (Variant D)\1ctm_swat_variantd\2";
}
bool NativeCatalog::available() {
    return engine_.acquire_schema();
}
std::span<const std::uint16_t> NativeCatalog::paintkits(std::uint16_t item) {
    const auto found = paintkits_.find(item);
    return found == paintkits_.end() ? std::span<const std::uint16_t>{} : found->second;
}
std::optional<std::u16string> NativeCatalog::paintkit_name(std::uint32_t id) {
    const auto* token = engine_.paintkit_token(id);
    return token ? engine_.localize(token) : std::nullopt;
}
std::span<const CatalogChoice> NativeCatalog::agent_models(std::int32_t team) {
    if (team == 2)
        return agents_t_;
    if (team == 3)
        return agents_ct_;
    return {};
}
std::span<const std::string> NativeCatalog::agent_paths(std::int32_t team) const {
    if (team == 2)
        return paths_t_;
    if (team == 3)
        return paths_ct_;
    return {};
}
void NativeCatalog::append_native_agents(std::string_view data, std::int32_t team) {
    auto& options = team == 2 ? agents_t_ : agents_ct_;
    auto& paths = team == 2 ? paths_t_ : paths_ct_;
    while (!data.empty()) {
        const auto first = data.find('\1');
        if (first == data.npos)
            return;
        const auto second = data.find('\2', first + 1);
        if (second == data.npos)
            return;
        const auto label = data.substr(0, first);
        const auto model = data.substr(first + 1, second - first - 1);
        paths.push_back("models/player/custom_player/legacy/" + std::string(model) + ".mdl");
        options.push_back(
            {std::uint32_t(paths.size() - 1), std::u16string(label.begin(), label.end())});
        data.remove_prefix(second + 1);
    }
}
void NativeCatalog::initialize(ui::SkinsMenu& menu) {
    if (available()) {
        const auto count = engine_.definition_count();
        std::int32_t processed = 0;
        for (std::uint32_t id = 0; id < 30000; ++id) {
            auto item = engine_.definition(id);
            if (!item)
                continue;
            if (item->type_token) {
                const auto type = events::event_hash(item->type_token);
                if (type == 0x2B77F7D8 && item->base_name_token && item->view_model) {
                    if (auto label = engine_.localize(item->base_name_token)) {
                        const std::string_view path = item->view_model;
                        const auto slash = path.rfind('/');
                        if (slash != path.npos && slash + 1 < path.size()) {
                            if (path[slash + 1] == 'c' && id != 0x13AD) {
                                paths_ct_.emplace_back(path);
                                agents_ct_.push_back(
                                    {std::uint32_t(paths_ct_.size() - 1), std::move(*label)});
                            } else if (path[slash + 1] == 't' && id != 0x13AC) {
                                paths_t_.emplace_back(path);
                                agents_t_.push_back(
                                    {std::uint32_t(paths_t_.size() - 1), std::move(*label)});
                            }
                        }
                    }
                } else if (type == 0x8E483246 && item->base_name_token && item->view_model &&
                           item->world_model) {
                    if (auto label = engine_.localize(item->base_name_token))
                        gloves_.push_back({id, std::move(*label)});
                }
            }
            if (++processed >= count)
                break;
        }
    }
    if (!native_agents_consumed_) {
        append_native_agents(native_t, 2);
        append_native_agents(native_ct, 3);
        native_agents_consumed_ = true;
    }
    menu.initialize_schema_options();
    auto directory = files_.current_directory();
    if (directory.empty())
        return;
    directory += u"\\csgo\\scripts\\items\\items_game.txt";
    auto contents = files_.read_file(directory);
    if (contents && !contents->empty())
        parse_weapon_icons(*contents);
}
void NativeCatalog::parse_weapon_icons(std::string_view text) {
    constexpr std::string_view opening = "\t\t\"weapon_icons\"\r\n\t\t{\r\n";
    constexpr std::string_view closing = "\t\t}\r\n\t}\r\n";
    constexpr std::string_view next = "\r\n\t\t\t\"";
    const auto begin = text.find(opening);
    if (begin == text.npos)
        return;
    auto cursor = begin + 0x1B;
    if (cursor > text.size())
        return;
    const auto close = text.find(closing, cursor);
    if (close == text.npos)
        return;
    const auto end = close - 6;
    std::uint32_t previous_paint = 0, previous_item = 0;
    const std::string terminated(text);
    while (cursor < text.size()) {
        const auto value = std::uint32_t(std::strtoul(terminated.c_str() + cursor, nullptr, 10));
        if (!value || value == 0xFFFFFFFF)
            return;
        const auto item = value >> 16, paint = (value >> 2) & 0x3FFF;
        if (previous_paint != paint || std::uint16_t(previous_item) != std::uint16_t(item)) {
            if (paint < 10000 && maximum_ < paint)
                maximum_ = paint;
            paintkits_[std::uint16_t(item)].push_back(std::uint16_t(paint));
            previous_item = item;
            previous_paint = paint;
        }
        if (cursor >= end)
            return;
        const auto found = text.find(next, cursor);
        if (found == text.npos || found >= end)
            return;
        cursor = found + next.size();
    }
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: native_entity_cosmetics.cpp
// ============================================================================

namespace skeet::skins {
namespace {
constexpr std::uint32_t invalid = 0xFFFFFFFF;
/* duplicate glove_wear, read, write removed */
void* subobject(void* object, std::ptrdiff_t offset) {
    return static_cast<std::byte*>(object) + offset;
}
/* duplicate entity_index removed */
void* complete_entity_list(void* object) {
    auto* table = read<std::byte*>(object, 0);
    auto* locator = read<void*>(table, -std::ptrdiff_t(sizeof(void*)));
    return subobject(object, -read<std::int32_t>(locator, 4));
}
}
NativeCosmeticOffsets native_cosmetic_offsets(const engine::NativeOffsetGlobals& globals) {
    auto offset = [&](std::uint32_t pointer, std::uint32_t mask,
                      std::uint32_t key) -> std::ptrdiff_t {
        return std::ptrdiff_t(*globals.pointers.at(pointer) ^ globals.keys.at(mask) ^ key);
    };
    return {offset(0x434688B0, 0x4346A518, 0x8B5586F6),
            offset(0x43468360, 0x4346891C, 0x40C98EF6),
            offset(0x43468E78, 0x434684E4, 0x9D920E35),
            offset(0x43468968, 0x43467EA0, 0xE96443C6),
            offset(0x4346A8A8, 0x43468568, 0xC6D22EE2),
            offset(0x4346908C, 0x43468BA0, 0x5C9F0FBB),
            offset(0x43468978, 0x43468F7C, 0x47877DEB),
            offset(0x4346A5F8, 0x43467B88, 0x950F7B20),
            offset(0x43468FEC, 0x4346A3C4, 0xA2A8247F) - 4,
            offset(0x43468B50, 0x43467B48, 0x38CA5228),
            offset(0x434683C8, 0x4346758C, 0xE4E2D1A7),
            offset(0x43467FCC, 0x43467938, 0x8D1BD8D0),
            offset(0x43468C9C, 0x43469280, 0x4E36DFD7),
            offset(0x43467A74, 0x434678C0, 0xCD812654),
            offset(0x43468D48, 0x43467E94, 0xC6C78C82),
            offset(0x43469230, 0x434679A8, 0xA1AF8FFC),
            offset(0x43468C1C, 0x43468F28, 0x3C5CCD57),
            offset(0x43467F9C, 0x43467188, 0xB4056E61),
            offset(0x4346A358, 0x43468B78, 0x1136BA97),
            offset(0x43468950, 0x43469200, 0x67FC28B7),
            offset(0x43469080, 0x434682B4, 0xD172D9BA),
            offset(0x4346A354, 0x43467500, 0x3353C75F),
            offset(0x43468960, 0x43467AB4, 0x71DCEE42),
            offset(0x4346A6CC, 0x43468EF8, 0x2F4E97C4),
            offset(0x43468F84, 0x43467E64, 0x3B704B41),
            offset(0x4346A5DC, 0x43468430, 0xCDE290D2),
            offset(0x43467E8C, 0x43467900, 0x32E4DAF1),
            offset(0x43469344, 0x43467018, 0x49D93E62) - 0x18,
            offset(0x43468D30, 0x43468ED4, 0x2A671B42),
            offset(0x43468368, 0x43467FAC, 0x3B1F6ECC),
            offset(0x43468BD4, 0x4346A678, 0x7501CB91),
            offset(0x43467044, 0x4346A3BC, 0xB7E9C94E),
            offset(0x43467964, 0x4346A744, 0x902A2BFD),
            offset(0x43469224, 0x4346718C, 0x83A6CF2B)};
}
NativeEntityCosmetics::NativeEntityCosmetics(const Settings& settings, NativeCosmeticOffsets fields,
                                             NativeCosmeticGlobals globals,
                                             NativeCosmeticBindings bindings)
    : settings_(settings), fields_(fields), globals_(globals), bindings_(bindings) {}
void NativeEntityCosmetics::initialize_glove_owner() noexcept {
    glove_handle_ = invalid;
    glove_factory_ = globals_.glove_class;
}
void NativeEntityCosmetics::initialize_mask_owner(NativeMaskResources& resources) noexcept {
    mask_handle_ = invalid;
    mask_factory_ = nullptr;
    mask_resources_ = &resources;
}
void NativeEntityCosmetics::resolve_mask_owner_class() {
    mask_factory_ = engine::find_client_class(bindings_.base_client, 0xF89B2C53);
}
void NativeEntityCosmetics::release_glove_owner(void* player) {
    teardown_glove(player);
}
void NativeEntityCosmetics::release_mask_owner() {
    teardown_mask();
}
void* NativeEntityCosmetics::resolve(std::uint32_t value) {
    return value == invalid ? nullptr : engine::invoke<void*>(bindings_.entity_list, 4, value);
}
std::uint32_t NativeEntityCosmetics::handle(void* entity) {
    return *engine::invoke<const std::uint32_t*>(entity, 2);
}
void* NativeEntityCosmetics::create(engine::ClientClass* factory, const char* model) {
    const auto old_highest = read<std::int32_t>(bindings_.entity_list, 0x24);
    using Create = void*(__cdecl*)(std::int32_t, std::int32_t);
    auto* created = reinterpret_cast<Create>(factory->create)(0x7FE, 0x7FF);
    if (!created)
        return nullptr;
    if (bindings_.register_entity && bindings_.unregister_entity) {
        auto* entity = engine::invoke<void*>(engine::invoke<void*>(created, 0), 7);
        bindings_.register_entity(complete_entity_list(bindings_.entity_list),
                                  entity_index(handle(entity)));
        write(bindings_.entity_list, 0x24, old_highest);
        if (engine::invoke<std::int32_t>(bindings_.model_info, 2, model) != -1 &&
            engine::invoke<bool>(entity, 0x184 / 4, model, false))
            write(entity, 0x64, entity_index(handle(entity)));
        else {
            auto value = invalid;
            bindings_.unregister_entity(complete_entity_list(bindings_.entity_list), &value,
                                        entity);
        }
    }
    engine::invoke<void>(created, 3, 0);
    return engine::invoke<void*>(engine::invoke<void*>(created, 0), 7);
}
void NativeEntityCosmetics::destroy(void* entity, bool recreating) {
    if (!entity)
        return;
    auto* networkable = engine::invoke<void*>(entity, 4);
    if (!networkable)
        return;
    if (recreating)
        engine::invoke<void>(networkable, 0x34 / 4);
    engine::invoke<void>(networkable, 3, 1);
    engine::invoke<void>(networkable, 1);
}
void* NativeEntityCosmetics::item_definition(std::uint32_t definition) {
    if (!globals_.item_system && bindings_.acquire_item_system)
        globals_.item_system = bindings_.acquire_item_system();
    if (!globals_.item_system)
        return nullptr;
    auto* schema = engine::invoke<void*>(globals_.item_system, 0);
    return schema ? engine::invoke<void*>(schema, 4, definition) : nullptr;
}
std::int32_t NativeEntityCosmetics::model_index(const char* path) {
    if (!path || !*path)
        return -1;
    auto index = engine::invoke<std::int32_t>(bindings_.model_info, 2, path);
    if (index != -1)
        return index;
    void* precache = nullptr;
    const auto count = engine::invoke<std::int32_t>(bindings_.string_tables, 5);
    for (std::int32_t i = 0; i < count; ++i) {
        auto* table = engine::invoke<void*>(bindings_.string_tables, 4, i);
        if (!table)
            continue;
        const auto* name = engine::invoke<const char*>(table, 1);
        if (name && engine::netvar_name_hash(name) == 0x9FE6B136) {
            precache = table;
            break;
        }
    }
    if (precache && engine::invoke<void*>(bindings_.model_info, 0xAC / 4, path))
        index = engine::invoke<std::int32_t>(precache, 8, 0, path, -1,
                                             static_cast<const void*>(nullptr));
    return index;
}
void NativeEntityCosmetics::set_glove(void* entity, std::int32_t paint, std::uint32_t definition,
                                      std::int32_t model) {
    write(entity, fields_.item_id_low, std::int32_t(-1));
    write(entity, fields_.item_id_high, std::int32_t(-1));
    write(entity, fields_.paintkit, paint);
    write(entity, fields_.wear, glove_wear);
    write(entity, fields_.definition, std::uint16_t(definition));
    std::array<std::byte, 0x158> info;
    write(info.data(), 8, std::uint32_t(0));
    write(info.data(), 12, std::uint32_t(0));
    const auto index = engine::invoke<std::int32_t>(bindings_.engine_client, 12);
    engine::invoke<bool>(bindings_.engine_client, 8, index, info.data());
    write(entity, fields_.account_id, read<std::uint32_t>(info.data(), 8));
    engine::invoke<void>(bindings_.model_cache, 0x84 / 4);
    engine::invoke<void>(entity, 0x12C / 4, model);
    engine::invoke<void>(bindings_.model_cache, 0x88 / 4);
}
void NativeEntityCosmetics::attribute(void* entity, std::int32_t definition, float value) {
    const auto count = read<std::int32_t>(entity, fields_.attributes + 0x10);
    auto* cursor = read<std::byte*>(entity, fields_.attributes + 4);
    for (std::int32_t i = 0; i < count; ++i, cursor += 0x18) {
        if (read<std::int32_t>(cursor, 4) != definition)
            continue;
        if (read<float>(cursor, 8) == value && read<float>(cursor, 12) == value)
            return;
        write(cursor, 8, value);
        write(cursor, 12, value);
        if (auto* cache = read<void*>(entity, fields_.attribute_cache)) {
            engine::invoke<void>(cache, 0, 1);
            write(entity, fields_.attribute_cache, static_cast<void*>(nullptr));
        }
        return;
    }
}
bool NativeEntityCosmetics::glove_matches(void* entity, std::int32_t paint,
                                          std::uint32_t definition, std::int32_t model) {
    return read<std::int32_t>(entity, fields_.paintkit) == paint &&
           std::uint32_t(read<std::uint16_t>(entity, fields_.definition)) == definition &&
           read<std::int16_t>(entity, fields_.model_index) == model;
}
void NativeEntityCosmetics::detach(void* entity, void* player) {
    engine::invoke<void>(entity, 0x454 / 4, player, false);
    engine::invoke<void>(entity, 0x478 / 4, player);
}
void* NativeEntityCosmetics::view_model(void* player, std::int32_t index) {
    auto* result =
        resolve(read<std::uint32_t>(player, fields_.view_models + std::ptrdiff_t(index) * 4));
    if (read<std::int32_t>(player, fields_.observer_mode) != 4)
        return result;
    auto* target = resolve(read<std::uint32_t>(player, fields_.observer_target));
    if (target && target != player && !read<std::uint8_t>(target, fields_.life_state) &&
        engine::invoke<std::int32_t>(target, 0x1E8 / 4) > 0 &&
        !engine::invoke<bool>(subobject(target, 8), 0x24 / 4) &&
        !read<std::uint8_t>(target, fields_.observer_excluded))
        result = view_model(target, index);
    return result;
}
void NativeEntityCosmetics::teardown_glove(void* player) {
    if (glove_handle_ == invalid)
        return;
    if (player && read<std::uint32_t>(player, fields_.wearable) != invalid &&
        read<std::uint32_t>(player, fields_.wearable) == glove_handle_) {
        if (auto* entity = resolve(read<std::uint32_t>(player, fields_.wearable))) {
            detach(entity, player);
            write(player, fields_.wearable_counter, std::uint32_t(0));
        }
        write(player, fields_.wearable, invalid);
    }
    destroy(resolve(glove_handle_), false);
    glove_handle_ = invalid;
}
void NativeEntityCosmetics::update_gloves(void* player) {
    if (!player || engine::invoke<bool>(bindings_.engine_client, 82) ||
        engine::invoke<bool>(bindings_.engine_client, 93)) {
        teardown_glove(player);
        return;
    }
    const bool ct = read<std::int32_t>(player, fields_.team) == 3;
    if (!(ct ? settings_.glove_ct.value : settings_.glove_t.value) ||
        read<std::uint8_t>(player, fields_.dormant)) {
        teardown_glove(player);
        return;
    }
    if (read<std::uint8_t>(player, fields_.life_state))
        return;
    const auto definition =
        std::uint32_t(ct ? settings_.glove_model_ct.value : settings_.glove_model_t.value);
    auto* item = item_definition(definition);
    if (!item)
        return;
    const auto* world_path = engine::invoke<const char*>(item, 7);
    const auto* view_path = engine::invoke<const char*>(item, 6);
    if (!world_path || !view_path)
        return;
    const auto model = engine::invoke<std::int32_t>(bindings_.model_info, 2, world_path);
    const auto view = engine::invoke<std::int32_t>(bindings_.model_info, 2, view_path);
    if (model == -1 || view == -1 || !glove_factory_)
        return;
    const auto paint = ct ? settings_.glove_paint_ct.value : settings_.glove_paint_t.value;
    if (resolve(read<std::uint32_t>(player, fields_.wearable)) && !resolve(glove_handle_)) {
        if (auto* worn = resolve(read<std::uint32_t>(player, fields_.wearable))) {
            if (!glove_matches(worn, paint, definition, model))
                set_glove(worn, paint, definition, model);
            attribute(worn, 6, float(paint));
            attribute(worn, 8, glove_wear);
            return;
        }
    }
    if (auto* owned = resolve(glove_handle_)) {
        if (engine::invoke<engine::ClientClass*>(subobject(owned, 8), 2) != glove_factory_)
            return;
        if (glove_matches(owned, paint, definition, model) &&
            read<std::uint32_t>(player, fields_.wearable) == glove_handle_) {
            if (read<std::int32_t>(player, fields_.transmission_state) != 1)
                engine::invoke<void>(owned, 0x454 / 4, player, true);
            return;
        }
        if (read<std::uint32_t>(player, fields_.wearable) != invalid) {
            if (auto* worn = resolve(read<std::uint32_t>(player, fields_.wearable))) {
                detach(worn, player);
                write(player, fields_.wearable_counter, std::uint32_t(0));
            }
            write(player, fields_.wearable, invalid);
        }
        destroy(resolve(glove_handle_), true);
    }
    glove_handle_ = invalid;
    auto* created = create(glove_factory_, world_path);
    if (!created)
        return;
    glove_handle_ = handle(created);
    const bool replace_local = globals_.local_override == nullptr;
    auto* saved_local = globals_.local_override;
    if (replace_local)
        globals_.local_override = player;
    write(created, fields_.entity_flags, read<std::uint32_t>(created, fields_.entity_flags) | 0x80);
    write(created, fields_.quality, std::int32_t(11));
    set_glove(created, paint, definition, model);
    const auto saved_handle = read<std::uint32_t>(player, fields_.model_update_handle);
    write(player, fields_.model_update_handle, invalid);
    const auto instance = engine::invoke<std::uint32_t>(subobject(created, 4), 0x98 / 4);
    engine::invoke<void>(created, 0x488 / 4, instance);
    write(player, fields_.model_update_handle, saved_handle);
    if (auto* worn = resolve(read<std::uint32_t>(player, fields_.wearable)))
        detach(worn, player);
    write(player, fields_.wearable, invalid);
    engine::invoke<void>(created, 0x468 / 4, player);
    engine::invoke<void>(created, 0x454 / 4, player, true);
    write(created, fields_.owner_handle, handle(player));
    engine::invoke<void>(created, 0x80 / 4);
    if (auto* weapon = resolve(read<std::uint32_t>(player, fields_.active_weapon))) {
        if (auto* model_entity =
                view_model(player, read<std::int32_t>(weapon, fields_.view_model_index))) {
            if (bindings_.invalidate_bones)
                bindings_.invalidate_bones(model_entity);
            engine::invoke<void>(model_entity, 0x3F8 / 4, 0x20);
        }
    }
    write(created, fields_.initialized, std::uint8_t(1));
    engine::invoke<void>(subobject(created, 8), 0x14 / 4, 0);
    write(player, fields_.wearable_counter, std::uint32_t(0));
    if (replace_local)
        globals_.local_override = saved_local;
}
void NativeEntityCosmetics::zero_transform(void* entity, std::ptrdiff_t offset,
                                           std::int32_t flags) {
    if (read<float>(entity, offset) != 0.0f || read<float>(entity, offset + 4) != 0.0f ||
        read<float>(entity, offset + 8) != 0.0f) {
        if (bindings_.invalidate_transform)
            bindings_.invalidate_transform(entity, flags);
        write(entity, offset, std::array<float, 3>{0.0f, 0.0f, 0.0f});
    }
}
void NativeEntityCosmetics::teardown_mask() {
    destroy(resolve(mask_handle_), false);
    mask_handle_ = invalid;
}
void NativeEntityCosmetics::update_mask(void* player) {
    if (!bindings_.set_parent)
        return;
    if (!player || engine::invoke<bool>(bindings_.engine_client, 82) ||
        engine::invoke<bool>(bindings_.engine_client, 93)) {
        teardown_mask();
        return;
    }
    const bool ct = read<std::int32_t>(player, fields_.team) == 3;
    if (!(ct ? settings_.mask_ct.value : settings_.mask_t.value)) {
        teardown_mask();
        return;
    }
    if (read<std::uint8_t>(player, fields_.dormant) ||
        !engine::invoke<void*>(subobject(player, 4), 0x20 / 4))
        return;
    auto* header = read<void*>(player, fields_.model_header);
    if (!header || !read<std::uint32_t>(header, 0) ||
        read<std::uint8_t>(player, fields_.life_state))
        return;
    const auto attachment = engine::invoke<std::int32_t>(subobject(player, 4), 0x84 / 4,
                                                         mask_resources_->attachment.begin);
    if (attachment == -1)
        return;
    const auto selected = ct ? settings_.mask_model_ct.value : settings_.mask_model_t.value;
    if (std::uint32_t(selected) >= mask_resources_->filenames.size())
        return;
    // The constructor's fixed directory/filenames fit the native256-byte path.
    std::array<char, 0x100> path;
    const auto directory_length = std::strlen(mask_resources_->directory.begin);
    const auto* filename = mask_resources_->filenames[std::size_t(selected)].begin;
    std::memcpy(path.data(), mask_resources_->directory.begin, directory_length);
    std::memcpy(path.data() + directory_length, filename, std::strlen(filename) + 1);
    const auto model = model_index(path.data());
    if (model == -1)
        return;
    auto* entity = resolve(mask_handle_);
    if (entity) {
        if (engine::invoke<engine::ClientClass*>(subobject(entity, 8), 2) != mask_factory_)
            return;
        if (read<std::int16_t>(entity, fields_.model_index) == model &&
            resolve(read<std::uint32_t>(entity, fields_.parent_handle)) == player &&
            read<std::uint32_t>(entity, fields_.model_update_handle) == handle(player) &&
            read<std::uint32_t>(entity, fields_.owner_handle) == handle(player))
            return;
    } else {
        mask_handle_ = invalid;
        entity = create(mask_factory_, path.data());
        if (!entity)
            return;
        mask_handle_ = handle(entity);
    }
    engine::invoke<void>(bindings_.model_cache, 0x84 / 4);
    engine::invoke<void>(entity, 0x12C / 4, model);
    engine::invoke<void>(bindings_.model_cache, 0x88 / 4);
    zero_transform(entity, fields_.local_origin, 1);
    zero_transform(entity, fields_.local_angles, 2);
    bindings_.set_parent(entity, player, attachment);
    zero_transform(entity, fields_.local_origin, 1);
    zero_transform(entity, fields_.local_angles, 2);
    write(entity, fields_.quality, std::int32_t(0));
    write(entity, fields_.mask_byte_43468bd4, std::uint8_t(0));
    engine::invoke<void>(entity, 0x3B8 / 4, 0);
    write(entity, fields_.mask_bits_43467044,
          std::uint16_t(read<std::uint16_t>(entity, fields_.mask_bits_43467044) | 4));
    write(entity, fields_.mask_byte_43467964, std::uint8_t(0));
    write(entity, fields_.mask_flags_43469224,
          read<std::uint32_t>(entity, fields_.mask_flags_43469224) & 0xFFFBFFFF);
    write(entity, fields_.model_update_handle, handle(player));
    write(entity, fields_.owner_handle, handle(player));
}
} //namespace skeet::skins

// ============================================================================
// Subcomponent: native_inventory.cpp
// ============================================================================

namespace skeet::skins {
namespace {
/* duplicate field removed; using skeet::field */
template <class T> void assign(void* object, std::ptrdiff_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}
struct Identity {
    std::uint32_t low, high;
};
bool local_identity(void* const& engine_client, Identity& value) {
    std::array<std::byte, 0x158> info{};
    const auto index = engine::invoke<std::int32_t>(engine_client, 12);
    const bool result = engine::invoke<bool>(engine_client, 8, index, info.data());
    value = {field<std::uint32_t>(info.data(), 8), field<std::uint32_t>(info.data(), 12)};
    return result;
}
}
NativeInventoryRuntime::NativeInventoryRuntime(const WeaponSkinsConfig& settings,
                                               NativeInventoryBindings bindings,
                                               CounterRandom& random, WeaponRefresh& refresh)
    : settings_(settings), bindings_(bindings), random_(random), refresh_(refresh) {}
std::int32_t NativeInventoryRuntime::stattrak_counter(void* item) {
    if (bindings_.counter_rows && bindings_.counters) {
        const auto* name = engine::invoke<const char*>(item, 143);
        if (name) {
            for (auto* row = bindings_.counter_rows - 4;; row += 0x1C) {
                const auto index = field<std::int32_t>(row, 0xC);
                if (index == -1)
                    break;
                if (field<std::int32_t>(row, 0x10) == -1 || index > 0x133)
                    continue;
                const auto* candidate = field<const char*>(row, 0);
                if (candidate && std::strcmp(name, candidate) == 0) {
                    const auto value = bindings_.counters[index];
                    if (value != -1)
                        return value;
                    break;
                }
            }
        }
    }
    return random_.random_integer(1, 10000);
}
void NativeInventoryRuntime::process(void* local_player, std::uintptr_t incoming) {
    if (!local_player)
        return;
    if (std::none_of(settings_.value.begin(), settings_.value.end(),
                     [](const WeaponSkin& value) { return value.enabled != 0; }))
        return;
    const auto& globals = bindings_.offsets;
    const auto offset = [&](std::uint32_t pointer, std::uint32_t mask, std::uint32_t key) {
        return std::ptrdiff_t(*globals.pointers.at(pointer) ^ globals.keys.at(mask) ^ key);
    };
    if (field<std::uint8_t>(local_player, offset(0x43468E78, 0x434684E4, 0x9D920E35)) != 0)
        return;
    const auto* snapshot = reinterpret_cast<const std::byte*>(incoming);
    const auto count = field<std::int32_t>(snapshot, 0x4044);
    if (count <= 0)
        return;
    const auto* entry = snapshot + 0x44;
    for (std::int32_t i = 0; i < count; ++i, entry += 8) {
        auto* networkable =
            engine::invoke<void*>(bindings_.resolver, 0, field<std::uint32_t>(entry, 0));
        if (!networkable)
            continue;
        auto* unknown = engine::invoke<void*>(networkable, 0);
        if (!unknown)
            continue;
        auto* base = engine::invoke<void*>(unknown, 7);
        if (!base || !engine::invoke<bool>(base, 166))
            continue;
        const auto tag = field<std::uint32_t>(entry, 4);
        auto* item = static_cast<std::byte*>(networkable) - 8;
        const auto owner_high =
            field<std::uint32_t>(item, offset(0x43468390, 0x434683E4, 0xA1493792));
        const auto owner_low =
            field<std::uint32_t>(item, offset(0x43469248, 0x4346798C, 0x3C1D8AA6));
        Identity identity{};
        const bool identity_valid = local_identity(bindings_.engine_client, identity);
        if (!identity_valid || (!identity.low && !identity.high) || owner_low != identity.low ||
            owner_high != identity.high) {
            if (field<std::uint32_t>(item, offset(0x43469248, 0x4346798C, 0x3C1D8AA6)) != 0 ||
                field<std::uint32_t>(item, offset(0x43468E5C, 0x43469334, 0x05FB21BA)) != 0 ||
                tag != 0)
                continue;
            const auto handle =
                field<std::uint32_t>(item, offset(0x4346A868, 0x43468998, 0x4A3412EC));
            const auto owner_index =
                handle == 0xFFFFFFFFu ? 0x1FFFu : std::uint32_t(static_cast<std::uint16_t>(handle));
            if (owner_index !=
                std::uint32_t(engine::invoke<std::int32_t>(bindings_.engine_client, 12)))
                continue;
        }
        const auto definition =
            field<std::uint16_t>(item, offset(0x4346908C, 0x43468BA0, 0x5C9F0FBB));
        std::int32_t team = -1;
        if (std::uint32_t(definition) - 500u < 26u)
            team = field<std::int32_t>(local_player, offset(0x434688B0, 0x4346A518, 0x8B5586F6));
        const auto slot = weapon_config_slot(definition, team);
        if (slot < 0)
            continue;
        const auto& skin = settings_.value[std::size_t(slot)];
        if (!skin.enabled)
            continue;
        assign(item, offset(0x43468C1C, 0x43468F28, 0x3C5CCD57), std::int32_t(-1));
        assign(item, offset(0x43467F9C, 0x43467188, 0xB4056E61), std::int32_t(-1));
        Identity account{};
        local_identity(bindings_.engine_client, account);
        if (account.low)
            assign(item, offset(0x4346A358, 0x43468B78, 0x1136BA97), account.low);
        const auto paintkit = offset(0x4346A8A8, 0x43468568, 0xC6D22EE2);
        const auto previous = field<std::int32_t>(item, paintkit);
        assign(item, paintkit, skin.paintkit);
        assign(item, offset(0x43468DC8, 0x43467A44, 0x2A595D03), skin.seed);
        assign(item, offset(0x43468950, 0x43469200, 0x67FC28B7), quality_to_wear(skin.quality));
        if (previous == skin.paintkit)
            continue;
        if (skin.stattrak) {
            const auto count = stattrak_counter(item);
            assign(item, offset(0x434682E8, 0x4346A6C8, 0xF2DC2C8A), count);
        }
        refresh_.schedule_full_update(refresh_.current_time() + 0.25f);
    }
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: native_schema.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::skins {
bool NativeSchemaEngine::acquire_schema() {
    if (!bindings_.item_system && bindings_.acquire_item_system)
        bindings_.item_system = bindings_.acquire_item_system();
    schema_ = bindings_.item_system ? engine::invoke<void*>(bindings_.item_system, 0) : nullptr;
    return schema_ != nullptr;
}
std::int32_t NativeSchemaEngine::definition_count() {
    return engine::invoke<std::int32_t>(schema_, 3); //433CD70E, vtable+0C
}
std::optional<SchemaItem> NativeSchemaEngine::definition(std::uint32_t id) {
    auto* item = engine::invoke<void*>(schema_, 4, id); //433CD70E, vtable+10
    if (!item)
        return std::nullopt;
    SchemaItem result;
    result.type_token = engine::invoke<const char*>(item, 3);
    if (result.type_token) {
        const auto type = events::event_hash(result.type_token);
        // Preserve the actual branch-specific virtual getter order. Agent
        // definitions never query the glove-only world model method.
        if (type == 0x2B77F7D8) {
            result.view_model = engine::invoke<const char*>(item, 6);
            result.base_name_token = engine::invoke<const char*>(item, 2);
        } else if (type == 0x8E483246) {
            result.world_model = engine::invoke<const char*>(item, 7);
            result.view_model = engine::invoke<const char*>(item, 6);
            result.base_name_token = engine::invoke<const char*>(item, 2);
        }
    }
    return result;
}
const char* NativeSchemaEngine::paintkit_token(std::uint32_t id) {
    if (!bindings_.paintkit_definition)
        return nullptr;
    const auto* definition = bindings_.paintkit_definition(schema_, id);
    if (!definition)
        return nullptr;
    const char* token;
    std::memcpy(&token, static_cast<const std::byte*>(definition) + 0x24, sizeof(token));
    return token;
}
std::optional<std::u16string> NativeSchemaEngine::localize(const char* token) {
    //43397087/433E533F ultimately call this actual localization vtable entry.
    const auto* text = engine::invoke<const char16_t*>(bindings_.localization, 11, token);
    if (!text)
        return std::nullopt;
    return std::u16string(text);
}
std::u16string NativeSchemaFiles::current_directory() {
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    std::array<wchar_t, 260> directory;
    const auto length = GetCurrentDirectoryW(DWORD(directory.size()), directory.data());
    if (!length)
        return {};
    if (length >= directory.size())
        throw std::length_error(
            "Native schema current directory exceeds its 260-character storage");
    return {reinterpret_cast<const char16_t*>(directory.data()), length};
}
extern "C" __declspec(dllimport) BOOLEAN NTAPI RtlDosPathNameToNtPathName_U(PCWSTR, PUNICODE_STRING, PCWSTR*, void*);
std::optional<std::string> NativeSchemaFiles::read_file(std::u16string_view path) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    const std::u16string terminated(path);
    UNICODE_STRING name{};
    if (!RtlDosPathNameToNtPathName_U(reinterpret_cast<const wchar_t*>(terminated.c_str()), &name,
                                      nullptr, nullptr))
        return std::nullopt;
    struct Release {
        UNICODE_STRING& name;
        ~Release() {
            RtlFreeUnicodeString(&name);
        }
    } release{name};
    const auto bytes = read_native_file(
        {name.Length, name.MaximumLength, reinterpret_cast<char16_t*>(name.Buffer)});
    if (!bytes)
        return std::nullopt;
    if (bytes->empty())
        return std::string{};
    return std::string(reinterpret_cast<const char*>(bytes->data()), bytes->size());
}
} //namespace skeet::skins

// ============================================================================
// Subcomponent: native_sequence_update.cpp
// ============================================================================

namespace skeet::skins {
namespace {
/* duplicate invalid, read, write removed */
void* at(void* object, std::ptrdiff_t offset) {
    return static_cast<std::byte*>(object) + offset;
}
std::uint32_t name_hash(const char* text) {
    std::uint32_t result = 0x811C9DC5;
    for (; *text; ++text)
        result = (result ^ std::uint8_t(*text)) * 0x1000193u;
    return result ^ 0x92CA5DF1;
}
void receive(engine::RecvVarProxy proxy, void* entity, std::ptrdiff_t offset, std::int32_t value) {
    //433E593A/433E5Fxx: only the two intermediate variant words are unspecified.
    std::array<std::uint32_t, 10> data;
    data[0] = data[1] = 0;
    data[2] = std::bit_cast<std::uint32_t>(value);
    data[5] = 1;
    data[6] = data[7] = data[8] = data[9] = 0;
    proxy(reinterpret_cast<const engine::RecvProxyData*>(data.data()), entity, at(entity, offset));
}
struct AnimationContext {
    void* animation;
    std::array<std::uint32_t, 6> unspecified;
    void* entity;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(AnimationContext, entity) == 0x1C);
#endif
}

std::ptrdiff_t NativeSequenceRuntime::offset(std::uint32_t pointer, std::uint32_t key,
                                             std::uint32_t mask) const {
    const auto& globals = bindings_.offsets.globals;
    return std::bit_cast<std::int32_t>(*globals.pointers.at(pointer) ^ globals.keys.at(key) ^ mask);
}
std::int32_t NativeSequenceRuntime::team(const void* player) const {
    return read<std::int32_t>(player, offset(0x434688B0, 0x4346A518, 0x8B5586F6));
}
bool NativeSequenceRuntime::enabled(const void* player) const {
    return (team(player) == 3 ? settings_.knife_ct.value : settings_.knife_t.value) != 0;
}
std::int32_t NativeSequenceRuntime::selection(const void* player) const {
    return team(player) == 3 ? settings_.knife_model_ct.value : settings_.knife_model_t.value;
}
bool NativeSequenceRuntime::same_owner(const void* entity) const {
    const auto high = read<std::uint32_t>(entity, offset(0x43468390, 0x434683E4, 0xA1493792));
    const auto low = read<std::uint32_t>(entity, offset(0x43469248, 0x4346798C, 0x3C1D8AA6));
    const auto index = engine::invoke<std::int32_t>(bindings_.engine_client, 12);
    std::array<std::byte, 0x158> information;
    write(information.data(), 8, std::uint32_t(0));
    write(information.data(), 12, std::uint32_t(0));
    if (!engine::invoke<bool>(bindings_.engine_client, 8, index, information.data()))
        return false;
    const auto current_low = read<std::uint32_t>(information.data(), 8);
    const auto current_high = read<std::uint32_t>(information.data(), 12);
    return (current_low || current_high) && low == current_low && high == current_high;
}
std::uint32_t NativeSequenceRuntime::class_id(void* entity) const {
    return engine::invoke<std::uint32_t>(at(entity, 8), 2);
}
void* NativeSequenceRuntime::resolve(std::uint32_t handle) const {
    return engine::invoke<void*>(bindings_.entity_list, 4, handle);
}
std::int32_t NativeSequenceRuntime::sequence_count(void* header) const {
    if (!read<std::int32_t>(header, 0x150))
        return read<std::int32_t>(header, 0xBC);
    auto* model = engine::invoke<void*>(bindings_.model_info, 26, header); //433FD399/433CCB8F
    return read<std::int32_t>(model, 0x14);
}
void* NativeSequenceRuntime::sequence_description(void* header, std::int32_t index) const {
    if (read<std::int32_t>(header, 0x150)) {
        if (auto* model = engine::invoke<void*>(bindings_.model_info, 26, header)) { //43417239
            const auto* sequences = read<const std::byte*>(model, 8);
            const auto group = read<std::int32_t>(sequences, std::ptrdiff_t(index) * 16 + 8);
            const auto* groups = read<const std::byte*>(model, 0x58);
            auto* group_header = engine::invoke<void*>(
                bindings_.model_info, 24, read<void*>(groups, std::ptrdiff_t(group) * 0x90));
            index = read<std::int32_t>(sequences, std::ptrdiff_t(index) * 16 + 12);
            header = group_header;
        }
    }
    if (index < 0 || index >= read<std::int32_t>(header, 0xBC))
        index = 0; //433E9F0C
    return at(header, std::ptrdiff_t(index) * 0xD4 + read<std::int32_t>(header, 0xC0));
}
std::int32_t NativeSequenceRuntime::weighted_sequence(void* entity, std::uint32_t seed) const {
    if (!bindings_.animation)
        return -1;
    auto* random_seed = bindings_.random_seed;
    std::int32_t previous_seed = 0;
    if (random_seed) {
        previous_seed = *random_seed;
        *random_seed = std::bit_cast<std::int32_t>(seed);
    }
    auto** prediction_player = bindings_.prediction_player;
    auto* local = bindings_.receive.local_player;
    void* previous_player = nullptr;
    if (prediction_player && local) {
        previous_player = *prediction_player;
        *prediction_player = local;
    }
    AnimationContext context;
    context.animation = bindings_.animation;
    context.entity = entity;
    const auto target =
        read<engine::MemberFunction<std::int32_t, std::int32_t>>(context.animation, 0x5C);
    const auto result = target(&context, 213);
    if (prediction_player && local)
        *prediction_player = previous_player;
    if (random_seed)
        *random_seed = previous_seed;
    return result;
}
std::int32_t NativeSequenceRuntime::inspection_sequence(void* entity) const {
    if (!bindings_.animation)
        return -1;
    AnimationContext context;
    context.animation = bindings_.animation;
    context.entity = entity;
    //433C0554 leaves these two words on the target's stack; it does not pass
    //the context as an ECX receiver, unlike the weighted+5C engine target.
    using Lookup = std::int32_t(__cdecl*)(AnimationContext*, const char*);
    return read<Lookup>(context.animation, 0x64)(&context, state_.inspection_sequence.data());
}

void NativeSequenceRuntime::remap(void* viewmodel, std::int32_t& model, std::int32_t& sequence) {
    auto* local = bindings_.receive.local_player;
    if (!local || engine::invoke<bool>(bindings_.engine_client, 82) ||
        engine::invoke<bool>(bindings_.engine_client, 93) || !enabled(local) || !state_.knife_class)
        return;
    auto* weapon_field =
        static_cast<std::byte*>(viewmodel) + offset(0x43468A98, 0x43467E78, 0xA116369A);
    const auto weapon_handle = read<std::uint32_t>(weapon_field, 0);
    if (weapon_handle == invalid)
        return;
    auto* weapon = resolve(weapon_handle);
    if (!weapon || !same_owner(weapon))
        return;
    const auto weapon_class = class_id(weapon);
    const bool knife = weapon_class == state_.knife_class;
    std::int32_t selected_model = -1;
    local = bindings_.receive.local_player;
    if (knife && local) {
        const auto index = std::uint32_t(selection(local));
        if (index >= state_.models.size())
            return;
        selected_model = state_.models[index].entity_model;
        if (selected_model == -1)
            return;
        const auto previous =
            model == -1 ? read<std::int16_t>(viewmodel, offset(0x43468978, 0x43468F7C, 0x47877DEB))
                        : model;
        if (previous != selected_model)
            model = selected_model;
    }
    const auto command = engine::invoke<std::uint32_t>(bindings_.engine_client, 205);
    //433E561F captures the live owner after the engine command query.
    const auto& history =
        misc::viewmodel_lag_owner_state(bindings_.viewmodel_owner)->snapshots[command % 150];
    if (history.command != command ||
        history.weapon_handle != read<std::uint32_t>(weapon_field, 0) ||
        history.animation_token_08 !=
            read<std::int32_t>(viewmodel, offset(0x43468DA0, 0x4346A6BC, 0x695FF325)))
        return;
    if (history.animation_token_0c ==
            read<std::int32_t>(viewmodel, offset(0x43467B4C, 0x43468414, 0xAEF72A04)) &&
        (!knife || history.selected_model == selected_model)) {
        sequence = history.sequence;
        return;
    }
    local = bindings_.receive.local_player;
    if (!local || read<std::uint8_t>(local, offset(0x43468E78, 0x434684E4, 0x9D920E35)))
        return;
    if (read<std::uint8_t>(local, offset(0x43467EC4, 0x43467AF8, 0x53E7C7D2))) {
        auto mapped = weighted_sequence(viewmodel, history.random_seed);
        if (mapped == -1)
            mapped = inspection_sequence(viewmodel);
        if (mapped != -1)
            sequence = mapped;
        return;
    }
    if (!knife || sequence < 0)
        return;
    const auto index = std::uint32_t(selection(bindings_.receive.local_player));
    const auto& target = state_.models[index];
    if (target.entity_model == state_.original_model || state_.original_model == -1)
        return;
    auto* source_model = engine::invoke<void*>(bindings_.model_info, 1, state_.original_model);
    if (!source_model)
        return;
    auto* source = engine::invoke<void*>(bindings_.model_info, 32, source_model);
    if (!source || sequence >= sequence_count(source))
        return;
    auto* description = sequence_description(source, sequence);
    const auto* name = static_cast<const char*>(description) + read<std::int32_t>(description, 4);
    if (!*name)
        name = nullptr;
    const auto activity = read<std::int32_t>(description, 0x10);
    auto* destination_model = engine::invoke<void*>(bindings_.model_info, 1, target.entity_model);
    if (!destination_model)
        return;
    auto* destination = engine::invoke<void*>(bindings_.model_info, 32, destination_model);
    if (!destination)
        return;
    const auto count = sequence_count(destination);
    if (name) {
        std::array<char, 64> copied{};
        ::strcpy_s(copied.data(), copied.size(), name);
        const auto variants = knife_models()[index].native_word_08;
        if (variants > 1 && name_hash(name) == 0xD4A1BEA9u) {
            const auto time = read<float>(weapon, offset(0x43467A50, 0x4346A458, 0xF71CE331));
            const auto ticks = time / *bindings_.interval + 0.5f;
            bindings_.seed_random(_mm_cvttss_si32(_mm_set_ss(ticks)));
            copied[7] =
                char(std::uint8_t(bindings_.random_integer(1, std::int32_t(variants))) + 0x30);
        }
        for (std::int32_t i = 0; i < count; ++i) {
            auto* candidate = sequence_description(destination, i);
            const auto* candidate_name =
                static_cast<const char*>(candidate) + read<std::int32_t>(candidate, 4);
            if (::_stricmp(candidate_name, copied.data()) == 0) {
                sequence = i;
                return;
            }
        }
    }
    if (activity != -1)
        for (std::int32_t i = 0; i < count; ++i)
            if (read<std::int32_t>(sequence_description(destination, i), 0x10) == activity) {
                sequence = i;
                return;
            }
}

void NativeSequenceRuntime::process(void* player, std::uintptr_t incoming) {
    // Keep each native vector's original end until its replay loop finishes.
    const auto end = state_.viewmodels.end();
    for (auto pending = state_.viewmodels.begin(); pending != end; ++pending) {
        if (pending->handle == invalid)
            continue;
        auto* entity = resolve(pending->handle);
        if (!entity)
            continue;
        std::int32_t model = pending->model, sequence = pending->sequence;
        remap(entity, model, sequence);
        if (model != -1)
            receive(bindings_.receive.model, entity, offset(0x43468978, 0x43468F7C, 0x47877DEB),
                    model);
        if (sequence != -1)
            receive(bindings_.receive.sequence, entity, offset(0x43467ABC, 0x4346A56C, 0xA25D8271),
                    sequence);
    }
    state_.viewmodels.clear();
    auto* local = bindings_.receive.local_player;
    if (player && !read<std::uint8_t>(player, offset(0x43468E78, 0x434684E4, 0x9D920E35)) &&
        local && !engine::invoke<bool>(bindings_.engine_client, 82) &&
        !engine::invoke<bool>(bindings_.engine_client, 93) && enabled(local)) {
        const auto* snapshot = reinterpret_cast<const std::byte*>(incoming);
        const auto count = read<std::int32_t>(snapshot, 0x4044);
        auto* resolver =
            state_.inventory_resolver; //captured by4348DE49, after descriptor registration
        for (std::int32_t i = 0; i < count; ++i) {
            const auto* entry = snapshot + 0x44 + std::ptrdiff_t(i) * 8;
            auto* networkable = engine::invoke<void*>(resolver, 0, read<std::uint32_t>(entry, 0));
            if (!networkable)
                continue;
            auto* unknown = engine::invoke<void*>(networkable, 0);
            if (!unknown || !engine::invoke<void*>(unknown, 7))
                continue;
            const auto entity_class = engine::invoke<std::uint32_t>(networkable, 2);
            if (!entity_class)
                continue;
            auto* entity = static_cast<std::byte*>(networkable) - 8;
            if (entity_class == state_.knife_class) {
                const auto tag = read<std::uint32_t>(entry, 4);
                if (!same_owner(entity))
                    continue;
                const auto index = std::uint32_t(selection(player));
                if (index >= state_.models.size())
                    continue;
                if (!tag) {
                    state_.original_model =
                        read<std::int16_t>(entity, offset(0x43468978, 0x43468F7C, 0x47877DEB));
                    state_.original_definition =
                        read<std::uint16_t>(entity, offset(0x4346908C, 0x43468BA0, 0x5C9F0FBB));
                }
                const auto& model = state_.models[index];
                write(entity, offset(0x4346908C, 0x43468BA0, 0x5C9F0FBB),
                      knife_models()[index].definition);
                write(entity, offset(0x43469370, 0x43468E74, 0x1D7092B7), model.entity_model);
                write(entity, offset(0x434679E4, 0x43467DE4, 0xE7BAED36), model.view_model);
                write(entity, offset(0x43467474, 0x43468904, 0x26F3C118), model.world_model);
                write(entity, offset(0x43468940, 0x4346A818, 0xADBAA629), std::int32_t(3));
                if (model.entity_model == -1 ||
                    model.entity_model ==
                        read<std::int16_t>(entity, offset(0x43468978, 0x43468F7C, 0x47877DEB)))
                    continue;
                engine::invoke<void>(bindings_.model_cache, 33);
                const auto selected_model = model.entity_model;
                engine::invoke<void>(bindings_.model_cache, 33);
                engine::invoke<void>(entity, 75, selected_model);
                engine::invoke<void>(bindings_.model_cache, 34);
                engine::invoke<void>(bindings_.model_cache, 34);
                if (tag)
                    refresh_.schedule_full_update(refresh_.current_time() + 0.25f);
            } else if (entity_class == state_.weapon_world_model_class) {
                local = bindings_.receive.local_player;
                if (!local)
                    continue;
                const auto index = std::uint32_t(selection(local));
                if (index >= state_.models.size())
                    continue;
                const auto owner =
                    read<std::uint32_t>(entity, offset(0x43468C9C, 0x43469280, 0x4E36DFD7));
                if (owner == invalid ||
                    std::uint32_t(std::uint16_t(owner)) !=
                        engine::invoke<std::uint32_t>(bindings_.engine_client, 12))
                    continue;
                const auto handle =
                    read<std::uint32_t>(entity, offset(0x4346922C, 0x434691D8, 0xA4CECBE2));
                if (handle == invalid)
                    continue;
                auto* weapon = resolve(handle);
                if (!weapon)
                    continue;
                const auto weapon_class = class_id(weapon);
                if (weapon_class != state_.knife_class || !same_owner(weapon))
                    continue;
                const auto model = state_.models[index].view_model;
                if (model == -1 ||
                    model ==
                        read<std::int16_t>(entity, offset(0x43468978, 0x43468F7C, 0x47877DEB)) ||
                    !bindings_.viewmodel_model)
                    continue;
                receive(bindings_.viewmodel_model, entity,
                        offset(0x43468978, 0x43468F7C, 0x47877DEB), model);
                write(entity, offset(0x43468978, 0x43468F7C, 0x47877DEB), std::int16_t(model));
            }
        }
    }
    const auto handles_end = state_.weapon_handles.end();
    for (auto pending = state_.weapon_handles.begin(); pending != handles_end; ++pending) {
        if (pending->handle == invalid)
            continue;
        if (auto* entity = resolve(pending->handle))
            receive(bindings_.receive.weapon_handle, entity,
                    offset(0x4346922C, 0x434691D8, 0xA4CECBE2),
                    std::bit_cast<std::int32_t>(pending->value));
    }
    state_.weapon_handles.clear();
}
} //namespace skeet::skins

// ============================================================================
// Subcomponent: refresh.cpp
// ============================================================================

namespace skeet::skins {
namespace {
template <class T> T read_field(const void* object, std::ptrdiff_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}
template <class T> void write_field(void* object, std::ptrdiff_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}
}
void WeaponRefresh::invalidate_weapon_models() { // 4334D9E5
    if (!engine::invoke<bool>(bindings_.engine_client, 26))
        return;
    const auto index = engine::invoke<std::int32_t>(bindings_.engine_client, 12);
    const auto player = engine::invoke<void*>(bindings_.entity_list, 3, index);
    if (player && read_field<std::uint8_t>(player, bindings_.life_state) == 0)
        globals_.full_update = 1;
}
float WeaponRefresh::current_time() const {
    return *globals_.current_time;
}
void WeaponRefresh::schedule_full_update(float time) {
    globals_.hud_refresh_at = time;
}
std::int32_t WeaponRefresh::agent_team() const { // 433821B6
    if (!engine::invoke<bool>(bindings_.engine_client, 26))
        return 2;
    if (engine::invoke<bool>(bindings_.engine_client, 82))
        return 2;
    if (engine::invoke<bool>(bindings_.engine_client, 93))
        return 2;
    return bindings_.local_player ? read_field<std::int32_t>(bindings_.local_player, bindings_.team)
                                  : 2;
}
void WeaponRefresh::clear_hud_weapon_selection() { // 433F11C8
    if (!bindings_.hud || !bindings_.find_hud)
        return;
    auto* element = bindings_.find_hud(bindings_.hud, "CCSGO_HudWeaponSelection");
    if (!element)
        return;
    auto* panel = read_field<void*>(element, 0x40);
    if (!panel ||
        reinterpret_cast<std::uintptr_t>(panel) != reinterpret_cast<std::uintptr_t>(element) - 0xA0)
        return;
    auto* entries = read_field<std::byte*>(panel, 0x74);
    if (!entries)
        return;
    const auto count = read_field<std::uint32_t>(panel, 0x80);
    if (count - 1u >= 64u)
        return;
    bool changed = false;
    for (std::uint32_t i = 0; i < count; ++i) {
        auto* entry = entries + std::size_t(i) * 0x34;
        if (read_field<std::int32_t>(entry, 0x28) != -1) {
            write_field(entry, 0x28, std::int32_t(-1));
            changed = true;
        }
    }
    if (!changed)
        return;
    auto* event = engine::invoke<void*, const char*, bool, std::int32_t>(bindings_.event_manager, 7,
                                                                         "round_start", false, 0);
    engine::invoke<void>(element, 1, event);
    engine::invoke<void>(bindings_.event_manager, 11, event);
}
void WeaponRefresh::process_deferred_hud_refresh() {
    if (globals_.hud_refresh && current_time() >= globals_.hud_refresh_at) {
        globals_.hud_refresh_at = std::numeric_limits<float>::max();
        clear_hud_weapon_selection();
    }
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: runtime_owner.cpp
// ============================================================================

namespace skeet::skins {
namespace {
template <class T> T& cell(client::ClientRuntimeStorage& storage, std::uint32_t address) {
    return storage.data().native_cell<T>(address);
}
std::ptrdiff_t offset(const engine::NativeOffsetGlobals& globals, std::uint32_t pointer,
                      std::uint32_t key, std::uint32_t mask) {
    return std::ptrdiff_t(*globals.pointers.at(pointer) ^ globals.keys.at(key) ^ mask);
}
Settings& require_inputs(client::ClientRuntimeStorage& storage,
                         client::ConfigurationRuntimeOwner& configuration,
                         const engine::BuildSkinBindings& imports) {
    if (&configuration.storage() != &storage || &imports.data != &storage.data())
        throw std::invalid_argument(
            "Skin runtime requires its canonical configuration and data owners");
    if (!imports.seed_random || !imports.random_integer || !imports.weapon_world_model_class ||
        !imports.knife_class)
        throw std::invalid_argument(
            "Skin runtime requires genuine engine random imports and client class objects");
    if (storage.event_registry().engine_interface != cell<void*>(storage, 0x4346A514u) ||
        !storage.event_registry().engine_interface ||
        storage.event_registry().event_manager != cell<void*>(storage, 0x43468364u) ||
        !storage.event_registry().event_manager)
        throw std::invalid_argument(
            "Skin death listeners require the canonically bound engine event registry");
    for (const auto address : {0x4346A514u, 0x4346A890u, 0x434683E0u, 0x4346A614u, 0x43469330u,
                               0x43467198u, 0x4346A31Cu, 0x43468C94u, 0x4346841Cu})
        if (!cell<void*>(storage, address))
            throw std::invalid_argument(
                "Skin runtime requires the initialized canonical engine interfaces and data operands");
    if (!cell<NativeSchemaBindings::AcquireItemSystem>(storage, 0x43468B9Cu) ||
        !cell<NativeSchemaBindings::PaintkitDefinition>(storage, 0x43467040u) ||
        !cell<NativeAgentBindings::SupportModels>(storage, 0x4346D64Cu) ||
        !cell<const float*>(storage, 0x4346A334u))
        throw std::invalid_argument(
            "Skin runtime requires its real item, schema, support-model and clock bindings");
    return configuration.skins_settings();
}
class NativeModels final : public ModelInfo {
public:
    explicit NativeModels(void* const& source) : source_(source) {}
    std::int32_t model_index(std::string_view path) override {
        const std::string text(path);
        return engine::invoke<std::int32_t>(source_, 2, text.c_str());
    }
    bool load_model(std::string_view path) override {
        const std::string text(path);
        return engine::invoke<void*>(source_, 43, text.c_str()) != nullptr;
    }

private:
    void* const& source_;
};
class NativeTable final : public StringTable {
public:
    explicit NativeTable(void* source) : source_(source) {}
    const char* name() override {
        return engine::invoke<const char*>(source_, 1);
    }
    std::int32_t add_string(std::int32_t flags, std::string_view path, std::int32_t length,
                            const void* data) override {
        const std::string text(path);
        return engine::invoke<std::int32_t>(source_, 8, flags, text.c_str(), length, data);
    }

private:
    void* source_;
};
class NativeTables final : public StringTables {
public:
    explicit NativeTables(void* const& source) : source_(source) {}
    std::int32_t count() override {
        return engine::invoke<std::int32_t>(source_, 5);
    }
    StringTable* at(std::int32_t index) override {
        auto* table = engine::invoke<void*>(source_, 4, index);
        if (!table)
            return nullptr;
        auto& view = views_[table];
        if (!view)
            view = std::make_unique<NativeTable>(table);
        return view.get();
    }

private:
    void* const& source_;
    std::map<void*, std::unique_ptr<NativeTable>> views_;
};
class NativeCounterRandom final : public CounterRandom {
public:
    explicit NativeCounterRandom(engine::BuildSkinBindings::RandomInteger function)
        : function_(function) {}
    std::int32_t random_integer(std::int32_t minimum, std::int32_t maximum) override {
        return function_(minimum, maximum);
    }

private:
    engine::BuildSkinBindings::RandomInteger function_;
};
NativeCosmeticBindings cosmetic_bindings(client::ClientRuntimeStorage& storage) {
    return {cell<void*>(storage, 0x434683E0u),
            cell<void*>(storage, 0x4346A514u),
            cell<void*>(storage, 0x4346A890u),
            cell<void*>(storage, 0x4346A614u),
            cell<void*>(storage, 0x43469330u),
            cell<void*>(storage, 0x43467198u),
            cell<NativeCosmeticBindings::AcquireItemSystem>(storage, 0x43468B9Cu),
            cell<NativeCosmeticBindings::RegisterEntity>(storage, 0x4346A884u),
            cell<NativeCosmeticBindings::UnregisterEntity>(storage, 0x43468E88u),
            cell<NativeCosmeticBindings::SetParent>(storage, 0x4346D644u),
            cell<NativeCosmeticBindings::InvalidateTransform>(storage, 0x43468C10u),
            cell<NativeCosmeticBindings::InvalidateBones>(storage, 0x434688A4u)};
}
events::WeaponDeathOffsets death_offsets(const engine::NativeOffsetGlobals& globals) {
    return {offset(globals, 0x434688B0u, 0x4346A518u, 0x8B5586F6u),
            offset(globals, 0x43467A74u, 0x434678C0u, 0xCD812654u),
            offset(globals, 0x4346908Cu, 0x43468BA0u, 0x5C9F0FBBu),
            offset(globals, 0x43469248u, 0x4346798Cu, 0x3C1D8AA6u),
            offset(globals, 0x43468390u, 0x434683E4u, 0xA1493792u),
            offset(globals, 0x434682E8u, 0x4346A6C8u, 0xF2DC2C8Au),
            offset(globals, 0x43469080u, 0x434682B4u, 0xD172D9BAu)};
}
}

struct SkinsRuntimeOwner::Data {
    client::ClientRuntimeStorage& storage;
    engine::BuildSkinBindings imports;
    Settings& settings;
    NativeModels models;
    NativeTables tables;
    NativeCounterRandom random;
    WeaponRefresh refresh;
    NativeSchemaCatalog schema;
    NativeEntityCosmetics cosmetics;
    AgentModelReceiver agent;
    SequenceRuntime sequence;
    SequenceReceiveBindings receive;
    SequenceReceivers receivers;
    engine::RecvVarProxy viewmodel_model = nullptr;
    NativeSequenceRuntime sequence_update;
    events::DeathBindings death;
    events::WeaponDeathOffsets death_fields;
    events::DeathWeaponIcons death_icons;
    events::DeathWeaponIconsListener death_listener;
    // The original networkable method returns ClientClass*, represented by a
    // dword in the source runtime. These are actual class-object identities.
    const std::uint32_t weapon_world_model_class, knife_class;
    client::SkinRuntimeFactoryBindings factories;
    SequenceOwnerBindings sequence_factory;

    Data(client::ClientRuntimeStorage& runtime, client::ConfigurationRuntimeOwner& configuration,
         engine::BuildSkinBindings bindings)
        : storage(runtime), imports(bindings),
          settings(require_inputs(storage, configuration, imports)),
          models(cell<void*>(storage, 0x4346A614u)), tables(cell<void*>(storage, 0x43467198u)),
          random(imports.random_integer),
          refresh({cell<std::uint8_t>(storage, 0x43475BD2u),
                   cell<const float*>(storage, 0x4346A334u), cell<float>(storage, 0x4346A8A0u),
                   cell<std::uint8_t>(storage, 0x4346838Bu)},
                  {cell<void*>(storage, 0x4346A514u), cell<void*>(storage, 0x4346A890u),
                   offset(storage.offsets().globals, 0x43468E78u, 0x434684E4u, 0x9D920E35u),
                   offset(storage.offsets().globals, 0x434688B0u, 0x4346A518u, 0x8B5586F6u),
                   cell<void*>(storage, 0x43475808u), cell<void*>(storage, 0x43468994u),
                   cell<RefreshBindings::FindHudElement>(storage, 0x4346A374u),
                   cell<void*>(storage, 0x43468364u)}),
          schema({cell<void*>(storage, 0x43476424u),
                  cell<NativeSchemaBindings::AcquireItemSystem>(storage, 0x43468B9Cu),
                  cell<NativeSchemaBindings::PaintkitDefinition>(storage, 0x43467040u),
                  cell<void*>(storage, 0x4346A31Cu)}),
          cosmetics(settings, native_cosmetic_offsets(storage.offsets().globals),
                    {cell<void*>(storage, 0x43476424u), *cell<void**>(storage, 0x43468C94u),
                     cell<engine::ClientClass*>(storage, 0x4346841Cu)},
                    cosmetic_bindings(storage)),
          agent(
              {cell<std::uint32_t>(storage, 0x4346D648u), cell<std::int32_t>(storage, 0x4346D650u)},
              {cell<void*>(storage, 0x4346A514u),
               cell<engine::RecvVarProxy>(storage, 0x43476A94u)}),
          receive{settings,
                  cell<void*>(storage, 0x43475808u),
                  cell<void*>(storage, 0x4346A514u),
                  storage.offsets(),
                  nullptr,
                  nullptr,
                  nullptr,
                  nullptr},
          receivers{sequence, receive},
          sequence_update(settings, sequence,
                          {storage.offsets(), cell<void*>(storage, 0x4346A514u),
                           cell<void*>(storage, 0x4346A890u), cell<void*>(storage, 0x4346A614u),
                           cell<void*>(storage, 0x43469330u), cell<void*>(storage, 0x4346A460u),
                           cell<misc::ViewmodelLagOwner*>(storage, 0x43475860u),
                           cell<std::int32_t*>(storage, 0x43468328u),
                           cell<void**>(storage, 0x434693A4u), cell<float*>(storage, 0x4346A338u),
                           receive, viewmodel_model, imports.seed_random, imports.random_integer},
                          refresh),
          death{cell<void*>(storage, 0x4346A514u), cell<void*>(storage, 0x4346A890u), nullptr},
          death_fields(death_offsets(storage.offsets().globals)), death_icons(settings, sequence),
          death_listener{death_icons, death, death_fields},
          weapon_world_model_class(static_cast<std::uint32_t>(
              reinterpret_cast<std::uintptr_t>(imports.weapon_world_model_class))),
          knife_class(
              static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(imports.knife_class))),
          factories{storage.configuration(),
                    {storage.owner_registries().at(0x43475B78u), storage.memory()},
                    storage.command_phases(),
                    storage.entity_observers(),
                    settings,
                    {settings.weapons, random, refresh, cell<ui::RootMenu*>(storage, 0x43476924u),
                     cell<std::array<std::uint32_t, 3>>(storage, 0x43474580u),
                     cell<void*>(storage, 0x4346A514u), cell<void*>(storage, 0x4346A890u),
                     cell<const std::byte*>(storage, 0x43467EF4u),
                     cell<const std::int32_t*>(storage, 0x434670C4u), storage.offsets().globals,
                     death, death_fields, storage.event_registry()},
                    {settings, cosmetics, cell<void*>(storage, 0x4346A514u),
                     cell<void*>(storage, 0x43475808u)},
                    {settings, schema.catalog(), agent, cell<void*>(storage, 0x4346A890u),
                     cell<NativeAgentBindings::SupportModels>(storage, 0x4346D64Cu),
                     storage.offsets().globals, models, tables, storage.data_update_observers()}},
          sequence_factory{settings,
                           sequence,
                           receivers,
                           sequence_update,
                           cell<void*>(storage, 0x434683E0u),
                           cell<void*>(storage, 0x4346A890u),
                           cell<void*>(storage, 0x43467198u),
                           weapon_world_model_class,
                           knife_class,
                           viewmodel_model,
                           death_listener,
                           storage.event_registry()} {}
};

SkinsRuntimeOwner::SkinsRuntimeOwner(client::ClientRuntimeStorage& storage,
                                     client::ConfigurationRuntimeOwner& configuration,
                                     engine::BuildSkinBindings imports)
    : data_(std::make_unique<Data>(storage, configuration, imports)) {}
SkinsRuntimeOwner::~SkinsRuntimeOwner() = default;
NativeCatalog& SkinsRuntimeOwner::catalog() noexcept {
    return data_->schema.catalog();
}
WeaponRefresh& SkinsRuntimeOwner::refresh() noexcept {
    return data_->refresh;
}
Settings& SkinsRuntimeOwner::settings() noexcept {
    return data_->settings;
}
NativeEntityCosmetics& SkinsRuntimeOwner::cosmetics() noexcept {
    return data_->cosmetics;
}
ModelInfo& SkinsRuntimeOwner::models() noexcept {
    return data_->models;
}
StringTables& SkinsRuntimeOwner::string_tables() noexcept {
    return data_->tables;
}
client::SkinRuntimeFactoryBindings& SkinsRuntimeOwner::factory_bindings() noexcept {
    return data_->factories;
}
SequenceOwnerBindings& SkinsRuntimeOwner::sequence_bindings() noexcept {
    return data_->sequence_factory;
}
SequenceRuntime& SkinsRuntimeOwner::sequence_state() noexcept {
    return data_->sequence;
}
SequenceReceivers& SkinsRuntimeOwner::sequence_receivers() noexcept {
    return data_->receivers;
}
NativeSequenceRuntime& SkinsRuntimeOwner::sequence_runtime() noexcept {
    return data_->sequence_update;
}
AgentModelReceiver& SkinsRuntimeOwner::agent_receiver() noexcept {
    return data_->agent;
}
void SkinsRuntimeOwner::bind_receive_patches(
    std::span<client::EntityUpdatePatch* const, 4> sequence,
    client::EntityUpdatePatch& agent) noexcept {
    bind_sequence_receive_runtime(data_->receivers, sequence);
    bind_agent_model_receiver(data_->agent, agent);
}
void SkinsRuntimeOwner::bind_receive_patches(const engine::BuildSkinReceivePatches& patches) {
    if (&patches.data() != &data_->storage.data())
        throw std::invalid_argument(
            "Skin receive patches require the same canonical runtime data owner");
    bind_receive_patches(patches.sequence(), patches.agent());
}
} //namespace skeet::skins

// ============================================================================
// Subcomponent: sequence_owner.cpp
// ============================================================================

namespace skeet::skins {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SEQUENCE_MEMBER __thiscall
#else
#define SEQUENCE_MEMBER
#endif
using OwnerEntry = void(SEQUENCE_MEMBER*)(void*);
struct SequenceOwner;
void SEQUENCE_MEMBER sequence_destroy(void*);
void SEQUENCE_MEMBER sequence_unchanged(void*) {} //433CCE8E
void SEQUENCE_MEMBER reset(void*);
void SEQUENCE_MEMBER reset_forwarder(void*);
void SEQUENCE_MEMBER precache(void*);
const std::array<OwnerEntry, 9> owner_table{sequence_destroy,   sequence_unchanged, sequence_unchanged,
                                            sequence_unchanged, sequence_unchanged, reset,
                                            reset_forwarder, precache,  sequence_unchanged};
/* register_integer already defined above */
void capture_proxy(void* client, std::uint32_t klass, std::uint32_t table, std::uint32_t property,
                   engine::RecvVarProxy& destination) {
    if (auto* found = engine::find_receive_property(client, klass, table, property))
        destination = found->proxy;
}
struct SequenceOwner {
    const OwnerEntry* vtable = nullptr;
    SequenceEntityObserverReceiver endpoint;
    SequenceOwnerBindings bindings;
    engine::JobMemory memory;
    SequenceOwner(engine::OwnerRegistryBinding owners, client::EntityObserverRegistry& updates,
                  ConfigRegistries& registry, SequenceOwnerBindings input)
        : bindings(input), memory(owners.memory) {
        owners.append(this); //433676A0, before the native vtables/configuration
        updates.append(endpoint, bindings.update); //4348DDD4 ->4333BF32, table still null
        vtable = owner_table.data(); //4348DDDC, primary4346FF18
        updates.publish(endpoint); //4348DDE4, observer4346DC68
        auto& setting = bindings.settings;
        initialize_boolean_config(registry.descriptors, setting.knife_t, 0xD347BC87);
        initialize_boolean_config(registry.descriptors, setting.knife_ct, 0xD4F2A320);
        register_integer(registry.descriptors, setting.knife_model_t, 0xE0BDA969);
        register_integer(registry.descriptors, setting.knife_model_ct, 0xB3C4B6CA);
        auto& state = bindings.state;
        state.inventory_resolver = bindings.entity_list;
        state.weapon_world_model_class = bindings.weapon_world_model_class;
        state.knife_class = bindings.knife_class;
        state.original_model = state.original_definition = -1;
        state.inspection_sequence = "lookat01";
        state.models.fill({0, 0, 0});
        state.default_models.fill({0, 0, 0});
        state.viewmodels.clear();
        state.weapon_handles.clear();
        bindings.receive.previous_handle = 0xFFFFFFFF;
        bindings.receive.previous_sequence = -1;
        bindings.viewmodel_model = nullptr;
        bindings.receive.bindings.model = nullptr;
        bindings.receive.bindings.sequence = nullptr;
        // These three lookups are unchanged original code4348E026..E06C.
        capture_proxy(bindings.base_client, 0xAEEB25F3, 0x6B34DA8F, 0x0BA094FB,
                      bindings.viewmodel_model);
        capture_proxy(bindings.base_client, 0xDD75A186, 0xB05C42F2, 0x0BA094FB,
                      bindings.receive.bindings.model);
        capture_proxy(bindings.base_client, 0xDD75A186, 0xB05C42F2, 0xA5BF8ECD,
                      bindings.receive.bindings.sequence);
        bindings.death_listener.bind(bindings.events); //single canonical433ABC62 route
    }
};
void SEQUENCE_MEMBER sequence_destroy(void* object) { //43369BFF ->433AFDB4 ->433837D0
    auto* owner = static_cast<SequenceOwner*>(object);
    if (!owner)
        return;
    unbind_sequence_receive_runtime(owner->bindings.receive);
    std::vector<PendingWeaponHandle>{}.swap(owner->bindings.state.weapon_handles);
    std::vector<PendingViewModel>{}.swap(owner->bindings.state.viewmodels);
    const auto memory = owner->memory;
    owner->~SequenceOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void SEQUENCE_MEMBER reset(void* object) {
    static_cast<SequenceOwner*>(object)->bindings.receive.reset();
}
void SEQUENCE_MEMBER reset_forwarder(void* object) {
    static_cast<SequenceOwner*>(object)->vtable[5](object);
}
void SEQUENCE_MEMBER precache(void* object) {
    auto& b = static_cast<SequenceOwner*>(object)->bindings;
    refresh_knife_precache(b.state, b.string_tables);
}
#undef SEQUENCE_MEMBER
}
std::size_t sequence_owner_size() noexcept {
    return sizeof(SequenceOwner);
}
void construct_sequence_owner(void* storage, engine::OwnerRegistryBinding owners,
                              client::EntityObserverRegistry& updates, ConfigRegistries& registry,
                              SequenceOwnerBindings bindings) {
    // Native43481F83 publishes only after4348DDB6 returns.
    auto* owner = ::new (storage) SequenceOwner(owners, updates, registry, bindings);
    bind_sequence_receive_runtime(owner->bindings.receive);
}
void* initialize_sequence_owner(engine::OwnerRegistryBinding owners,
                                client::EntityObserverRegistry& updates, ConfigRegistries& registry,
                                SequenceOwnerBindings bindings) {
    const auto size = sizeof(SequenceOwner);
    auto* allocation = owners.memory.allocate(owners.memory.process_heap, 8, size + 0x20);
    if (!allocation) {
        unbind_sequence_receive_runtime(bindings.receive);
        return nullptr;
    }
    const auto base = reinterpret_cast<std::uintptr_t>(allocation);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - base);
    const auto allocation_size = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    auto* storage = reinterpret_cast<void*>(header + 16);
    construct_sequence_owner(storage, owners, updates, registry, bindings);
    return storage;
}
} //namespace skeet::skins

// ============================================================================
// Subcomponent: sequence_receive.cpp
// ============================================================================

namespace skeet::skins {
namespace {
SequenceReceivers* active_receivers = nullptr;
/* duplicate field removed; using skeet::field */
std::uint32_t handle(void* entity) {
    return *engine::invoke<const std::uint32_t*>(entity, 2);
}
std::ptrdiff_t offset(const SequenceReceiveBindings& b, std::uint32_t pointer, std::uint32_t key,
                      std::uint32_t mask) {
    return std::bit_cast<std::int32_t>(*b.offsets.globals.pointers.at(pointer) ^
                                       b.offsets.globals.keys.at(key) ^ mask);
}
bool active(const SequenceReceiveBindings& bindings) {
    auto* const player = bindings.local_player;
    if (!player || engine::invoke<bool>(bindings.engine, 82) ||
        engine::invoke<bool>(bindings.engine, 93))
        return false;
    return (field<std::int32_t>(player, offset(bindings, 0x434688B0, 0x4346A518, 0x8B5586F6)) == 3
                ? bindings.settings.knife_ct.value
                : bindings.settings.knife_t.value) != 0;
}
void sequence_proxy(const engine::RecvProxyData* data, void* entity, void* output) {
    active_receivers->receive_sequence(data, entity, output);
}
void model_proxy(const engine::RecvProxyData* data, void* entity, void* output) {
    active_receivers->receive_model(data, entity, output);
}
void parity_proxy(const engine::RecvProxyData* data, void* entity, void* output) {
    active_receivers->receive_parity(data, entity, output);
}
void weapon_proxy(const engine::RecvProxyData* data, void* entity, void* output) {
    active_receivers->receive_weapon_handle(data, entity, output);
}
constexpr std::array<SequenceReceiveDescriptor, 4> descriptors{{
    {0xA48, 0xDD75A186, 0xB05C42F2, 0xA5BF8ECD, 0x43365510, 0x434769BC, sequence_proxy},
    {0xA5C, 0xDD75A186, 0xB05C42F2, 0x0BA094FB, 0x433B1669, 0x434769AC, model_proxy},
    {0xA70, 0xDD75A186, 0xB05C42F2, 0x91E87C84, 0x433FD58A, 0x43476B0C, parity_proxy},
    {0xA84, 0xAEEB25F3, 0x6B34DA8F, 0x623C927E, 0x4334B4A0, 0x43476A6C, weapon_proxy},
}};
}
void SequenceReceivers::receive_sequence(const engine::RecvProxyData* data, void* entity,
                                         void* output) {
    if (!active(bindings)) {
        bindings.sequence(data, entity, output);
        return;
    }
    previous_handle = handle(entity);
    previous_sequence = field<std::int32_t>(data, 8);
    // Native queries the engine handle again after publishing both words.
    state.viewmodels.push_back({handle(entity), -1, field<std::int16_t>(data, 8)});
}
void SequenceReceivers::receive_model(const engine::RecvProxyData* data, void* entity,
                                      void* output) {
    if (!active(bindings)) {
        bindings.model(data, entity, output);
        return;
    }
    state.viewmodels.push_back({handle(entity), field<std::int16_t>(data, 8), -1});
}
void SequenceReceivers::receive_parity(const engine::RecvProxyData* data, void* entity,
                                       void* output) {
    if (active(bindings)) {
        const auto value = handle(entity);
        const auto end = state.viewmodels.end();
        auto found = state.viewmodels.begin();
        while (found != end && found->handle != value)
            ++found;
        if ((found == end || found->sequence == -1) && previous_handle == value &&
            previous_sequence !=
                field<std::int32_t>(entity, offset(bindings, 0x43467ABC, 0x4346A56C, 0xA25D8271)))
            state.viewmodels.push_back({value, -1, std::int16_t(previous_sequence)});
    }
    bindings.parity(data, entity, output);
}
void SequenceReceivers::receive_weapon_handle(const engine::RecvProxyData* data, void* entity,
                                              void* output) {
    if (!active(bindings)) {
        bindings.weapon_handle(data, entity, output);
        return;
    }
    bindings.weapon_handle(data, nullptr, output);
    const auto value = handle(entity);
    state.weapon_handles.push_back({value, field<std::uint32_t>(data, 8)});
}
void SequenceReceivers::reset() noexcept {
    state.models.fill({-1, -1, -1});
    state.default_models.fill({-1, -1, -1});
    previous_handle = 0xFFFFFFFF;
    previous_sequence = -1;
    state.viewmodels.clear();
}
std::span<const SequenceReceiveDescriptor, 4> sequence_receive_descriptors() noexcept {
    return descriptors;
}
void bind_sequence_receive_runtime(SequenceReceivers& runtime) noexcept {
    active_receivers = &runtime;
}
void bind_sequence_receive_runtime(
    SequenceReceivers& runtime, std::span<client::EntityUpdatePatch* const, 4> patches) noexcept {
    bind_sequence_receive_runtime(runtime);
    const auto original = [](const client::EntityUpdatePatch* patch) {
        return reinterpret_cast<engine::RecvVarProxy>(std::uintptr_t(std::uint32_t(patch->after)));
    };
    runtime.bindings.sequence = original(patches[0]);
    runtime.bindings.model = original(patches[1]);
    runtime.bindings.parity = original(patches[2]);
    runtime.bindings.weapon_handle = original(patches[3]);
    for (std::size_t index = 0; index < descriptors.size(); ++index)
        patches[index]->before = std::bit_cast<std::int32_t>(
            std::uint32_t(reinterpret_cast<std::uintptr_t>(descriptors[index].replacement)));
}
void unbind_sequence_receive_runtime(SequenceReceivers&) noexcept {
    active_receivers = nullptr;
}
} //namespace skeet::skins

// ============================================================================
// Subcomponent: sequences.cpp
// ============================================================================

namespace skeet::skins {
namespace {
std::int32_t selection(const Settings& settings, const SequenceEntity& player) {
    return player.team == 3 ? settings.knife_model_ct.value : settings.knife_model_t.value;
}
bool enabled(const Settings& settings, const SequenceEntity& player) {
    return (player.team == 3 ? settings.knife_ct.value : settings.knife_t.value) != 0;
}
bool same_owner(const SequenceEntity& entity, SequenceEngine& engine) {
    PlayerIdentity identity;
    return engine.local_identity(identity) && (identity.low || identity.high) &&
           entity.original_owner.low == identity.low && entity.original_owner.high == identity.high;
}
std::uint32_t sequence_hash(std::string_view value) {
    std::uint32_t hash = 0x811C9DC5;
    for (const auto ch : value)
        hash = (hash ^ std::uint8_t(ch)) * 0x1000193;
    return hash ^ 0x92CA5DF1;
}
std::int32_t weighted_sequence(SequenceEntity& entity, std::uint32_t seed, SequenceEngine& engine) {
    if (!engine.animation_interface_available())
        return -1;
    auto* random_seed = engine.prediction_random_seed();
    std::int32_t previous_seed = 0;
    if (random_seed) {
        previous_seed = *random_seed;
        *random_seed = std::int32_t(seed);
    }
    auto** prediction_player = engine.prediction_player();
    auto* local = engine.local_player();
    SequenceEntity* previous_player = nullptr;
    if (prediction_player && local) {
        previous_player = *prediction_player;
        *prediction_player = local;
    }
    const auto result = engine.select_weighted_sequence(entity, 213);
    if (prediction_player && local)
        *prediction_player = previous_player;
    if (random_seed)
        *random_seed = previous_seed;
    return result;
}
void remap(const Settings& settings, const SequenceRuntime& state, SequenceEntity& viewmodel,
           std::int32_t& model, std::int32_t& sequence, SequenceEngine& engine) {
    auto* local = engine.local_player();
    if (!local || engine.demo_playback() || engine.hltv() || !enabled(settings, *local))
        return;
    if (!state.knife_class || viewmodel.weapon_handle == 0xFFFFFFFF)
        return;
    auto* weapon = engine.entity_for_handle(viewmodel.weapon_handle);
    if (!weapon || !same_owner(*weapon, engine))
        return;
    const bool knife = weapon->class_id == state.knife_class;
    std::int32_t selected_model = -1;
    if (knife && engine.local_player()) {
        const auto index = selection(settings, *engine.local_player());
        if (std::uint32_t(index) >= state.models.size())
            return;
        selected_model = state.models[std::size_t(index)].entity_model;
        if (selected_model == -1)
            return;
        const auto original = model == -1 ? viewmodel.model_index : model;
        if (original != selected_model)
            model = selected_model;
    }
    const auto command = engine.acknowledged_command();
    const auto& history = state.history[command % 150];
    if (history.command != command || history.weapon_handle != viewmodel.weapon_handle ||
        history.animation_token_08 != viewmodel.animation_token_08)
        return;
    if (history.animation_token_0c == viewmodel.animation_token_0c &&
        (!knife || history.selected_model == selected_model)) {
        sequence = history.sequence;
        return;
    }
    local = engine.local_player();
    if (!local || local->dead)
        return;
    if (local->weighted_activity) {
        auto mapped = weighted_sequence(viewmodel, history.random_seed, engine);
        if (mapped == -1 && engine.animation_interface_available())
            mapped = engine.lookup_sequence(viewmodel, state.inspection_sequence);
        if (mapped != -1)
            sequence = mapped;
        return;
    }
    if (!knife || sequence < 0)
        return;
    const auto index = selection(settings, *local);
    const auto& target = state.models[std::size_t(index)];
    if (target.entity_model == state.original_model || state.original_model == -1)
        return;
    const auto* source = engine.model_sequences(state.original_model);
    if (!source || sequence >= std::int32_t(source->descriptions.size()))
        return;
    const auto& description = source->descriptions[std::size_t(sequence)];
    const auto* destination = engine.model_sequences(target.entity_model);
    if (!destination)
        return;
    const auto count = destination->descriptions.size();
    if (!description.name.empty()) {
        std::array<char, 64> name{};
        ::strcpy_s(name.data(), name.size(), description.name.data());
        const auto variants = knife_models()[std::size_t(index)].native_word_08;
        if (variants > 1 && sequence_hash(description.name) == 0xD4A1BEA9) {
            const auto ticks = weapon->sequence_time / engine.interval_per_tick() + 0.5f;
            engine.random_seed(_mm_cvttss_si32(_mm_set_ss(ticks)));
            name[7] = char(std::uint8_t(engine.random_integer(1, std::int32_t(variants))) + 0x30);
        }
        for (std::size_t i = 0; i < count; ++i) {
            if (::_stricmp(destination->descriptions[i].name.data(), name.data()) == 0) {
                sequence = std::int32_t(i);
                return;
            }
        }
    }
    if (description.activity != -1) {
        for (std::size_t i = 0; i < count; ++i) {
            if (destination->descriptions[i].activity == description.activity) {
                sequence = std::int32_t(i);
                return;
            }
        }
    }
}
}
void update_sequences(const Settings& settings, SequenceRuntime& state,
                      std::span<const InventoryEntry> inventory, SequenceEngine& engine) {
    for (const auto& pending : state.viewmodels) {
        if (pending.handle == 0xFFFFFFFF)
            continue;
        auto* entity = engine.entity_for_handle(pending.handle);
        if (!entity)
            continue;
        std::int32_t model = pending.model, sequence = pending.sequence;
        remap(settings, state, *entity, model, sequence, engine);
        if (model != -1)
            engine.receive_model_index(*entity, model);
        if (sequence != -1)
            engine.receive_sequence(*entity, sequence);
    }
    state.viewmodels.clear();
    auto* player = engine.inventory_player();
    auto* local = engine.local_player();
    if (player && !player->dead && local && !engine.demo_playback() && !engine.hltv() &&
        enabled(settings, *local)) {
        for (const auto& entry : inventory) {
            auto* entity = engine.inventory_entity(entry.handle);
            if (!entity || !entity->class_id)
                continue;
            if (entity->class_id == state.knife_class) {
                if (!same_owner(*entity, engine))
                    continue;
                const auto index = selection(settings, *player);
                if (std::uint32_t(index) >= state.models.size())
                    continue;
                const auto& model = state.models[std::size_t(index)];
                if (!entry.tag) {
                    state.original_model = entity->model_index;
                    state.original_definition = entity->definition;
                }
                entity->definition = knife_models()[std::size_t(index)].definition;
                entity->entity_model = model.entity_model;
                entity->view_model = model.view_model;
                entity->world_model = model.world_model;
                entity->model_kind = 3;
                if (model.entity_model == -1 || model.entity_model == entity->model_index)
                    continue;
                engine.begin_model_cache_lock();
                engine.begin_model_cache_lock();
                engine.set_model_index(*entity, model.entity_model);
                engine.end_model_cache_lock();
                engine.end_model_cache_lock();
                if (entry.tag)
                    engine.schedule_full_update(engine.current_time() + 0.25f);
            } else if (entity->class_id == state.weapon_world_model_class) {
                local = engine.local_player();
                if (!local)
                    continue;
                const auto index = selection(settings, *local);
                if (std::uint32_t(index) >= state.models.size() ||
                    entity->owner_handle == 0xFFFFFFFF ||
                    std::uint16_t(entity->owner_handle) !=
                        std::uint32_t(engine.local_player_index()) ||
                    entity->weapon_handle == 0xFFFFFFFF)
                    continue;
                auto* weapon = engine.entity_for_handle(entity->weapon_handle);
                if (!weapon || weapon->class_id != state.knife_class ||
                    !same_owner(*weapon, engine))
                    continue;
                const auto model = state.models[std::size_t(index)].view_model;
                if (model == -1 || model == entity->model_index ||
                    !engine.has_viewmodel_model_proxy())
                    continue;
                engine.receive_viewmodel_model(*entity, model);
                entity->model_index = std::int16_t(model);
            }
        }
    }
    for (const auto& pending : state.weapon_handles) {
        if (pending.handle == 0xFFFFFFFF)
            continue;
        auto* entity = engine.entity_for_handle(pending.handle);
        if (entity)
            engine.receive_weapon_handle(*entity, pending.value);
    }
    state.weapon_handles.clear();
}
} // namespace skeet::skins

// ============================================================================
// Subcomponent: stattrak.cpp
// ============================================================================

namespace skeet::skins {
void player_death(const WeaponSkinsConfig& settings, events::GameEventReader& event,
                  StatTrakEngine& engine) {
    events::FieldValue attacker;
    if (event.read(0x455B4DBF, events::FieldType::integer, attacker))
        return;
    if (engine.player_for_user_id(attacker.integer) != engine.entity_index())
        return;
    auto* weapon = engine.active_weapon();
    if (!weapon)
        return;
    std::uint32_t low = 0, high = 0;
    if (!engine.local_identity(low, high) || (!low && !high))
        return;
    if (weapon->owner_low != low || weapon->owner_high != high)
        return;
    const auto slot = weapon_config_slot(weapon->definition, engine.team());
    if (slot < 0)
        return;
    const auto& record = settings.value[slot];
    if (!record.enabled || !record.stattrak)
        return;
    ++weapon->counter;
    for (auto& attribute : weapon->attributes)
        if (attribute.id == 80) {
            ++attribute.value;
            break;
        }
}
} // namespace skeet::skins
