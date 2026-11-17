#pragma once

// Skin Changer Subsystem Declarations
#include "skeet/skins/agent_receive.hpp"
#include "skeet/skins/native_agents.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/netvars.hpp"
#include <cstdint>
#include "skeet/skins/config.hpp"
#include "skeet/skins/model_resources.hpp"
#include <optional>
#include <span>
#include "skeet/skins/catalog.hpp"
#include <string_view>
#include <string>
#include <cstddef>
#include <utility>
#include "skeet/client/command_frame.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/skins/refresh.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/config.hpp"
#include <array>
#include "skeet/skins/native_entity_cosmetics.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/skins/native_catalog.hpp"
#include "skeet/skins/apply.hpp"
#include "skeet/skins/native_inventory.hpp"
#include "skeet/skins/command.hpp"
#include "skeet/events/feature_listeners.hpp"
#include "skeet/skins/sequences.hpp"
#include "skeet/skins/gloves.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/network/data_update_receiver.hpp"
#include <map>
#include <vector>
#include "skeet/misc/native_text.hpp"
#include "skeet/skins/sequence_receive.hpp"
#include "skeet/client/skin_runtime_factories.hpp"
#include "skeet/engine/build_skin_bindings.hpp"
#include "skeet/skins/native_schema.hpp"
#include "skeet/skins/native_sequence_update.hpp"
#include "skeet/skins/sequence_owner.hpp"
#include <memory>
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/skins/inventory.hpp"
#include "skeet/skins/models.hpp"
#include "skeet/events/game_events.hpp"
#include "skeet/combobox.hpp"
#include <bit>
#include <intrin.h>


// Declarations from agent_owner.hpp

namespace skeet::skins {
struct AgentOwnerBindings {
    Settings& settings;
    NativeCatalog& catalog;
    // The runtime derives both capture references and its engine-client
    // reference from this receiver. No second original-model/handle store.
    AgentModelReceiver& receiver;
    void* const& entity_list;
    NativeAgentBindings::SupportModels const& support_models;
    const engine::NativeOffsetGlobals& offsets;
    ModelInfo& models;
    StringTables& tables;
    network::DataUpdateObserverRegistry& observers;
};
// Whole 4347DAAA constructor (4347DAB7 is its secondary registration), table
// 4346F358. The primary owner registers before the runtime's actual 4337EFD9
// observer append; both precede the four agent descriptor registrations.
// Its Settings links are fresh; production uses initialize_shared rather than
// the separate full Settings::initialize semantic/fixture initializer.
// The receiver and its 4346D648/4346D650 cells outlive the owner's destruction and
// remain available until the real receive-property patch is restored.
std::size_t agent_owner_size() noexcept;
// Requires valid storage returned by allocate_agent_owner_storage.
void construct_agent_owner(void* native_header_storage, engine::OwnerRegistryBinding,
                           ConfigRegistries&, AgentOwnerBindings);
// The 43387448 allocation used by 4348574E, including its native alignment
// header. Returns null on allocation failure; the original factory's failure
// continuation at 43485761 is unresolved and is not implemented here.
void* allocate_agent_owner_storage(engine::JobMemory);
// Wire the existing 4335CCD8 source callback through its preserved metadata
// patch. Neither its target nor its position in the patch list is replaced.
void bind_agent_owner_receiver(void* owner, client::EntityUpdatePatch&) noexcept;
} //namespace skeet::skins

// Declarations from agent_receive.hpp

namespace skeet::client {
struct EntityUpdatePatch;
}
namespace skeet::skins {
struct AgentModelCaptureGlobals {
    std::uint32_t& original_handle; //4346D648
    std::int32_t& original_model; //4346D650
};
struct AgentModelReceiveBindings {
    void* const& engine_client; //4346A514
    engine::RecvVarProxy& original; //43476A94, patch+8
};
class AgentModelReceiver {
public:
    AgentModelReceiver(AgentModelCaptureGlobals globals, AgentModelReceiveBindings bindings)
        : globals_(globals), bindings_(bindings) {}
    void receive(const engine::RecvProxyData*, void* entity, void* output); //4335CCD8
    const AgentModelCaptureGlobals& capture_globals() const noexcept {
        return globals_;
    }
    const AgentModelReceiveBindings& receive_bindings() const noexcept {
        return bindings_;
    }

private:
    friend void bind_agent_model_receiver(AgentModelReceiver&, client::EntityUpdatePatch&) noexcept;
    AgentModelCaptureGlobals globals_;
    AgentModelReceiveBindings bindings_;
};
struct AgentModelReceiveDescriptor {
    std::uint32_t class_hash, table_hash, property_hash, native_entry, patch_address;
    engine::RecvVarProxy replacement;
};
const AgentModelReceiveDescriptor& agent_model_receive_descriptor() noexcept;
// The preserved initialization metadata supplies this actual patch node.
// Retain its target, after callback and link order; replace only before.
void bind_agent_model_receiver(AgentModelReceiver&, client::EntityUpdatePatch&) noexcept;
} //namespace skeet::skins

// Declarations from agents.hpp

namespace skeet::skins {
struct AgentModelState {
    std::int32_t original_model = -1;
    std::uint32_t original_handle = 0;
};
struct AgentEntity {
    std::uint32_t handle = 0, ragdoll_handle = 0xFFFFFFFF;
    std::int32_t team = 0;
    bool dormant = false, dead = false;
    std::int16_t model_index = -1;
};
class AgentEngine {
public:
    virtual ~AgentEngine() = default;
    virtual bool demo_playback() = 0;
    virtual bool hltv() = 0;
    virtual std::span<const std::string_view> agent_paths(std::int32_t team) = 0;
    // Native engine helper 0x4346D64C returns the two associated models at +8/+C.
    virtual std::optional<std::array<std::string_view, 2>>
    support_models(std::string_view path) = 0;
    virtual AgentEntity* ragdoll(std::uint32_t handle) = 0;
};
// Complete 0x433466F7: live entity/ragdoll selection and original-model restore.
void apply_agent_model(const Settings&, const AgentModelState&, AgentEntity*, AgentEngine&,
                       ModelInfo&, StringTables&);
} // namespace skeet::skins

// Declarations from apply.hpp

namespace skeet::skins {
// Semantic entity fields supplied by the engine's resolved field accessors.
struct FallbackAttributes {
    std::int32_t item_id_high = 0, item_id_low = 0;
    std::uint32_t account_id = 0;
    std::int32_t paintkit = 0, seed = 0;
    float wear = 0;
    std::int32_t stattrak = 0;
};
struct CounterClass {
    std::string_view name;
    std::int32_t catalog_index = -1, counter = -1;
    bool valid = false;
};
class CounterRandom {
public:
    virtual ~CounterRandom() = default;
    // The native vstdlib RandomInt call uses inclusive bounds 1 and 10000.
    virtual std::int32_t random_integer(std::int32_t minimum, std::int32_t maximum) = 0;
};
// Native attribute-writing slice 0x4339D965..0x4339DBD1. Ownership, entity
// lookup, item classification and the enabled-record gate belong to its caller.
// Returns whether changing the paintkit scheduled a full update.
bool apply_attributes(const WeaponSkin&, FallbackAttributes&, std::uint32_t local_account,
                      std::string_view entity_class, std::span<const CounterClass>, CounterRandom&,
                      Refresh&);
// 0x4339DA75..0x4339DA97, including the native lower bound of 0.00001.
float quality_to_wear(std::int32_t quality) noexcept;
} // namespace skeet::skins

