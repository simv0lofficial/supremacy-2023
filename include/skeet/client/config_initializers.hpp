#pragma once
#include "skeet/config.hpp"

namespace skeet::anti_aim {
struct GlobalConfig;
}
namespace skeet::esp {
struct OtherEspSettings;
struct PlayerEspSettings;
struct EffectsSettings;
}
namespace skeet::misc {
struct PingSpikeConfig;
}
namespace skeet::rage {
struct GlobalSettings;
}

namespace skeet::client {
class NativeStartupSources;

// Borrow the canonical menu/runtime descriptors. Their declarations retain the
// native image's initial hash/type/size; attaching these references writes no
// metadata or values. Keep this context alive throughout startup dispatch.
struct ConfigInitializerBindings {
    ConfigRegistries& configuration;
    esp::OtherEspSettings& other_esp;
    esp::PlayerEspSettings& player_esp;
    esp::EffectsSettings& effects;
    rage::GlobalSettings& rage;
    misc::PingSpikeConfig& ping_spike;
    anti_aim::GlobalConfig& anti_aim;
    BooleanConfig& target_mask; // descriptor43467974, hash347A4FA6
};

// Register eighteen independent ordinary CRT source bodies. This does not run
// them or select their order; NativeStartupRuntime follows the live operands.
void bind_native_config_initializers(NativeStartupSources&, ConfigInitializerBindings&);

} // namespace skeet::client
