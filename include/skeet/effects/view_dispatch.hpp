#pragma once
#include "view.hpp"
#include "view_fov.hpp"
#include "skeet/misc/fov.hpp"
#include "skeet/network/update_observers.hpp"

namespace skeet::effects {
// Actual secondary-interface owners. These endpoints bind concrete recovered
// methods; their empty slots are the native433CCE8E/4337D653/4334A269 entries.
struct ScopeViewObserver {
    network::UpdateObserver endpoint;
    ViewRuntime* runtime;
    explicit ScopeViewObserver(ViewRuntime&);
    void initialize_vtable() noexcept;
    ScopeViewObserver(engine::OwnerRegistryBinding, ViewRuntime&);
};
struct FovViewObserver {
    network::UpdateObserver endpoint;
    misc::ZoomSensitivityRuntime* sensitivity;
    ViewFovRuntime* runtime;
    // Unregistered constructor; the full4347DDCD owner assigns both vtables
    // only after its lifecycle/update registrations.
    FovViewObserver(misc::ZoomSensitivityRuntime&, ViewFovRuntime&);
    void initialize_vtable() noexcept;
    FovViewObserver(engine::OwnerRegistryBinding, misc::ZoomSensitivityRuntime&, ViewFovRuntime&);
};
// Slot5 loop from433C843E..8471; the end pointer is captured before calls.
void dispatch_player_view(const engine::OwnerRegistry&, void* player, std::uintptr_t first,
                          std::uintptr_t second, std::uintptr_t third);
// Complete433B91CB, including its false AL result. Origin, angles and FOV are
// live mutable engine arguments; every endpoint comes from the native owner graph.
bool dispatch_view(const engine::OwnerRegistry&, float* origin, float* angles, float* fov);
} // namespace skeet::effects