// Declarations from catalog.hpp

namespace skeet::skins {
struct CatalogChoice {
    std::uint32_t value;
    std::u16string name;
};
// Explicit engine schema/localization boundary: no baked-in replacement catalog.
class Catalog {
public:
    virtual ~Catalog() = default;
    virtual bool available() = 0;
    virtual std::uint32_t maximum_paintkit() const = 0;
    virtual std::span<const std::uint16_t> paintkits(std::uint16_t item) = 0;
    virtual std::optional<std::u16string> paintkit_name(std::uint32_t id) = 0;
    virtual std::span<const CatalogChoice> glove_models() = 0;
    virtual std::span<const CatalogChoice> agent_models(std::int32_t team) = 0;
};
class Refresh {
public:
    virtual ~Refresh() = default;
    // 0x4334D9E5 and 0x4346A8A0 scheduling, implemented by the engine adapter.
    virtual void invalidate_weapon_models() = 0;
    virtual float current_time() const = 0;
    virtual void schedule_full_update(float time) = 0;
    // 0x433821B6 queries live team; disconnected/demo/HLTV use team 2.
    virtual std::int32_t agent_team() const = 0;
};
} // namespace skeet::skins

// Declarations from catalog_sort.hpp

namespace skeet::skins {
namespace catalog_detail {
template <class Option>
void adjust_heap(std::span<Option> rows, std::size_t first, std::size_t count, std::size_t hole,
                 Option value) {
    const auto top = hole;
    auto child = hole * 2 + 2;
    while (child < count) {
        if (rows[first + child].name < rows[first + child - 1].name)
            --child;
        rows[first + hole] = std::move(rows[first + child]);
        hole = child;
        child = hole * 2 + 2;
    }
    if (child == count) {
        rows[first + hole] = std::move(rows[first + child - 1]);
        hole = child - 1;
    }
    while (hole > top) {
        const auto parent = (hole - 1) / 2;
        if (!(rows[first + parent].name < value.name))
            break;
        rows[first + hole] = std::move(rows[first + parent]);
        hole = parent;
    }
    rows[first + hole] = std::move(value);
}
template <class Option>
void heap_sort(std::span<Option> rows, std::size_t first, std::size_t last) {
    const auto count = last - first;
    if (count < 2)
        return;
    for (auto parent = (count - 2) / 2 + 1; parent != 0;) {
        --parent;
        auto value = std::move(rows[first + parent]);
        adjust_heap(rows, first, count, parent, std::move(value));
    }
    for (auto length = count; length > 1; --length) {
        auto value = std::move(rows[first + length - 1]);
        rows[first + length - 1] = std::move(rows[first]);
        adjust_heap(rows, first, length - 1, 0, std::move(value));
    }
}
template <class Option>
void introsort(std::span<Option> rows, std::size_t first, std::size_t last, std::size_t depth) {
    for (;;) {
        const auto count = last - first;
        if (count < 29 || depth == 0) {
            if (depth == 0)
                heap_sort(rows, first, last);
            return;
        }
        auto middle = first + count / 2;
        const auto tail = last - 1;
        if (rows[first].name < rows[middle].name) {
            if (!(rows[middle].name < rows[tail].name))
                middle = rows[first].name < rows[tail].name ? tail : first;
        } else if (rows[first].name < rows[tail].name)
            middle = first;
        else if (rows[middle].name < rows[tail].name)
            middle = tail;
        const auto pivot = rows[middle].name;
        auto left = first, right = last;
        for (;;) {
            while (rows[left].name < pivot)
                ++left;
            do {
                --right;
            } while (pivot < rows[right].name);
            if (left >= right)
                break;
            std::swap(rows[left], rows[right]);
            ++left;
        }
        --depth;
        introsort(rows, left, last, depth);
        last = left;
    }
}
template <class Option>
void insertion_sort(std::span<Option> rows, std::size_t first, std::size_t last, bool guarded) {
    for (auto index = first; index < last; ++index) {
        auto value = std::move(rows[index]);
        auto hole = index;
        while ((!guarded || hole != 0) && value.name < rows[hole - 1].name) {
            rows[hole] = std::move(rows[hole - 1]);
            --hole;
        }
        rows[hole] = std::move(value);
    }
}
} //namespace catalog_detail
// Complete433B69E8 family for glove options and the identical20-byte-row
// sorting policy in434084F0 for weapon paints. The native median-of-three,
// right-first partition recursion, threshold28 and heap tie choices determine
// the order of IDs sharing a label. UTF16 names compare unsigned code units.
template <class Option> void sort_catalog_choices(std::span<Option> rows) {
    if (rows.empty())
        return;
    auto bits = std::size_t(0);
    for (auto count = rows.size(); count; count >>= 1)
        ++bits;
    catalog_detail::introsort(rows, 0, rows.size(), bits * 2 - 2);
    const auto boundary = rows.size() > 28 ? std::size_t(28) : rows.size();
    catalog_detail::insertion_sort(rows, 1, boundary, true);
    if (rows.size() > 28)
        catalog_detail::insertion_sort(rows, boundary, rows.size(), false);
}
} //namespace skeet::skins

// Declarations from command.hpp

namespace skeet::ui {
class SkinsMenu;
class LegitWeaponSelector;
class RageWeaponSelector;
class RootMenu;
}
namespace skeet::skins {
// Native table 43472868, gated by classification 43474580 and the target
// group's canonical Enabled descriptor. Returns -1 when native leaves it alone.
std::int32_t weapon_rage_group(std::int32_t definition, const ui::RageWeaponSelector&,
                               std::span<const std::uint32_t, 3> classification) noexcept;
void sync_rage_weapon_group(std::int32_t definition, ui::RageWeaponSelector&,
                            std::span<const std::uint32_t, 3> classification); // 4334DB07
void sync_legit_weapon_kind(std::int32_t kind, ui::LegitWeaponSelector&); // 43398ECF

// Secondary owner4346F1F4, registered by434902DF's43345EBC call.
// Only phase 4 is substantive; the native cached definition is a signed word.
class CommandRuntime {
public:
    CommandRuntime(ui::RootMenu* const& menu, WeaponRefresh& refresh,
                   const engine::NativeOffsetGlobals& offsets,
                   std::span<const std::uint32_t, 3> classification)
        : menu_(menu), refresh_(refresh), offsets_(offsets), classification_(classification) {}
    void phase4(const client::CommandFrame&, void* local_player); // complete 43344A6C
    std::int16_t previous_weapon = 0; // native owner +18 / phase receiver +10
private:
    ui::RootMenu* const& menu_; // live 43476924, reread across widget callbacks
    WeaponRefresh& refresh_;
    const engine::NativeOffsetGlobals& offsets_;
    std::span<const std::uint32_t, 3> classification_;
};
} // namespace skeet::skins

