#pragma once
#include "game_events.hpp"
#include "skeet/esp/player_runtime.hpp"
#include "skeet/skins/config.hpp"
#include "skeet/skins/sequences.hpp"
#include <array>
#include <cstddef>
#include <span>

namespace skeet::events {
struct DeathBindings {
    void* engine = nullptr;
    void* entity_list = nullptr;
    void* local_player = nullptr;
};
struct WeaponDeathOffsets {
    std::ptrdiff_t team = 0;
    std::ptrdiff_t active_weapon = 0;
    std::ptrdiff_t definition = 0;
    std::ptrdiff_t original_owner_low = 0, original_owner_high = 0;
    std::ptrdiff_t stattrak = 0, attributes = 0;
};
struct DeathWeaponIcons {
    DeathWeaponIcons(const skins::Settings& settings,
                     const skins::SequenceRuntime& sequence) noexcept;
    const skins::Settings& settings;
    const skins::SequenceRuntime& sequence; // live native owner+7C producer
    // Constructor4348DFCB copies the canonical knife rows into owner+80.
    // Selection indexes these same definitions; valid selection is required.
    std::span<const skins::KnifeModel> selected_models;
    const char* weapon_key = "weapon"; // native owner +348
    void player_death(MutableGameEvent&, const DeathBindings&,
                      const WeaponDeathOffsets&); // 4341A07D
};
struct StatTrakDeath {
    // The menu, configuration loader and death event share descriptor B6218FEC.
    const skins::WeaponSkinsConfig& weapons;
    void player_death(GameEventReader&, const DeathBindings&,
                      const WeaponDeathOffsets&); // 43336CC1
};
// Native 433E881F, clearing geometry alpha only for a dormant player entity.
void death_clear_dormant_alpha(GameEventReader&, const DeathBindings&, esp::PlayerRuntimeState&);
} // namespace skeet::events
