#pragma once
#include "skeet/esp/other_config.hpp"
#include "skeet/render_queue.hpp"
#include "skeet/core.hpp"
#include "skeet/engine/offset_storage.hpp"

namespace skeet::hud {
struct CrosshairOffsets {
    // Resolvers 9D920E35, CACAEECC, CD812654, A1AF8FFC respectively.
    std::uint32_t life_state, scoped, active_weapon, weapon_draw_flag;
};
struct CrosshairBindings {
    void* entity_list = nullptr;
    // Native owner +4/+8. Parent-linked ConVars with the address-xored
    // integer at +30; the second pointer is optional.
    const void* style_variable = nullptr;
    const void* weapon_mode_variable = nullptr;
    CrosshairOffsets offsets;
};

// Complete 433E4593. Uses entity-list slot 4, weapon slot 461 for weapon
// data, and crosshair drawing at weapon slot 404.
// The engine draw observes type=3 and flag=0, then both fields are restored.
void draw_crosshair(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                    const CrosshairBindings& bindings, void* local_player, bool suspended,
                    Point screen_size);
struct NativeCrosshairBindings {
    void* const& entity_list;
    const void* const& style_variable; //actual complete owner+C
    const void* const& weapon_mode_variable; //actual complete owner+10
    const engine::NativeOffsetStorage& offsets;
};
// The player294 callback consumes no drawing ConVars. Its live entity-list
// view is separate from the complete draw owner's style/mode bindings.
struct CrosshairViewBindings {
    void* const& entity_list;
    CrosshairOffsets offsets;
};
// The actual owner path uses the same draw body, with offset/global reads at
// consumption. Suspended rendering queries this supplied queue's own screen.
void draw_live_crosshair(ui::RenderQueue&, const esp::ScreenOverlayConfig&,
                         const NativeCrosshairBindings&, void* local_player,
                         const std::uint8_t& suspended);

// Complete valid-receiver 433B34C0. The native hook hashes six bytes at its
// return address before setting view_setup +E0/+E1. The original virtual slot
// 294 still runs after every ordinary rejection and after successful mutation.
// original_vtable is the saved table in the native hook record at vtable[-2].
void prepare_crosshair_view(const esp::ScreenOverlayConfig& config,
                            const CrosshairBindings& bindings, void* local_player, void* view_setup,
                            bool suspended, const std::byte* return_address,
                            void** original_vtable);
void prepare_crosshair_view(const esp::ScreenOverlayConfig&, const CrosshairViewBindings&,
                            void* local_player, void* view_setup, bool suspended,
                            const std::byte* return_address, void** original_vtable);
} // namespace skeet::hud