// Declarations from config.hpp

namespace skeet::skins {
// Native 0x434902DE: 65 consecutive records, registered as tag 6 / 0x410.
struct WeaponSkin {
    std::uint8_t enabled = 0, stattrak = 0;
    std::uint16_t reserved = 0;
    std::int32_t quality = 100, seed = 0, paintkit = 0;
};
static_assert(sizeof(WeaponSkin) == 16);
using WeaponSkinsConfig = ConfigValue<std::array<WeaponSkin, 65>>;

// T and CT descriptors are independently serialized, as in the native owners.
struct Settings {
    BooleanConfig mask_t, mask_ct, glove_t, glove_ct, agent_t, agent_ct, knife_t, knife_ct;
    IntegerConfig mask_model_t, mask_model_ct, glove_model_t, glove_model_ct;
    IntegerConfig glove_paint_t, glove_paint_ct, knife_model_t, knife_model_ct;
    IntegerConfig agent_model_t, agent_model_ct;
    // Image descriptor 4346A488, including its initially false value.
    BooleanConfig filter_by_weapon{{{nullptr, nullptr, 0x9AB5333C}, 1, ConfigType::boolean, 0}, 0};
    WeaponSkinsConfig weapons;
    // Production native owners register their own mask/glove/agent/knife and
    // inventory links. The shared CRT initializer 43492E21 registers only
    // 9AB5333C; it does not choose or reorder owner startup.
    void initialize_shared(ConfigRegistries& registries);
    // Explicit aggregate initializer for standalone semantic users/fixtures.
    // Do not also register these links through the production owner ctors.
    void initialize(ConfigRegistries& registries);
};

// 0x4333FF8D: 500..525 bind to the team's ordinary knife record.
std::int32_t weapon_config_slot(std::int32_t item, std::int32_t team) noexcept;
// 0x433F4B01: update IDs 1..64; record zero never participates.
bool synchronize_enabled(const std::array<WeaponSkin, 65>& settings,
                         std::array<std::uint32_t, 32>& enabled) noexcept;
} // namespace skeet::skins

// Declarations from cosmetic_owners.hpp

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

// Declarations from gloves.hpp

namespace skeet::skins {
inline constexpr std::uint32_t invalid_glove_handle = 0xFFFFFFFF;
struct GloveEntity;
class GloveFactory;
class GloveUnknown {
public:
    virtual ~GloveUnknown() = default;
    virtual GloveEntity* base_entity() = 0; // native unknown +0x1C
};
class GloveNetworkable {
public:
    virtual ~GloveNetworkable() = default;
    virtual GloveUnknown* unknown() = 0; // +0
    virtual void release() = 0; // +4
    virtual GloveFactory* client_class() = 0; // +8
    virtual void notify_transmission(std::int32_t) = 0; // +0xC
    virtual void on_data_changed(std::int32_t) = 0; // +0x14
    virtual void mark_destroyed_on_recreate() = 0; // +0x34
};
class GloveFactory {
public:
    virtual ~GloveFactory() = default;
    // The actual ClientClass create function, not a reconstructed lifecycle callback.
    virtual GloveNetworkable* create(std::int32_t index, std::int32_t serial) = 0;
};
struct GloveAttribute {
    std::uint32_t definition = 0;
    float value = 0, initial_value = 0;
};
// These fields are engine-owned, resolved storage. Their encrypted native field
// identities are retained in gloves.cpp; no Source SDK byte layout is assumed.
struct GloveEntity {
    std::uint32_t handle = invalid_glove_handle, flags = 0, account_id = 0;
    std::int32_t item_id_high = 0, item_id_low = 0, paintkit = 0, quality = 0;
    float wear = 0;
    std::uint16_t definition = 0;
    std::int16_t model_index = -1;
    std::uint32_t owner_handle = invalid_glove_handle;
    std::int32_t creation_index = 0;
    bool initialized = false;
    std::span<GloveAttribute> attributes;
    void* attribute_cache = nullptr;
    GloveNetworkable* networkable = nullptr;

    // Player / held-weapon fields used by the same native traversal.
    std::int32_t team = 0, observer_mode = 0, transmission_state = 0, view_model_index = 0;
    bool dormant = false, dead = false, observer_excluded = false;
    std::uint32_t wearable_handle = invalid_glove_handle,
                  active_weapon_handle = invalid_glove_handle;
    std::uint32_t model_update_handle = invalid_glove_handle; // native field 0x43467FCC
    std::uint32_t observer_target = invalid_glove_handle;
    std::span<const std::uint32_t> view_models;
    std::uint32_t wearable_change_counter = 0;
};
struct GloveOwnerState {
    std::uint32_t handle = invalid_glove_handle; // native owner +0x7C
    GloveFactory* factory = nullptr; // native owner +0x80
};
class GloveEngine {
public:
    virtual ~GloveEngine() = default;
    virtual bool demo_playback() = 0; // engine +0x148
    virtual bool hltv() = 0; // engine +0x174
    virtual GloveEntity* resolve(std::uint32_t handle) = 0; // entity list +0x10
    virtual bool entity_registration_available() = 0; // both 0x4346A884 / 0x43468E88 resolved
    virtual std::int32_t& highest_entity_index() = 0; // native entity list +0x24
    virtual void register_entity(std::uint32_t index) = 0; // 0x4346A884
    virtual void unregister_entity(std::uint32_t& handle, GloveEntity&) = 0; // 0x43468E88
    virtual GloveEntity*& local_override() = 0; // dereferenced 0x43468C94
    virtual std::uint32_t local_account_id() = 0; // engine +0x30 / +0x20 player info
    virtual void begin_model_cache() = 0; // model cache +0x84
    virtual void end_model_cache() = 0; // model cache +0x88
    virtual bool initialize_model(GloveEntity&, const char* path, bool) = 0; // entity +0x184
    virtual void set_model_index(GloveEntity&, std::int32_t) = 0; // entity +0x12C
    virtual std::uint32_t model_instance(GloveEntity&) = 0; // renderable +0x98
    virtual void set_model_instance(GloveEntity&, std::uint32_t) = 0; // entity +0x488
    virtual void attach(GloveEntity&, GloveEntity& player) = 0; // entity +0x468
    virtual void equip(GloveEntity&, GloveEntity& player, bool) = 0; // entity +0x454
    virtual void unequip(GloveEntity&, GloveEntity& player) = 0; // entity +0x478
    virtual void post_equip(GloveEntity&) = 0; // entity +0x80
    virtual std::int32_t health(GloveEntity&) = 0; // entity +0x1E8
    virtual bool network_dormant(GloveEntity&) = 0; // networkable +0x24
    virtual void invalidate_bone_cache(GloveEntity&) = 0; // optional resolved 0x434688A4
    virtual void invalidate_physics(GloveEntity&, std::int32_t) = 0; // entity +0x3F8
    virtual void release_attribute_cache(void* cache, bool) = 0; // cache +0
};
// Complete 0x43342310. Internal helpers 0x43364942, 0x433E320A,
// 0x433E2266 and 0x43342DD7 are implemented here, not engine callbacks.
void apply_gloves(const Settings&, GloveOwnerState&, GloveEntity* local, GloveEngine&,
                  SchemaEngine&, ModelInfo&);
} // namespace skeet::skins

