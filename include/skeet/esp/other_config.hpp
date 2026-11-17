#pragma once
#include "skeet/config.hpp"

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
