#pragma once
#include "skeet/config.hpp"

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
