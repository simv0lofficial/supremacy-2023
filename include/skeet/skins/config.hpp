#pragma once
#include "skeet/config.hpp"
#include <array>

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