// Declarations from inventory.hpp

namespace skeet::skins {
struct PlayerIdentity {
    std::uint32_t low = 0, high = 0;
};
struct WeaponOwnership {
    PlayerIdentity original_owner;
    // Exact engine field identities: encrypted resolver 0x43468E5C and
    // 0x4346A868. Their values are read, without assuming an SDK offset.
    std::uint32_t zero_marker = 0, owner_handle = 0xFFFFFFFF;
};
bool inventory_owner_matches(const WeaponOwnership&, std::uint32_t inventory_tag,
                             bool identity_valid, PlayerIdentity local,
                             std::int32_t local_index) noexcept;
struct InventoryEntry {
    std::uint32_t handle = 0, tag = 0;
};
struct InventoryWeapon {
    std::uint16_t definition = 0;
    bool is_weapon = false;
    WeaponOwnership ownership;
    std::string_view entity_class;
    FallbackAttributes attributes;
};
class InventoryEngine : public CounterRandom, public Refresh {
public:
    virtual bool local_player_exists() const = 0;
    virtual bool local_player_dead() const = 0;
    virtual std::int32_t local_player_index() const = 0;
    virtual std::int32_t local_team() const = 0;
    virtual bool local_identity(PlayerIdentity&) = 0;
    // Resolve the handle/networkable/unknown/base-entity chain. Return null
    // when any engine interface in that chain returns null.
    virtual InventoryWeapon* resolve(std::uint32_t handle) = 0;
    virtual std::span<const CounterClass> counter_classes() const = 0;
};
// Complete policy and inventory traversal of 0x4339D6B2. Entity access and
// resolved field storage are explicit engine adapter responsibilities.
void apply_inventory(const WeaponSkinsConfig&, std::span<const InventoryEntry>, InventoryEngine&);
} // namespace skeet::skins

// Declarations from inventory_owner.hpp

namespace skeet::skins {
class InventoryLifecycleOwner;
struct InventoryOwnerBindings {
    WeaponSkinsConfig& weapons;
    CounterRandom& random;
    WeaponRefresh& refresh;
    ui::RootMenu* const& menu; //live43476924; construction does not dereference it
    std::span<const std::uint32_t, 3> classification; //43474580
    void* const& engine_client;
    void* const& entity_list; //captured at43490315, after all three registrations
    const std::byte* const& counter_rows;
    const std::int32_t* const& counters;
    const engine::NativeOffsetGlobals& offsets; //shared live encrypted offset words
    events::DeathBindings& death;
    events::WeaponDeathOffsets& death_offsets;
    events::EventRegistry& events;
};

std::size_t inventory_owner_size() noexcept;
// Actual constructor entry434902DF (the factory's JMP target). Its primary,
// command and entity-update registrations retain that order. This creates the
// one CommandRuntime, NativeInventoryRuntime and StatTrakDeath listener over
// the same borrowed WeaponSkinsConfig/CounterRandom/WeaponRefresh.
// weapons must not already be linked by Settings::initialize: this constructor
// registers only B6218FEC and sets its 65 records' native defaults. It preserves
// each record's reserved word and the descriptor's reserved byte.
// storage is a native-header allocation sized by inventory_owner_size();
// primary slot0 frees it. Registry shutdown retains ownership of its ranges.
InventoryLifecycleOwner& construct_inventory_owner(void* storage,
                                                   engine::OwnerRegistryBinding lifecycle,
                                                   client::CommandPhaseRegistry& commands,
                                                   client::EntityObserverRegistry&,
                                                   ConfigRegistries&, InventoryOwnerBindings);
} //namespace skeet::skins

// Declarations from knife_precache.hpp

namespace skeet::skins {
// Complete4338A892 and its433AE8B7 table lookup. string_tables is the actual
// network-string-table container43467198. Model strings and indices remain
// engine-owned; this only publishes the three resolved indices per knife.
void refresh_knife_precache(SequenceRuntime&, void* string_tables);
} // namespace skeet::skins

// Declarations from masks.hpp

namespace skeet::skins {
struct MaskEntity : GloveEntity {
    // Native field 0x43467E8C points to the player's model-header wrapper.
    // The native gate requires both this pointer and its first word nonzero.
    const std::uint32_t* model_header = nullptr;
    std::uint32_t parent_handle = invalid_glove_handle; // field 0x43469344 - 0x18
    std::array<float, 3> local_origin{}; // field 0x43468D30
    std::array<float, 3> local_angles{}; // field 0x43468368
    std::uint16_t bits_43467044 = 0;
    bool flag_43468bd4 = false, flag_43467964 = false;
    std::uint32_t flags_43469224 = 0;
};
struct MaskOwnerState {
    std::uint32_t handle = invalid_glove_handle; // owner +0x54
    // The native constructor resolves class hash 0xF89B2C53. Its factory must
    // construct a MaskEntity adapter through the same real 0x43364942 chain.
    GloveFactory* factory = nullptr; // owner +0x58
};
class MaskEngine : public GloveEngine {
public:
    virtual MaskEntity* resolve(std::uint32_t handle) override = 0;
    virtual bool parent_function_available() = 0; // resolved 0x4346D644
    virtual const void* render_model(MaskEntity&) = 0; // renderable +0x20
    virtual std::int32_t attachment(MaskEntity&, std::string_view name) = 0; // renderable +0x84
    virtual void set_parent(MaskEntity&, MaskEntity& parent,
                            std::int32_t attachment) = 0; // 0x4346D644
    // Call the optional resolved 0x43468C10 when present. The field writes
    // remain in the source; this method only performs that engine operation.
    virtual void invalidate_transform(MaskEntity&, std::int32_t flags) = 0;
    // Actual decoded virtual call +0x3B8. Its precise engine name is not
    // established, so no Source SDK method identity is assigned to it.
    virtual void entity_boolean_3b8(MaskEntity&, bool) = 0;
};
// Exact constructor 0x4347EB9D catalog, with all 22 native selection values.
std::span<const std::string_view, 22> mask_model_paths() noexcept;
// Complete native 0x43408846. Factory creation is the same engine sequence as
// gloves; model lookup/loading uses the recovered 0x43331B0C implementation.
void apply_mask(const Settings&, MaskOwnerState&, MaskEntity* local, MaskEngine&, ModelInfo&,
                StringTables&);
} // namespace skeet::skins

// Declarations from model_resources.hpp

namespace skeet::skins {
class ModelInfo {
public:
    virtual ~ModelInfo() = default;
    virtual std::int32_t model_index(std::string_view path) = 0; // engine vtable +8
    virtual bool load_model(std::string_view path) = 0; // +0xAC
};
class StringTable {
public:
    virtual ~StringTable() = default;
    virtual const char* name() = 0;
    virtual std::int32_t add_string(std::int32_t flags, std::string_view path, std::int32_t length,
                                    const void* data) = 0;
};
class StringTables {
public:
    virtual ~StringTables() = default;
    virtual std::int32_t count() = 0;
    virtual StringTable* at(std::int32_t index) = 0;
};
// 0x43331B0C and 0x433AE8B7: find or load the model, then register it in
// the engine's modelprecache string table. All calls below cross engine APIs.
std::int32_t resolve_model_index(std::string_view path, ModelInfo&, StringTables&);
} // namespace skeet::skins

