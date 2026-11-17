#pragma once
#include "skeet/esp/projection.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/esp/weapon_icon.hpp"
#include "skeet/esp/weapon_text.hpp"
#include <limits>
#include <string_view>
#include <vector>

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
