#pragma once
#include "composite_handlers.hpp"
#include "skeet/client/command_owner_dispatch.hpp"
#include <vector>

namespace skeet::hud {
struct ScreenOverlayOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
};
inline constexpr std::size_t screen_overlay_native_owner_size = 0x70;
std::size_t screen_overlay_owner_size() noexcept;
// Whole4348DBD4. settings is the canonical mutable object referenced by
// command.settings; only recoil/inaccuracy/inaccuracy_color are registered.
// rendering must reference that same command.state/settings. Registries,
// bindings and canonical owners outlive this borrowed endpoint allocation.
// Publisher globals are retained.
void construct_screen_overlay_owner(void* storage, const ScreenOverlayOwnerRegistries&,
                                    ConfigRegistries&, esp::ScreenOverlayConfig& settings,
                                    client::ScreenOverlayCommandOwner& command,
                                    const ScreenOverlayHudBinding& rendering);
//4349095E..43490966 requests exactly70 native bytes. Exposes the nullable
//43387448 allocation separately; failure continuation at43490971 is unresolved.
void* allocate_screen_overlay_owner_storage(engine::JobMemory);
// Proved nonnull tail4349096A..43490970. Projects zeroed native backing into
// the one canonical state and the three owned descriptors, then constructs.
void construct_allocated_screen_overlay_owner(void* storage, const ScreenOverlayOwnerRegistries&,
                                              ConfigRegistries&, esp::ScreenOverlayConfig&,
                                              client::ScreenOverlayCommandOwner&,
                                              const ScreenOverlayHudBinding&);
// Whole primary43472308 slot0,433B1A7A ->433837D0. Registry entries are
// borrowed and remain in their native vectors until registry teardown.
void destroy_screen_overlay_owner(void* storage);
// Primaryslot1,43486688: actual lookup order FC225A4F,CA80F9ED,63660060.
void bind_screen_overlay_owner_settings(void* storage);
} //namespace skeet::hud