// Declarations from models.hpp

namespace skeet::skins {
struct KnifeModel {
    std::uint16_t definition;
    std::uint32_t view_model_hash;
    std::uint32_t native_word_08, native_word_0c, native_word_10;
};
// 0x43467FD0, copied into the owner by 0x4348DFCB. Hashes identify actual
// view-model paths, e.g. models/weapons/v_knife_bayonet.mdl.
std::span<const KnifeModel, 19> knife_models() noexcept;
const KnifeModel* knife_model(std::int32_t selection) noexcept;
} // namespace skeet::skins

// Declarations from native_agents.hpp

namespace skeet::network {
class DataUpdateObserverRegistry;
}
namespace skeet::skins {
struct NativeAgentBindings {
    using SupportModels = engine::MemberFunction<const std::byte*>;
    void* const& engine_client; // live 4346A514
    void* const& entity_list; // live 4346A890
    const std::int32_t& original_model; // 4346D650
    const std::uint32_t& original_handle; // 4346D648
    SupportModels const& support_models; // actual 4346D64C, ECX=model path
    const engine::NativeOffsetGlobals& offsets;
};
// Concrete43472978 slot1 body registered at secondary receiver+8 by
// 4347DAB7/4337EFD9 in the separate43475760 data-update registry.
class NativeAgentRuntime {
public:
    NativeAgentRuntime(const Settings& settings, NativeCatalog& catalog,
                       NativeAgentBindings bindings, ModelInfo& models, StringTables& tables,
                       network::DataUpdateObserverRegistry& observers);
    void process(void* player, std::int32_t update); // complete433466F7
private:
    friend class network::DataUpdateObserverRegistry;
    const Settings& settings_;
    NativeCatalog& catalog_;
    NativeAgentBindings bindings_;
    ModelInfo& models_;
    StringTables& tables_;
    network::NativeDataUpdateReceiver data_update_receiver_{};
};
} // namespace skeet::skins

// Declarations from native_catalog.hpp

namespace skeet::ui {
class SkinsMenu;
}
namespace skeet::skins {
struct SchemaItem {
    const char* type_token = nullptr;
    const char* base_name_token = nullptr;
    const char* view_model = nullptr;
    const char* world_model = nullptr;
};
class SchemaEngine {
public:
    virtual ~SchemaEngine() = default;
    virtual bool acquire_schema() = 0;
    virtual std::int32_t definition_count() = 0;
    virtual std::optional<SchemaItem> definition(std::uint32_t index) = 0;
    virtual const char* paintkit_token(std::uint32_t id) = 0;
    virtual std::optional<std::u16string> localize(const char* token) = 0;
};
class SchemaFiles {
public:
    virtual ~SchemaFiles() = default;
    virtual std::u16string current_directory() = 0;
    virtual std::optional<std::string> read_file(std::u16string_view path) = 0;
};
// Concrete 0x433CD70E compositor, including its static native agent additions
// and the items_game weapon_icons mapping parser.
class NativeCatalog final : public Catalog {
public:
    NativeCatalog(SchemaEngine& engine, SchemaFiles& files) : engine_(engine), files_(files) {}
    void initialize(ui::SkinsMenu& menu);
    bool available() override;
    std::uint32_t maximum_paintkit() const override {
        return maximum_;
    }
    std::span<const std::uint16_t> paintkits(std::uint16_t item) override;
    std::optional<std::u16string> paintkit_name(std::uint32_t id) override;
    std::span<const CatalogChoice> glove_models() override {
        return gloves_;
    }
    std::span<const CatalogChoice> agent_models(std::int32_t team) override;
    std::span<const std::string> agent_paths(std::int32_t team) const;
    // Native parser slice 0x433CD830..0x433CDB2B, also usable on captured data.
    void parse_weapon_icons(std::string_view text);

private:
    void append_native_agents(std::string_view data, std::int32_t team);
    SchemaEngine& engine_;
    SchemaFiles& files_;
    std::vector<CatalogChoice> gloves_, agents_t_, agents_ct_;
    std::vector<std::string> paths_t_, paths_ct_;
    std::map<std::uint16_t, std::vector<std::uint16_t>> paintkits_;
    std::uint32_t maximum_ = 0;
    bool native_agents_consumed_ = false;
};
} // namespace skeet::skins

// Declarations from native_entity_cosmetics.hpp

