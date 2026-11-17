#pragma once
#include "skeet/config.hpp"

namespace skeet::legit {
using skeet::BooleanArrayConfig;
using HotkeyConfigs = std::array<HotkeyConfig, 9>;
struct Settings {
    BooleanArrayConfig enabled;
    IntegerArrayConfig maximum_fov, speed, attack_speed, speed_scale_fov, reaction_time;
    BooleanArrayConfig through_smoke, while_blind, quick_stop;
    IntegerArrayConfig recoil_pitch, recoil_yaw, maximum_lock_time;
    BooleanArrayConfig head, chest, stomach;
    IntegerArrayConfig first_shot_delay;
    BooleanArrayConfig trigger_enabled;
    IntegerArrayConfig minimum_hit_chance, minimum_damage;
    BooleanArrayConfig burst_fire;
    IntegerArrayConfig burst_time;
    BooleanArrayConfig config_8a823c30;
    IntegerArrayConfig config_90b2820a;
    BooleanArrayConfig penetration, shoot_smoke, shoot_blind;
    BooleanArrayConfig trigger_head, trigger_chest, trigger_stomach;
    IntegerArrayConfig trigger_reaction, accuracy_boost, accuracy_range;
    BooleanArrayConfig standalone_recoil;
    HotkeyConfigs aim_keys, trigger_keys;
};
constexpr std::uint32_t hotkey_hash(std::uint32_t base, std::uint32_t index) noexcept {
    return ((base ^ 0x92CA5DF1u) + index) ^ 0x92CA5DF1u;
}
// The whole owner places its selective runtime construction between these
// descriptor registrations and the final per-category overrides.
void register_settings(ConfigRegistries& registries, Settings& settings) noexcept;
void finish_settings_initialization(Settings& settings) noexcept;
// Configuration portion of 0x434845CF..0x43484919. Engine state is outside it.
void initialize_settings(ConfigRegistries& registries, Settings& settings) noexcept;
} // namespace skeet::legit
