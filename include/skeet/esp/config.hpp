#pragma once
#include "skeet/config.hpp"

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
