#pragma once
#include "composite_runtime.hpp"
#include "crosshair.hpp"
#include "feature_indicators.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/misc/ping_spike_channel.hpp"

namespace skeet::hud {
// One stable process context projects the globals used by the original bare
// primary/HUD objects. It owns no second latency history or per-owner state.
// Bind before invoking either factory; all borrowed owners outlive callbacks.
struct SimpleHudOwnersContext {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding hud; //43475A9C
    script::Bridge& scripts;
    const IntegerConfig& feature_indicators; //43468B94
    misc::PingSpikeChannelRuntime& ping_channel; //canonical4347A500 history
    const esp::IndicatorLayout& indicator_layout;
    const esp::ScreenOverlayConfig& overlays;
    void* const& entity_list; //4346A890
    void* const& cvar; //4346A87C, loaded again for the second lookup
    const engine::NativeOffsetStorage& offsets;
    const std::uint8_t& suspended; //43468E58
};
void bind_simple_hud_owners_context(const SimpleHudOwnersContext&);
inline constexpr std::size_t ping_hud_native_owner_size = 0x0c;
inline constexpr std::size_t crosshair_hud_native_owner_size = 0x14;

// Whole inline static factories4347DD87 /43486BF7. Both request their exact
// native allocation and return normally on allocation failure. No active
// owner pointer is published by either original factory.
void initialize_ping_hud_owner();
void initialize_crosshair_hud_owner();
// Their proved nonnull constructor blocks, preserving unassigned native words.
void construct_ping_hud_owner(void* storage);
void construct_crosshair_hud_owner(void* storage);
// Actual primary methods: shared slot0, Ping slot7, Crosshair slot1.
void destroy_simple_hud_owner(void* storage); //433B1A7A ->433837D0
void reset_ping_hud_history(void* storage); //433F182B, ignores receiver
void bind_crosshair_hud_variables(void* storage); //4347E7EB
} //namespace skeet::hud
