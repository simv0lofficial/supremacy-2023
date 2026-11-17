#pragma once
#include "composite_handlers.hpp"
#include "skeet/client/command_owner_dispatch.hpp"
#include <vector>

namespace skeet::hud {
struct PenetrationReticleOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
};
inline constexpr std::size_t penetration_reticle_native_owner_size = 0x34;
std::size_t penetration_reticle_owner_size() noexcept;
// Whole434824CC. settings/environment are the same objects referenced by
// command.settings/bindings; rendering references that same state/settings.
// Only descriptor1E7432A5 is registered. color+28 is cleared; ready+2C is
// preserved. The actual43468304 engine-trace cell is read after registration
// and captured into environment.point_contents (native owner+30).
// The registry, heap binding and borrowed owners outlive this endpoint.
void construct_penetration_reticle_owner(void* storage, const PenetrationReticleOwnerRegistries&,
                                         ConfigRegistries&, esp::ScreenOverlayConfig&,
                                         client::PenetrationReticleCommandOwner&,
                                         PenetrationReticleEnvironment&, void* const& engine_trace,
                                         const PenetrationHudBinding&);
//43481DE4..43481DEC requests exactly34 bytes. Allocation failure remains a
// nullable result; its native INT1 continuation43481DF7 is not reconstructed.
void* allocate_penetration_reticle_owner_storage(engine::JobMemory);
// Proved nonnull factory tail43481DF0..43481DF6. Projects zeroed native
// backing into only this owner's descriptor/state before the sparse ctor.
void construct_allocated_penetration_reticle_owner(
    void* storage, const PenetrationReticleOwnerRegistries&, ConfigRegistries&,
    esp::ScreenOverlayConfig&, client::PenetrationReticleCommandOwner&,
    PenetrationReticleEnvironment&, void* const& engine_trace, const PenetrationHudBinding&);
void destroy_penetration_reticle_owner(void* storage); //433B1A7A ->433837D0
void reset_penetration_reticle_owner(void* storage); //43381D40
void refresh_penetration_reticle_owner(void* storage); //433CF2F5 ->current slot5
} //namespace skeet::hud
