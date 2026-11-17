#pragma once
#include "command_owner.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/hud/feature_indicators.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include <vector>

namespace skeet::anti_aim {
struct OwnerRegistries {
    engine::OwnerRegistryBinding primary; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
    engine::OwnerRegistryBinding frames; //43475AD8
};
struct OwnerHudBindings {
    script::Bridge& scripts;
    const IntegerConfig& indicators;
    const esp::IndicatorLayout& layout;
    std::uint32_t life_state_9d920e35;
};
struct OwnerBindings {
    CommandOwner& command;
    const OwnerHudBindings& hud;
    void* const& allocator; //43467204, animation-vector Free virtual5
    CommandOwner*& active; //canonical43475940, also used by generatedcommands
};
std::size_t owner_size() noexcept;
// Complete43490435, on fresh canonical owner configuration. Four actual
// registrars retain their local order; independent global AA descriptors use
// their own CRT initializer. All command/animation/HUD state is borrowed once.
void construct_owner(void* storage, const OwnerRegistries&, ConfigRegistries&, OwnerBindings);
//43387448 allocation contract. Canonical projected values are zeroed as by
// HEAP_ZERO_MEMORY before construction. Storage allocation must succeed.
// The protected startup caller owns publication of the returned owner into
//43475940; the constructor itself has no such native store.
void* create_owner(const OwnerRegistries&, ConfigRegistries&, OwnerBindings);
// Complete433D6575 ->433E06EE ->433CF41B/4336B1E9 ->433837D0.
// Clears active before the null test, then releases animation, text and owner.
void destroy_owner(void* storage, CommandOwner*& active);
// Whole primary slots1/7. Slot1 borrows the actual96DB2E1C payload; slot7
// calls the current slot5 before resolving the owned "+spray_menu" command.
void bind_owner_settings(void* storage); //4348E637
void refresh_owner_command(void* storage); //433DF0C2
void draw_owner_hud(CommandOwner&, const OwnerHudBindings&, ui::RenderQueue&, void* local_player,
                    std::int32_t& cursor); //433E8FD3
} //namespace skeet::anti_aim
