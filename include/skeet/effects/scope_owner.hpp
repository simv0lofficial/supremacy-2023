#pragma once
#include "view_dispatch.hpp"
#include "skeet/hud/composite_runtime.hpp"

namespace skeet::effects {
struct ScopeOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding hud; //43475A9C
    engine::OwnerRegistryBinding updates; //43475988
};
struct ScopeOwnerBindings {
    ViewRuntime& runtime;
    esp::EffectsSettings& settings; //same canonical object referenced by runtime
};
std::size_t scope_owner_size() noexcept;
// Whole ctor43486002..43486055 and primary434727D8. Storage is the real
// aligned43387448 allocation; primary0 releases its header, primary7 discovers
// HudZoom. The HUD and update endpoints share one runtime.scope_panel.
void construct_scope_owner(void* storage, const ScopeOwnerRegistries&, ConfigRegistries&,
                           ScopeOwnerBindings);
} // namespace skeet::effects
