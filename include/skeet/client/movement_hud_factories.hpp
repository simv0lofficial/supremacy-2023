#pragma once
#include "skeet/movement/owner_lifecycle.hpp"
#include "skeet/movement/blockbot_owner.hpp"
#include "skeet/hud/screen_overlay_owner.hpp"
#include "skeet/hud/penetration_reticle_owner.hpp"

namespace skeet::client {
class NativeStartupSources;

// Borrowed contexts for the four proven movement/HUD factory identities.
// Registry records, command views, settings and render bindings remain at
// stable addresses through destruction of their registered whole owners.
struct MovementHudFactoryBindings {
    ConfigRegistries& configuration;
    MovementOwnerRegistries movement_registries;
    MovementOwnerBindings movement;
    BlockbotConfig& blockbot_settings;
    BlockbotState& blockbot_state;
    const BlockbotBindings& blockbot;
    const esp::Projection& blockbot_projection;
    const std::uint32_t& blockbot_life_state_offset;
    hud::ScreenOverlayOwnerRegistries overlay_registries;
    ScreenOverlayCommandOwner& overlay_command;
    const hud::ScreenOverlayHudBinding& overlay_rendering;
    hud::PenetrationReticleOwnerRegistries reticle_registries;
    PenetrationReticleCommandOwner& reticle_command;
    hud::PenetrationReticleEnvironment& reticle_environment;
    void* const& engine_trace; //43468304, captured by the reticle constructor
    const hud::PenetrationHudBinding& reticle_rendering;
    esp::ScreenOverlayConfig& screen_settings;
};

//434856D6 publishes movement's canonical434759B0 slot, including null.
//4348DA54/4349095E/43481DE4 have proven nonnull constructor tails and no
//published owner globals. Their unrecovered INT1 allocation-failure paths
//explicitly throw bad_alloc; this does not claim those protected tails.
void bind_movement_hud_factories(NativeStartupSources&, MovementHudFactoryBindings&);
} //namespace skeet::client
