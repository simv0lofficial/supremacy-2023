#pragma once
#include "skeet/menu/misc_menu.hpp"
#include "command_source.hpp"
#include "entity_update.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/hud/composite_handlers.hpp"

namespace skeet::client {
class CommandPhaseRegistry;
}
namespace skeet::rage {
class LifecycleOwner;
struct OwnerBindings {
    engine::OwnerRegistryBinding lifecycle;
    client::CommandPhaseRegistry& command_owners;
    engine::OwnerRegistryBinding hud_owners;
    client::EntityObserverRegistry& entity_owners;
    ConfigRegistries& configuration;
    AimbotSettings& aimbot;
    esp::AimbotVisualizationConfig& visualization;
    CommandEndpoint commands;
    hud::LiveRageHudBinding hud;
    EntityUpdateBindings entities;
};
// Complete434803BC owner construction. Aimbot/visualization descriptors must
// be unregistered, and commands.acquisition.evaluations must be empty. Their
// canonical storage and every borrowed binding outlive the returned owner.
// Registration is lifecycle -> command -> HUD -> entity, before descriptors.
LifecycleOwner* construct_rage_owner(OwnerBindings, LifecycleOwner*& active);
//433F19F5: clear the canonical43475810 slot before evaluation/primary frees.
void destroy_rage_owner(LifecycleOwner*& active);
// Exposes the native primary object for the common lifecycle dispatcher.
void* rage_lifecycle_object(LifecycleOwner&) noexcept;
}
