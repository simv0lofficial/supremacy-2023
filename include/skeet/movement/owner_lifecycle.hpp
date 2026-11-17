#pragma once
#include "command_wrapper.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/hud/movement_indicators.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <vector>

namespace skeet {
struct MovementOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
};
struct MovementOwnerHudBindings {
    script::Bridge& scripts;
    const IntegerConfig& feature_indicators; //43468B94
    const std::uint8_t& infinite_duck; //43467CEC
    const rage::GlobalSettings& rage;
    const std::uint32_t& command_flags; //434759C0
    const esp::IndicatorLayout& layout;
    const esp::Projection& projection;
    std::uint32_t life_state_9d920e35;
};
struct MovementOwnerBindings {
    MovementCommandPhaseOwner& command;
    const MovementOwnerHudBindings& hud;
    void* const& engine_client; //4346A514, virtual170 FindCommand
    void* const& input_system; //4346A89C, virtual46
    engine::NativeConVarReference air_accelerate; //434674E4 ^DCEAEAC8
    engine::NativeConVarReference jump_speed_cap; //43467FA0 ^4B52A9BF
    MovementCommandOwner*& active; //canonical434759B0
};

// Native4348FA90 registers primary, normal-command and HUD in this order,
// then publishes their tables before constructing settings/state/two strings.
// Canonical command state and bindings are borrowed and must outlive these
// endpoints. Pass fresh, unregistered configuration descriptors.
std::size_t movement_owner_size() noexcept;
void construct_movement_owner(void* storage, const MovementOwnerRegistries&, ConfigRegistries&,
                              MovementOwnerBindings);
// Allocate through the actual process-heap boundary with43387448's alignment
// header. The canonical active slot is published after construction.
void* create_movement_owner(const MovementOwnerRegistries&, ConfigRegistries&,
                            MovementOwnerBindings);
// Complete43328BC8, including clearing the global before the null test,
// reverse string destruction433DF641 and original allocation-header free.
void destroy_movement_owner(void* storage, MovementCommandOwner*& active);

// Whole primary slot7 (4337EBEF): reset through slot5, resolve "+duck" by
// engine170, reject0/-1, then input46 publishes the same last_key+540.
void refresh_movement_owner_key(void* storage);
// HUD43470424 slot0 consumes this frame's raw entity and reads its live life
// byte before invoking the existing complete43384B72 renderer.
void draw_movement_owner_hud(MovementCommandPhaseOwner&, const MovementOwnerHudBindings&,
                             ui::RenderQueue&, void* local_player, std::int32_t& cursor);
} // namespace skeet
