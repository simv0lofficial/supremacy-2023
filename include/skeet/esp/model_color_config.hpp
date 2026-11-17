#pragma once
#include "skeet/config.hpp"

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