namespace skeet::skins {
// All fields come from the recovered encrypted offset globals. There are no
// SDK-layout defaults. The decoder below requires actual initialized storage.
struct NativeCosmeticOffsets {
    std::ptrdiff_t team, dormant, life_state, wearable, paintkit, definition, model_index;
    std::ptrdiff_t transmission_state, wearable_counter, entity_flags, quality;
    std::ptrdiff_t model_update_handle, owner_handle, active_weapon, view_model_index;
    std::ptrdiff_t initialized, item_id_high, item_id_low, account_id, wear;
    std::ptrdiff_t attributes, attribute_cache, view_models, observer_mode, observer_target,
        observer_excluded;
    std::ptrdiff_t model_header, parent_handle, local_origin, local_angles;
    std::ptrdiff_t mask_byte_43468bd4, mask_bits_43467044, mask_byte_43467964, mask_flags_43469224;
};
NativeCosmeticOffsets native_cosmetic_offsets(const engine::NativeOffsetGlobals&);
struct NativeCosmeticGlobals {
    void*& item_system; //cached43476424
    void*& local_override; //dereferenced43468C94
    engine::ClientClass* const& glove_class; //4346841C, copied by glove constructor
};
struct NativeCosmeticBindings {
    using AcquireItemSystem = void*(__cdecl*)(); //resolved43468B9C
    using RegisterEntity = engine::MemberFunction<void, std::uint32_t>; //4346A884
    using UnregisterEntity = engine::MemberFunction<void, std::uint32_t*, void*>; //43468E88
    using SetParent = engine::MemberFunction<void, void*, std::int32_t>; //4346D644
    using InvalidateTransform = engine::MemberFunction<void, std::int32_t>; //43468C10
    using InvalidateBones = engine::MemberFunction<void>; //434688A4
    void* base_client = nullptr;
    void* engine_client = nullptr;
    void* entity_list = nullptr;
    void* model_info = nullptr;
    void* model_cache = nullptr;
    void* string_tables = nullptr;
    AcquireItemSystem acquire_item_system = nullptr;
    RegisterEntity register_entity = nullptr;
    UnregisterEntity unregister_entity = nullptr;
    SetParent set_parent = nullptr;
    InvalidateTransform invalidate_transform = nullptr;
    InvalidateBones invalidate_bones = nullptr;
};
struct NativeMaskResources {
    misc::NativeText attachment, directory;
    std::array<misc::NativeText, 22> filenames;
};
// Concrete raw engine owners for the two real BA8 slot1 bodies. These execute
// the lifecycle directly against engine fields/interfaces; they neither build
// semantic entity copies nor delegate an internal feature to a callback.
class NativeEntityCosmetics final {
public:
    NativeEntityCosmetics(const Settings&, NativeCosmeticOffsets, NativeCosmeticGlobals,
                          NativeCosmeticBindings);
    // These are called at the actual primary-owner constructor boundaries,
    // after registration/configuration and before the corresponding updates.
    void initialize_glove_owner() noexcept; //434907F5..43490807
    void initialize_mask_owner(NativeMaskResources&) noexcept; //4347EC33..3A
    void resolve_mask_owner_class(); //4347ED32..3C, after empty text construction
    void release_glove_owner(void* player); //4331EB65, caller has applied the connected gate
    void release_mask_owner(); //43386F30
    void update_gloves(void* player); //complete43342310; incoming stack argument unused
    void update_mask(void* player); //complete43408846; incoming stack argument unused
    std::uint32_t glove_handle() const noexcept {
        return glove_handle_;
    }
    std::uint32_t mask_handle() const noexcept {
        return mask_handle_;
    }
    const Settings& settings() const noexcept {
        return settings_;
    }

private:
    void* resolve(std::uint32_t);
    std::uint32_t handle(void*);
    void* create(engine::ClientClass*, const char*); //43364942
    void destroy(void*, bool recreating);
    void* item_definition(std::uint32_t);
    std::int32_t model_index(const char*); //43331B0C/433AE8B7
    void set_glove(void*, std::int32_t, std::uint32_t, std::int32_t); //433E320A
    void attribute(void*, std::int32_t, float); //433E2266
    bool glove_matches(void*, std::int32_t, std::uint32_t, std::int32_t);
    void detach(void*, void*);
    void* view_model(void*, std::int32_t); //43342DD7
    void teardown_glove(void*);
    void teardown_mask();
    void zero_transform(void*, std::ptrdiff_t, std::int32_t);
    const Settings& settings_;
    NativeCosmeticOffsets fields_;
    NativeCosmeticGlobals globals_;
    NativeCosmeticBindings bindings_;
    std::uint32_t glove_handle_ = 0xFFFFFFFF, mask_handle_ = 0xFFFFFFFF;
    engine::ClientClass* glove_factory_ = nullptr;
    engine::ClientClass* mask_factory_ = nullptr;
    NativeMaskResources* mask_resources_ = nullptr;
};
} //namespace skeet::skins

// Declarations from native_inventory.hpp

namespace skeet::skins {
struct NativeInventoryBindings {
    void* const& engine_client; // live 4346A514
    void* const& resolver; // owner's capture of4346A890 at43490315, receiver+4
    const std::byte* const& counter_rows; // live 43467EF4, first row +4
    const std::int32_t* const& counters; // live 434670C4
    const engine::NativeOffsetGlobals& offsets;
};

// Concrete entity-update receiver for 4347030C slot1. The incoming engine
// snapshot has eight-byte entries at +44 and its signed count at +4044.
// No semantic entity resolver or feature-level write callback is substituted.
class NativeInventoryRuntime {
public:
    NativeInventoryRuntime(const WeaponSkinsConfig& settings, NativeInventoryBindings bindings,
                           CounterRandom& random, WeaponRefresh& refresh);
    void process(void* local_player, std::uintptr_t incoming); // complete 4339D6B2
private:
    std::int32_t stattrak_counter(void* item);
    const WeaponSkinsConfig& settings_;
    NativeInventoryBindings bindings_;
    CounterRandom& random_;
    WeaponRefresh& refresh_;
};
} // namespace skeet::skins

// Declarations from native_schema.hpp

namespace skeet::skins {
struct NativeSchemaBindings {
    using AcquireItemSystem = void*(__cdecl*)();
    using PaintkitDefinition = engine::MemberFunction<const void*, std::uint32_t>;
    void*& item_system; //shared cached43476424
    AcquireItemSystem const& acquire_item_system; //live resolved43468B9C
    PaintkitDefinition const& paintkit_definition; //live resolved43467040
    void* const& localization; //live4346A31C, slot11
};

// The concrete schema boundary used by433CD70E,43368201 and434084F0.
// All pointers above borrow the same engine cells as the cosmetic owners.
class NativeSchemaEngine final : public SchemaEngine {
public:
    explicit NativeSchemaEngine(NativeSchemaBindings bindings) noexcept : bindings_(bindings) {}
    bool acquire_schema() override;
    std::int32_t definition_count() override;
    std::optional<SchemaItem> definition(std::uint32_t id) override;
    const char* paintkit_token(std::uint32_t id) override;
    std::optional<std::u16string> localize(const char* token) override;

private:
    NativeSchemaBindings bindings_;
    void* schema_ = nullptr; //current acquisition;433CD70E keeps this in its local frame
};

//43493058 chooses GetCurrentDirectoryW's actual process directory and converts
// the supplied UTF16 DOS path to a counted NT name.433EBC7E is shared with the
// preset store; there is no second file reader or alternate catalog path.
class NativeSchemaFiles final : public SchemaFiles {
public:
    std::u16string current_directory() override;
    std::optional<std::string> read_file(std::u16string_view path) override;
};

// Own the adapter lifetimes required by the main menu's NativeCatalog& binding.
class NativeSchemaCatalog final {
public:
    explicit NativeSchemaCatalog(NativeSchemaBindings bindings) noexcept
        : engine_(bindings), catalog_(engine_, files_) {}
    NativeSchemaCatalog(const NativeSchemaCatalog&) = delete;
    NativeSchemaCatalog& operator=(const NativeSchemaCatalog&) = delete;
    NativeCatalog& catalog() noexcept {
        return catalog_;
    }

private:
    NativeSchemaEngine engine_;
    NativeSchemaFiles files_;
    NativeCatalog catalog_;
};
} //namespace skeet::skins

// Declarations from native_sequence_update.hpp

