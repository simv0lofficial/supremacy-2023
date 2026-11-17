#pragma once
#include "skeet/config.hpp"

namespace skeet::esp {
struct OtherEspSettings;
struct EffectsSettings;
struct ColoredModelsSettings;
}
namespace skeet::misc {
struct AntiUntrustedConfig;
struct HideFromObsConfig;
struct CustomEventsConfig;
struct FreeLookConfig;
}
namespace skeet::rage {
struct GlobalSettings;
}
namespace skeet::ui {
class SettingsMenuStorage;
}

namespace skeet::client {
class NativeStartupSources;

// Every reference is the existing descriptor used by its menu/runtime owner.
// The image-data metadata belongs to those declarations; the CRT bodies below
// only append their links and perform their own explicit value/key writes.
struct GlobalConfigInitializerBindings {
    ConfigRegistries& configuration;
    rage::GlobalSettings& rage;
    esp::OtherEspSettings& other_esp;
    esp::EffectsSettings& effects;
    esp::ColoredModelsSettings& colored_models;
    misc::AntiUntrustedConfig& anti_untrusted;
    misc::HideFromObsConfig& hide_from_obs;
    misc::CustomEventsConfig& custom_events;
    misc::FreeLookConfig& free_look;
    ui::SettingsMenuStorage& settings_menu;
};

// Thirty-six independent callbacks at operands0B8/0BC/130/134/1B8/240,
// 24C..270 and278..2C4. Binding does not invoke them or choose their order.
void bind_global_config_initializers(NativeStartupSources&, GlobalConfigInitializerBindings&);

} //namespace skeet::client
