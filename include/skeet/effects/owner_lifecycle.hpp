#pragma once
#include "bullet_impact_command_owner.hpp"
#include "model_render_lifecycle.hpp"
#include "world_callbacks.hpp"
#include "skeet/events/server_impacts.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/client/entity_observer_registry.hpp"

namespace skeet::effects {
class ModelRenderHooks;
struct EffectsOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
    client::EntityObserverRegistry& entities; //43475BA8, concrete effects ID
    engine::OwnerRegistryBinding updates; //43475988
    engine::OwnerRegistryBinding frames; //43475AD8
    client::CommandEmptyTickRegistry& empty_ticks; //43475978
};
struct EffectsOwnerBindings {
    EffectsCommandOwner& command;
    ModelRenderLifecycle& lifecycle;
    ModelRenderHooks& hooks;
    WorldCallbacks& updates;
    events::EventRegistry& event_registry;
    EffectsCommandOwner*& active; //canonical43475A1C slot
    engine::NativeDataImage& data; //same canonical constructor ConVars/RTTI cells
    ModelClassification& classification; //constructor captures consumed renderable tables here
};
// Storage is genuine43387448-style aligned storage; lifecycle slot0 releases
// its original allocation header through lifecycle.memory. All semantic owners
// are canonical borrowed objects and must outlive their registered endpoints.
std::size_t model_effects_owner_size() noexcept;
// Complete seven-interface registration/configuration composition4348E6E2.
// Pass fresh canonical model/world state and its unregistered52 owned config
// descriptors. Independent static settings use their own initializers.
void construct_model_effects_owner(void* storage, const EffectsOwnerRegistries&, ConfigRegistries&,
                                   EffectsOwnerBindings);
} // namespace skeet::effects