namespace skeet::misc {
class ViewmodelLagOwner;
}
namespace skeet::skins {
struct NativeSequenceBindings {
    using RandomSeed = void(__cdecl*)(std::int32_t);
    using RandomInteger = std::int32_t(__cdecl*)(std::int32_t, std::int32_t);
    const engine::NativeOffsetStorage& offsets;
    void* const& engine_client; //4346A514
    void* const& entity_list; //4346A890
    void* const& model_info; //4346A614
    void* const& model_cache; //43469330
    void* const& animation; //4346A460, actual engine targets+5C/+64
    misc::ViewmodelLagOwner* const& viewmodel_owner; //live 43475860
    std::int32_t* const& random_seed; //43468328
    void** const& prediction_player; //434693A4
    float* const& interval; //4346A338
    const SequenceReceiveBindings& receive;
    const engine::RecvVarProxy& viewmodel_model; //nativeowner+64
    RandomSeed seed_random; //vstdlib!RandomSeed
    RandomInteger random_integer; //vstdlib!RandomInt
};

// Concrete4346DC68 slot1 producer. Queued proxy replay precedes the saved
// player's life/team reads and the incoming+44/+4044 inventory traversal.
// Entity fields and decoded offset words stay live across engine calls.
class NativeSequenceRuntime {
public:
    NativeSequenceRuntime(const Settings& settings, SequenceRuntime& state,
                          NativeSequenceBindings bindings, WeaponRefresh& refresh)
        : settings_(settings), state_(state), bindings_(bindings), refresh_(refresh) {}
    void process(void* player, std::uintptr_t incoming); //complete433E53A5
private:
    std::ptrdiff_t offset(std::uint32_t pointer, std::uint32_t key, std::uint32_t mask) const;
    std::int32_t team(const void*) const;
    bool enabled(const void*) const;
    std::int32_t selection(const void*) const;
    bool same_owner(const void*) const;
    std::uint32_t class_id(void*) const;
    void* resolve(std::uint32_t) const;
    std::int32_t sequence_count(void*) const;
    void* sequence_description(void*, std::int32_t) const;
    std::int32_t weighted_sequence(void*, std::uint32_t) const;
    std::int32_t inspection_sequence(void*) const;
    void remap(void*, std::int32_t& model, std::int32_t& sequence);
    const Settings& settings_;
    SequenceRuntime& state_;
    NativeSequenceBindings bindings_;
    WeaponRefresh& refresh_;
};
} //namespace skeet::skins

// Declarations from refresh.hpp

namespace skeet::skins {
struct RefreshGlobals {
    std::uint8_t& full_update; // 43475BD2, shared with network transition
    const float*& current_time; // 4346A334
    float& hud_refresh_at; // 4346A8A0
    const std::uint8_t& hud_refresh; // 4346838B
};
struct RefreshBindings {
    using FindHudElement = engine::MemberFunction<void*, const char*>;
    void* const& engine_client; // live 4346A514
    void* const& entity_list; // live 4346A890
    std::ptrdiff_t life_state; // 43468E78 ^ 434684E4 ^ 9D920E35
    std::ptrdiff_t team; // 434688B0 ^ 4346A518 ^ 8B5586F6
    void* const& local_player; // 43475808
    void* const& hud; // 43468994
    FindHudElement const& find_hud; // 4346A374, actual engine HUD lookup
    void* const& event_manager; // live 43468364, reread after HUD notification
};

// Concrete internal refresh bodies used by the recovered menu callbacks.
// Engine calls remain external; no feature-level invalidation callback is used.
class WeaponRefresh final : public Refresh {
public:
    WeaponRefresh(RefreshGlobals globals, RefreshBindings bindings)
        : globals_(globals), bindings_(bindings) {}
    void invalidate_weapon_models() override; // complete 4334D9E5
    float current_time() const override;
    void schedule_full_update(float time) override;
    std::int32_t agent_team() const override; // complete 433821B6 team query
    void clear_hud_weapon_selection(); // complete 433F11C8
    void process_deferred_hud_refresh(); // 43344A6C..43344A9C
private:
    RefreshGlobals globals_;
    RefreshBindings bindings_;
};
} // namespace skeet::skins

