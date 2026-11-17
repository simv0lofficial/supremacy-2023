#pragma once
#include "skeet/esp/effects_config.hpp"
#include "skeet/esp/model_color_config.hpp"

namespace skeet::effects {
// Exact52 owned descriptors4348E73C..4348EC94. Pass the unregistered canonical
// objects. Static descriptors and other effects owners are initialized by their
// own constructors, rather than being registered a second time here.
void initialize_model_effects_owner_settings(ConfigRegistries&, esp::EffectsSettings&,
                                             esp::ColoredModelsSettings&) noexcept;
} // namespace skeet::effects
