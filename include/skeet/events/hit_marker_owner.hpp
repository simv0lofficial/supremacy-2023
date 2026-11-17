#pragma once
#include "feature_listeners.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/hud/composite_runtime.hpp"

namespace skeet::events {
class HitMarkerLifecycleOwner;
struct HitMarkerOwnerBindings {
    esp::HitMarkerConfig& settings;
    float& remaining; // one canonical native+38 cell
    misc::NativeText& sound; // one canonical native+3C text backing
    HitMarkerBindings& events;
    void* const& global_variables; // live43467B04, frame time+14
    engine::DecodedOffset rejection_byte; //43468E78 ^434684E4 ^9D920E35
};
inline constexpr std::size_t native_hit_marker_allocation_size = 0x4C;
std::size_t hit_marker_owner_size() noexcept;

// Whole43486513 constructor and primary4347253C. Caller supplies native-header
// storage large enough for hit_marker_owner_size(). The canonical settings,
// remaining/text cells, event bindings and all registries outlive this owner.
// Native order: primary -> HUD -> capture surface -> descriptors -> remaining0
// -> sound text -> one permanent hurt listener. Do not separately initialize
// these descriptors or register another HitMarkerListener/HUD handler.
HitMarkerLifecycleOwner& construct_hit_marker(void* storage, engine::OwnerRegistryBinding lifecycle,
                                              engine::OwnerRegistryBinding hud, ConfigRegistries&,
                                              EventRegistry&, HitMarkerOwnerBindings);
HitMarker& hit_marker_runtime(HitMarkerLifecycleOwner&) noexcept;

// Factory43484954 requests0x4C and tail-calls43486513 on success. Its failure
// target43484967 is not recovered by the supplied instruction records; this
// constructor entry leaves that protected edge and startup order unchanged.
} // namespace skeet::events
