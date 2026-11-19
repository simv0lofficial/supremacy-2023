#pragma once

// Visuals & ESP Subsystem Declarations
#include "skeet/render_queue.hpp"
#include "skeet/config.hpp"
#include "skeet/engine/jobs.hpp"
#include <array>
#include <iterator>
#include "skeet/esp/projection.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/esp/weapon_icon.hpp"
#include "skeet/esp/weapon_text.hpp"
#include <limits>
#include <string_view>
#include <vector>
#include "skeet/lagcomp/records.hpp"
#include <chrono>
#include <mutex>
#include <optional>
#include <memory>
#include "skeet/core.hpp"
#include "skeet/hud/observer_panel.hpp"
#include "skeet/players/state.hpp"
#include <cstddef>
#include <cstdint>
#include "skeet/events/registry.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <string>
#include "skeet/rage_hitboxes.hpp"
#include "skeet/engine/trace.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/movement/simulation_capture.hpp"
#include "skeet/movement/simulation_environment.hpp"
#include <span>
#include "skeet/animation/bones.hpp"
#include <functional>
#include <variant>
#include "skeet/script/bridge.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/events/feature_listeners.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/engine/offset_storage.hpp"
#include <unordered_map>
#include "skeet/engine/build_esp_bindings.hpp"
#include "skeet/movement/collision.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/effects/glow.hpp"
#include "skeet/network/update_observers.hpp"
#include "skeet/engine/scene_queries.hpp"
#include "skeet/effects/view.hpp"
#include "skeet/effects/world.hpp"
#include "skeet/events/hit_marker.hpp"
#include "skeet/esp/hitbox_highlight_engine.hpp"
#include "skeet/movement/blockbot.hpp"
#include <utility>
#include "skeet/anti_aim/config.hpp"
#include "skeet/anti_aim/state.hpp"
#include "skeet/esp/player_runtime.hpp"
#include "skeet/misc/ping_spike.hpp"
#include "skeet/rage/settings.hpp"
#include "skeet/esp/hitbox_highlight.hpp"
#include "skeet/network/packet_control.hpp"
#include "skeet/movement/quick_peek.hpp"
#include "skeet/client/command_owner_dispatch.hpp"
#include "skeet/hud/penetration_reticle.hpp"
#include "skeet/rage/ballistics.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/legit/runtime.hpp"
#include "skeet/lagcomp/interpolation.hpp"
#include "skeet/prediction/history.hpp"
#include "skeet/misc/ping_spike_channel.hpp"
#include <algorithm>
#include <bit>
#include <charconv>
#include <cmath>
#include <immintrin.h>
#include "skeet/esp/flags.hpp"
#include "skeet/esp/grenade_live.hpp"
#include "skeet/esp/grenade_live_bindings.hpp"
#include "skeet/esp/grenade_engine.hpp"
#include <cstring>
#include "skeet/esp/world_engine.hpp"


// Declarations from c4.hpp

namespace skeet::esp {

struct DroppedC4State {
    bool local_player_present;
    // Byte read through decoder 0x4346921C (XOR 0x82F5AD08).
    std::uint8_t local_state;
    // Dword read through decoder 0x43468F40 (XOR 0x71A5AC08).
    std::int32_t entity_state;
    Vector3 origin, view_origin;
};
// Native CC4 class handler 0x433AE05A; entity presence is a caller contract.
// The native gate is the dropped-weapons mask, while its color comes from
// the Bomb descriptor. The mutable label is held in owner storage +0xF70.
void draw_dropped_c4(ui::RenderQueue& queue, const WorldEspConfig& config,
                     const Projection& projection, const DroppedC4State& state,
                     std::u16string_view label);

struct ActiveBomb {
    const void* entity;
    bool ticking;
    Vector3 origin;
};
struct BombDamageTarget {
    Vector3 origin, view_offset;
    std::int32_t armor;
};
struct BombDamageRay {
    Vector3 start, delta;
    bool swept;
};
struct BombGroundResult {
    float fraction;
    Vector3 end_position, plane_normal;
};
struct BombTraceProvider {
    void* context;
    // EngineTrace::TraceRay, mask 0x200400B. The filter ignores bomb_entity.
    BombGroundResult (*trace)(void*, const BombDamageRay&, std::uint32_t mask,
                              const void* bomb_entity);
};
// 0x433CD3A7. A null bomb represents an empty/missing engine bomb registry.
float bomb_damage(const ActiveBomb* bomb, const BombDamageTarget& target, float peak_damage,
                  BombTraceProvider trace);

} // namespace skeet::esp

// Declarations from config.hpp

namespace skeet::esp {

// Values consumed by the recovered player overlay blocks. Engine projection,
// entity selection and the surrounding 0x43408EC9 owner are separate callers.
struct OverlayConfig {
    BooleanConfig bounding_box, health_bar, name, flags, ammo, distance;
    ColorConfig box_color, name_color, ammo_color;
    BooleanConfig out_of_fov;
    IntegerConfig arrow_size, arrow_distance;
    ColorConfig arrow_color;
    ColorConfig health_color{{{nullptr, nullptr, 0xD2E84B64u}, 1, ConfigType::color, 0},
                             0xFF50E178u};
};
// Registers this recovered subset in its original relative order. Boolean
// values survive registration, like the native descriptor constructors.
void initialize_overlay_config(ConfigLink& registry, OverlayConfig& config) noexcept;

// The player category's descriptors from native owner 0x43482632. These
// additional settings retain their native metadata while their consumers
// remain in the weapon, glow, sound, skeleton and player-selection subsystems.
struct PlayerEspConfig : OverlayConfig {
    HotkeyConfig activation;
    BooleanConfig weapon_text, weapon_icon, glow, visualize_sounds;
    BooleanConfig line_of_sight, skeleton, teammates, dormant;
    ColorConfig weapon_icon_color, glow_color, sound_color, sight_color, skeleton_color;
};
struct HitMarkerConfig {
    BooleanConfig enabled, sound;
};
struct AimbotVisualizationConfig {
    BooleanConfig enabled, safe_point;
    ColorConfig color, safe_point_color;
};
void initialize_player_esp_config(ConfigRegistries& registries, PlayerEspConfig& config) noexcept;
void initialize_hit_marker_config(ConfigLink& registry, HitMarkerConfig& config) noexcept;
void initialize_aimbot_visualization_config(ConfigLink& registry,
                                            AimbotVisualizationConfig& config) noexcept;

// All 34 settings used by the native Player ESP menu category. Each owner
// keeps its own fields and constructor order. This aggregate chooses an
// explicit owner registration order; it does not claim to reproduce startup.
struct PlayerEspSettings {
    PlayerEspConfig player;
    HitMarkerConfig hit_marker;
    AimbotVisualizationConfig aimbot_visualization;
    BooleanConfig money{{{nullptr, nullptr, 0x53970838u}, 1, ConfigType::boolean, 0}, 0};
};
void initialize_player_esp_settings(ConfigRegistries& registries,
                                    PlayerEspSettings& settings) noexcept;

} // namespace skeet::esp

// Declarations from effects_config.hpp

namespace skeet::esp {

struct EffectsSettings {
    BooleanConfig remove_flash{{{nullptr, nullptr, 0x461D3721u}, 1, ConfigType::boolean, 0},
                               0}; //43468FF4
    BooleanConfig remove_smoke, remove_fog, remove_sky;
    IntegerConfig visual_recoil{{{nullptr, nullptr, 0x70BA27E0u}, 4, ConfigType::integer, 0}, 0};
    IntegerConfig wall_transparency, prop_transparency, brightness;
    ColorConfig night_color;
    BooleanConfig remove_scope, instant_scope, disable_post_processing;
    BooleanConfig third_person_alive{{{nullptr, nullptr, 0xD35F8540u}, 1, ConfigType::boolean, 0},
                                     0}; //43468A00
    HotkeyConfig third_person_key{{{nullptr, nullptr, 0x3685FCE3u}, 8, ConfigType::hotkey, 0},
                                  {}}; //43468438
    BooleanConfig third_person_dead{{{nullptr, nullptr, 0x264E3E21u}, 1, ConfigType::boolean, 0},
                                    0}; //4346DAE0
    BooleanConfig disable_teammates{{{nullptr, nullptr, 0x4CBB0E86u}, 1, ConfigType::boolean, 0},
                                    0}; //43468544
    BooleanConfig disable_ragdolls{{{nullptr, nullptr, 0xB589F662u}, 1, ConfigType::boolean, 0},
                                   0}; //43469010
    BooleanConfig bullet_tracers;
    ColorConfig tracer_color;
    BooleanConfig bullet_impacts;
    IntegerConfig impact_duration;
    BooleanConfig harmless_molotovs;
    ColorConfig harmless_molotov_color;
    IntegerConfig molotov_percent;
};
void initialize_effects_settings(ConfigRegistries& registries, EffectsSettings& settings) noexcept;

} // namespace skeet::esp

// Declarations from entity_models.hpp

namespace skeet::effects {
class GlowRuntime;
}
namespace skeet::esp {
struct EspFrameOwner;
// Specialized native4340B322 map: uint16 model index -> shared constructor
// descriptor. Policy is the actual owner defaults, load1 and growth2.
class DangerZoneModels {
public:
    struct Entry {
        std::uint16_t model_index;
        DangerZoneItem* item;
    };

private:
    struct Node {
        Entry entry;
        Node* next;
    };

public:
    class Iterator {
    public:
        using value_type = Entry;
        using pointer = const Entry*;
        using reference = const Entry&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        Iterator() = default;
        const Entry& operator*() const noexcept {
            return node_->entry;
        }
        const Entry* operator->() const noexcept {
            return &node_->entry;
        }
        Iterator& operator++() noexcept;
        Iterator operator++(int) noexcept {
            auto previous = *this;
            ++*this;
            return previous;
        }
        friend bool operator==(const Iterator&, const Iterator&) = default;

    private:
        friend class DangerZoneModels;
        Iterator(const DangerZoneModels* map, std::uint32_t bucket, Node* node)
            : map_(map), bucket_(bucket), node_(node) {}
        const DangerZoneModels* map_ = nullptr;
        std::uint32_t bucket_ = 0;
        Node* node_ = nullptr;
    };
    explicit DangerZoneModels(engine::JobMemory memory = engine::native_job_memory());
    ~DangerZoneModels();
    DangerZoneModels(const DangerZoneModels&) = delete;
    DangerZoneModels& operator=(const DangerZoneModels&) = delete;
    bool insert(std::uint16_t, DangerZoneItem&); //4340B322, duplicates retain first
    DangerZoneItem* find(std::uint16_t) const noexcept;
    void clear(); //433CF5BD: retain buckets and next-resize policy
    void release_storage(); //433848F0, at the native owner destruction point
    Iterator begin() const noexcept; //4340C443
    Iterator end() const noexcept {
        return {this, bucket_count_, nullptr};
    }
    std::uint32_t size() const noexcept {
        return size_;
    }
    std::uint32_t bucket_count() const noexcept {
        return bucket_count_;
    }

private:
    void* allocate(std::size_t);
    void free(void*);
    void rehash(std::uint32_t); //43384D3A
    engine::JobMemory memory_;
    Node* initial_ = nullptr;
    Node** buckets_ = &initial_;
    std::uint32_t bucket_count_ = 1, size_ = 0, next_resize_ = 0;
};
struct EntityModelState {
    explicit EntityModelState(engine::JobMemory memory = engine::native_job_memory());
    std::array<DangerZoneItem, 20> descriptors;
    DangerZoneModels models;
    void bind_config(const WorldEspConfig&) noexcept;
};
void reset_danger_zone_models(EntityModelState&) noexcept;
void collect_danger_zone_model(EntityModelState&, const WorldEntity&,
                               Vector3 local_origin) noexcept;
void draw_danger_zone_models(ui::RenderQueue&, const Projection&, std::int32_t game_mode,
                             bool enabled, const EntityModelState&);
// Complete43343465 and433E2C67. Glow fields and sound model belong to this
// same native owner; its kit-model index survives reset until a new match.
void reset_esp_frame_owner(EspFrameOwner&, effects::GlowRuntime&);
void initialize_esp_entity_models(EspFrameOwner&, effects::GlowRuntime&, const WorldEspConfig&,
                                  void* const& network_string_tables);
} // namespace skeet::esp

// Declarations from entity_overlay.hpp

namespace skeet::esp {
class WorldEntityEngine;

enum class EntityOverlayHandler : std::uint8_t {
    none,
    hostage,
    dropped_bomb,
    planted_bomb,
    grenade_projectile,
    inferno,
    danger_zone_item,
    defuse_kit
};
// Constructor 0x43483E90..0x43483FD9; hashes are plain case-sensitive FNV-1a.
EntityOverlayHandler entity_overlay_handler(std::uint32_t class_name_hash) noexcept;
std::uint32_t entity_class_hash(std::string_view name) noexcept;
// Hashes follow the engine client-class linked-list order.
std::vector<EntityOverlayHandler> entity_overlay_handlers(std::span<const std::uint32_t> hashes);

struct WorldEntity {
    std::uintptr_t identity = 0;
    std::uint32_t index = 0, class_index = 0;
    bool dormant = false;
    // Engine entity virtual +0x298, decoded through native forwarding gate.
    bool weapon = false;
    std::int32_t owner_handle = -1;
    std::uint16_t definition = 0;
    std::int16_t model_index = -1;
    std::int32_t ammunition = -1, reserve_ammunition = 0;
    Vector3 origin{};
};
struct DroppedEntity {
    const WorldEntity* entity = nullptr;
    float distance_squared = 0;
    Point screen{};
};
struct SpecializedEntity {
    EntityOverlayHandler handler;
    const WorldEntity* entity;
};
struct WorldEntityScan {
    std::array<DroppedEntity, 65> dropped{};
    std::vector<SpecializedEntity> specialized;
};
// Process one engine-indexed entity. The owner dispatches a returned built-in
// handler immediately, then continues scanning; this preserves reentrant
// script/engine side effects between entity visits.
EntityOverlayHandler select_world_entity(WorldEntityScan& result, const WorldEntity& entity,
                                         std::span<const EntityOverlayHandler> handlers,
                                         std::int32_t maximum_players,
                                         std::int32_t highest_entity_index,
                                         std::uint32_t dropped_options,
                                         std::int32_t defuse_kit_model,
                                         const Projection& projection, Vector3 view_origin);
// 0x43409062..0x434092BF plus cold branch 0x434099E2..0x43409AFB.
// Entities are in engine index order. Registered specialized handlers consume
// an entity before the dropped-weapon selection gate, regardless of settings.
// Referenced entity records remain alive until drawing consumes the result.
WorldEntityScan scan_world_entities(std::span<const WorldEntity> entities,
                                    std::span<const EntityOverlayHandler> handlers,
                                    std::int32_t maximum_players, std::int32_t highest_entity_index,
                                    std::uint32_t dropped_options, std::int32_t defuse_kit_model,
                                    const Projection& projection, Vector3 view_origin);
bool ammunition_weapon(std::uint16_t definition) noexcept; // 0x433D24BB

struct DroppedEntityProviders {
    WeaponIconProvider icons;
    WeaponTextProvider names;
    void* context = nullptr;
    // Actual weapon virtual +0x5DC; called only by the enabled ammo-bar path.
    std::int32_t (*ammunition_capacity)(void*, const WorldEntity&) = nullptr;
};
// Complete 0x434092D2..0x43409660. Options: 1 icon, 2 text, 8 ammo, 16 distance.
void draw_dropped_entities(ui::RenderQueue&, const WorldEntityScan&, std::uint32_t options,
                           std::uint32_t color, DroppedEntityProviders);
// Canonical owner path: settings are reloaded across icon/name/SDK calls,
// and ammo is read from the raw entity at its actual drawing use-site.
void draw_dropped_entities(ui::RenderQueue&, const WorldEntityScan&, const WorldEspConfig&,
                           DroppedEntityProviders, WorldEntityEngine&);

struct DroppedBombState {
    // Game-rules boolean queried by this handler; absent rules allow C4.
    bool game_rules_present = false, game_rules_allow = false;
    std::int32_t bomb_state = 0;
    Vector3 origin{};
};
void draw_dropped_bomb(ui::RenderQueue&, const Projection&, Vector3 view_origin,
                       std::uint32_t dropped_options, std::uint32_t color,
                       std::u16string_view label,
                       const DroppedBombState&); // 0x433AE05A
void draw_defuse_kit(ui::RenderQueue&, const Projection&, Vector3 view_origin,
                     std::uint32_t dropped_options, std::uint32_t color, std::int32_t local_team,
                     std::int16_t model_index, std::int32_t defuse_kit_model,
                     Vector3 origin); // 0x433F135F

struct HostageHitbox {
    std::uint32_t bone = 0;
    Vector3 minimum{}, maximum{};
};
struct HostageState {
    bool dormant = false;
    std::uint8_t effects = 0;
    std::int32_t life_state = 0;
    bool bones_available = false, model_available = false;
    std::span<const Matrix3x4> bones;
    std::span<const HostageHitbox> hitboxes;
    Vector3 origin{};
};
void draw_hostage(ui::RenderQueue&, const Projection&, Vector3 local_origin, bool enabled,
                  bool bounding_box, bool distance, std::uint32_t color,
                  const HostageState&); // 0x4341925D

struct DangerZoneItem {
    std::int16_t model_index = -1;
    bool enabled = false;
    std::uint32_t color = 0;
    std::u16string_view label;
    float distance = std::numeric_limits<float>::max();
    Vector3 closest_origin{};
    // Constructor-owned entries retain this descriptor. Collection reloads
    // enabled for every entity; drawing reloads its current color.
    const ColoredWorldItem* setting = nullptr;
};
struct DangerZoneItemTemplate {
    std::uint32_t model_hash;
    std::u16string_view label;
    ColoredWorldItem WorldEspConfig::* setting;
};
// Native 0x43482FD5..0x43483AC9, preserving all three armor model entries.
std::array<DangerZoneItemTemplate, 20> danger_zone_item_templates() noexcept;
void reset_danger_zone_items(std::span<DangerZoneItem>) noexcept;
void collect_danger_zone_item(std::span<DangerZoneItem>, const WorldEntity&,
                              Vector3 local_origin) noexcept; // 0x433ED3B1
void draw_danger_zone_items(ui::RenderQueue&, const Projection&, std::int32_t game_mode,
                            bool enabled, std::span<const DangerZoneItem>); // 43409666..9815

} // namespace skeet::esp


// Declarations from flag_table_initializer.hpp

namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;
}
namespace skeet::engine {
class PreservedInitialization;
class NativeDataImage;
}
namespace skeet::esp {
// Actual43479120 row representation. The fourth string word is not touched by
// the first15 constructors; the three trailing rows are cleared in full.
struct NativeFlagRow {
    std::uint32_t mask, color;
    std::uint32_t first, last, limit, preserved;
};
static_assert(sizeof(NativeFlagRow) == 24);
static_assert(offsetof(NativeFlagRow, first) == 8);
static_assert(offsetof(NativeFlagRow, preserved) == 20);

// This binds the retained native rows, without initializing or copying them.
// CRT43407D58 owns construction and registration of the terminal destructor.
// Destruction of the view itself does not release native string allocations.
class FlagTable {
public:
    FlagTable(engine::PreservedInitialization&, engine::JobMemory);
    FlagTable(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory);
    FlagTable(const FlagTable&) = delete;
    FlagTable& operator=(const FlagTable&) = delete;
    FlagTable(FlagTable&&) = delete;
    FlagTable& operator=(FlagTable&&) = delete;
    std::span<NativeFlagRow, 18> rows() noexcept {
        return rows_;
    }
    std::span<const NativeFlagRow, 18> rows() const noexcept {
        return rows_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    std::u16string_view text(std::size_t index) const noexcept;
    void initialize(); //complete43485097
    void finalize(); //43401BC2 ->433C9151, reverse18 terminal releases
private:
    void construct_text(NativeFlagRow&, const char16_t*);
    std::shared_ptr<engine::NativeDataImage> image_;
    engine::JobMemory memory_;
    std::array<NativeFlagRow, 18>& rows_;
    char16_t& shared_empty_; //actual434759AC
};

class FlagTableInitialization {
public:
    FlagTableInitialization(FlagTable& flags, client::ExitCallbacks& callbacks) noexcept
        : flags(flags), callbacks(callbacks) {}
    FlagTableInitialization(const FlagTableInitialization&) = delete;
    FlagTableInitialization& operator=(const FlagTableInitialization&) = delete;
    FlagTableInitialization(FlagTableInitialization&&) = delete;
    FlagTableInitialization& operator=(FlagTableInitialization&&) = delete;
    FlagTable& flags;
    client::ExitCallbacks& callbacks; //same512-slot process exit registry
    void (*source_finalizer(std::uint32_t native_address))();
};
// Individual ordinary callback43407D58 (operand1A8), not a grouped startup.
// Table, context and borrowed process heap must outliveExitCallbacks::run().
void bind_flag_table_initializer(client::NativeStartupSources&, FlagTableInitialization&);
} //namespace skeet::esp

// Declarations from flags.hpp

namespace skeet::esp {

struct FlagEntry {
    std::uint32_t mask, color;
    std::u16string_view text;
};
// Detached decoded values for explicit snapshot consumers.
std::array<FlagEntry, 18> default_flag_entries() noexcept;
class FlagTable;
struct AnimationLayer {
    std::int32_t count = 0;
    float weight = 0;
    std::int32_t activity = 0;
    float cycle = 0;
};
float reload_cycle(const AnimationLayer& layer) noexcept;

struct RecordHistory {
    std::uint32_t count = 0, first = 0;
    std::array<std::int32_t, 32> ticks{};
    std::uint32_t correction_count = 0, correction_index = 0;
    struct Correction {
        std::int32_t counter = 0, limit = 0;
    };
    std::array<Correction, 6> corrections{};
};
bool correction_pending(const RecordHistory& history) noexcept;
bool fake_duck(const RecordHistory& history, float duck_amount, float duck_speed) noexcept;

// Netvars, entity handles and engine calls are decoded by the engine adapter;
// these are the actual values read by 0x43417468, not guessed memory offsets.
struct PlayerFlagState {
    std::int32_t game_mode = 0;
    bool helmet = false;
    std::int32_t armor = 0;
    bool dormant = false, scoped = false;
    float flash_alpha = 0;
    bool vip = false;
    std::int32_t team = 0;
    bool defusing = false;
    float duck_amount = 0, duck_speed = 0;
    bool weapon_valid = false;
    std::int32_t ammunition = -1, weapon_kind = 0;
    bool pin_pulled = false;
    AnimationLayer animation;
    std::int32_t entity_index = 1, bomb_owner_index = -1;
    float engine_time = 0, tick_interval = 1;
    std::int32_t last_hit_tick = 0;
    bool on_shot_enabled = false, target_is_player = false;
    bool local_player_present = false, local_player_disallowed = false;
    bool on_shot_extra_enabled = false;
    float simulation_time = 0, simulation_tick_interval = 1;
    RecordHistory history;
};
std::uint32_t active_flags(const PlayerFlagState& player) noexcept;

// Advances the side cursor only for entries whose mutable mask matches.
void draw_flags(ui::RenderQueue& queue, Point& cursor, std::int32_t line_height,
                std::uint32_t active_mask, std::span<const FlagEntry, 18> table);
void draw_flags(ui::RenderQueue& queue, Point& cursor, std::int32_t line_height,
                std::uint32_t active_mask, const FlagTable& table);

} // namespace skeet::esp

// Declarations from frame.hpp

namespace skeet::esp {
class WorldEntityEngine;
class EnginePlayers;
struct FramePlantedBomb {
    const PlantedBombState& state;
    const ActiveBomb* registered_bomb = nullptr;
    BombTraceProvider trace{};
    const BombSites* sites = nullptr;
};

struct FrameWorldEntity {
    WorldEntity entity;
    // Each registered class supplies its actual decoded engine fields. Only
    // the matching handler member is read; no rendering decision is delegated.
    const HostageState* hostage = nullptr;
    const DroppedBombState* dropped_bomb = nullptr;
    const FramePlantedBomb* planted_bomb = nullptr;
    const GrenadeEntityState* grenade = nullptr;
    const InfernoTeamState* inferno_team = nullptr;
};
struct EspFrameOwner {
    explicit EspFrameOwner(client::PlayerStateCache& shared) : players(shared) {}
    EspFrameOwner(client::PlayerStateCache& shared, std::uint8_t& activation)
        : frame{0, 0, PlayerFrameActivation{activation}}, players(shared) {}
    PlayerFrameState frame;
    PlayerRuntimeState players;
    SoundOverlay sounds; // same owner: model +5C and conditional listener bytes
    TabletUpgradeState tablet; // same owner: upgrade snapshot/frame marker
    float bomb_peak_damage = 500.0f; // constructor +6B0, next to frame.timer (+6AC)
    std::uint32_t state_6b4 = 0;
    PickupObservations pickups; // constructor +71C/+720, shared with dormant update
    std::int32_t notice_entity = 0; //calloc0; lifecycle reset writes-1
    std::int32_t notice_texture = -1;
    std::int32_t defuse_kit_model = -1;
    std::span<const EntityOverlayHandler> class_handlers;
    EntityModelState entity_models;
    std::u16string_view dropped_bomb_label;
    GrenadeEntityClasses grenade_classes{};
    GrenadeEntityLabels grenade_labels{};
};
struct EspFrameInputs {
    const PlayerEspConfig& player_config;
    const WorldEspConfig& world_config;
    const GrenadeEspConfig& grenade_config;
    const PlayerDrawContext& draw;
    DroppedEntityProviders dropped;
    effects::GlowEngine* glow = nullptr;
    const IndicatorLayout& indicator_layout;
    InputBitmap keys;
    // Live native byte4347BE09 + slot*32, shared with PLAYERS and plist.
    const skeet::players::State& player_adjustments;
};
struct EspFrameContext : EspFrameInputs {
    EspFrameContext(EspFrameInputs input, EnginePlayers& source, WorldEntityEngine& world)
        : EspFrameInputs(input), players(source), entities(world) {}
    EnginePlayers& players;
    WorldEntityEngine& entities;
};

// Complete semantic 43408EC9 owner. World class handlers execute immediately
// after each GetClientEntity visit. Player preparation, drawing and state
// restoration remain in the same loop iteration, including observer panels.
void draw_esp_frame(ui::RenderQueue&, EspFrameOwner&, std::int32_t& indicator_cursor,
                    const EspFrameContext&);

} // namespace skeet::esp

// Declarations from frame_event_state.hpp

namespace skeet::esp {
struct PickupObservation {
    std::uint16_t player_index, item_index;
    std::int32_t frame;
};
static_assert(sizeof(PickupObservation) == 8);
// Native constructor owner+71C/+720. Newest entries are at index0; the shared
// dormant update consumer and event callbacks use this one queue.
struct PickupObservations {
    std::uint32_t count = 0;
    std::array<PickupObservation, 128> entries{};
};
static_assert(offsetof(PickupObservations, entries) == 4);
} // namespace skeet::esp

// Declarations from frame_events.hpp

namespace skeet::esp {
// Actual43482632 event bodies. The counter pointer is the borrowed43468D4C
// global; it is reloaded after external calls, like the native owner.
class EspFrameEvents {
public:
    EspFrameEvents(EspFrameOwner& owner, const PlayerEspConfig& config,
                   const PlayerRuntimeEngine& players, std::int32_t* const& frame)
        : owner_(owner), config_(config), players_(players), frame_(frame) {}
    void item_pickup_slerp(events::GameEventReader&, events::EventPlayerIndices&); //433E0236
    void player_spawn(events::GameEventReader&, events::EventPlayerIndices&); //433986DA
    void item_equip(events::GameEventReader&, events::EventPlayerIndices&); //4334C7A3
    void bomb_begin_plant(events::GameEventReader&); //4338A457
    void reset_plant(events::GameEventReader&); //4332CA32
    void round_start(events::GameEventReader&); //433D5AB3
    // Preserve43483E11..43483E6D construction order around the separately
    // recovered death listener at43483E3F.
    void bind_before_death(events::EventRegistry&);
    void bind_after_death(events::EventRegistry&);
    // The owner stores these descriptors at+EE0/+EF0/+F00.4337F9F6 gates
    // item_equip by Dormant and begin/abortplant by WorldESP Bomb. Its update
    // prefix adds/removes them; the event bodies keep their own native gates.
    std::array<events::EventListener, 3> dormant_listeners() noexcept;

private:
    RuntimePlayer* event_player(std::uint32_t user_id, events::EventPlayerIndices&,
                                std::int32_t& index);
    void site(events::GameEventReader&);
    static void pickup(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void spawn(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void equip(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void begin(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void reset(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void round(void*, events::GameEventReader&, events::EventPlayerIndices&);
    EspFrameOwner& owner_;
    const PlayerEspConfig& config_;
    const PlayerRuntimeEngine& players_;
    std::int32_t* const& frame_;
};
} // namespace skeet::esp

// Declarations from frame_handler.hpp

namespace skeet::esp {
// Constructor43482632 registers its secondary interface at owner+8 through
//433D20B5, then installs vtable43471E04 (slot0 =43408EC9). This concrete
// endpoint joins that same borrowed HUD registry to the full ESP body.
// Construct at the owner's registration point; owner/context outlive callbacks.
class EspFrameHandler final : public hud::HudFrameCallback {
public:
    EspFrameHandler(engine::OwnerRegistryBinding registry, EspFrameOwner& owner,
                    const EspFrameContext& context);

private:
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall draw(void*, ui::RenderQueue*, void*, std::int32_t*);
#else
    static void draw(void*, ui::RenderQueue*, void*, std::int32_t*);
#endif
    inline static constexpr hud::HudFrameEntry entries_[1]{draw};
    EspFrameOwner& owner_;
    const EspFrameContext& context_;
};
} // namespace skeet::esp

// Declarations from grenade_auxiliary.hpp

namespace skeet::esp {
struct GrenadeFireDistanceContext {
    const rage::TargetBindings& targeting;
    std::int32_t maximum_players;
    const void* smoke_projectile_class;
    std::ptrdiff_t smoke_active;
};
struct GrenadeAuxiliaryContext {
    const rage::TargetBindings& targeting;
    GrenadeDamageEngine& engine;
    RageTrigonometry trigonometry;
    // Actual imported __libm_sse2_atan2 boundary (77509EE0).
    double (*arc_tangent)(double y, double x);
    std::int32_t maximum_players;
    const void* smoke_projectile_class; // runtime ClientClass at43468238
    std::ptrdiff_t smoke_active; // decoded58AE2423
    std::ptrdiff_t view_angles; // decodedE23A9BE1
    std::uint8_t* trace_flag; // actual434683D0 byte
};
struct GrenadeAuxiliaryLiveOffsets {
    explicit GrenadeAuxiliaryLiveOffsets(const engine::NativeOffsetGlobals&);
    std::array<engine::DecodedOffset, 4> rejection_bytes;
    engine::DecodedOffset team, survival_team, origin, duck_amount, smoke_active, view_angles;
};
struct GrenadeAuxiliaryLiveContext {
    const players::State& player_adjustments;
    engine::NativeConVarReference game_type;
    engine::NativeConVarReference teammates_are_enemies;
    void* const& local_player; //43475808, sampled at the enemy-predicate site
    void* const& entity_list; //4346A890, reloaded for each engine visit
    GrenadeDamageEngine& engine;
    RageTrigonometry trigonometry;
    double (*arc_tangent)(double y, double x);
    const std::int32_t& maximum_players; //sampled once at each helper entry
    const void* const& smoke_projectile_class; //43468238, live after SDK calls
    GrenadeAuxiliaryLiveOffsets offsets;
    std::uint8_t* trace_flag;
};
struct GrenadeFlashResult {
    void* target = nullptr;
    float distance_squared = 9000000.0f;
    float exposure = 0.0f;
    std::int32_t percent = 0;
};
struct GrenadeAuxiliaryResult {
    std::int32_t magnitude = 0;
    std::u16string annotation;
    bool highlight = false;
    bool draw_path = true;
};
// 433BE3C4..433BF139. position is the already raised endpoint (+1Z), as in
// the native owner. Eligibility, nearest-player choice, hull height, all seven
// possible trace calls and view-facing attenuation are concrete here.
GrenadeFlashResult evaluate_grenade_flash(Vector3 position, std::uint32_t ignored_entity,
                                          const GrenadeAuxiliaryContext&);
GrenadeFlashResult evaluate_grenade_flash(Vector3 position, std::uint32_t ignored_entity,
                                          const GrenadeAuxiliaryLiveContext&);
// Complete4338385A. A missing nearby target or overlapping live smoke leaves
// both caller-owned outputs untouched; the highlight byte is only set true.
void evaluate_grenade_fire_distance(const GrenadeState&, const GrenadeAuxiliaryContext&,
                                    std::int32_t& distance, bool& highlight);
void evaluate_grenade_fire_distance(const GrenadeState&, const GrenadeFireDistanceContext&,
                                    std::int32_t& distance, bool& highlight);
void evaluate_grenade_fire_distance(const GrenadeState&, const GrenadeAuxiliaryLiveContext&,
                                    std::int32_t& distance, bool& highlight);
// Non-HE presentation433BE36D..433BF2EC; magnitude selects path/circle color.
GrenadeAuxiliaryResult evaluate_grenade_auxiliary(const GrenadeState&,
                                                  const GrenadeAuxiliaryContext&);
GrenadeAuxiliaryResult evaluate_grenade_auxiliary(const GrenadeState&,
                                                  const GrenadeAuxiliaryLiveContext&);
} // namespace skeet::esp

// Declarations from grenade_damage.hpp

namespace skeet::engine {
class DecodedOffset;
}
namespace skeet::esp {
struct GrenadeDamagePlayer {
    std::uint32_t entity;
    Vector3 origin; // decoded A9EE6062, used by the five body samples
    Vector3 distance_origin; // caller-selected live/history/extrapolated origin
    Vector3 absolute_angles; // entity virtual +2C
    float duck_amount; // decoded 3309C0CF
    std::int32_t armor;
    bool alternate_eye; // decoded D070684C
    Vector3 virtual_origin; // entity virtual +28, alternate-eye path only
    Vector3 view_offset; // decoded C246315E, alternate-eye path only
};
enum class GrenadeDamageFilterKind {
    ignore_entity,
    exclude_players
};
struct GrenadeDamageFilter {
    GrenadeDamageFilterKind kind;
    std::uint32_t ignored_entity;
};
struct GrenadeDamageFilterEntity {
    bool base_filter_accepts; // actual engine filter at *434758B4
    bool converted_entity_present; // engine 4333D32B conversion result
    bool virtual_278;
    bool virtual_298;
};
// Complete client extension 4334EA3E to the genuine engine filter. The ordinary
// filter and GetTraceType stay actual engine dependencies (433E1CF5/433F9028).
bool grenade_damage_should_hit(GrenadeDamageFilterKind, const GrenadeDamageFilterEntity&) noexcept;
struct GrenadeDamageTrace {
    Vector3 end;
    float fraction = 1;
    bool all_solid = false, start_solid = false;
    std::uint32_t entity = 0;
};
class GrenadeDamageEngine {
public:
    virtual ~GrenadeDamageEngine() = default;
    virtual GrenadeDamageTrace trace(const GrenadeRay&, std::uint32_t mask,
                                     const GrenadeDamageFilter&) = 0;
    virtual std::uint32_t world_entity() = 0; // entity-list +0C, index zero
    virtual std::uint32_t owner_handle(std::uint32_t entity) = 0; // decoded 8D1BD8D0
    virtual std::uint32_t entity_for_handle(std::uint32_t) = 0; // entity-list +10
    virtual std::uint32_t physics_object(std::uint32_t entity) = 0; // A249061A minus C
    virtual std::int32_t physics_material(std::uint32_t object) = 0; // virtual +98
    virtual float surface_density(std::int32_t material) = 0; // surface-props +10
};
struct GrenadeDamageMath {
    RageTrigonometry trigonometry; // actual imported sinf/cosf
    float (*exponential)(float); // actual import 7750AAF0, __libm_sse2_expf
};
struct GrenadeDamageResult {
    float damage = 0, exposure = 0;
    Vector3 sample_position{};
    Vector3 distance_point{};
    bool in_range = false;
};
// Complete 4331E7F5; also inlined in the eye-position helper 433849B5.
float grenade_player_view_height(float duck_amount) noexcept;
// The repeated native two-ray/material path, e.g. 433BB853..433BBA9F.
float grenade_surface_transmission(Vector3 explosion, Vector3 sample, std::uint32_t target,
                                   GrenadeDamageEngine&);
// Shared geometry core for the live owner. Eye capture precedes the absolute
// angle getter; body origin is read after that getter, as in the native chain.
std::array<Vector3, 5> make_grenade_damage_samples(Vector3 body_origin, Vector3 captured_eye,
                                                   float captured_eye_offset, Vector3 captured_top,
                                                   Vector3 absolute_angles,
                                                   const GrenadeDamageMath&);
// Raw Gaussian result. The live caller performs its later armor-property read.
GrenadeDamageResult evaluate_he_damage_geometry(Vector3 explosion, std::uint32_t target,
                                                Vector3 distance_point,
                                                const std::array<Vector3, 5>&, GrenadeDamageEngine&,
                                                const GrenadeDamageMath&);
// The third body ray decodes and reads the real origin property after the
// preceding two engine traces, including every offset candidate.
GrenadeDamageResult
evaluate_he_damage_live_geometry(Vector3 explosion, std::uint32_t target, Vector3 distance_point,
                                 const std::array<Vector3, 5>&, const void* actual_entity,
                                 const engine::DecodedOffset& origin_offset, GrenadeDamageEngine&,
                                 const GrenadeDamageMath&);
// Per-target native chain 433BB35C..433BE1E3, duplicated in 43337BC0.
// The caller owns player eligibility and the selected distance_origin. All five
// body rays, thirteen collision-adjusted offset probes, material transmission,
// strict exposure selection and the final Gaussian/armor arithmetic stay here.
GrenadeDamageResult evaluate_he_damage(Vector3 explosion, const GrenadeDamagePlayer&,
                                       GrenadeDamageEngine&, const GrenadeDamageMath&);
} // namespace skeet::esp

// Declarations from grenade_engine.hpp

namespace skeet::esp {
struct GrenadeEngineOffsets {
    explicit GrenadeEngineOffsets(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset origin, velocity, initial_velocity, simulation_time;
    engine::DecodedOffset effects, explosion_effect_tick, thrower_handle, owner_handle;
    engine::DecodedOffset damage, collision_group, projectile_flags, inferno_start_tick;
    engine::DecodedOffset team, survival_team, cached_team_valid, cached_team;
    engine::DecodedOffset physics_object; // decodedA249061A minus0C
    engine::DecodedOffset fire_count, fire_active, fire_x, fire_y, fire_z;
    engine::DecodedOffset smoke_active;
};
// Constructs live operands, without sampling any decoded value.
GrenadeEngineOffsets capture_grenade_engine_offsets(const engine::NativeOffsetGlobals&);
struct GrenadeEngineBindings {
    void* const& entity_list; //4346A890
    void* const& engine_trace; //43468304
    void* const& surface_properties; //43469268
    void* const& spatial_partition; //43467F10
    void* const& static_props; //43467014, virtual3 from4333D32B
    void* const* const& skip_filter_vtable; //genuine434758B4
    void* const* const& entity_filter_vtable; //genuine43475B90
    void* const& player_resource_slot; //43468FF0, optional pointer-to-resource
    const void* const& smoke_projectile_class; //43468238
    GrenadeEngineOffsets offsets;
    const void* const& base_projectile_class; //43468E30
    const void* const& molotov_projectile_class; //43467E88
    const void* const& inferno_class; //434674E8
};

// Only genuine engine calls and raw property reads cross this boundary.
// Prediction, damage, collector acceptance and owner policies remain source.
class EngineGrenades final : public GrenadeWarningEngine {
public:
    explicit EngineGrenades(const GrenadeEngineBindings& bindings) : bindings_(bindings) {}
    const GrenadeEngineBindings& bindings() const noexcept {
        return bindings_;
    }
    GrenadeTraceProvider provider(std::uint8_t* trace_flag) noexcept;
    GrenadeDamageTrace trace(const GrenadeRay&, std::uint32_t, const GrenadeDamageFilter&) override;
    std::uint32_t world_entity() override;
    std::uint32_t owner_handle(std::uint32_t) override;
    std::uint32_t entity_for_handle(std::uint32_t) override;
    std::uint32_t physics_object(std::uint32_t) override;
    std::int32_t physics_material(std::uint32_t) override;
    float surface_density(std::int32_t) override;
    bool snapshot_for_handle(std::uint32_t, GrenadeWarningSnapshot&) override;
    bool snapshot_for_index(std::uint32_t, GrenadeWarningSnapshot&) override;
    std::span<const GrenadeWarningPartitionEntity> partition_entities(Vector3, float, std::uint32_t,
                                                                      bool) override;
    // Native433E92B5 collector: class filtering and sixteen accepted pointers.
    // The owner reads their origins/active bytes only after the query returns.
    std::span<void* const> collect_smokes(Vector3, float, std::uint32_t, bool);
    void* by_index(std::int32_t) const;
    void* by_handle(std::uint32_t) const;
    const void* client_class(void*) const;
    std::int32_t index(void*) const;
    bool player(void*) const;
    bool dormant(void*) const;
    Vector3 live_origin(void*) const;
    std::span<const Vector3> fire_cells(void*);

private:
    // Copy the reference-bearing aggregate; callers need not retain a temporary
    // aggregate, while every engine/class/filter cell remains live.
    GrenadeEngineBindings bindings_;
    std::array<void*, 16> smoke_entities_{};
    std::array<GrenadeWarningPartitionEntity, 16> smoke_views_{};
    const void* collected_smoke_class_ = nullptr;
    std::vector<Vector3> cells_;
    bool snapshot(void*, GrenadeWarningSnapshot&);
    static GrenadeTrace trace_projectile(void*, const GrenadeRay&, std::uint32_t,
                                         const GrenadeTraceFilter&);
};
} // namespace skeet::esp

// Declarations from grenade_environment.hpp

namespace skeet::esp {
struct GrenadeEnvironmentOffsets {
    explicit GrenadeEnvironmentOffsets(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset fire_origin_a9ee6062, fire_count_3584e3a5, fire_active_fc1ef3a1;
    engine::DecodedOffset fire_x_24a16ff6, fire_y_94e8c7f7, fire_z_d7600aac;
    engine::DecodedOffset collision_flags_b7e9c94e, movement_type_7501cb91, state_f0374295;
    engine::DecodedOffset owner_8d1bd8d0, activity_8dbc9874, activity_68140cd8;
};
// Constructs bound operands only; no pointer/key/pointee is sampled here.
GrenadeEnvironmentOffsets capture_grenade_environment_offsets(const engine::NativeOffsetGlobals&);
// Actual engine storage, read again at each native physics/trace boundary.
// No scalar physics snapshot is accepted by the production overloads.
struct GrenadeEnvironmentBindings {
    const float& tick_interval; // *4346A338
    const float& frame_interval; //43467B04 pointee+20
    engine::NativeConVarReference gravity_convar; //43467A54 xorB882FC15
    engine::NativeConVarReference incendiary_duration_convar; //43467048 xor983081BF
    engine::NativeConVarReference incendiary_max_slope_convar; //43467088 xor8FD87158
    void* const& entity_list; //4346A890, index virtual3/highest field24
    void* const& breakable_class; //4346A440
    void* const& physics_class; //43467478
    void* const& alternate_breakable_class; //43468CA0
    void* const& slope_exempt_class; //4346719C
    const void* const& inferno_class; //434674E8
    GrenadeEnvironmentOffsets offsets;
    float gravity() const noexcept;
    float incendiary_duration() const noexcept;
    float incendiary_max_slope() const noexcept;
    std::uint32_t world_entity() const;
};
} // namespace skeet::esp

// Declarations from grenade_live.hpp

namespace skeet::misc {
struct GrenadeCommandState;
struct GrenadePredictionConfig;
}
namespace skeet::esp {
struct GrenadeEnvironmentBindings;
struct GrenadeLiveBindings;

// Production scene reader. Every method implements its native stage directly
// from real engine interfaces, decoded offsets and canonical runtime owners.
// No feature callbacks or precomputed per-player candidate arrays are accepted.
class GrenadeLiveScene {
public:
    explicit GrenadeLiveScene(const GrenadeLiveBindings&);
    ~GrenadeLiveScene();
    GrenadeLiveScene(const GrenadeLiveScene&) = delete;
    GrenadeLiveScene& operator=(const GrenadeLiveScene&) = delete;
    GrenadeTraceProvider trace() noexcept;
    const GrenadeEnvironmentBindings& environment() const noexcept;
    GrenadeWarningEngine& engine() noexcept;
    MovementCollisionEngine& movement() noexcept;
    const GrenadeDamageMath& damage_math() const noexcept;
    const GrenadeAuxiliaryLiveContext& auxiliary();
    // Candidate capture occurs at each actual client ordinal, after preceding
    // targets' traces. The native selector is not fed a bulk snapshot.
    GrenadeDamageSelection select_damage(Vector3 explosion, const IntegerConfig& threshold);
    // Re-reads the stage's scalar/local fields at their actual use sites.
    void assess_warning(GrenadeWarningRecord&, void* selected_player,
                        std::uint32_t& command_buttons);
    // incoming is the actual +44 stride8 / +4044 mutable packet container.
    void update_remote_warnings(GrenadeWarningOwner&, const GrenadeEspConfig&,
                                void* selected_player, std::uintptr_t incoming);
    void draw_prediction(ui::RenderQueue&, const misc::GrenadePredictionConfig&,
                         const GrenadeEspConfig&, GrenadeTrajectoryOwner&,
                         misc::GrenadeCommandState&, GrenadeWarningOwner&);

private:
    struct Storage;
    std::unique_ptr<Storage> storage_;
};
} // namespace skeet::esp

// Declarations from grenade_live_bindings.hpp

namespace skeet::engine {
class SceneQueries;
struct NativeOffsetGlobals;
}
namespace skeet::ui {
class RendererBackend;
}
namespace skeet::esp {
struct GrenadeLiveOffsets {
    explicit GrenadeLiveOffsets(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset armor; //47A6F50C
    engine::DecodedOffset view_offset; //C246315E
    engine::DecodedOffset life_state; //9D920E35
    engine::DecodedOffset movement_type; //7501CB91
    engine::DecodedOffset observer_mode; //2F4E97C4
    engine::DecodedOffset observer_target; //3B704B41
    engine::DecodedOffset view_angles; //E23A9BE1
    engine::DecodedOffset alternate_eye; //D070684C
    engine::DecodedOffset player_flags; //E2FBE695
    engine::DecodedOffset weapon_definition; //5C9F0FBB
    engine::DecodedOffset weapon_throw_time; //3B93667B
    engine::DecodedOffset weapon_strength; //1B9654D9
    engine::DecodedOffset weapon_state; //9E2078A1
    engine::DecodedOffset recoil; //1BAE1168
    engine::DecodedOffset game_player_override; //D720AEA6
    engine::DecodedOffset teammate_damage_disabled; //F57BEAA9
    engine::DecodedOffset rejection_cde290d2;
    engine::DecodedOffset rejection_af649dac;
    engine::DecodedOffset duck_amount; //3309C0CF
    engine::DecodedOffset active_weapon; //CD812654
};
struct GrenadeLiveConvars {
    engine::NativeConVarReference friendly_fire; //211963A6
    engine::NativeConVarReference self_he_damage; //02081323
    engine::NativeConVarReference teammate_he_damage; //6B6A9C24
    engine::NativeConVarReference teammate_fire_damage; //D5F0540C
    engine::NativeConVarReference recoil_scale; //C429E1BF
    engine::NativeConVarReference game_type; //AC9299CC
    engine::NativeConVarReference teammates_are_enemies; //9190E2E6
};
GrenadeLiveOffsets capture_grenade_live_offsets(const engine::NativeOffsetGlobals&);
// All references bind canonical runtime cells, not copies of player/global
// state. The common query adapters have one process-lifetime physical owner.
struct GrenadeLiveBindings {
    engine::SceneQueries& queries;
    const MovementCaptureInput& movement_capture;
    const MovementEnvironmentBindings& movement_environment;
    const players::State& player_adjustments;
    client::PlayerStateCache& players;
    void* const& engine_client; //4346A514
    void* const& local_player; //43475808
    void* const& game_rules_slot; //4346A3A4, optional pointer-to-rules
    const std::int32_t& maximum_players; //*43469300
    const std::int32_t& tick_count; //*43468D4C
    const std::int32_t& command_count; //*434757DC
    const std::int32_t& last_outgoing; //*43475B2C
    const float& current_time; //*43467EB8
    const float& engine_clock_interval; //43467B04 pointee+20
    const float& interpolation; //43467B04 pointee+24
    const std::uint8_t& grenade_entities_enabled; //43467CD4
    const Projection& projection; //canonical434798E0 matrix/scales
    ui::RendererBackend& renderer; //actual render backend screen/texture storage
    const GrenadeWarningTexture& explosive_texture;
    const GrenadeWarningTexture& fire_texture;
    const GrenadeDamageMath& math;
    double (*arc_tangent)(double, double); //genuine importedatan2
    std::uint8_t* trace_flag; //actualoptional434683D0 byte
    std::uint32_t& command_buttons; //434759C0
    GrenadeLiveConvars convars;
};
} // namespace skeet::esp

// Declarations from grenade_prediction.hpp

namespace skeet::effects {
struct GlowEngine;
}
namespace skeet::ui {
class RendererBackend;
}
namespace skeet::rage {
struct TargetBindings;
}
namespace skeet {
struct MovementSimulationPlayer;
struct MovementSimulationRules;
struct MovementSimulationEnvironment;
struct MovementAngles;
class MovementCollisionEngine;
}
namespace skeet::esp {
struct GrenadeEnvironmentBindings;

enum class GrenadeKind : std::int16_t {
    flash = 43,
    explosive = 44,
    smoke = 45,
    molotov = 46,
    decoy = 47,
    incendiary = 48,
    tactical = 82,
    diversion = 83
};

// Engine entity fields used by 0x43331B86/0x4336DA23. Class identifiers are
// runtime ClientClass pointers, compared with the engine's registered classes.
struct GrenadeHitEntity {
    std::uint32_t identity = 0, handle = 0xffffffffu, native_class = 0;
    bool player = false;
    bool incendiary_contact_exempt = false; // entity virtual +0x284
    std::uint8_t movement_type = 0, state = 0, collision_state = 0;
    std::uint32_t flags = 0, solid_flags = 0;
    std::int32_t solid_type = 0;
    std::uint32_t owner_handle = 0xffffffffu;
    float collision_activity[2]{}; // decoders 8DBC9874 and 68140CD8
};
struct GrenadeRay {
    Vector3 start, delta, extents;
    bool swept, is_ray;
};
struct GrenadeTrace {
    Vector3 end, normal;
    float fraction = 1;
    std::uint32_t contents = 0;
    std::uint16_t surface_flags = 0;
    bool all_solid = false, start_solid = false;
    GrenadeHitEntity entity;
};
struct GrenadeTraceFilter {
    std::uint32_t ignored_entity, ignored_handle;
    std::int32_t collision_group;
    std::uint16_t projectile_flags = 0; // current raw bounce word, native filter+16
    GrenadeKind kind = static_cast<GrenadeKind>(0); //native filter+14
};
struct GrenadeTraceProvider {
    void* context;
    // The only callable boundary: EngineTrace::TraceRay. Entity fields above
    // are decoded engine output; prediction, retries and collision stay here.
    GrenadeTrace (*trace)(void*, const GrenadeRay&, std::uint32_t, const GrenadeTraceFilter&);
    std::uint8_t* trace_flag = nullptr; // optional engine byte at native 434683D0
};
struct GrenadeFire {
    Vector3 origin;
    // World positions of active fire cells, from the entity's integer offsets.
    std::span<const Vector3> cells;
};
struct GrenadeEnvironment {
    float tick_interval, frame_interval, gravity;
    float incendiary_duration, incendiary_max_slope;
    std::uint32_t world_entity;
    // Native registry slots 4346A440, 43467478 and 43468CA0 respectively.
    std::uint32_t breakable_class, physics_class, alternate_breakable_class;
    std::span<const GrenadeFire> fires;
};
struct GrenadeState {
    GrenadeKind kind;
    Vector3 position, velocity, last_plane;
    float time = 0, detonate_time = 0;
    std::int32_t next_think_tick = 0, collision_group = 13;
    std::uint32_t owner_entity = 0, ignored_handle = 0xffffffffu;
    std::int16_t bounces = 0;
    bool fire_ground = false, extinguished = false, sky = false;
    bool hit_world = false, finalized = false, collided = false;
};
struct GrenadeScreenPoint {
    float x, y;
};
struct GrenadePath {
    std::vector<GrenadeScreenPoint> points, bounces;
};
struct GrenadeThrowInput {
    Vector3 eye_position, player_velocity, view_angles, aim_punch;
    float strength, throw_velocity, recoil_scale;
    bool compensate_movement;
    std::uint32_t owner_entity;
    bool zero_slow_movement = true; //433BA5DF clears speed<10; command4348675E retains it
};
struct GrenadeLaunch {
    Vector3 position, velocity, angles;
    float strength, speed;
};
// Complete native compensation helper 433E76A9. Non-finite solutions retain
// the input angles. The coefficients are the native owner's constructor state.
GrenadeCompensation compensate_grenade_movement(Vector3 velocity, float speed, Vector3& angles) noexcept;
// Launch chain 433BAB00..433BB16C: strength/speed clamps, pitch adjustment,
// optional recoil/movement compensation, 22-unit hull trace, six-unit setback.
GrenadeLaunch prepare_grenade_throw(const GrenadeThrowInput&, GrenadeTraceProvider);
struct GrenadeTrajectoryOwner {
    Vector3 smoothed_velocity{};
    GrenadeState state{};
    GrenadePath path;
    GrenadeLaunch launch{};
    Vector3 drawing_position{}; //433BB1C9 projects before HE/flash +1Z
};
struct GrenadeTrajectoryFrame {
    // Actual engine virtual +148/+174 and helper433DF22C read results.
    bool engine_reject_148, engine_reject_174, game_player_override;
    std::uint32_t player_entity;
    std::uint8_t movement_type, life_state;
    bool selected_is_local;
    float field_e23a9be1;
    bool weapon_present;
    GrenadeKind kind;
    std::int32_t field_9e2078a1;
    float throw_time;
    // Owner's cached input vector +100, and player byte D070684C.
    Vector3 motion_target;
    bool cached_motion;
    std::int32_t motion_tick, prediction_tick, prediction_shift;
    bool two_tick_preroll;
    GrenadeThrowInput throw_input;
};
struct GrenadeMovementContext {
    Vector3 origin;
    std::uint32_t player_flags;
    const MovementSimulationPlayer* player;
    const MovementSimulationRules* rules;
    const MovementSimulationEnvironment* environment;
    const UserCommand* command;
    const MovementAngles* command_angles;
    MovementCollisionEngine* collision;
};
// 433BA600..433BA845, including unconditional cached-velocity smoothing.
bool begin_grenade_trajectory(GrenadeTrajectoryOwner&, const GrenadeEspConfig&,
                              const GrenadeTrajectoryFrame&) noexcept;
// Actual caller composition, including concrete movement-simulation preroll.
// Movement context is required when the native cached-motion/tick gate passes.
bool predict_grenade_trajectory(GrenadeTrajectoryOwner&, const GrenadeEspConfig&,
                                const GrenadeTrajectoryFrame&, const GrenadeMovementContext&,
                                const GrenadeEnvironment&, GrenadeTraceProvider, const Projection&);
struct GrenadeDamagePlayer;
class GrenadeDamageEngine;
struct GrenadeDamageMath;
struct GrenadeAuxiliaryContext;
struct GrenadeDamageCandidate {
    const GrenadeDamagePlayer* player;
    const void* entity; // actual engine pointer used by the shared enemy predicate
    bool is_player;
    std::uint8_t life_state, field_d070684c, field_cde290d2, field_af649dac;
    bool dormant;
    float history_health;
    Vector3 history_origin;
    std::int32_t health; // actual virtual +1E8 result
};
struct GrenadeDamageSelection {
    float damage = 0;
    bool reached_threshold = false;
    std::int32_t target_index = -1;
};
// Native 433BE1E3..433BE2A8; returns true when the caller stops scanning.
bool consider_grenade_damage(GrenadeDamageSelection&, float damage, std::int32_t armor,
                             std::int32_t health, std::int32_t threshold,
                             std::int32_t target_index) noexcept;
// Candidate storage follows engine client indices, including index zero.
GrenadeDamageSelection select_grenade_damage(Vector3 explosion,
                                             std::span<const GrenadeDamageCandidate>,
                                             std::int32_t maximum_players, std::int32_t threshold,
                                             const rage::TargetBindings&, GrenadeDamageEngine&,
                                             const GrenadeDamageMath&);

// Constructor 4338A122 and simulation 433684B2. Inputs are the engine's
// projectile origin/velocity and absolute projectile/creation times.
// world_path uses derived4331FC5B: append after each tick while time>=0,
// before the time increment. Its owner supplies initial/final endpoints.
void simulate_grenade(GrenadeState& state, GrenadeKind kind, Vector3 origin, Vector3 velocity,
                      float current_time, float creation_time,
                      const GrenadeEnvironment& environment, GrenadeTraceProvider trace,
                      const Projection* projection = nullptr, GrenadePath* path = nullptr,
                      std::int32_t collision_group = 13, std::uint16_t projectile_flags = 0,
                      std::vector<Vector3>* world_path = nullptr);
void simulate_grenade(GrenadeState& state, GrenadeKind kind, Vector3 origin, Vector3 velocity,
                      float current_time, float creation_time, const GrenadeEnvironmentBindings&,
                      GrenadeTraceProvider, const Projection* projection = nullptr,
                      GrenadePath* path = nullptr, std::int32_t collision_group = 13,
                      std::uint16_t projectile_flags = 0,
                      std::vector<Vector3>* world_path = nullptr);
// Full tick chain 433B7469 -> 433F541B -> 43331B86 -> 4336DA23.
void step_grenade(GrenadeState& state, const GrenadeEnvironment& environment,
                  GrenadeTraceProvider trace, const Projection* projection = nullptr,
                  GrenadePath* path = nullptr);
void step_grenade(GrenadeState&, const GrenadeEnvironmentBindings&, GrenadeTraceProvider,
                  const Projection* projection = nullptr, GrenadePath* path = nullptr);
// 43369342; HE final ground placement and incendiary ground search.
void finalize_grenade(GrenadeState& state, GrenadeTraceProvider trace);
// Float projection callbacks 43396CE8 and 4337CF74.
bool project_grenade_point(const Projection& projection, Vector3 position,
                           GrenadeScreenPoint& output) noexcept;
void record_grenade_point(GrenadePath& path, const Projection& projection, Vector3 position,
                          bool bounce);
// 433BF2F2..433BF498: path, collision circles and detonation circle.
void draw_grenade_path(ui::RenderQueue& queue, const GrenadeEspConfig& config,
                       const GrenadePath& path, const Projection& projection,
                       Vector3 detonation_position, std::int32_t damage);
// Native enabled gate and endpoint projection have already occurred. Keeping
// that cached point avoids re-reading the matrix after target engine calls.
void draw_active_grenade_path(ui::RenderQueue&, const GrenadeEspConfig&, const GrenadePath&,
                              const GrenadeScreenPoint* endpoint, std::int32_t magnitude);

struct GrenadeEntityClasses {
    std::int32_t base_projectile, decoy_projectile, molotov_projectile, smoke_projectile, inferno;
};
struct GrenadeEntityLabels {
    std::u16string_view explosive, flash, smoke, decoy, fire, other;
};
struct GrenadeEntityState {
    void* entity;
    std::int32_t class_index, glow_index;
    Vector3 origin;
    float damage, damage_radius;
    std::int32_t explosion_effect_tick;
    bool smoke_effect;
    std::int32_t smoke_start_tick, inferno_start_tick;
};
// Full 4332226A. Runtime class IDs and mutable labels come from the owner
// registry; suspended rendering retains the ordinary opaque label.
void draw_grenade_entity(ui::RenderQueue&, const GrenadeEspConfig&, const Projection&,
                         const GrenadeEntityClasses&, const GrenadeEntityLabels&,
                         const GrenadeEntityState&, std::int32_t current_tick, float tick_interval,
                         bool suspended, effects::GlowEngine* glow);
struct InfernoTeamState {
    bool friendly_fire, owner_present;
    std::int32_t local_team, owner_team;
};
// 43393ECC. The caller supplies a local player, as the native dispatcher does;
// owner fields are the resolved m_hOwnerEntity engine snapshot.
void draw_inferno_entity(ui::RenderQueue&, const GrenadeEspConfig&, const Projection&,
                         const GrenadeEntityClasses&, const GrenadeEntityLabels&,
                         const GrenadeEntityState&, std::int32_t current_tick, float tick_interval,
                         bool suspended, effects::GlowEngine* glow, const InfernoTeamState&);

struct GrenadeWarningRecord {
    std::uint32_t handle;
    Vector3 position;
    float damage;
    std::uint32_t type;
    GrenadeKind kind;
    bool suppressed;
    float creation_time, end_time;
};
struct GrenadeWarningEntity {
    bool present, effects_bit5; //38CA5228; distinct from networkable dormancy
    std::int32_t explosion_effect_tick;
    Vector3 origin;
    std::span<const Vector3> fire_cells;
};
struct GrenadeWarningFrame {
    bool local_present;
    std::uint8_t life_state, movement_type;
    std::int32_t observer_mode, health, width, height, tick_count;
    Vector3 local_position;
    float tick_interval, interpolation;
};
struct GrenadeWarningTexture {
    std::uint32_t texture = 0xffffffffu;
    std::int32_t width = 0, height = 0;
};
struct GrenadeWarningLiveFrame {
    void* player;
    ui::RendererBackend& renderer;
    const std::int32_t& tick_count;
    const float& tick_interval;
    const float& interpolation;
    const GrenadeWarningTexture& explosive_texture;
    const GrenadeWarningTexture& fire_texture;
};
bool project_grenade_warning(const Projection&, Vector3, Point&,
                             bool& in_front) noexcept; // 433AB8BD
// 433BF806..433BFD94: complete proximity-warning render stage.
void draw_grenade_warning(ui::RenderQueue&, const Projection&, const GrenadeWarningRecord&,
                          float distance, const GrenadeWarningFrame&, GrenadeWarningTexture);
void draw_grenade_warning(ui::RenderQueue&, const Projection&, const GrenadeWarningRecord&,
                          float distance, const GrenadeWarningLiveFrame&);
// 433BF4A8..433BF806: live entity/life-state gates and closest fire-cell range.
// Entities correspond to records in engine iteration order. Missing/inactive
// entities invalidate their records exactly as the native owner does.
void draw_grenade_warnings(ui::RenderQueue&, const GrenadeEspConfig&, const Projection&,
                           std::span<GrenadeWarningRecord>, std::span<const GrenadeWarningEntity>,
                           const GrenadeWarningFrame&, GrenadeWarningTexture explosive_texture,
                           GrenadeWarningTexture fire_texture);

struct GrenadePredictionScene {
    const Projection& projection;
    const GrenadeEnvironment& environment;
    GrenadeTraceProvider trace;
    const GrenadeMovementContext& movement;
    const rage::TargetBindings& targeting;
    GrenadeDamageEngine& damage_engine;
    const GrenadeDamageMath& damage_math;
    const GrenadeAuxiliaryContext& auxiliary;
    std::span<const GrenadeDamageCandidate> damage_candidates;
    std::int32_t maximum_players, damage_threshold;
    std::span<GrenadeWarningRecord> warning_records;
    std::span<const GrenadeWarningEntity> warning_entities;
    const GrenadeWarningFrame& viewer;
    GrenadeWarningTexture explosive_texture, fire_texture;
};
struct GrenadePredictionDrawResult {
    bool trajectory = false;
    GrenadeDamageSelection damage;
};
// Concrete owner presentation: predicts the selected throw, performs native HE
// target selection, draws trajectory/damage and updates proximity warnings.
// Engine mode exits precede both overlays, but retain velocity smoothing.
GrenadePredictionDrawResult render_grenade_prediction(ui::RenderQueue&, const GrenadeEspConfig&,
                                                      GrenadeTrajectoryOwner&,
                                                      const GrenadeTrajectoryFrame&,
                                                      const GrenadePredictionScene&);

namespace grenade_detail {
GrenadeTrace move(GrenadeState&, Vector3 displacement, GrenadeTraceProvider);
GrenadeTrace move(GrenadeState&, Vector3 displacement, GrenadeTraceProvider,
                  const GrenadeEnvironmentBindings&);
void collide(GrenadeState&, GrenadeTrace&, Vector3& displacement, const GrenadeEnvironment&,
             GrenadeTraceProvider);
void collide(GrenadeState&, GrenadeTrace&, Vector3& displacement, const GrenadeEnvironmentBindings&,
             GrenadeTraceProvider);
}
} // namespace skeet::esp

// Declarations from grenade_prediction_handler.hpp

namespace skeet::esp {
// Native secondary owner+18, registered at43486787. The frame and scene are
// the engine's current decoded snapshots, including selected observer inputs.
struct GrenadePredictionHudHandler : hud::HudFrameCallback {
    GrenadePredictionHudHandler(engine::OwnerRegistryBinding, const GrenadeEspConfig&,
                                GrenadeTrajectoryOwner&, const GrenadeTrajectoryFrame&,
                                const GrenadePredictionScene&);
    GrenadePredictionHudHandler(const GrenadePredictionHudHandler&) = delete;
    GrenadePredictionHudHandler& operator=(const GrenadePredictionHudHandler&) = delete;
    const GrenadeEspConfig& config;
    GrenadeTrajectoryOwner& owner;
    const GrenadeTrajectoryFrame& frame;
    const GrenadePredictionScene& scene;
};
} // namespace skeet::esp

// Declarations from grenade_warning_owner.hpp

namespace skeet::esp {
struct GrenadeWarningRules {
    bool friendly_fire, teammates_are_enemies;
    float self_he_damage, teammate_he_damage, teammate_fire_damage;
    // Concrete leaves 433DF22C / 433A104B, decoded game-rules bytes.
    bool game_player_override, teammate_damage_disabled;
    std::int32_t game_type;
};
struct GrenadeWarningSnapshot {
    std::uint32_t identity = 0, reference_handle = 0xffffffffu, native_class = 0;
    std::int32_t index = 0;
    bool player = false, dormant = false;
    std::int32_t team = 0, survival_team = 0, cached_team = 0;
    bool cached_team_valid = false;
    Vector3 origin{}, virtual_origin{}, velocity{}, initial_velocity{};
    std::uint32_t thrower_handle = 0xffffffffu, owner_handle = 0xffffffffu;
    std::uint32_t flags = 0; // 38CA5228
    std::int32_t explosion_effect_tick = 0, inferno_start_tick = 0;
    std::int32_t collision_group = 13; // E4E2D1A7
    std::uint16_t projectile_flags = 0; // 7418868F
    float damage = 0, simulation_time = 0;
    std::span<const Vector3> fire_cells; // decoded active world-space inferno cells
};
struct GrenadeWarningClasses {
    std::uint32_t base_projectile, molotov_projectile, inferno, smoke_projectile;
};
struct GrenadeWarningPacketUpdate {
    std::uint32_t index, state;
};
struct GrenadeWarningPartitionEntity {
    // Actual IHandleEntity +08 -> entity-list +10 -> entity +1C results.
    bool resolved, base_entity_present;
    std::uint32_t native_class; // networkable +08
    Vector3 origin; // base entity +28
    bool smoke_active; // 58AE2423
};
class GrenadeWarningEngine : public GrenadeDamageEngine {
public:
    // Actual entity-list +10/+0C and decoded engine fields/virtual results.
    virtual bool snapshot_for_handle(std::uint32_t, GrenadeWarningSnapshot&) = 0;
    virtual bool snapshot_for_index(std::uint32_t, GrenadeWarningSnapshot&) = 0;
    // Actual spatial-partition +34 candidate order. Class filtering and the
    // native collector's sixteen accepted entities stay in source below.
    virtual std::span<const GrenadeWarningPartitionEntity>
    partition_entities(Vector3 center, float radius, std::uint32_t mask, bool coarse) = 0;
};
struct GrenadeWarningUpdateFrame {
    const GrenadeWarningSnapshot* local; // param2, selected damage target
    const GrenadeDamagePlayer* local_damage;
    const GrenadeWarningSnapshot* actual_local; // global43475808, damage policy only
    std::int32_t tick_count;
    float current_time; // *43467EB8
    float engine_time; // actual EngineClient virtual +38
    float engine_clock_interval; // actual client-state +20
    bool grenade_entities_enabled; // global43467CD4
    std::uint32_t command_buttons; // global434759C0
    GrenadeWarningRules rules;
    GrenadeWarningClasses classes;
    std::span<const GrenadeWarningPacketUpdate> packet_updates;
};
struct GrenadeWarningOwner {
    std::vector<GrenadeWarningRecord> records;
    // Native +B0 scratch vector is reset for each eligible existing projectile.
    std::vector<Vector3> projectile_points;
    std::uint32_t command_buttons = 0; // updated global434759C0 value
};
std::int32_t grenade_warning_team(const GrenadeWarningSnapshot&, std::int32_t game_type) noexcept;
bool grenade_warning_expired(const GrenadeWarningRecord&, bool entity_present,
                             std::int32_t explosion_effect_tick, std::int32_t tick_count,
                             float tick_interval) noexcept;
bool grenade_warning_admitted(std::uint32_t type, const GrenadeWarningSnapshot* thrower,
                              const GrenadeWarningSnapshot& local,
                              const GrenadeWarningRules&) noexcept;
// Post-exposure policy 4333B6DC..4333B8B0 deliberately reads ordinary team,
// unlike the admission gate's dormancy/survival-aware 43341683.
float grenade_warning_damage_scale(std::uint32_t type, const GrenadeWarningSnapshot* thrower,
                                   const GrenadeWarningSnapshot& local,
                                   const GrenadeWarningRules&) noexcept;
float grenade_projectile_elapsed(float initial_z, float current_z, float gravity,
                                 float tick_interval) noexcept;
bool grenade_warning_smoke_overlap(Vector3 fire, Vector3 smoke) noexcept;
// Shared43337BC0 tail / command-owner4338AD80 assessment. The caller owns
// config/local-state gates and invalid-handle mutation; prior damage is kept
// until the HE protection comparison. Type2 reads active inferno cells.
void assess_grenade_warning_record(GrenadeWarningRecord&, const GrenadeWarningUpdateFrame&,
                                   GrenadeWarningEngine&, GrenadeTraceProvider,
                                   const GrenadeDamageMath&, std::uint32_t& command_buttons);
// Complete remote record owner 43337BC0: stable purge, existing endpoint
// refresh, packet admission, concrete simulation, HE exposure and fire policy.
void update_grenade_warning_records(GrenadeWarningOwner&, const GrenadeEspConfig&,
                                    const GrenadeWarningUpdateFrame&, const GrenadeEnvironment&,
                                    GrenadeWarningEngine&, GrenadeTraceProvider,
                                    const GrenadeDamageMath&);
// Live entity-list resolution followed by the recovered433BF4A8 warning owner.
void draw_grenade_warning_records(ui::RenderQueue&, const GrenadeEspConfig&, const Projection&,
                                  GrenadeWarningOwner&, GrenadeWarningEngine&,
                                  const GrenadeWarningFrame&,
                                  GrenadeWarningTexture explosive_texture,
                                  GrenadeWarningTexture fire_texture);
} // namespace skeet::esp

// Declarations from hitbox_highlight.hpp

namespace skeet::esp {
struct HitboxHighlightEntry {
    std::int32_t hitbox;
    std::uint32_t flags;
};
struct HitboxHighlightState {
    std::int32_t target_handle = -1; // full aimbot visualization owner +180
    std::array<HitboxHighlightEntry, 4> entries{}; // +184..+1A3
};
struct HitboxHighlightTarget {
    RuntimePlayer* player = nullptr;
    std::uint32_t movement_flags = 0;
    const animation::BoneModel* model = nullptr;
    // Studio hitbox order, its actual bone-index mapping, and the entity's
    // live bone-cache matrices. Model hitbox_bones matches hitboxes.size().
    std::span<const RageLocalHitbox> hitboxes;
    std::span<const Matrix3x4> bones;
};
struct HitboxHighlightContext {
    bool active; // native global byte 4346A65C
    const Projection& projection;
    RageVector view_origin; // 43479CCC
    const RageTrigonometry& trigonometry;
    const rage::TargetBindings& targets;
    void* engine_context = nullptr;
    // Genuine entity-list GetClientEntityFromHandle plus model/netvar/cache
    // reads; eligibility, hitbox selection and all drawing remain concrete.
    HitboxHighlightTarget (*by_handle)(void*, std::int32_t) = nullptr;
};
// Complete 433AFDF0: 12 capsule support directions, floating-point projection,
// and a 36-vertex center/transparent-edge triangle fan, native queue opcode7.
void draw_capsule_highlight(ui::RenderQueue&, const Projection&, const RageVector& view_origin,
                            const RageCapsule&, std::uint32_t color, const RageTrigonometry&);
// Complete successor 433D12F7. Both config booleans form only the outer gate;
// entry flags select its color. Invalid target eligibility clears the handle,
// while model/radius/index/projection exits retain it and prior queued draws.
void draw_hitbox_highlight(ui::RenderQueue&, const AimbotVisualizationConfig&,
                           HitboxHighlightState&, const RuntimePlayer& local,
                           const HitboxHighlightContext&);
} // namespace skeet::esp

// Declarations from hitbox_highlight_engine.hpp

namespace skeet::esp {
class EspOwnerRuntime;
class HitboxHighlightPlayers {
    std::variant<std::reference_wrapper<EnginePlayers>,
                 std::reference_wrapper<EspOwnerRuntime* const>>
        source_;

public:
    HitboxHighlightPlayers(EnginePlayers& fixture) noexcept : source_(std::ref(fixture)) {}
    HitboxHighlightPlayers(EspOwnerRuntime* const& live) noexcept : source_(std::cref(live)) {}
    EnginePlayers& get() const;
};
// Native433D12F7 reads these globals at draw time. The player identities and
// decoded offset words are the same owners used by the ESP frame and RAGE.
struct HitboxHighlightEngineContext {
    HitboxHighlightPlayers players;
    const engine::NativeOffsetStorage& offsets;
    const std::uint8_t& active; //4346A65C
    const Projection& projection;
    const RageVector& view_origin; //43479CCC
    const RageTrigonometry& trigonometry;
    const skeet::players::State& player_adjustments; //same4347BE08 whitelist table
    std::uint8_t life_state(const void* local) const;
};

// Complete live433D12F7. Model/set/descriptor/cache reads occur after actual
// entity eligibility, and each enabled entry reloads its descriptor and bone.
// This path does not populate a HitboxHighlightTarget snapshot or use its
// by_handle callback. Capsule construction/drawing remain recovered source.
void draw_live_hitbox_highlight(ui::RenderQueue&, const AimbotVisualizationConfig&,
                                HitboxHighlightState&, void* local,
                                const HitboxHighlightEngineContext&);
} //namespace skeet::esp

// Declarations from indicators.hpp

namespace skeet::ui {
class RendererBackend;
}
namespace skeet::esp {

struct IndicatorLayout {
    float scale = 1.0f;
    std::int32_t screen_height = 0;
    // Dimensions of the renderer's texture table, indexed by texture handle.
    // Native 0x433D7503 returns zero dimensions for an invalid handle.
    std::span<const Point> textures;
    // Production metrics use the actual renderer at the native query site;
    // standalone authored layouts supply the explicit dimension span above.
    const ui::RendererBackend* texture_source = nullptr;
};

// Native drawing branch 0x434004DE..0x434008B8. The caller routes an active
// Lua indicator hook before entering this branch (0x43400499..0x434004D9).
// cursor is the accumulated distance from the bottom edge of the screen.
void draw_indicator(ui::RenderQueue& queue, std::int32_t& cursor, std::string_view utf8,
                    std::uint32_t color, const IndicatorLayout& layout, std::int32_t texture = -1,
                    float fraction = -1.0f);

// Complete 0x4340047A routing: an installed Lua hook consumes the call even
// when its same-frame recursion guard suppresses dispatch.
inline void draw_indicator(script::Bridge& scripts, ui::RenderQueue& queue, std::int32_t& cursor,
                           std::string_view utf8, std::uint32_t color,
                           const IndicatorLayout& layout, std::int32_t texture = -1,
                           float fraction = -1.0f) {
    if (utf8.empty())
        return;
    if (script::has_indicator(scripts)) {
        script::dispatch_indicator(scripts, &queue, &cursor, utf8, color);
        return;
    }
    draw_indicator(queue, cursor, utf8, color, layout, texture, fraction);
}

} // namespace skeet::esp

// Declarations from line_of_sight.hpp

namespace skeet::esp {
struct LineOfSightBindings {
    void* const& engine; //4346A514; connected26/demo82/HLTV93
    void* const& debug_overlay; //optional43468260, AddLineOverlayAlpha20
    engine::NativeConVarReference interpolation_cvar; //4346A850^478720C2
    std::int32_t* const& maximum_players; //43469300, inclusive upper bound
    std::uint8_t* const& trace_flag; //optional434683D0, exact guard byte
    MovementCollisionEngine& collision;
    RageTrigonometry trigonometry; //actual SSE sine/cosine CRT boundaries
    std::ptrdiff_t duck_amount; //3309C0CF, float
    std::ptrdiff_t eye_angles; //E23A9BE1, pitch/yaw
};
struct LineOfSightInputs {
    const PlayerEspConfig& config;
    const players::State& adjustments;
    const PlayerFrameState& frame; //canonical4346A65C activation result
    const std::uint8_t& suppressed; //43468E58, reread at native gate
    const LineOfSightBindings& bindings;
};
struct LineOfSightContext : LineOfSightInputs {
    LineOfSightContext(LineOfSightInputs inputs, EnginePlayers& source)
        : LineOfSightInputs(inputs), players(source) {}
    EnginePlayers& players;
};
// Complete4337FA44..4337FDC9: engine/player gates, smoothstep eye origin,
// rounded2048-unit ray, reject-entities trace and alpha debug-overlay line.
void draw_line_of_sight(const LineOfSightContext&);

// The actual secondary interface at43482632 owner+28, vtable434708D0.
// Its one slot4337F9F6 synchronizes event subscriptions before sight drawing.
class LineOfSightHandler final : public engine::FrameUpdateEndpoint {
public:
    LineOfSightHandler(engine::OwnerRegistryBinding, SoundOverlay&, EspFrameEvents&,
                       events::EventRegistry&, const WorldEspConfig&, const LineOfSightContext&);

private:
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall update(void*, std::uint8_t);
#else
    static void update(void*, std::uint8_t);
#endif
    inline static constexpr engine::FrameUpdateEntry entries_[1]{update};
    SoundOverlay& sounds_;
    EspFrameEvents& events_;
    events::EventRegistry& event_registry_;
    const WorldEspConfig& world_;
    const LineOfSightContext& context_;
};
} // namespace skeet::esp

// Declarations from map_entity_parser.hpp

namespace skeet::esp {
struct EspFrameOwner;
#if defined(_M_IX86) || defined(__i386__)
using MapScalarEntry = void(__vectorcall*)(void*, float);
using MapTextEntry = void(__fastcall*)(void*, const char*);
#else
using MapScalarEntry = void (*)(void*, float);
using MapTextEntry = void (*)(void*, const char*);
#endif
struct MapEntityParserTable {
    MapScalarEntry scalar;
    MapTextEntry text;
};
struct MapEntityParserEndpoint {
    const MapEntityParserTable* vtable = nullptr;
};
// Actual433817AA registration, vector43475770/74/78.
MapEntityParserEndpoint* register_map_entity_parser(engine::OwnerRegistryBinding,
                                                    MapEntityParserEndpoint*);
//43328352 forwards the stack float in XMM0;43325578 reads actual engine
//virtual99 and forwards the returned map-entity text in EDX when nonnull.
void dispatch_map_scalar(const engine::OwnerRegistry&, float);
void dispatch_map_entity_text(const engine::OwnerRegistry&, void* engine);

// Native43472B58 at ESP owner+30. Scalar slot433CCE8E is return-only;
//4339F24A parses the constructor-owned bomb-radius prefix and updates+6B0.
class EspMapEntityParser final : public MapEntityParserEndpoint {
public:
    EspMapEntityParser(engine::OwnerRegistryBinding, EspFrameOwner&, const std::string& prefix);

private:
#if defined(_M_IX86) || defined(__i386__)
    static void __vectorcall scalar(void*, float);
    static void __fastcall text(void*, const char*);
#else
    static void scalar(void*, float);
    static void text(void*, const char*);
#endif
    inline static constexpr MapEntityParserTable entries_{scalar, text};
    EspFrameOwner& owner_;
    const std::string& prefix_;
};
} //namespace skeet::esp

// Declarations from model_color_config.hpp

namespace skeet::esp {

struct ModelColorGroup {
    BooleanConfig enabled;
    ColorConfig primary_color;
    IntegerConfig material;
    ColorConfig secondary_color;
};
struct OccludedModelColorGroup : ModelColorGroup {
    BooleanConfig behind_wall;
    ColorConfig behind_wall_color;
};
struct ColoredModelsSettings {
    BooleanConfig props;
    ColorConfig props_color;
    OccludedModelColorGroup player, teammate;
    ModelColorGroup local_player, local_fake, on_shot, hands, weapon_viewmodel, weapons;
    IntegerConfig local_transparency, on_shot_duration;
    BooleanConfig shadow;
    BooleanConfig ragdolls{{{nullptr, nullptr, 0x8CCAF8DCu}, 1, ConfigType::boolean, 0},
                           0}; //4346A680
    BooleanConfig disable_model_occlusion{
        {{nullptr, nullptr, 0x8E0781AEu}, 1, ConfigType::boolean, 0}, 0}; //43467ADC
    ColorConfig shadow_color;
};
// 42 native owner descriptors at 0x4348E79F..0x4348EC45, plus two static
// descriptors. Only fields referenced by the Colored models menu are included.
void initialize_colored_models_settings(ConfigRegistries& registry,
                                        ColoredModelsSettings& settings) noexcept;

} // namespace skeet::esp

// Declarations from offscreen.hpp

namespace skeet::esp {

// Native geometry/animation block 0x43409F44..0x4340A18B. The caller has passed
// the local-player, enabled, screen visibility and angular-FOV gates.
void draw_out_of_fov_arrow(ui::RenderQueue& queue, Point screen, std::int32_t size,
                           std::int32_t distance_percent, std::uint32_t color, float yaw_delta,
                           float animation_time);
struct OffscreenInput {
    Point screen;
    Vector3 origin, target;
    // Globals 0x43479CD8/DC, in degrees.
    float pitch, yaw, animation_time;
    float camera_fov; //live4346D548, after the actual view/FOV update
};
struct LocalPlayerPredicate {
    void* context;
    bool (*evaluate)(void*);
};
// Full checkbox/predicate/FOV/draw block 0x43409D3D..0x4340A195. The enclosing
// owner has already rejected missing/local targets and on-screen players.
void draw_offscreen_indicator(ui::RenderQueue& queue, const OverlayConfig& config,
                              const OffscreenInput& input, LocalPlayerPredicate predicate);
// Continuation after the native checkbox and IsPlayer gates. The live engine
// owner reads screen/camera inputs after that SDK call, then enters here.
void draw_selected_offscreen_indicator(ui::RenderQueue&, const OverlayConfig&,
                                       const OffscreenInput&);

} // namespace skeet::esp

// Declarations from other_config.hpp

namespace skeet::esp {

struct ColoredWorldItem {
    BooleanConfig enabled;
    ColorConfig color;
};
struct WorldEspConfig {
    BooleanConfig radar, spectators;
    IntegerConfig dropped_weapons;
    ColorConfig dropped_weapon_color;
    ColoredWorldItem hostages, bomb;
    BooleanConfig upgrade_tablet, danger_zone_items;
    ColoredWorldItem drone_gun, blackhawk, drone, random_case, tool_case;
    ColoredWorldItem pistol_case, explosive_case, heavy_weapon_case, light_weapon_case;
    ColoredWorldItem dufflebag, jammer, ammobox, armor, parachute_pack, briefcase;
    ColoredWorldItem tablet_upgrade_zone, tablet_upgrade_drone, cash_stack;
};
struct ScreenOverlayConfig {
    BooleanConfig recoil, inaccuracy;
    ColorConfig inaccuracy_color;
    BooleanConfig penetration_reticle;
    BooleanConfig crosshair{{{nullptr, nullptr, 0xAD37A2EBu}, 1, ConfigType::boolean, 0}, 0};
};
struct GrenadeEspConfig {
    BooleanConfig entities{{{nullptr, nullptr, 0x25B4B670u}, 1, ConfigType::boolean, 0}, 0};
    ColorConfig entity_color{{{nullptr, nullptr, 0xB8C2C5BCu}, 4, ConfigType::color, 0}, 0};
    BooleanConfig trajectory;
    ColorConfig trajectory_color, trajectory_secondary_color;
    BooleanConfig proximity_warning;
};
struct SharedEspConfig {
    HotkeyConfig activation;
    BooleanConfig other_team;
    BooleanConfig enabled{{{nullptr, nullptr, 0x0E533025u}, 1, ConfigType::boolean, 0},
                          0}; //43467F88
    BooleanConfig restrict_updates{{{nullptr, nullptr, 0xCBE162F8u}, 1, ConfigType::boolean, 0},
                                   0}; //43468588
};
struct OtherEspSettings {
    WorldEspConfig world;
    ScreenOverlayConfig overlays;
    GrenadeEspConfig grenades;
    SharedEspConfig shared;
    IntegerConfig feature_indicators{{{nullptr, nullptr, 0x59A226C2u}, 4, ConfigType::integer, 0},
                                     0};
};

void initialize_world_esp_config(ConfigLink& registry, WorldEspConfig& config) noexcept;
void initialize_screen_overlay_config(ConfigLink& registry, ScreenOverlayConfig& config) noexcept;
void initialize_grenade_esp_config(ConfigLink& registry, GrenadeEspConfig& config) noexcept;
void initialize_shared_esp_config(ConfigRegistries& registry, SharedEspConfig& config) noexcept;
// All 62 settings referenced by the Other ESP category. The owner groups are
// registered explicitly; this aggregate is not a native startup reconstruction.
void initialize_other_esp_settings(ConfigRegistries& registry, OtherEspSettings& settings) noexcept;

} // namespace skeet::esp

// Declarations from owner_runtime.hpp

namespace skeet::esp {
// The linked class records returned by native43467470 virtual10. Class IDs
// are the traversal counter, not a field read from these records.
struct EspClientClass {
    const char* name;
    void* unused_4;
    EspClientClass* next;
};
struct EspOwnerRegistries {
    engine::OwnerRegistryBinding owners; //43475B78
    engine::OwnerRegistryBinding hud; //43475A9C
    client::EntityObserverRegistry& histories; //43475BA8
    engine::OwnerRegistryBinding updates; //43475988
    engine::OwnerRegistryBinding packets; //4347584C
    engine::OwnerRegistryBinding frame_update; //43475AD8
    engine::OwnerRegistryBinding map_parser; //43475770
    events::EventRegistry& events;
    ConfigRegistries& configuration;
};
struct EspOwnerBindings {
    // All referenced contexts/cells are canonical and outlive this owner.
    EspOwnerRegistries registries;
    EspFrameOwner& state;
    PlayerEspConfig& player;
    WorldEspConfig& world;
    EffectsSettings& effects;
    EspFrameInputs frame;
    PlayerEngineBindings player_engine;
    WorldEngineBindings world_engine;
    SoundOverlayInputs sound;
    const TabletUpgradeBindings& tablet;
    skeet::effects::GlowRuntime& glow;
    LineOfSightInputs line_of_sight;
    const PacketTracerBindings& packet;
    events::DeathBindings& death;
    const void* const& engine_globals;
    const float* const& tick_interval;
    std::int32_t* const& event_frame;
    void* const& network_string_tables; //43467198
    void* const& client_classes; //actual43467470, reloaded after event registration
    void* const& beams; //actual4346A898, captured by this constructor
};

// Complete source owner43482632, primary vtable434725A0. Every secondary
// endpoint borrows state; none allocates a second ESP/player/sound cache.
class EspOwnerRuntime {
public:
#if defined(_M_IX86) || defined(__i386__)
    using PrimaryEntry = void(__thiscall*)(void*);
#else
    using PrimaryEntry = void (*)(void*);
#endif
    const PrimaryEntry* vtable = nullptr;
    EspOwnerRuntime(EspOwnerBindings, EspOwnerRuntime*& active);
    ~EspOwnerRuntime(); //4338580A; does not invent registry/config unlinking
    EspOwnerRuntime(const EspOwnerRuntime&) = delete;
    EspOwnerRuntime& operator=(const EspOwnerRuntime&) = delete;
    EspFrameOwner& state() const noexcept {
        return bindings_.state;
    }
    const PlayerEspConfig& player_config() const noexcept {
        return bindings_.player;
    }
    const std::int32_t* override_fov() const noexcept {
        return override_fov_;
    }
    PacketTracerState& tracers() noexcept {
        return tracers_;
    }
    EnginePlayers& player_engine() noexcept {
        return players_;
    }
    WorldEntityEngine& world_engine() noexcept {
        return world_;
    }
    const EspFrameContext& frame_context() const noexcept {
        return frame_;
    }
    const SoundOverlayContext& sound_context() const noexcept {
        return sound_;
    }
    void bind_configuration(); //43485C12
    void reset(); //43343465
    void initialize_models(); //433E2C67
private:
    friend void destroy_esp_owner(EspOwnerRuntime*&, EspOwnerRuntime*);
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_ESP_PRIMARY __thiscall
#else
#define SKEET_ESP_PRIMARY
#endif
    static void SKEET_ESP_PRIMARY destroy(void*);
    static void SKEET_ESP_PRIMARY configuration(void*);
    static void SKEET_ESP_PRIMARY reset_entry(void*);
    static void SKEET_ESP_PRIMARY models(void*);
    static void SKEET_ESP_PRIMARY unchanged(void*);
    inline static constexpr PrimaryEntry entries_[9]{destroy,   configuration, unchanged,
                                                     unchanged, unchanged,     reset_entry,
                                                     unchanged, models,        unchanged};
#undef SKEET_ESP_PRIMARY
    void initialize_configuration();
    void initialize_state();
    void initialize_classes();
    EspOwnerBindings bindings_;
    EspOwnerRuntime** active_;
    EnginePlayers players_;
    WorldEntityEngine world_;
    EspFrameContext frame_;
    SoundOverlayContext sound_;
    LineOfSightContext line_of_sight_;
    const std::int32_t* override_fov_ = nullptr; //+6D4, canonical MISC FC225A4F
    IntegerConfig field_8ce3b4b2_; //native descriptor+0x17C, value+0x18C
    ColoredWorldItem field_5aadd6ed_,
        field_b1054809_; //constructed but not exposed by the native menu
    PacketTracerState tracers_;
    EspFrameEvents events_;
    events::DormantAlphaDeathListener death_;
    // Optional storage postpones construction until each actual registration
    // point. These are concrete endpoints with their recovered method bodies.
    std::optional<EspFrameHandler> hud_;
    std::optional<SoundHistoryReceiver> histories_;
    std::optional<PlayerEspUpdateHandler> updates_;
    std::optional<PacketTracerHandler> packets_;
    std::optional<LineOfSightHandler> sight_;
    std::optional<EspMapEntityParser> parser_;
    std::vector<EntityOverlayHandler> class_handlers_;
    std::array<std::u16string, 20> item_labels_;
    std::array<std::u16string, 7> labels_;
    std::string radius_prefix_;
};
// Native43481EAB allocates the0xF90 native representation and publishes the
// canonical434758DC owner. This source representation has host-sized fields.
void initialize_esp_owner(EspOwnerRuntime*& active, EspOwnerBindings bindings);
// Complete43331A5C, including clearing the canonical slot when object is null.
void destroy_esp_owner(EspOwnerRuntime*& active, EspOwnerRuntime* object);
} //namespace skeet::esp

// Declarations from packet_tracers.hpp

namespace skeet::esp {

struct PacketTracerOffsets {
    DormantRadarOffsets radar;
    std::ptrdiff_t recoil_index; // 6777C9CE, float
    std::ptrdiff_t seed; // F300F6B0, int32; valid native table index0..255
    std::ptrdiff_t fire_mode; // EC1739AA, int32
    std::ptrdiff_t weapon_id; // 571B7D9A, uint16
    std::ptrdiff_t inaccuracy; // C6C62EBA, float
    std::ptrdiff_t spread; // E9392423, float
    std::ptrdiff_t angles; // F62F1430, Vector3; native roll is replaced by0
};
struct PacketTracerState {
    void* beams = nullptr; // owner+60, original4346A898 interface
    std::uint32_t first = 0, count = 0; // owner+64/+68
    std::array<void*, 3> recent{}; // owner+6C..74, same live beam pointers
};
struct PacketTracerBindings {
    PacketTracerOffsets offsets;
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    const RageTrigonometry& trigonometry;
    engine::NativeConVarReference shotgun_patterns_convar; //43468520^394BFF56
    void* const& weapon_system; // optional actual43467B54
    BallisticEngine& ballistics;
    void* const* trace_filter_vtable; // genuine external43475B90
    const anti_aim::TargetTraceEntities& trace_entities;
    const float* const& prediction_time; // *43467EB8, reloaded after beam creation
};

// Engine-owned BeamInfo_t used by actual IViewRenderBeams::CreateBeamPoints.
// The source preserves the native zeroes as well as explicitly assigned fields.
struct PacketBeamDescription {
    std::int32_t type = 0;
    void* start_entity = nullptr;
    std::int32_t start_attachment = 0;
    void* end_entity = nullptr;
    std::int32_t end_attachment = 0;
    Vector3 start{}, end{};
    std::int32_t model_index = 0;
    const char* model_name = nullptr;
    std::int32_t halo_index = -1;
    const char* halo_name = nullptr;
    float halo_scale = 0, life = 0, width = 3, end_width = 3;
    float fade_length = 0, amplitude = 0, brightness = 0, speed = 0;
    std::int32_t start_frame = 0;
    float frame_rate = 0, red = 0, green = 0, blue = 0;
    std::uint8_t renderable = 1;
    std::array<std::byte, 3> padding_71{};
    std::int32_t segments = -1, flags = 8;
    Vector3 center{};
    float start_radius = 0, end_radius = 0;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(PacketBeamDescription) == 0x90);
static_assert(offsetof(PacketBeamDescription, start) == 0x14);
static_assert(offsetof(PacketBeamDescription, model_index) == 0x2c);
static_assert(offsetof(PacketBeamDescription, brightness) == 0x54);
static_assert(offsetof(PacketBeamDescription, flags) == 0x78);
#endif

// Complete433FBE76..433FC6BF. The same decoded packet first updates dormant
// radar geometry, then reconstructs the enemy shot and creates its beam.
void process_esp_packet(const void* packet, PacketTracerState&, SoundOverlay&,
                        const SoundOverlayContext&, const EffectsSettings&,
                        const PacketTracerBindings&);

#if defined(_M_IX86) || defined(__i386__)
using EspPacketEntry = void(__fastcall*)(void*, const void*);
#else
using EspPacketEntry = void (*)(void*, const void*);
#endif
struct EspPacketEndpoint {
    const EspPacketEntry* vtable = nullptr;
};
// Actual owner+20 interface4347082C, registered by43400957 in4347584C.
class PacketTracerHandler final : public EspPacketEndpoint {
public:
    PacketTracerHandler(engine::OwnerRegistryBinding, PacketTracerState&, SoundOverlay&,
                        const SoundOverlayContext&, const EffectsSettings&,
                        const PacketTracerBindings&);

private:
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall update(void*, const void*);
#else
    static void update(void*, const void*);
#endif
    inline static constexpr EspPacketEntry entries_[1]{update};
    PacketTracerState& state_;
    SoundOverlay& sounds_;
    const SoundOverlayContext& sound_context_;
    const EffectsSettings& settings_;
    const PacketTracerBindings& bindings_;
};

} // namespace skeet::esp

// Declarations from planted_bomb.hpp

namespace skeet::esp {

struct PlantedBombOwner {
    // Owner +0x6AC is cleared at entry, including the disabled path.
    std::int32_t transient = 0;
    float peak_damage = 0.0f; // owner +0x6B0
};
struct PlantedBombBinding {
    // The full frame uses a float timer at constructor +6AC. This handler
    // clears that same four-byte word; it does not own another counter.
    std::byte* transient_word;
    const float& peak_damage;
};
struct PlantedBombState {
    bool ticking;
    float detonation_time, defuse_end;
    std::int32_t defuser_state;
    Vector3 origin;
};
struct BombSites {
    Vector3 first, second;
};
struct PlantedBombPlayer {
    std::uint8_t life_state;
    BombDamageTarget damage_target;
    void* context;
    // Actual entity virtual health query. The owner reads it twice when the
    // predicted damage is nonzero; the values need not remain identical.
    std::int32_t (*health)(void*);
};
struct PlantedBombPlayerView {
    const std::uint8_t& life_state;
    const BombDamageTarget& damage_target;
    void* context;
    std::int32_t (*health)(void*);
    PlantedBombPlayerView(const PlantedBombPlayer& value)
        : life_state(value.life_state), damage_target(value.damage_target), context(value.context),
          health(value.health) {}
    PlantedBombPlayerView(const std::uint8_t& life, const BombDamageTarget& target, void* object,
                          std::int32_t (*query)(void*))
        : life_state(life), damage_target(target), context(object), health(query) {}
};
struct PlantedBombInput {
    const PlantedBombState& bomb;
    PlantedBombPlayerView player;
    // Registry selection used by 0x433CD3A7 is independent of bomb above.
    const ActiveBomb* registered_bomb;
    BombTraceProvider trace;
    const BombSites* sites;
    float current_time;
    std::int32_t label_texture;
    script::Bridge* scripts = nullptr;
};

// 0x43345EF2..0x4334636B, including the defuse bar, nearest-site label and
// damage/FATAL indicators. Bomb/player arguments obey the native caller's
// valid-entity contract. Layout supplies the actual renderer backend values.
void draw_planted_bomb(ui::RenderQueue& queue, const WorldEspConfig& config,
                       PlantedBombOwner& owner, std::int32_t& indicator_cursor,
                       const PlantedBombInput& input, const IndicatorLayout& layout);
void draw_planted_bomb(ui::RenderQueue& queue, const WorldEspConfig& config,
                       PlantedBombBinding owner, std::int32_t& indicator_cursor,
                       const PlantedBombInput& input, const IndicatorLayout& layout);

} // namespace skeet::esp

// Declarations from player_draw.hpp

namespace skeet::script {
struct Bridge;
struct ReferenceStrings;
}
namespace skeet::esp {

struct RuntimeWeapon {
    void* engine_entity = nullptr;
    std::uint16_t definition = 0;
    std::int32_t ammunition = -1;
    bool pin_pulled = false;
};
struct WeaponInformation {
    float penetration, range, range_modifier;
    std::int32_t kind;
};
struct PlayerWeaponMetadata {
    std::uint16_t definition = 38, padding = 0;
    float penetration = 2.5f, range = 8192.0f, range_modifier = 0.98f;
    float armor_ratio = 1.65f, damage = 80.0f;
    std::int32_t kind = 1;
};
static_assert(sizeof(PlayerWeaponMetadata) == 28);
struct PlayerSkeletonState {
    bool model_present = false, studio_present = false;
    bool counter_present = false;
    std::int32_t entity_counter = 0, global_counter = 0;
    std::span<const SkeletonBone> bones;
    std::span<const Matrix3x4> transforms;
};
struct PlayerDrawState {
    std::int32_t active_weapon_handle = -1;
    std::array<std::int32_t, 2> inventory_handles{-1, -1};
    std::int32_t money = 0;
    Vector3 view_offset{};
    Vector3 network_origin{}; // decoder 2A671B42, distinct from interpolated origin
    PlayerFlagState flags;
    PlayerSkeletonState skeleton;
    // Decoded damage/health binding for this same engine player. The world
    // dispatcher selects the actual effective view player's binding itself.
    PlantedBombPlayer bomb_target{};
    // Decoded FB1E8DAB spotted-byte snapshot.
    std::uint8_t spotted = 0;
    void* spotted_destination = nullptr; // actual entity netvar address, required by a bound proxy
};
struct SpottedProxyData {
    // Native explicitly initializes the integer +8 and type tag +0x14 to 1.
    // Remaining receive-proxy packet storage is unspecified in the owner.
    std::array<std::uint32_t, 11> words{};
};
using SpottedProxy = void (*)(const SpottedProxyData*, void* entity, void* destination);
struct PlayerDrawEngine {
    const PlayerRuntimeEngine& players;
    void* context = nullptr;
    RuntimeWeapon* (*weapon_by_handle)(void*, std::int32_t) = nullptr; // entity-list slot 4
    bool (*weapon_is_weapon)(void*, const RuntimeWeapon&) = nullptr; // weapon slot 166
    WeaponInformation (*weapon_information)(void*, const RuntimeWeapon&) = nullptr; // slot 461
    float (*weapon_armor_ratio)(void*, const RuntimeWeapon&) = nullptr; // slot 434
    std::int32_t (*weapon_damage)(void*, const RuntimeWeapon&) = nullptr; // slot 431
    std::int32_t (*weapon_capacity)(void*, const RuntimeWeapon&) = nullptr; // slot 375
    std::int32_t (*player_health)(void*, const RuntimePlayer&) = nullptr; // slot 122
    // IVEngineClient::GetPlayerInfo. Native consumes its first 20 name bytes.
    bool (*player_info)(void*, std::int32_t, std::string_view&) = nullptr;
    bool (*setup_bones)(void*, const RuntimePlayer&, std::int32_t maximum, std::uint32_t mask,
                        float time) = nullptr; // renderable slot 13
    WeaponIconProvider icons{};
    WeaponTextProvider names{};
    SpottedProxy spotted_proxy = nullptr; // *0x43477B48, optional actual RecvProxyFn
    std::int32_t (*weapon_kind)(void*, const RuntimeWeapon&) = nullptr; // slot 455
};
struct PlayerDrawRules {
    const PlayerRuntimeRules& players;
    // Integer array decoded by 9BB80163, indexed by networkable index.
    std::span<const std::int32_t> dormant_health;
};
struct PlayerFlagGlobals {
    // Actual pointer-to-pointer globals. The native helper checks both levels
    // before reading the decoded game-rules and player-resource members.
    void** const& game_rules; // borrowed global4346A3A4
    void** const& player_resource; // borrowed global43468FF0
    std::ptrdiff_t rules_guard; // D000A932, byte
    std::ptrdiff_t rules_state; // 82F5AD08, byte
    std::ptrdiff_t carrier_index; // 23B532EF, int32
    const std::uint8_t* rage_enabled; // 43467224
    const std::int32_t* on_shot_target; // 4346DB1C
    const std::uint8_t* on_shot_active; // 4347CA30
};
using PlayerSequenceActivity = engine::MemberFunction<std::int32_t, std::int32_t>;
struct PlayerReloadBindings {
    std::ptrdiff_t layers; //B538BBA5, data pointer+0/count+C
    const PlayerSequenceActivity& sequence_activity; //borrowed optional4346A364
};
// Complete433CEFB3/433880F9. The actual engine callback may change cycle;
// the saved layer's +64 cycle is read only after that callback returns.
float player_reload_cycle(const RuntimePlayer&, const PlayerReloadBindings&);
// Registry/rules gate embedded in 43417468, before the second index query.
std::int32_t bomb_carrier_index(const PlayerFlagGlobals&) noexcept;
// Full native helpers 433C652F, 43328CA6 and 433C8C4D.
std::int32_t overlay_player_health(const RuntimePlayer*, const PlayerDrawEngine&,
                                   const PlayerDrawRules&);
PlayerWeaponMetadata player_weapon_metadata(const RuntimePlayer&, const PlayerDrawState&,
                                            const PlayerDrawEngine&);
// 433FC78E, including x86 shift masking for the hitgroup argument.
bool armored_hitgroup(std::int32_t armor, bool helmet, std::int32_t hitgroup) noexcept;
void set_player_spotted(const RuntimePlayer&, PlayerDrawState&, SpottedProxy); // 433C11EB
// 4340A270..4340A44D. Other weapons retain the native number threshold 92.
std::int32_t
player_health_number_threshold(const RuntimePlayer& target, const PlayerDrawState& target_state,
                               const RuntimePlayer& view, const PlayerDrawState& view_state,
                               const PlayerDrawEngine& engine, client::PlayerStateCache& cache);

// Input projection retained for separately recovered pure leaf helpers. The
// ordinary ESP owner uses PlayerDrawContext and EnginePlayers below.
struct PlayerDrawSnapshotContext {
    const Projection& projection;
    const PlayerDrawRules& rules;
    const PlayerDrawEngine& engine;
    client::PlayerStateCache& cache;
    const PlayerFlagGlobals& flag_globals;
    const float& camera_fov; //actual4346D548, borrowed across frame callbacks
    const PlayerReloadBindings& reload;
    Vector3 view_origin{};
    Point screen{};
    float view_pitch = 0, view_yaw = 0, animation_time = 0, current_time = 0;
    const std::uint8_t& money;
    bool suspended = false;
    std::span<const FlagEntry, 18> flag_entries;
    script::Bridge* scripts = nullptr;
    script::ReferenceStrings* script_flags = nullptr;
};
struct PlayerDrawContext {
    const Projection& projection;
    client::PlayerStateCache& cache;
    const float& camera_fov;
    const std::uint8_t& money;
    FlagTable& flags;
    const WeaponIconProvider& icons;
    const WeaponTextProvider& names;
    script::Bridge* scripts = nullptr;
    script::ReferenceStrings* script_flags = nullptr;
};
// Full 43417468 binding: actual index/dormancy/weapon/time calls and shared
// ring/correction storage, followed by the independently recovered bit rules.
std::uint32_t player_overlay_flags(const RuntimePlayer&, const PlayerDrawState&,
                                   const PlayerDrawSnapshotContext&);
// The caller has applied prepare_player. This consumes every overlay setting
// in native order, then skeleton/radar and renderer restoration. Snapshot
// fields above are decoded netvars/cache data, not substituted decisions.
void draw_prepared_player(ui::RenderQueue&, const PlayerEspConfig&, const WorldEspConfig&,
                          const PreparedPlayer&, PlayerDrawState& target_state,
                          const PlayerDrawState& view_state, const PlayerDrawSnapshotContext&);

} // namespace skeet::esp

// Declarations from player_engine.hpp

namespace skeet::esp {
// Borrow the actual pointer cells: engine callbacks may replace their values.
struct PlayerEngineBindings {
    void* const& entity_list; //4346A890
    void* const& engine_client; //4346A514
    void* const& local_player; //43475808
    void** const& player_resource; //43468FF0
    void** const& game_rules; //4346A3A4
    engine::NativeConVarReference game_type; //43467A8C^AC9299CC
    engine::NativeConVarReference game_mode; //4346A694^D9328889, distinct from game_type
    engine::NativeConVarReference teammates_are_enemies; //4346A568^9190E2E6
    const std::int32_t* const& bone_counter; //43469060
    const float* const& current_time; //43467EB8
    const void* const& engine_globals; //43467B04, absframe+8/frame+14/tick+20
    const float* const& simulation_tick_interval; //4346A338
    const Vector3& view_origin; //43479CCC/CD0/CD4
    const float& view_pitch; //43479CD8
    const float& view_yaw; //43479CDC
    const float* const& animation_time; //4346A334
    const std::uint8_t& suspended; //43468E58
    const std::uint8_t& rage_enabled; //43467224
    const std::int32_t& on_shot_target; //4346DB1C
    const std::uint8_t& on_shot_active; //4347CA30
    const PlayerSequenceActivity& sequence_activity; //4346A364, optional genuine engine entry
    const std::int32_t* const& maximum_players; //43469300
    const engine::NativeOffsetStorage& offsets;
};

// Identity views contain no decoded player snapshots. Every field below is
// read from the live entity at its native use site. The only shared player
// history/fade/definition storage remains client::PlayerStateCache.
class EnginePlayers {
public:
    explicit EnginePlayers(PlayerEngineBindings);
    EnginePlayers(const EnginePlayers&) = delete;
    EnginePlayers& operator=(const EnginePlayers&) = delete;
    RuntimePlayer* local();
    RuntimePlayer* identity(void* entity) {
        return view(entity);
    }
    RuntimePlayer* by_index(std::int32_t);
    RuntimePlayer* by_handle(std::int32_t);
    RuntimeWeapon* weapon(std::int32_t);
    bool is_player(const RuntimePlayer&) const;
    bool is_weapon(const RuntimePlayer&) const;
    std::uint16_t item_definition(const RuntimePlayer&) const;
    std::int32_t owner_handle(const RuntimePlayer&) const;
    std::int32_t local_index() const;
    bool dormant(const RuntimePlayer&) const;
    std::int32_t index(const RuntimePlayer&) const;
    Vector3 origin(const RuntimePlayer&) const;
    Vector3 minimum(const RuntimePlayer&) const;
    Vector3 maximum(const RuntimePlayer&) const;
    float time() const;
    std::uint8_t life(const RuntimePlayer&) const;
    std::uint8_t movement(const RuntimePlayer&) const;
    std::int32_t observer_mode(const RuntimePlayer&) const;
    std::int32_t observer_target(const RuntimePlayer&) const;
    std::int32_t team(const RuntimePlayer&) const;
    std::int32_t squad(const RuntimePlayer&) const;
    std::uint8_t eligibility(const RuntimePlayer&) const;
    std::uint8_t suppressed(const RuntimePlayer&) const;
    float pose(const RuntimePlayer&, unsigned) const;
    Vector3 network_origin(const RuntimePlayer&) const;
    Vector3 view_offset(const RuntimePlayer&) const;
    std::int32_t armor(const RuntimePlayer&) const;
    bool helmet(const RuntimePlayer&) const;
    bool scoped(const RuntimePlayer&) const;
    float flash(const RuntimePlayer&) const;
    bool vip(const RuntimePlayer&) const;
    bool defusing(const RuntimePlayer&) const;
    float duck_amount(const RuntimePlayer&) const;
    float duck_speed(const RuntimePlayer&) const;
    float simulation_time(const RuntimePlayer&) const;
    std::int32_t active_weapon(const RuntimePlayer&) const;
    std::int32_t inventory_weapon(const RuntimePlayer&, unsigned) const;
    std::int32_t money(const RuntimePlayer&) const;
    std::int32_t health(const RuntimePlayer&) const;
    std::uint16_t definition(const RuntimeWeapon&) const;
    std::int32_t ammunition(const RuntimeWeapon&) const;
    bool pin_pulled(const RuntimeWeapon&) const;
    float reload_cycle(const RuntimePlayer&) const;
    bool is_weapon(const RuntimeWeapon&) const;
    WeaponInformation information(const RuntimeWeapon&) const;
    const void* information_pointer(const RuntimeWeapon&) const;
    float armor_ratio(const RuntimeWeapon&) const;
    std::int32_t damage(const RuntimeWeapon&) const;
    std::int32_t capacity(const RuntimeWeapon&) const;
    std::int32_t kind(const RuntimeWeapon&) const;
    bool player_info(std::int32_t, std::string_view&);
    bool player_info(std::int32_t, hud::ObserverPlayerInfo&);
    std::int32_t game_type() const;
    std::int32_t game_mode() const;
    bool teammates_are_enemies() const;
    void* resource() const;
    bool connected(const void*, std::int32_t) const;
    bool alive(const void*, std::int32_t) const;
    std::int32_t resource_team(const void*, std::int32_t) const;
    std::int32_t resource_health(const void*, std::int32_t) const;
    bool retain_dormant_alpha() const;
    float rule_time() const; //433FC87C
    std::int32_t bomb_carrier() const;
    bool rage_enabled() const noexcept {
        return bindings_.rage_enabled != 0;
    }
    std::int32_t on_shot_target() const noexcept {
        return bindings_.on_shot_target;
    }
    bool on_shot_active() const noexcept {
        return bindings_.on_shot_active != 0;
    }
    float tick_interval() const noexcept;
    float frame_time() const noexcept;
    float absolute_frame_time() const noexcept;
    std::int32_t maximum_players() const noexcept {
        return *bindings_.maximum_players;
    }
    float simulation_tick_interval() const noexcept {
        return *bindings_.simulation_tick_interval;
    }
    const Vector3& view_origin() const noexcept {
        return bindings_.view_origin;
    }
    float view_pitch() const noexcept {
        return bindings_.view_pitch;
    }
    float view_yaw() const noexcept {
        return bindings_.view_yaw;
    }
    float animation_time() const noexcept {
        return *bindings_.animation_time;
    }
    bool suspended() const noexcept {
        return bindings_.suspended != 0;
    }
    void set_spotted(const RuntimePlayer&, SpottedProxy) const;
    void set_spotted(const RuntimePlayer&) const;
    bool spotted(const RuntimePlayer&) const;
    void skeleton(ui::RenderQueue&, const Projection&, const RuntimePlayer&, std::uint32_t) const;
    // SDK query adapter for existing event callbacks; decoded fields remain
    // accessible only through this live owner, never through identity views.
    PlayerRuntimeEngine& runtime_engine();

private:
    std::ptrdiff_t offset(std::uint32_t pointer, std::uint32_t key, std::uint32_t mask) const;
    RuntimePlayer* view(void*);
    PlayerEngineBindings bindings_;
    std::unordered_map<void*, RuntimePlayer> players_;
    std::unordered_map<void*, RuntimeWeapon> weapons_;
    std::array<std::byte, 0x158> player_info_{};
    PlayerRuntimeEngine runtime_{};
};

// Complete43328CA6 using live engine reads, shared by ESP and incoming-fire probes.
PlayerWeaponMetadata player_weapon_metadata(const RuntimePlayer&, EnginePlayers&);

RuntimePlayer* effective_view_player(EnginePlayers&);
bool live_enemy(const RuntimePlayer*, const RuntimePlayer*, EnginePlayers&);
bool dormant_enemy(const RuntimePlayer*, const RuntimePlayer*, EnginePlayers&);
bool dormant_alive(const RuntimePlayer*, EnginePlayers&);
void update_player_geometry(PlayerGeometry&, const RuntimePlayer&, EnginePlayers&, Vector3);
PreparedPlayer prepare_live_player(ui::RenderQueue&, PlayerRuntimeState&, const PlayerEspConfig&,
                                   const WorldEspConfig&, EnginePlayers&, RuntimePlayer*,
                                   std::int32_t, bool, bool);
std::uint32_t player_overlay_flags(const RuntimePlayer&, const PlayerDrawContext&, EnginePlayers&);
void draw_live_prepared_player(ui::RenderQueue&, const PlayerEspConfig&, const WorldEspConfig&,
                               const PreparedPlayer&, const PlayerDrawContext&, EnginePlayers&);
void draw_live_observer(ui::RenderQueue&, PlayerFrameState&, EnginePlayers&, RuntimePlayer*,
                        const PreparedPlayer&);
} //namespace skeet::esp

// Declarations from player_geometry.hpp

namespace skeet::esp {
// Native per-player segment at record +0x1E8. Used by the shared player cache,
// preparation/fade owner and projected player overlay.
struct PlayerGeometry {
    std::int32_t last_live_tick = 0;
    float alpha = 0;
    Vector3 lower{}, center{}, origin{}, upper{};
    float pose_adjustment = 0, eye_height = 0;
};
static_assert(sizeof(PlayerGeometry) == 64);
} // namespace skeet::esp

// Declarations from player_overlay.hpp

namespace skeet::esp {

// 0x4340A644: native player names are terminated after 20 UTF-8 bytes before
// MultiByteToWideChar(CP_UTF8, 0), including its incomplete-sequence behavior.
std::u16string player_name(std::string_view utf8);

void draw_box(ui::RenderQueue& queue, Rectangle box, std::uint32_t color);
void draw_health(ui::RenderQueue& queue, Rectangle box, std::int32_t health,
                 std::int32_t number_threshold);
void draw_health(ui::RenderQueue& queue, Rectangle box, std::int32_t health,
                 std::int32_t number_threshold, std::uint32_t color);
void draw_name(ui::RenderQueue& queue, Rectangle box, std::u16string_view name, std::uint32_t color,
               std::int32_t normal_font_height);
// Requires a valid weapon, ammunition != -1, and positive capacity, matching
// the native gate. Returns the new baseline for labels below the player.
std::int32_t draw_ammo(ui::RenderQueue& queue, Rectangle box, std::int32_t baseline,
                       std::int32_t ammunition, std::int32_t capacity, float reload_progress,
                       std::uint32_t color);
std::int32_t distance_in_feet(float dx, float dy, float dz) noexcept;
void draw_distance(ui::RenderQueue& queue, std::int32_t center_x, std::int32_t baseline, float dx,
                   float dy, float dz);
struct OverlayInput {
    Rectangle box;
    std::int32_t health, health_number_threshold;
    bool player_info_available;
    std::string_view name_utf8;
    bool dormant;
    bool money_enabled;
    std::int32_t money;
    std::uint32_t flags;
    bool weapon_valid;
    std::int32_t ammunition, capacity;
    float reload_progress;
    float view_delta_x, view_delta_y, view_delta_z;
    std::int32_t entity_index;
};
struct OverlayCallbacks {
    void* context = nullptr;
    // Native 0x4336918D extension boundary, called after the 18 built-in
    // entries even when their mask is zero. The cursor is mutable.
    void (*extra_flags)(void*, ui::RenderQueue&, std::int32_t, Point&, std::int32_t) = nullptr;
};
struct OverlayLayout {
    Point side;
    std::int32_t baseline;
};
// Composes the recovered box/name/health/money/flags/ammo/distance blocks in
// original order. Input is a successfully projected, selected player. Weapon
// text/icon, skeleton, glow and offscreen gates belong to the enclosing owner.
OverlayLayout draw_player_overlay(ui::RenderQueue& queue, const OverlayConfig& config,
                                  const OverlayInput& input,
                                  std::span<const FlagEntry, 18> flag_table,
                                  OverlayCallbacks callbacks = {});

} // namespace skeet::esp

// Declarations from player_runtime.hpp

namespace skeet::esp {
struct PlayerDrawState;

struct RuntimePlayer {
    void* engine_entity = nullptr;
    std::uint8_t life_state = 0, move_type = 0;
    std::int32_t observer_mode = 0, observer_handle = -1;
    std::int32_t team = 0, squad = -1;
    // Raw bytes from decoders D070684C and CDE290D2 respectively.
    std::uint8_t eligibility_state = 0, suppressed = 0;
    float pose_parameter_1 = 0, pose_parameter_12 = 0;
    // Concrete decoded netvars/cache bound to this engine entity. Required by
    // the complete frame renderer; preparation alone does not consume it.
    PlayerDrawState* draw = nullptr;
};
struct RuntimeRulePlayer {
    bool connected = false, alive = false;
    std::int32_t team = 0;
};
struct PlayerRuntimeRules {
    std::int32_t game_mode = 0;
    bool teammates_are_enemies = false;
    bool present = false;
    // The scalar flag read by 0x433DF22C; bypasses dormant alpha decay.
    bool retain_dormant_alpha = false;
    std::span<const RuntimeRulePlayer> players;
};
struct PlayerRuntimeEngine {
    void* context = nullptr;
    RuntimePlayer* local_player = nullptr;
    RuntimePlayer* (*by_index)(void*, std::int32_t) = nullptr;
    RuntimePlayer* (*by_handle)(void*, std::int32_t) = nullptr;
    bool (*is_player)(void*, const RuntimePlayer&) = nullptr; // entity slot 158
    std::int32_t (*index)(void*, const RuntimePlayer&) = nullptr; // network slot 10
    bool (*dormant)(void*, const RuntimePlayer&) = nullptr; // network slot 9
    Vector3 (*absolute_origin)(void*, const RuntimePlayer&) = nullptr; // entity slot 10
    Vector3 (*minimum_bounds)(void*, const RuntimePlayer&) = nullptr; // entity slot 76
    Vector3 (*maximum_bounds)(void*, const RuntimePlayer&) = nullptr; // entity slot 77
    float (*time)(void*) = nullptr; // engine slot 14
};
struct PlayerRuntimeState {
    // Native 0x433B8A2B chooses records with entity_index & 63. This is the
    // geometry/fade segment at each record +0x1E8, not its other subsystems.
    client::PlayerStateCache& cache;
    explicit PlayerRuntimeState(client::PlayerStateCache& shared) : cache(shared) {}
    PlayerGeometry& geometry(std::uint32_t index) {
        return cache.geometry(index);
    }
};
struct PlayerRuntimeTiming {
    float frame_time = 0, tick_interval = 0;
};
enum class PreparedPlayerStage : std::uint32_t {
    skipped,
    overlay,
    observer_panel
};
struct PreparedPlayer {
    PreparedPlayerStage stage = PreparedPlayerStage::skipped;
    RuntimePlayer* player = nullptr;
    RuntimePlayer* view_player = nullptr;
    PlayerGeometry* geometry = nullptr;
    std::int32_t entity_index = 0;
    bool dormant = false, teammate = false;
    client::PlayerState* cache_state = nullptr;
};

// Complete helpers 43359AFA, 4339EF8C, 433E4E4E and 433D7563.
RuntimePlayer* effective_view_player(const PlayerRuntimeEngine& engine);
bool live_enemy(const RuntimePlayer* first, const RuntimePlayer* second,
                const PlayerRuntimeRules& rules) noexcept;
bool dormant_enemy(const RuntimePlayer* first, const RuntimePlayer* second,
                   const PlayerRuntimeEngine& engine, const PlayerRuntimeRules& rules);
bool dormant_alive(const RuntimePlayer* player, const PlayerRuntimeEngine& engine,
                   const PlayerRuntimeRules& rules);
// 4341AF4A, including the repeated maximum-bounds engine query.
void update_player_geometry(PlayerGeometry& geometry, const RuntimePlayer& player,
                            const PlayerRuntimeEngine& engine, Vector3 origin);

// Per-player owner chain 4340983F..43409D3D with rejection exits AE82..AE9C.
// A returned overlay stage hands the selected entity and modified renderer
// state to the existing offscreen/projection/overlay blocks. Network index is
// nonzero for an accepted player, matching the native engine contract.
PreparedPlayer prepare_player(ui::RenderQueue& queue, PlayerRuntimeState& state,
                              const PlayerEspConfig& config, const WorldEspConfig& world,
                              const PlayerRuntimeEngine& engine, const PlayerRuntimeRules& rules,
                              PlayerRuntimeTiming timing, RuntimePlayer* initial_view_player,
                              std::int32_t slot, bool activation, bool blocked);
// Common post-draw renderer restoration 4340AE66..4340AE80.
void finish_player_overlay(ui::RenderQueue& queue);

// The source frame and glow/sound callbacks share4346A65C. Authored standalone
// helper states keep their explicit local value; production binds its byte.
class PlayerFrameActivation {
public:
    PlayerFrameActivation(bool value = false) noexcept : local_(value) {}
    explicit PlayerFrameActivation(std::uint8_t& cell) noexcept : native_(&cell) {}
    operator bool() const noexcept {
        return native_ ? *native_ != 0 : local_;
    }
    PlayerFrameActivation& operator=(bool value) noexcept {
        if (native_)
            *native_ = static_cast<std::uint8_t>(value);
        else
            local_ = value;
        return *this;
    }

private:
    bool local_ = false;
    std::uint8_t* native_ = nullptr;
};
struct PlayerFrameState {
    std::int32_t transient = 0; // owner +0x6A0
    float timer = 0; // owner +0x6A4
    PlayerFrameActivation activation; //global4346A65C in source production
};
// 43408EE3..43408F34, before the optional timed bomb-site indicator.
void begin_player_frame(PlayerFrameState& state, const HotkeyBinding& binding, InputBitmap input,
                        float absolute_frame_time);
using RuntimeBombSites = std::array<Vector3, 2>;
// Complete 433AC842: site nearest the entity's bounds midpoint, default A.
char runtime_bomb_site(const PlayerRuntimeEngine& engine, std::int32_t entity_index,
                       const RuntimeBombSites* sites);
// Remaining frame prefix 43408F34..43408FB6, including its three-second ring.
void draw_player_frame_notice(ui::RenderQueue& queue, const PlayerFrameState& state,
                              const WorldEspConfig& world, const PlayerRuntimeEngine& engine,
                              std::int32_t entity_index, const RuntimeBombSites* sites,
                              std::int32_t& indicator_cursor, const IndicatorLayout& layout,
                              std::int32_t texture);

} // namespace skeet::esp

// Declarations from projection.hpp

namespace skeet::esp {

struct Vector3 {
    float x, y, z;
};
struct Projection {
    // Native global matrix 0x434798E0. Rows 0, 1 and 3 are used.
    std::array<float, 16> matrix;
    float horizontal_scale, vertical_scale;
};
bool project_point(const Projection& projection, Vector3 position, Point& screen) noexcept;
// Draw-state endpoints +0x214 and +0x1F0. Failure leaves output unchanged.
bool player_box(const Projection& projection, Vector3 first, Vector3 second,
                Rectangle& output) noexcept;
float convert_field_of_view(float degrees, float aspect_scale) noexcept;
float angle_to_target(Vector3 origin, Vector3 target, Vector3 axis) noexcept;

} // namespace skeet::esp

// Declarations from runtime_bindings.hpp

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::esp {
// Canonical live fields for the EnginePlayers constructed inside the complete
// ESP owner. This supplies no identity cache, decoded entity snapshot or second
// ESP state. Encoded ConVar words retain their actual native XOR view.
PlayerEngineBindings native_player_engine_bindings(client::ClientRuntimeStorage&,
                                                   const client::EngineRuntimeBindings&);
// Shares the actual trace adapter and nullable debug/temp-entity interfaces.
// Sound state and SoundOverlayContext still belong to EspOwnerRuntime itself.
SoundOverlayBindings native_sound_overlay_bindings(client::ClientRuntimeStorage&,
                                                   const client::EngineRuntimeBindings&,
                                                   engine::SceneQueryRuntimeOwner&);
} // namespace skeet::esp

// Declarations from runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
class VisualRuntimeOwner;
class CrtRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::ui {
class RendererAssets;
}
namespace skeet::esp {
struct EspOwnerBindings;
struct EspFrameOwner;
class EspOwnerRuntime;
// Production lifetime for all contexts borrowed by the complete43481EAB ESP
// factory. That factory creates the sole EnginePlayers and its sound/frame/
// sight endpoints. This context creates no feature/configuration registration,
// calls no game factory and installs no hook during construction.
class EspRuntimeOwner {
public:
    EspRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                    client::ConfigurationRuntimeOwner&, client::PlayerRuntimeOwner&,
                    engine::SceneQueryRuntimeOwner&, client::VisualRuntimeOwner&,
                    client::CrtRuntimeOwner&, script::SourceScriptRuntimeOwner&,
                    ui::RendererAssets&, engine::BuildEspBindings);
    ~EspRuntimeOwner();
    EspRuntimeOwner(const EspRuntimeOwner&) = delete;
    EspRuntimeOwner& operator=(const EspRuntimeOwner&) = delete;
    EspRuntimeOwner(EspRuntimeOwner&&) = delete;
    EspRuntimeOwner& operator=(EspRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    EspOwnerBindings& factory_bindings() noexcept;
    EspOwnerRuntime*& active() const noexcept; //canonical434758DC
    EspFrameOwner& state() noexcept;
    effects::GlowRuntime& glow() noexcept;
    const rage::ShotgunPattern& shotgun_pattern() const noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} // namespace skeet::esp

// Declarations from skeleton.hpp

namespace skeet::esp {

struct SkeletonBone {
    std::uint32_t flags;
    std::uint32_t parent;
};
// 0x4340ACA4..0x4340AE26, after native model/setup-bones validation.
// Source studio bone flags/parent indices and cached matrix storage are
// supplied by the engine adapter. The native workspace holds 256 bones.
void draw_skeleton(ui::RenderQueue& queue, const Projection& projection,
                   std::span<const SkeletonBone> bones, std::span<const Matrix3x4> transforms,
                   std::uint32_t color);

} // namespace skeet::esp

// Declarations from sound_overlay.hpp

namespace skeet::esp {

struct ActiveSound {
    Vector3 origin;
    std::array<std::byte, 0x2c> unused_0c;
    std::int32_t source_index; // native sound +38
    std::array<std::byte, 0x20> unused_3c;
};
static_assert(sizeof(ActiveSound) == 0x5c);
struct ActiveSoundNode {
    std::int32_t left, right, parent;
    std::uint32_t unused_0c;
    ActiveSound value;
};
static_assert(sizeof(ActiveSoundNode) == 0x6c);
// Actual 43476420 tree header. The engine owns its nodes and index bounds.
struct ActiveSoundTree {
    std::uint32_t unused_00;
    const ActiveSoundNode* nodes;
    std::uint32_t allocation_count, unused_0c;
    std::int32_t root;
    std::uint32_t count, unused_18;
    std::int32_t last_allocated;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(ActiveSoundTree, nodes) == 4);
static_assert(offsetof(ActiveSoundTree, root) == 0x10);
static_assert(offsetof(ActiveSoundTree, last_allocated) == 0x1c);
#endif

struct SoundOverlayBindings {
    MovementCollisionEngine& collision;
    std::uint8_t* const& trace_flag; // optional *434683D0, nested guards restore it
    void* const& debug_overlay; // optional 43468260
    void* const& temp_entities; // optional 434679EC, BeamRingPoint virtual9
};
struct SoundOverlayInputs {
    const PlayerEspConfig& config;
    const SoundOverlayBindings& bindings;
    const skeet::players::State& player_adjustments; // live4347BE09 + index*32
    const PlayerFrameState& frame; // canonical activation, global4346A65C
};
struct SoundOverlayContext : SoundOverlayInputs {
    SoundOverlayContext(SoundOverlayInputs inputs, EnginePlayers& engine_players,
                        PlayerRuntimeState& player_state, const PickupObservations& observations)
        : SoundOverlayInputs(inputs), players(engine_players), state(player_state),
          pickups(observations) {}
    EnginePlayers& players;
    PlayerRuntimeState& state; // same physical canonical player records
    const PickupObservations& pickups;
};
struct DormantRadarOffsets {
    std::ptrdiff_t zero_based_index; // B6F85C08, int32
    std::ptrdiff_t origin; // 2A671B42, Vector3
};

class SoundOverlay {
public:
    std::int32_t ring_model = -1; // native ESP owner +5C
    std::array<std::uint8_t, 3> listener_enabled{}; // +EEC/+EFC/+F0C

    // Sound-model tail of 433E2C67, including genuine network string tables.
    void initialize_model(void* network_string_tables);
    void initialize_model_table(void* actual_model_precache_table);
    // Complete 43330FEE. All source/owner resolution, admission, ground trace,
    // dormant geometry, ring rate limiting and native rendering stay here.
    void consume(RuntimePlayer* view, const ActiveSound&, const SoundOverlayContext&);
    // 4337F9F6 prefix -> complete 4336B697. First listener follows dormant;
    // the second and third follow the bomb bar. Their native order is kept.
    void synchronize_listeners(bool refresh, const PlayerEspConfig&, const WorldEspConfig&,
                               std::array<events::EventListener, 3>, events::EventRegistry&);
};

// Complete 433F0AEB: source registry traversal in native in-order sequence.
// Borrowing the pointer cell preserves the native reload after each consumer.
void process_sound_overlay(RuntimePlayer* view, SoundOverlay&, const ActiveSoundTree* const&,
                           const SoundOverlayContext&);
// Complete 433E0F98, borrowing the same pickup queue as the event producers.
void expire_pickup_observations(PickupObservations&, std::int32_t engine_frame,
                                float tick_interval) noexcept;

// The real four-slot secondary interface at owner+10 / vtable43472670.
// It registers in43475BA8 and consumes the shared queue at incoming slot3.
// Its complete ESP owner publishes the table after all seven registrations.
class SoundHistoryReceiver : public client::NativeEntityObserver {
public:
    SoundHistoryReceiver(client::EntityObserverRegistry&, PickupObservations&,
                         const void* const& engine_globals, const float* const& interval);
    void process_incoming(); //complete433E0F98; native player/delta are unused
private:
    PickupObservations& observations_;
    const void* const& globals_;
    const float* const& interval_;
};
// Dormant radar-position branch 433FBE76..433FC15D. The complete packet
// endpoint composes this prefix with its following bullet-tracer branch.
void update_dormant_radar(const void* packet, DormantRadarOffsets, const SoundOverlayContext&);

} // namespace skeet::esp

// Declarations from tablet_upgrade.hpp

namespace skeet::esp {
struct TabletUpgradeState {
    std::int32_t frame = 0; //constructor+718 is calloc0; lifecycle reset writes-1
    std::uint8_t status = 0; //saved constructor+E90
    std::array<std::uint32_t, 3> value_bits{}; //+E94
    std::array<std::int32_t, 8> identifiers{}; //+EA0
    std::array<std::uint32_t, 8> time_bits{}; //+EC0
};
struct TabletUpgradeOffsets {
    std::ptrdiff_t active_weapon; //CD812654, player handle
    std::ptrdiff_t definition; //5C9F0FBB, uint16
    std::ptrdiff_t status; //581805BB, byte
    std::ptrdiff_t values; //D6E91C78, three float words
    std::ptrdiff_t identifiers; //68484E0D, eight int32 identifiers
    std::ptrdiff_t times; //54E478AA, eight float words
};
struct TabletUpgradeBindings {
    const WorldEspConfig& settings; //OtherESP553D3C3F, distinct from MISC
    void* const& local_player; //43475808
    void* const& entity_list; //4346A890
    std::int32_t* const& frame_count; //434678F8
    float* const& current_time; //43467EB8
    TabletUpgradeOffsets offsets;
};
// Complete433E1983 and4331E0CC. Only tablet definition72 is admitted. All
// modified fields are saved before mutation and restored in the same frame.
void prepare_tablet_upgrade(TabletUpgradeState&, const TabletUpgradeBindings&);
void restore_tablet_upgrade(TabletUpgradeState&, const TabletUpgradeBindings&);

// Native43472170 at constructor+18 combines tablet pre/post with glow phase3.
// Borrow the same state held by EspFrameOwner and the canonical GlowRuntime.
class PlayerEspUpdateHandler final : public network::UpdateObserver {
public:
    PlayerEspUpdateHandler(engine::OwnerRegistryBinding, TabletUpgradeState&,
                           const TabletUpgradeBindings&, effects::GlowRuntime&);

private:
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_ESP_MEMBER __thiscall
#define SKEET_ESP_FAST __fastcall
#else
#define SKEET_ESP_MEMBER
#define SKEET_ESP_FAST
#endif
    static void SKEET_ESP_MEMBER prepare(void*);
    static void SKEET_ESP_MEMBER restore(void*);
    static void SKEET_ESP_MEMBER glow(void*);
    static void SKEET_ESP_MEMBER unchanged(void*);
    static void SKEET_ESP_FAST unchanged_player(void*, void*, std::uintptr_t, std::uintptr_t,
                                                std::uintptr_t);
    static void SKEET_ESP_FAST unchanged_view(void*, float*, float*, float*);
    static void SKEET_ESP_FAST unchanged_post(void*, void*);
    inline static constexpr network::UpdateObserverVtable entries_{
        {prepare, restore, unchanged, glow, unchanged},
        unchanged_player,
        unchanged_view,
        unchanged_post};
#undef SKEET_ESP_MEMBER
#undef SKEET_ESP_FAST
    TabletUpgradeState& state_;
    const TabletUpgradeBindings& bindings_;
    effects::GlowRuntime& glow_;
};
} // namespace skeet::esp

// Declarations from user_message_handlers.hpp

namespace skeet::engine {
class PreservedInitialization;
class NativeDataImage;
}
namespace skeet::esp {
class EspOwnerRuntime;
struct UserMessageBindings {
    EspOwnerRuntime* const& active_owner; //434758DC, captured once by each entry
    const std::uint8_t& recording_enabled; //4346754C, reloaded at its native gates
    const players::State& adjustments; //correction_active byte4347BE0E + index*32
};

// The SDK has already decoded these messages. The incoming pointers retain
// their original x86 generated-message layout; no message IDs are inferred or
// parsed here. State belongs to the active ordinary ESP/player-cache owner.
class UserMessageHandlers {
public:
    UserMessageHandlers(engine::PreservedInitialization&, UserMessageBindings);
    UserMessageHandlers(std::shared_ptr<engine::NativeDataImage>, UserMessageBindings);
    UserMessageHandlers(const UserMessageHandlers&) = delete;
    UserMessageHandlers& operator=(const UserMessageHandlers&) = delete;
    void weapon_sound(const void* packet); //433E7234
    void spotted(const void* packet); //433FCBD6
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }

private:
    void spotted_entry(EspOwnerRuntime&, const void* entry);
    std::shared_ptr<engine::NativeDataImage> image_;
    UserMessageBindings bindings_;
};

// The ordinary-entry bridge binds this process-lifetime context before it
// installs either packet entry and keeps it alive until those entries retire.
void bind_user_message_handlers(UserMessageHandlers&) noexcept;
void unbind_user_message_handlers(UserMessageHandlers&) noexcept;
// The observed entries use plain RET and one stack argument. The preserved
// registration dispatcher's use of a return register is a separate ABI input.
extern "C" void __cdecl weapon_sound_user_message_hook(const void* packet);
extern "C" void __cdecl spotted_user_message_hook(const void* packet);
} //namespace skeet::esp

// Declarations from weapon_icon.hpp

namespace skeet::esp {

struct WeaponIcon {
    std::int32_t texture = -1, width = 0, height = 12;
};
struct WeaponIconProvider {
    void* context;
    // Native cache boundary 0x433591C2; normalized definition, actual
    // definition, requested raster height, output texture and dimensions.
    bool (*load)(void*, ui::RenderQueue&, std::uint16_t, std::uint16_t, std::int32_t, WeaponIcon&);
};
void draw_weapon_icon(ui::RenderQueue& queue, WeaponIconProvider provider, std::uint16_t definition,
                      Point position, std::uint32_t color, std::int32_t* width = nullptr);

} // namespace skeet::esp

// Declarations from weapon_text.hpp

namespace skeet::esp {

struct WeaponTextProvider {
    void* context;
    // Engine item-schema lookup (factory -> schema -> definition -> name).
    // A missing interface/definition/name is represented by a null result.
    const char* (*schema_name)(void*, std::int32_t definition);
    // Native localization boundary 0x43397087. Null selects the native
    // schema-token path, which removes its 13-byte #SFUI_WPNHUD_ prefix.
    const char16_t* (*localize)(void*, const char* token);
};
struct WeaponTextBindings {
#if defined(_M_IX86) || defined(__i386__)
    using ItemSystemFactory = void*(__cdecl*)();
#else
    using ItemSystemFactory = void* (*)();
#endif
    void*& item_system; //canonical43476424, shared with post-initialization/icons
    ItemSystemFactory const& item_system_factory; //43468B9C
    void* const& localization; //4346A31C, actual Localize_001
};
// Concrete engine adapter: item-system slot0 -> schema slot4 -> item slot2;
// localization slot11 is the native 0x43397087 lookup through 0x433E533F.
WeaponTextProvider weapon_text_provider(WeaponTextBindings& bindings) noexcept;
// Native 0x433F25A1. Early lookup failure leaves measured_size unchanged.
// Schema labels contain fewer than 64 characters after prefix
// removal/localization; this function rejects violations of that contract.
void draw_weapon_text(ui::RenderQueue& queue, WeaponTextProvider provider, std::int32_t definition,
                      Point position, std::uint32_t color, Point* measured_size = nullptr);

} // namespace skeet::esp

// Declarations from world_engine.hpp

namespace skeet::esp {
using WorldDecodedOffset = engine::DecodedOffset;
struct WorldEngineOffsets {
    explicit WorldEngineOffsets(const engine::NativeOffsetGlobals&);
    // Each conversion decodes the current pointer/key words at the actual
    // property use site; offset initialization is the canonical engine owner.
    WorldDecodedOffset origin; //2A671B42, live network origin
    WorldDecodedOffset model_index; //47877DEB,int16
    WorldDecodedOffset owner_handle; //8D1BD8D0
    WorldDecodedOffset definition; //5C9F0FBB,uint16
    WorldDecodedOffset ammunition; //CA5EC698,int32
    WorldDecodedOffset reserve_ammunition; //49C770CA,int32
    WorldDecodedOffset team; //8B5586F6
    WorldDecodedOffset hostage_effects; //BB78DC43,byte
    WorldDecodedOffset hostage_life_state; //CB6C59C2,int32
    WorldDecodedOffset studio_wrapper; //32E4DAF1,pointer
    WorldDecodedOffset hitbox_set; //C2B6FB10,int32
    WorldDecodedOffset rules_allow_bomb; //82F5AD08,byte
    WorldDecodedOffset dropped_bomb_state; //71A5AC08,int32
    WorldDecodedOffset grenade_damage; //727317C4,float
    WorldDecodedOffset grenade_radius; //9D63BB7F,float
    WorldDecodedOffset explosion_effect_tick; //01E80986,int32
    WorldDecodedOffset glow_index; //442A38AE,int32
    WorldDecodedOffset smoke_active; //58AE2423,byte
    WorldDecodedOffset smoke_start_tick; //F267E956,int32
    WorldDecodedOffset inferno_start_tick; //0DCBA89C,int32
    WorldDecodedOffset bomb_ticking; //0E5BECE5,byte
    WorldDecodedOffset bomb_detonation_time; //D280D4E1,float
    WorldDecodedOffset bomb_defuse_end; //800BB0B1,float
    WorldDecodedOffset bomb_defuser_state; //950F7B20,int32
    WorldDecodedOffset bomb_origin; //A9EE6062, distinct from network origin2A671B42
    WorldDecodedOffset bomb_site_first, bomb_site_second; //FCCAF99B/E92192CA,player resource
    WorldDecodedOffset view_offset; //C246315E,Vector3
    WorldDecodedOffset armor; //47A6F50C,int32
    WorldDecodedOffset life_state; //9D920E35,byte
};
struct WorldEngineBindings {
    engine::SceneQueries& queries;
    void** const& game_rules; //4346A3A4, optional pointer-to-rules
    void* const& planted_bombs; //434688C0, data+0/count+C
    engine::NativeConVarReference friendly_fire; //43467C3C^211963A6 ConVar
    engine::NativeConVarReference game_type; //43467A8C^AC9299CC ConVar
    const std::int32_t& maximum_players; //*43469300
    const float& current_time; //*43467EB8
    const std::int32_t& bomb_label_texture; //43467DA8,reloaded after label formatting
    const std::int32_t& current_tick; //43475A60 pointee+4
    const float& tick_interval; //*4346A338
    const Vector3& view_origin; //43479CCC..43479CD4, read at each range use
    const std::uint8_t& suspended; //43468E58
    WorldEngineOffsets offsets;
};

// This is the actual world side of43408EC9. by_index returns an untouched raw
// entity; only the selected handler reads its properties, at the corresponding
// native use sites. No decoder callbacks or prebuilt FrameWorldEntity are used.
class WorldEntityEngine {
public:
    explicit WorldEntityEngine(WorldEngineBindings bindings) : bindings_(bindings) {}
    void* by_index(std::int32_t) const;
    bool dormant(void*) const;
    std::uint32_t client_class_index(void*) const;
    bool is_weapon(void*) const;
    std::int32_t highest_entity_index() const;
    std::int32_t maximum_players() const noexcept {
        return bindings_.maximum_players;
    }
    std::int32_t game_type() const;
    std::int32_t indicator_texture() const noexcept {
        return bindings_.bomb_label_texture;
    }
    std::int32_t ammunition(const WorldEntity&) const;
    std::int32_t ammunition_capacity(const WorldEntity&) const;
    char bomb_site(std::int32_t entity_index) const; //complete433AC842, live resource
    void dispatch(ui::RenderQueue&, EspFrameOwner&, std::int32_t& indicator_cursor,
                  const EspFrameContext&, RuntimePlayer& view, void*, EntityOverlayHandler);
    void collect_dropped(WorldEntityScan&, std::array<WorldEntity, 65>& frame_entities, void*,
                         std::int32_t index, std::uint32_t captured_options, const EspFrameOwner&,
                         const EspFrameContext&);
    const WorldEngineBindings& bindings() const noexcept {
        return bindings_;
    }

private:
    WorldEngineBindings bindings_;
    void hostage(ui::RenderQueue&, const EspFrameContext&, RuntimePlayer&, void*);
    void dropped_bomb(ui::RenderQueue&, EspFrameOwner&, const EspFrameContext&, void*);
    void planted_bomb(ui::RenderQueue&, EspFrameOwner&, std::int32_t&, const EspFrameContext&,
                      RuntimePlayer&, void*);
    float bomb_damage(EspFrameOwner&, void* target);
    void grenade(ui::RenderQueue&, EspFrameOwner&, const EspFrameContext&, void*);
    void inferno(ui::RenderQueue&, EspFrameOwner&, const EspFrameContext&, RuntimePlayer&, void*);
    void defuse_kit(ui::RenderQueue&, const EspFrameOwner&, const EspFrameContext&, RuntimePlayer&,
                    void*);
    void danger_zone(EspFrameOwner&, RuntimePlayer&, void*);
};
} // namespace skeet::esp

// Declarations from composite_handlers.hpp

namespace skeet::hud {
// Each binding below names a concrete recovered implementation. Referenced
// owners/configuration keep their canonical lifetime; no feature state is
// copied into the callback endpoint. Construct these at the corresponding
//433D20B5 call, in the actual owner-construction order.
template <class Binding> class ConcreteHudHandler : public HudFrameCallback {
public:
    Binding binding;
    explicit ConcreteHudHandler(Binding value) : binding(std::move(value)) {}
    ConcreteHudHandler(engine::OwnerRegistryBinding registry, Binding value)
        : ConcreteHudHandler(std::move(value)) {
        register_hud_frame(registry, this);
        publish();
    }
    void publish() noexcept {
        vtable = table_.data();
    }
    ConcreteHudHandler(const ConcreteHudHandler&) = delete;
    ConcreteHudHandler& operator=(const ConcreteHudHandler&) = delete;

private:
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall entry(void* object, ui::RenderQueue* queue, void* player,
                                 std::int32_t* cursor) {
#else
    static void entry(void* object, ui::RenderQueue* queue, void* player, std::int32_t* cursor) {
#endif
        static_cast<ConcreteHudHandler*>(object)->binding.draw(*queue, player, *cursor);
    }
    inline static const std::array<HudFrameEntry, 1> table_{entry};
};

struct FreestandingHudBinding { //43472654 ->433E8FD3
    script::Bridge& scripts;
    const anti_aim::GlobalConfig& global;
    const anti_aim::Config& config;
    anti_aim::State& state;
    const IntegerConfig& indicators;
    const esp::RuntimePlayer& local;
    const InputBitmap& input;
    const esp::IndicatorLayout& layout;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct PingHudBinding { //434727D0 ->433DF3FC
    script::Bridge& scripts;
    const IntegerConfig& indicators;
    const esp::RuntimePlayer& local;
    const misc::PingSpikeRuntime& ping;
    const esp::IndicatorLayout& layout;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct RageHudBinding { //43472C54 ->4337F589
    script::Bridge& scripts;
    const rage::GlobalSettings& global;
    const rage::AimbotSettings& config;
    const IntegerConfig& indicators;
    const esp::RuntimePlayer& local;
    const InputBitmap& input;
    const esp::IndicatorLayout& layout;
    const esp::AimbotVisualizationConfig& visualization;
    esp::HitboxHighlightState& highlight;
    const esp::HitboxHighlightContext& highlight_context;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct LiveRageHudBinding { //43472C54 ->4337F589, actual incoming local entity
    script::Bridge& scripts;
    const rage::GlobalSettings& global;
    const rage::AimbotSettings& config;
    const IntegerConfig& indicators;
    const InputBitmap& input;
    const esp::IndicatorLayout& layout;
    const esp::AimbotVisualizationConfig& visualization;
    esp::HitboxHighlightState& highlight;
    const esp::HitboxHighlightEngineContext& highlight_context;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct ExploitHudBinding { //43472688 ->433E1B2F
    script::Bridge& scripts;
    network::ExploitState& state;
    float& shared_timer;
    const IntegerConfig& indicators;
    const ExploitIndicatorBindings& interfaces;
    const float& frame_time;
    const esp::IndicatorLayout& layout;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct MovementHudBinding { //43470424 ->43384B72
    script::Bridge& scripts;
    const IntegerConfig& indicators;
    const std::uint8_t& duck_enabled;
    const rage::GlobalSettings& rage;
    const std::uint32_t& command_flags;
    const QuickPeekConfig& quick_peek;
    MovementIndicatorState state;
    const esp::RuntimePlayer& local;
    const InputBitmap& input;
    const esp::IndicatorLayout& layout;
    const esp::Projection& projection;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct CrosshairHudBinding { //4346F878 ->433E4593
    const esp::ScreenOverlayConfig& config;
    const CrosshairBindings& interfaces;
    const std::uint8_t& suspended;
    const Point& screen_size;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct PenetrationHudBinding { //4346DC88 ->4341CC34
    const esp::ScreenOverlayConfig& config;
    const PenetrationReticleState& state;
    const PenetrationReticleBindings& interfaces;
    const Point& screen_size;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct ScreenOverlayHudBinding { //43472720 ->43358A6A
    const esp::ScreenOverlayConfig& config;
    const ScreenOverlayState& state;
    const std::uint32_t& life_state_offset;
    const Point& screen_size;
    const esp::Projection& projection;
    const RageVector& view_origin;
    const RageTrigonometry& trigonometry;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct ScopeHudBinding { //43470428 ->433E273D
    effects::ViewRuntime& state;
    const Point& screen_size;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct WorldHudBinding { //434704B8 ->4336A4E3 ->433B9D00
    effects::WorldRuntime& state;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct HitMarkerHudBinding { //4346F6A8 ->433D066C
    events::HitMarker& state;
    const events::HitMarkerView& view;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct PerformanceHudBinding { //4346F010 ->43325F7B
    script::Bridge& scripts;
    const rage::PerformanceSettings& config;
    const std::uint8_t& low_fps; // same owner full+40, written by its update
    const std::uint8_t* const& rage_enabled; // full+44, published by lifecycle slot1
    const rage::GlobalSettings& rage;
    const InputBitmap& input;
    const esp::IndicatorLayout& layout;
    const std::uint32_t& life_state_offset;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct BlockbotHudBinding { //43472498 ->4331FC7C
    const BlockbotState& state;
    const esp::Projection& projection;
    const std::uint32_t& life_state_offset;
    void draw(ui::RenderQueue&, void*, std::int32_t&) const;
};
struct EmptyHudBinding { //434701CC ->4334A269 is exactly RET8
    void draw(ui::RenderQueue&, void*, std::int32_t&) const noexcept {}
};
using FreestandingHudHandler = ConcreteHudHandler<FreestandingHudBinding>;
using PingHudHandler = ConcreteHudHandler<PingHudBinding>;
using RageHudHandler = ConcreteHudHandler<RageHudBinding>;
using LiveRageHudHandler = ConcreteHudHandler<LiveRageHudBinding>;
using ExploitHudHandler = ConcreteHudHandler<ExploitHudBinding>;
using MovementHudHandler = ConcreteHudHandler<MovementHudBinding>;
using CrosshairHudHandler = ConcreteHudHandler<CrosshairHudBinding>;
using PenetrationHudHandler = ConcreteHudHandler<PenetrationHudBinding>;
using ScreenOverlayHudHandler = ConcreteHudHandler<ScreenOverlayHudBinding>;
using ScopeHudHandler = ConcreteHudHandler<ScopeHudBinding>;
using WorldHudHandler = ConcreteHudHandler<WorldHudBinding>;
using HitMarkerHudHandler = ConcreteHudHandler<HitMarkerHudBinding>;
using PerformanceHudHandler = ConcreteHudHandler<PerformanceHudBinding>;
using BlockbotHudHandler = ConcreteHudHandler<BlockbotHudBinding>;
using EmptyHudHandler = ConcreteHudHandler<EmptyHudBinding>;
} // namespace skeet::hud

// Declarations from composite_runtime.hpp

namespace skeet::ui {
class RendererContainer;
class MenuWindow;
}
namespace skeet::script {
struct Bridge;
}
namespace skeet::hud {
#if defined(_M_IX86) || defined(__i386__)
using HudFrameEntry = void(__fastcall*)(void*, ui::RenderQueue*, void*, std::int32_t*);
#else
using HudFrameEntry = void (*)(void*, ui::RenderQueue*, void*, std::int32_t*);
#endif
// The one-slot secondary interface registered by433D20B5. This is distinct
// from the lifecycle vector43475B78 and the blur vector43475A84.
struct HudFrameCallback {
    const HudFrameEntry* vtable = nullptr;
};

// Complete433D20B5. Call at the native owner's actual construction point;
// neither this function nor dispatch_hud_frame sorts the borrowed entries.
HudFrameCallback* register_hud_frame(engine::OwnerRegistryBinding, HudFrameCallback*);
//433AD5FC..433AD620. Both endpoints are captured once. Every callback receives
// the same resolved local entity and the address of the same indicator cursor.
void dispatch_hud_frame(const engine::OwnerRegistry&, ui::RenderQueue&, void* local_player,
                        std::int32_t& cursor);

struct CompositeFrameContext {
    ui::RendererContainer* const& renderer; // canonical4347690C slot
    const std::int32_t& blocked; //4347C9FC
    const float& scale; //4346D490
    void* engine; //4346A514, virtual26/12
    void* entity_list; //4346A890, virtual3
    const engine::OwnerRegistry& callbacks; //43475A9C/A0/A4
    script::Bridge& scripts;
    ui::MenuWindow* const& windows; //4347691C
    const std::uint32_t& accent;
};
// Complete433AD576..433AD66C. The renderer reserves/publishes its canonical
// command frame. HUD callbacks need connected+local; paint_ui and transitioning
// menu windows still run after an ordinary disconnected/no-local rejection.
bool draw_composite_frame(const CompositeFrameContext&);
} // namespace skeet::hud

// Declarations from crosshair.hpp

namespace skeet::hud {
struct CrosshairOffsets {
    // Resolvers 9D920E35, CACAEECC, CD812654, A1AF8FFC respectively.
    std::uint32_t life_state, scoped, active_weapon, weapon_draw_flag;
};
struct CrosshairBindings {
    void* entity_list = nullptr;
    // Native owner +4/+8. Parent-linked ConVars with the address-xored
    // integer at +30; the second pointer is optional.
    const void* style_variable = nullptr;
    const void* weapon_mode_variable = nullptr;
    CrosshairOffsets offsets;
};

// Complete 433E4593. Uses entity-list slot 4, weapon slot 461 for weapon
// data, and crosshair drawing at weapon slot 404.
// The engine draw observes type=3 and flag=0, then both fields are restored.
void draw_crosshair(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                    const CrosshairBindings& bindings, void* local_player, bool suspended,
                    Point screen_size);
struct NativeCrosshairBindings {
    void* const& entity_list;
    const void* const& style_variable; //actual complete owner+C
    const void* const& weapon_mode_variable; //actual complete owner+10
    const engine::NativeOffsetStorage& offsets;
};
// The player294 callback consumes no drawing ConVars. Its live entity-list
// view is separate from the complete draw owner's style/mode bindings.
struct CrosshairViewBindings {
    void* const& entity_list;
    CrosshairOffsets offsets;
};
// The actual owner path uses the same draw body, with offset/global reads at
// consumption. Suspended rendering queries this supplied queue's own screen.
void draw_live_crosshair(ui::RenderQueue&, const esp::ScreenOverlayConfig&,
                         const NativeCrosshairBindings&, void* local_player,
                         const std::uint8_t& suspended);

// Complete valid-receiver 433B34C0. The native hook hashes six bytes at its
// return address before setting view_setup +E0/+E1. The original virtual slot
// 294 still runs after every ordinary rejection and after successful mutation.
// original_vtable is the saved table in the native hook record at vtable[-2].
void prepare_crosshair_view(const esp::ScreenOverlayConfig& config,
                            const CrosshairBindings& bindings, void* local_player, void* view_setup,
                            bool suspended, const std::byte* return_address,
                            void** original_vtable);
void prepare_crosshair_view(const esp::ScreenOverlayConfig&, const CrosshairViewBindings&,
                            void* local_player, void* view_setup, bool suspended,
                            const std::byte* return_address, void** original_vtable);
} // namespace skeet::hud

// Declarations from feature_indicators.hpp

namespace skeet::hud {
// Complete valid-player render owner 433E8FD3. This consumes the native
// feature-indicator descriptor 59A226C2 and the anti-aim owner descriptors.
void draw_freestanding_indicator(ui::RenderQueue& queue, script::Bridge& scripts,
                                 const anti_aim::GlobalConfig& global,
                                 const anti_aim::Config& config, anti_aim::State& state,
                                 const IntegerConfig& feature_indicators,
                                 const esp::RuntimePlayer& local_player, InputBitmap input,
                                 std::int32_t& cursor, const esp::IndicatorLayout& layout);
// Complete valid-player 433DF3FC. Calls the recovered PingSpikeRuntime::active
// owner and engine/channel virtuals, then the native HSV color/indicator chain.
void draw_ping_indicator(ui::RenderQueue& queue, script::Bridge& scripts,
                         const IntegerConfig& feature_indicators,
                         const esp::RuntimePlayer& local_player, const misc::PingSpikeRuntime& ping,
                         std::int32_t& cursor, const esp::IndicatorLayout& layout);
// Admitted433DF454..433DF54E: actual channel/latencies and native HSV draw.
// Both the semantic entry and the exact-layout HUD owner use this same body.
void draw_ping_latency_indicator(ui::RenderQueue&, script::Bridge&, const misc::PingSpikeRuntime&,
                                 std::int32_t& cursor, const esp::IndicatorLayout&);
// Complete valid-player 4337F589, including the unconditional concrete
// hitbox-highlight successor 433D12F7 after SAFE, BODY, MD in native order.
void draw_rage_indicators(ui::RenderQueue& queue, script::Bridge& scripts,
                          const rage::GlobalSettings& global, const rage::AimbotSettings& config,
                          const IntegerConfig& feature_indicators,
                          const esp::RuntimePlayer& local_player, InputBitmap input,
                          std::int32_t& cursor, const esp::IndicatorLayout& layout,
                          const esp::AimbotVisualizationConfig& visualization,
                          esp::HitboxHighlightState& highlight_state,
                          const esp::HitboxHighlightContext& highlight_context);
struct ExploitIndicatorBindings {
    void* entity_list = nullptr;
    std::uint32_t life_state, active_weapon, weapon_definition;
};
// Complete valid-player 433E1B2F. Both timers decay before the mode gate.
// shared_timer is native global43475890; state is the network interface's
// shared exploit state, whose timer+124 aliases render-interface+134.
void draw_exploit_indicators(ui::RenderQueue& queue, script::Bridge& scripts,
                             network::ExploitState& state, float& shared_timer,
                             const IntegerConfig& feature_indicators, void* local_player,
                             const ExploitIndicatorBindings& bindings, float frame_time,
                             std::int32_t& cursor, const esp::IndicatorLayout& layout);
} // namespace skeet::hud

// Declarations from movement_indicators.hpp

namespace skeet::hud {
// References bind the same movement owner fields changed by gameplay. The
// crouch-cycle writer has not yet assigned semantic names to every byte.
struct MovementIndicatorState {
    QuickPeekState& quick_peek;
    std::uint8_t& byte_51c;
    float& duck_target_52c;
    std::int32_t& value_540;
    std::uint8_t& crouch_active_544;
};
// Complete valid-player43384B72. DUCK is the four-byte label constructed by
// 4347DCB2. The activation chain is433305FB ->4341D036 ->actual hotkey;
// command_flags is global434759C0 and duck_enabled is byte43467CEC.
void draw_movement_indicators(ui::RenderQueue& queue, script::Bridge& scripts,
                              const IntegerConfig& feature_indicators, std::uint8_t duck_enabled,
                              const rage::GlobalSettings& rage, std::uint32_t command_flags,
                              const QuickPeekConfig& quick_peek, MovementIndicatorState state,
                              const esp::RuntimePlayer& local_player, InputBitmap input,
                              std::int32_t& cursor, const esp::IndicatorLayout& layout,
                              const esp::Projection& projection);
} // namespace skeet::hud

// Declarations from observer_panel.hpp

namespace skeet::hud {

struct ObserverPlayerInfo {
    std::array<char, 128> name{};
    bool fake_player = false;
    bool hltv = false;
};
struct ObserverEngine {
    const esp::PlayerRuntimeEngine& players;
    void* context = nullptr;
    // IVEngineClient::GetPlayerInfo, engine virtual slot 8.
    bool (*player_info)(void*, std::int32_t, ObserverPlayerInfo&) = nullptr;
};

// Complete 4334E581. The counter is native object +6A8, also exposed as
// PlayerFrameState::transient through the drawing owner's +8 adjustment.
// begin_player_frame resets that counter before the per-player loop.
void draw_observer_panel(ui::RenderQueue& queue, esp::PlayerFrameState& state,
                         const ObserverEngine& engine, const esp::RuntimePlayer* initial_view,
                         const esp::RuntimePlayer* observer, std::int32_t entity_index,
                         std::int32_t screen_width);

// The observer_panel handoff emitted by prepare_player (4340996B..43409974).
void draw_prepared_observer(ui::RenderQueue& queue, esp::PlayerFrameState& state,
                            const ObserverEngine& engine, const esp::RuntimePlayer* initial_view,
                            const esp::PreparedPlayer& prepared, std::int32_t screen_width);

} // namespace skeet::hud

// Declarations from penetration_reticle.hpp

namespace skeet::hud {
struct PenetrationReticleState {
    std::uint32_t color = 0; // full owner +28, render interface +18
    bool ready = false; // full owner +2C, render interface +1C
};
struct PenetrationReticleBindings {
    void* entity_list = nullptr;
    std::uint32_t life_state, active_weapon, weapon_definition;
};
// Complete 4341CC34. The gameplay owner publishes ready/color; rendering
// retains them and queries the genuine entity-list handle virtual.
void draw_penetration_reticle(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                              const PenetrationReticleState& state, void* local_player,
                              const PenetrationReticleBindings& bindings, Point screen_size);
} // namespace skeet::hud

// Declarations from penetration_reticle_owner.hpp

namespace skeet::hud {
struct PenetrationReticleOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
};
inline constexpr std::size_t penetration_reticle_native_owner_size = 0x34;
std::size_t penetration_reticle_owner_size() noexcept;
// Whole434824CC. settings/environment are the same objects referenced by
// command.settings/bindings; rendering references that same state/settings.
// Only descriptor1E7432A5 is registered. color+28 is cleared; ready+2C is
// preserved. The actual43468304 engine-trace cell is read after registration
// and captured into environment.point_contents (native owner+30).
// The registry, heap binding and borrowed owners outlive this endpoint.
void construct_penetration_reticle_owner(void* storage, const PenetrationReticleOwnerRegistries&,
                                         ConfigRegistries&, esp::ScreenOverlayConfig&,
                                         client::PenetrationReticleCommandOwner&,
                                         PenetrationReticleEnvironment&, void* const& engine_trace,
                                         const PenetrationHudBinding&);
//43481DE4..43481DEC requests exactly34 bytes. Allocation failure remains a
// nullable result; its native INT1 continuation43481DF7 is not reconstructed.
void* allocate_penetration_reticle_owner_storage(engine::JobMemory);
// Proved nonnull factory tail43481DF0..43481DF6. Projects zeroed native
// backing into only this owner's descriptor/state before the sparse ctor.
void construct_allocated_penetration_reticle_owner(
    void* storage, const PenetrationReticleOwnerRegistries&, ConfigRegistries&,
    esp::ScreenOverlayConfig&, client::PenetrationReticleCommandOwner&,
    PenetrationReticleEnvironment&, void* const& engine_trace, const PenetrationHudBinding&);
void destroy_penetration_reticle_owner(void* storage); //433B1A7A ->433837D0
void reset_penetration_reticle_owner(void* storage); //43381D40
void refresh_penetration_reticle_owner(void* storage); //433CF2F5 ->current slot5
} //namespace skeet::hud

// Declarations from penetration_runtime.hpp

namespace skeet::hud {
struct PenetrationReticleFrame {
    void* weapon; // Native frame+C.
    void* local_player; // Explicit player argument to phase4334B1DA.
    RageVector origin; // Frame+DC.
    RageVector direction; // Frame+1A8, already a direction, not Euler angles.
};
struct PenetrationReticleEnvironment {
    const rage::TargetBindings& targeting;
    const std::int32_t* maximum_clients; // Dereferenced native43469300.
    BallisticEngine& ballistics;
    std::ptrdiff_t target_tick; // Decoder2B554AD0.
    const std::int32_t* predicted_tick; // Native43469060; optional.
    std::ptrdiff_t collision_group; // DecoderE4E2D1A7.
    std::ptrdiff_t move_parent_handle; // Decoder49D93E62 minus18.
    void* point_contents; // Gameplay interface+28, IEngineTrace slot0.
    void* const* trace_filter_vtable; // Genuine external table43475B90, slots0..4.
};
// Complete4332AA97, including native43358962 eligibility and434012F7 geometry.
// Equal distances retain the earlier entity; unordered distances replace it.
void* closest_penetration_target(const rage::TargetBindings&, std::int32_t maximum_clients,
                                 const RageVector& origin, const RageVector& direction,
                                 float maximum_distance);
// Complete gameplay phase4334B1DA. Renderer4341CC34 consumes the same state.
void update_penetration_reticle(PenetrationReticleState&, const esp::ScreenOverlayConfig&,
                                const PenetrationReticleFrame&,
                                const PenetrationReticleEnvironment&);
}

// Declarations from screen_overlay.hpp

namespace skeet::hud {
// Shared runtime values published by433361BA. +58 references the Remove
// recoil descriptor; +6C marks the published recoil vector as valid.
struct ScreenOverlayState {
    const std::uint8_t* byte_58 = nullptr;
    float radius_5c = 0, recoil_pitch_60 = 0, recoil_yaw_64 = 0, recoil_roll_68 = 0;
    bool byte_6c = false;
};
// Complete43358A6A, including actual entity virtual171, recoil projection,
// radius-dependent segment count, and native opcode7 transparent-center fan.
void draw_screen_overlay(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                         const ScreenOverlayState& state, void* local_player,
                         std::uint32_t life_state_offset, Point screen_size,
                         const esp::Projection& projection, const RageVector& view_origin,
                         const RageTrigonometry& trigonometry);
} // namespace skeet::hud

// Declarations from screen_overlay_owner.hpp

namespace skeet::hud {
struct ScreenOverlayOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
};
inline constexpr std::size_t screen_overlay_native_owner_size = 0x70;
std::size_t screen_overlay_owner_size() noexcept;
// Whole4348DBD4. settings is the canonical mutable object referenced by
// command.settings; only recoil/inaccuracy/inaccuracy_color are registered.
// rendering must reference that same command.state/settings. Registries,
// bindings and canonical owners outlive this borrowed endpoint allocation.
// Publisher globals are retained.
void construct_screen_overlay_owner(void* storage, const ScreenOverlayOwnerRegistries&,
                                    ConfigRegistries&, esp::ScreenOverlayConfig& settings,
                                    client::ScreenOverlayCommandOwner& command,
                                    const ScreenOverlayHudBinding& rendering);
//4349095E..43490966 requests exactly70 native bytes. Exposes the nullable
//43387448 allocation separately; failure continuation at43490971 is unresolved.
void* allocate_screen_overlay_owner_storage(engine::JobMemory);
// Proved nonnull tail4349096A..43490970. Projects zeroed native backing into
// the one canonical state and the three owned descriptors, then constructs.
void construct_allocated_screen_overlay_owner(void* storage, const ScreenOverlayOwnerRegistries&,
                                              ConfigRegistries&, esp::ScreenOverlayConfig&,
                                              client::ScreenOverlayCommandOwner&,
                                              const ScreenOverlayHudBinding&);
// Whole primary43472308 slot0,433B1A7A ->433837D0. Registry entries are
// borrowed and remain in their native vectors until registry teardown.
void destroy_screen_overlay_owner(void* storage);
// Primaryslot1,43486688: actual lookup order FC225A4F,CA80F9ED,63660060.
void bind_screen_overlay_owner_settings(void* storage);
} //namespace skeet::hud

// Declarations from screen_overlay_update.hpp

namespace skeet::hud {
// The Remove recoil descriptor (63660060), looked up at434866A8..434866B8,
// supplies full owner+58. Its lifetime is the canonical settings lifetime.
void bind_screen_overlay_recoil_config(ScreenOverlayState&, const rage::GlobalSettings&) noexcept;
void reset_screen_overlay_state(ScreenOverlayState&) noexcept; // complete433CDDCF
struct ScreenOverlayFields {
    std::ptrdiff_t weapon_definition; // 5C9F0FBB
    std::ptrdiff_t weapon_recoil_index; // 6777C9CE
    std::ptrdiff_t recoil_angles; // 1BAE1168
    std::ptrdiff_t recoil_velocity; // 48442721
    std::ptrdiff_t weapon_time_f71ce331;
    std::ptrdiff_t player_time_0176a381;
    std::ptrdiff_t player_time_de530a8b;
    std::ptrdiff_t weapon_penalty_d2563274;
};
// Proven fields of the shared weapon context consumed by this owner. Spread
// uses the existing canonical +C4/+D0 projection, rather than another model.
struct ScreenOverlayCommandFrame {
    std::uint32_t flags;
    void* weapon;
    std::array<float, 2> recoil_view; // native context +9C/+A0
    rage::SpreadParameters spread;
};
ScreenOverlayCommandFrame screen_overlay_command_frame(const void* native_context);
struct ScreenOverlayUpdateBindings {
    void* engine; // virtual5 screen dimensions
    void* input; // virtual32(-1), native final radius adjustment
    void* cvar; // native linked command list at +30
    const float* current_time;
    const float* interval;
    const float* receive_interval;
    const float* field_of_view;
    engine::ConVarBinding recoil_scale; //ConVar4346794C ^C429E1BF
    engine::ConVarBinding first_decay; //ConVar43467104 ^E974CD65
    engine::ConVarBinding linear_decay;
    engine::ConVarBinding angle_decay;
    engine::ConVarBinding velocity_decay;
    client::EntityCommandOwner* const& history_owner; //live43475898, getter433952F0
    lagcomp::RotationMath rotation;
    // Actual scalar SSE CRT boundaries, not internal algorithm replacements.
    float (*exponential)(float);
    float (*tangent)(float);
};
class ScreenOverlayPublisher {
    bool standalone_initialized_ = false;
    void* standalone_variable_ = nullptr;

public:
    ScreenOverlayPublisher() noexcept
        : prediction_variable_initialized(standalone_initialized_),
          prediction_variable(standalone_variable_) {}
    ScreenOverlayPublisher(bool& initialized, void*& variable) noexcept
        : prediction_variable_initialized(initialized), prediction_variable(variable) {}
    ScreenOverlayPublisher(const ScreenOverlayPublisher&) = delete;
    ScreenOverlayPublisher& operator=(const ScreenOverlayPublisher&) = delete;
    bool& prediction_variable_initialized; // native43475894
    void*& prediction_variable; // native43475A24
    // Complete433361BA..4333692C. Native receiver is full owner+8; this writes
    // the same ScreenOverlayState read by draw_screen_overlay(43358A6A).
    void publish(ScreenOverlayState&, const esp::ScreenOverlayConfig&,
                 const ScreenOverlayCommandFrame&, void* local, const client::Command&,
                 const ScreenOverlayFields&, const ScreenOverlayUpdateBindings&);
};
} // namespace skeet::hud

// Declarations from simple_owners.hpp

namespace skeet::hud {
// One stable process context projects the globals used by the original bare
// primary/HUD objects. It owns no second latency history or per-owner state.
// Bind before invoking either factory; all borrowed owners outlive callbacks.
struct SimpleHudOwnersContext {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding hud; //43475A9C
    script::Bridge& scripts;
    const IntegerConfig& feature_indicators; //43468B94
    misc::PingSpikeChannelRuntime& ping_channel; //canonical4347A500 history
    const esp::IndicatorLayout& indicator_layout;
    const esp::ScreenOverlayConfig& overlays;
    void* const& entity_list; //4346A890
    void* const& cvar; //4346A87C, loaded again for the second lookup
    const engine::NativeOffsetStorage& offsets;
    const std::uint8_t& suspended; //43468E58
};
void bind_simple_hud_owners_context(const SimpleHudOwnersContext&);
inline constexpr std::size_t ping_hud_native_owner_size = 0x0c;
inline constexpr std::size_t crosshair_hud_native_owner_size = 0x14;

// Whole inline static factories4347DD87 /43486BF7. Both request their exact
// native allocation and return normally on allocation failure. No active
// owner pointer is published by either original factory.
void initialize_ping_hud_owner();
void initialize_crosshair_hud_owner();
// Their proved nonnull constructor blocks, preserving unassigned native words.
void construct_ping_hud_owner(void* storage);
void construct_crosshair_hud_owner(void* storage);
// Actual primary methods: shared slot0, Ping slot7, Crosshair slot1.
void destroy_simple_hud_owner(void* storage); //433B1A7A ->433837D0
void reset_ping_hud_history(void* storage); //433F182B, ignores receiver
void bind_crosshair_hud_variables(void* storage); //4347E7EB
} //namespace skeet::hud

// Declarations from simple_runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class VisualRuntimeOwner;
class NativeStartupSources;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::hud {
class SimpleHudRuntimeOwner final {
public:
    SimpleHudRuntimeOwner(client::ClientRuntimeStorage&, client::ConfigurationRuntimeOwner&,
                          client::VisualRuntimeOwner&, engine::HookRuntimeOwner&,
                          script::SourceScriptRuntimeOwner&);
    SimpleHudRuntimeOwner(const SimpleHudRuntimeOwner&) = delete;
    SimpleHudRuntimeOwner& operator=(const SimpleHudRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    SimpleHudOwnersContext& context() noexcept {
        return context_;
    }
    void bind_initializers(client::NativeStartupSources&);

private:
    client::ClientRuntimeStorage& storage_;
    SimpleHudOwnersContext context_;
};
}

// Declarations from detail.hpp

namespace skeet::esp::detail {
using i32 = std::int32_t;
inline i32 add(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) + std::uint32_t(b));
}
inline i32 sub(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) - std::uint32_t(b));
}
inline i32 mul(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) * std::uint32_t(b));
}
inline i32 truncate(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
inline float sse_max(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
inline float sse_min(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
inline std::u16string decimal(i32 value) {
    char buffer[12];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    return std::u16string(buffer, result.ptr);
}
} // namespace skeet::esp::detail

// Declarations from flag_table_detail.hpp

namespace skeet::esp::detail {
// The single decoded value recipe is shared by the ordinary native-table
// constructor and the explicit snapshot helper. It owns no second table.
FlagEntry initial_flag_entry(std::size_t index) noexcept;
}

// Declarations from grenade_live_detail.hpp

namespace skeet::esp::grenade_live_detail {
template <class T> T field(const void* object, std::ptrdiff_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
inline std::uint32_t identity(const void* value) noexcept {
    return std::uint32_t(reinterpret_cast<std::uintptr_t>(value));
}
inline std::int32_t truncate(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
inline Vector3 difference(Vector3 a, Vector3 b) noexcept {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline float squared(Vector3 v) noexcept {
    return (v.y * v.y + v.x * v.x) + v.z * v.z;
}
inline float convar_float(const void* value) noexcept {
    while (value != field<const void*>(value, 0x1c))
        value = field<const void*>(value, 0x1c);
    return std::bit_cast<float>(field<std::uint32_t>(value, 0x2c) ^ identity(value));
}
inline std::int32_t convar_integer(const void* value) noexcept {
    while (value != field<const void*>(value, 0x1c))
        value = field<const void*>(value, 0x1c);
    return std::bit_cast<std::int32_t>(field<std::uint32_t>(value, 0x30) ^ identity(value));
}
inline float clamp(float value, float low, float high) noexcept {
    return _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_set_ss(low)), _mm_set_ss(high)));
}
}
namespace skeet::esp {
struct GrenadeLiveScene::Storage {
    GrenadeLiveBindings bindings;
    EngineGrenades& raw;
    GrenadeLiveOffsets offsets;
    GrenadeAuxiliaryLiveContext auxiliary_context;
    explicit Storage(const GrenadeLiveBindings&);
    bool enemy(void*) const;
    bool game_flag(std::ptrdiff_t) const;
    std::int32_t team(void*);
    bool admitted(std::uint32_t, void* thrower, void* selected);
    void* thrower(void* projectile, std::uint32_t type);
    float damage_scale(std::uint32_t handle, std::uint32_t type);
    GrenadeDamageResult damage(void*, Vector3 explosion, Vector3 distance_origin);
    GrenadeDamageSelection select_damage(Vector3, const IntegerConfig&);
    void assess(GrenadeWarningRecord&, void*, std::uint32_t&);
    void update(GrenadeWarningOwner&, const GrenadeEspConfig&, void*, std::uintptr_t);
    void draw(ui::RenderQueue&, const misc::GrenadePredictionConfig&, const GrenadeEspConfig&,
              GrenadeTrajectoryOwner&, misc::GrenadeCommandState&, GrenadeWarningOwner&);
    void draw_warnings(ui::RenderQueue&, const GrenadeEspConfig&, GrenadeWarningOwner&);
};
}

// Declarations from world_engine_detail.hpp

namespace skeet::esp::world_detail {
template <class T> T field(const void* object, std::ptrdiff_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
inline std::int32_t convar_integer(const void* object) {
    for (;;) {
        auto* parent = field<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return std::bit_cast<std::int32_t>(
        field<std::uint32_t>(object, 0x30) ^
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object)));
}
inline float squared(Vector3 a, Vector3 b) {
    const auto x = a.x - b.x, y = a.y - b.y, z = a.z - b.z;
    return (y * y + x * x) + z * z;
}
inline void distance(ui::RenderQueue& queue, Point at, float squared, std::uint32_t color) {
    auto text = detail::decimal(
        detail::mul(detail::sub(2, detail::truncate(std::sqrt(squared) / -12.0f)) / 5, 5));
    text += u" FT";
    queue.text(at.x, at.y, color, 0x14, 0, text);
}
} // namespace skeet::esp::world_detail