// Declarations from runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
}
namespace skeet::engine {
class BuildSkinReceivePatches;
}
namespace skeet::skins {
// Stable production lifetime shared by skin feature constructors and RootMenu.
// Construction creates source adapters and live views only. The existing four
// skin factories and43481F6A retain their actual registration/startup order;
// RootMenu's catalog initializer performs its genuine lazy ItemSystem call.
// Keep this context alive through feature destruction and proxy restoration.
class SkinsRuntimeOwner {
public:
    SkinsRuntimeOwner(client::ClientRuntimeStorage&, client::ConfigurationRuntimeOwner&,
                      engine::BuildSkinBindings);
    ~SkinsRuntimeOwner();
    SkinsRuntimeOwner(const SkinsRuntimeOwner&) = delete;
    SkinsRuntimeOwner& operator=(const SkinsRuntimeOwner&) = delete;
    SkinsRuntimeOwner(SkinsRuntimeOwner&&) = delete;
    SkinsRuntimeOwner& operator=(SkinsRuntimeOwner&&) = delete;
    NativeCatalog& catalog() noexcept;
    WeaponRefresh& refresh() noexcept;
    Settings& settings() noexcept;
    NativeEntityCosmetics& cosmetics() noexcept;
    ModelInfo& models() noexcept;
    StringTables& string_tables() noexcept;
    client::SkinRuntimeFactoryBindings& factory_bindings() noexcept;
    SequenceOwnerBindings& sequence_bindings() noexcept;
    SequenceRuntime& sequence_state() noexcept;
    SequenceReceivers& sequence_receivers() noexcept;
    NativeSequenceRuntime& sequence_runtime() noexcept;
    AgentModelReceiver& agent_receiver() noexcept;
    // The actual patch nodes/targets/order are supplied by their source
    // producer. This binds only the existing source proxy callbacks/originals.
    void bind_receive_patches(std::span<client::EntityUpdatePatch* const, 4>,
                              client::EntityUpdatePatch&) noexcept;
    void bind_receive_patches(const engine::BuildSkinReceivePatches&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::skins

// Declarations from sequence_owner.hpp

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

// Declarations from sequence_receive.hpp

namespace skeet::skins {
struct SequenceReceiveBindings {
    const Settings& settings;
    void* const& local_player; //43475808
    void* const& engine;
    const engine::NativeOffsetStorage& offsets;
    // The original callbacks are patch+8, not the installed property slot.
    engine::RecvVarProxy sequence, model, parity, weapon_handle;
};
struct SequenceReceivers {
    SequenceRuntime& state;
    SequenceReceiveBindings& bindings;
    std::uint32_t previous_handle = 0xFFFFFFFF; //owner43475B88+320
    std::int32_t previous_sequence = -1; //+324
    void receive_sequence(const engine::RecvProxyData*, void*, void*); //43365510
    void receive_model(const engine::RecvProxyData*, void*, void*); //433B1669
    void receive_parity(const engine::RecvProxyData*, void*, void*); //433FD58A
    void receive_weapon_handle(const engine::RecvProxyData*, void*, void*); //4334B4A0
    // Complete434199D2 for the canonical19knife rows and sequence queue.
    // The weapon-handle queue is deliberately retained by this native reset.
    void reset() noexcept;
};
struct SequenceReceiveDescriptor {
    std::uint32_t operand, class_hash, table_hash, property_hash, native_entry, patch_address;
    engine::RecvVarProxy replacement;
};
std::span<const SequenceReceiveDescriptor, 4> sequence_receive_descriptors() noexcept;
// Native43481F83 publication after construction, before later patch setup.
void bind_sequence_receive_runtime(SequenceReceivers&) noexcept;
// Bind the one native43475B88 receiver. Patch nodes are the actual four nodes
// created by the preserved initialization metadata; retain their link order,
// targets and original after words, and replace only their before callbacks.
void bind_sequence_receive_runtime(SequenceReceivers&,
                                   std::span<client::EntityUpdatePatch* const, 4> patches) noexcept;
void unbind_sequence_receive_runtime(SequenceReceivers&) noexcept;
} //namespace skeet::skins

// Declarations from sequences.hpp

namespace skeet::skins {
struct SequenceDescription {
    // Engine studio sequence names are NUL terminated; source names fit 63 bytes.
    std::string_view name;
    std::int32_t activity = -1;
};
struct StudioSequences {
    // For a virtual studio model the adapter resolves the engine virtual-model
    // sequence table, as 0x433CCB8F / 0x433E2727 do for studiohdr_t +0x150.
    std::span<const SequenceDescription> descriptions;
};
struct KnifeSequenceModels {
    std::int32_t view_model = -1, world_model = -1, entity_model = -1; // owner row +8C/+90/+94
};
struct SequenceEntity {
    std::uint32_t class_id = 0;
    PlayerIdentity original_owner;
    std::uint32_t weapon_handle = 0xFFFFFFFF, owner_handle = 0xFFFFFFFF;
    std::int16_t model_index = -1;
    std::uint16_t definition = 0;
    std::int32_t entity_model = -1, view_model = -1, world_model = -1, model_kind = 0;
    std::int32_t sequence = -1;
    // Resolved fields 0x43468DA0 and 0x43467B4C; their history comparison is exact.
    std::int32_t animation_token_08 = 0, animation_token_0c = 0;
    float sequence_time = 0; // resolved field 0x43467A50
    std::int32_t team = 2;
    bool dead = false, weighted_activity = false; // 0x43468E78 / 0x43467EC4
};
struct SequenceHistory {
    std::uint32_t command = 0, random_seed = 0, weapon_handle = 0;
    std::int32_t sequence = 0, animation_token_08 = 0, animation_token_0c = 0;
    std::uint32_t field_18 = 0;
    std::uint8_t player_byte_1c = 0, padding_1d = 0;
    std::int16_t selected_model = 0;
};
static_assert(sizeof(SequenceHistory) == 0x20);
static_assert(offsetof(SequenceHistory, field_18) == 0x18);
static_assert(offsetof(SequenceHistory, selected_model) == 0x1E);
struct PendingViewModel {
    std::uint32_t handle;
    std::int16_t model = -1, sequence = -1;
};
struct PendingWeaponHandle {
    std::uint32_t handle, value;
};
struct SequenceRuntime {
    void* inventory_resolver = nullptr; //4348DE49 captured4346A890, owner+5C
    std::uint32_t knife_class = 0, viewmodel_class = 0;
    std::int32_t original_model = -1, original_definition = 0;
    std::string_view inspection_sequence = "lookat01"; //4348DF7D..DFC6 ->owner+358
    std::array<KnifeSequenceModels, 19> models{};
    std::array<KnifeSequenceModels, 2> default_models{}; //native42/59 rows+2E0/+300
    std::array<SequenceHistory, 150> history{};
    std::vector<PendingViewModel> viewmodels;
    std::vector<PendingWeaponHandle> weapon_handles;
};
class SequenceEngine : public CounterRandom {
public:
    virtual SequenceEntity* local_player() = 0; // 0x43475808
    virtual SequenceEntity* inventory_player() = 0; // EDX at 0x433E53A5
    virtual bool demo_playback() = 0; // engine +0x148
    virtual bool hltv() = 0; // engine +0x174
    virtual std::int32_t local_player_index() = 0;
    virtual bool local_identity(PlayerIdentity&) = 0;
    virtual std::uint32_t acknowledged_command() = 0; // engine +0x334
    virtual SequenceEntity* entity_for_handle(std::uint32_t) = 0;
    virtual SequenceEntity* inventory_entity(std::uint32_t) = 0;
    virtual const StudioSequences* model_sequences(std::int32_t model) = 0;
    virtual bool animation_interface_available() = 0; // resolved engine interface 0x4346A460
    virtual std::int32_t* prediction_random_seed() = 0; // 0x43468328, optional engine global
    virtual SequenceEntity** prediction_player() = 0; // 0x434693A4, optional engine global
    // Actual external engine targets at animation interface +0x5C / +0x64.
    virtual std::int32_t select_weighted_sequence(SequenceEntity&, std::int32_t activity) = 0;
    virtual std::int32_t lookup_sequence(SequenceEntity&, std::string_view name) = 0;
    virtual void random_seed(std::int32_t) = 0; // vstdlib!RandomSeed
    virtual float interval_per_tick() = 0;
    // Each method calls the original engine receive proxy with an integer
    // payload at +8 and DVariant type 1 at +0x14. Native header words +0/+4
    // and trailing words +0x18..+0x24 are zero; +0xC/+0x10 are unspecified.
    // The remaining arguments are the entity and indicated output field.
    virtual void receive_model_index(SequenceEntity&, std::int32_t) = 0;
    virtual void receive_sequence(SequenceEntity&, std::int32_t) = 0;
    virtual bool has_viewmodel_model_proxy() = 0;
    virtual void receive_viewmodel_model(SequenceEntity&, std::int32_t) = 0;
    virtual void receive_weapon_handle(SequenceEntity&, std::uint32_t) = 0;
    virtual void begin_model_cache_lock() = 0; // model cache +0x84
    virtual void end_model_cache_lock() = 0; // model cache +0x88
    virtual void set_model_index(SequenceEntity&, std::int32_t) = 0; // entity +0x12C
    virtual float current_time() = 0;
    virtual void schedule_full_update(float) = 0;
};
// Complete 0x433E53A5: replay queued proxies, map actual studio sequences,
// update owned knife/viewmodel inventory entities, then replay weapon handles.
void update_sequences(const Settings&, SequenceRuntime&, std::span<const InventoryEntry>,
                      SequenceEngine&);
} // namespace skeet::skins

// Declarations from stattrak.hpp

namespace skeet::skins {
struct AttributeCounter {
    std::uint32_t id = 0, value = 0;
};
struct StatTrakWeapon {
    std::uint16_t definition = 0;
    std::uint32_t owner_low = 0, owner_high = 0, counter = 0;
    std::span<AttributeCounter> attributes;
};
class StatTrakEngine : public events::EventPlayerIndices {
public:
    virtual std::int32_t entity_index() const = 0;
    virtual std::int32_t team() const = 0;
    virtual StatTrakWeapon* active_weapon() = 0;
    virtual bool local_identity(std::uint32_t& low, std::uint32_t& high) = 0;
};
// 0x43336CC1, registered for player_death by 0x4349035C.
void player_death(const WeaponSkinsConfig&, events::GameEventReader&, StatTrakEngine&);
} // namespace skeet::skins

// Declarations from widget_setters.hpp

namespace skeet::skins {
// Complete43398ECF. This atomic setter is used by glove-paint replacement and
// automatic LEGIT group selection; ComboBox::set_value is the distinct433F49B3.
inline void set_combo_value_atomic(ui::ComboBox& combo, std::uint32_t requested) {
    const auto previous = combo.value ? *combo.value : 0;
    if (previous == requested)
        return;
    if (combo.value) {
        static_assert(sizeof(long) == sizeof(std::uint32_t));
        _InterlockedExchange(reinterpret_cast<volatile long*>(combo.value),
                             std::bit_cast<long>(requested));
    }
    combo.value_changed();
}
} //namespace skeet::skins
