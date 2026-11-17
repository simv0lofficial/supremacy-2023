#pragma once
#include "skeet/skins/sequence_owner.hpp"
#include "skeet/rage/owner.hpp"
#include "skeet/legit/owner.hpp"
#include "skeet/esp/owner_runtime.hpp"
#include "skeet/effects/blur_runtime.hpp"
#include "skeet/misc/console_output_owner.hpp"
#include "skeet/misc/fov_owner.hpp"
#include "skeet/misc/purchase_log_owner.hpp"

namespace skeet::client {
class NativeStartupSources;

// Borrowed canonical contexts for eight allocation/constructor wrappers.
// Keep this record and its referenced state alive through owner destruction.
// Registration below selects bodies only; runtime_owner_initializers supplies
// their order together with the remaining recovered owner families.
struct RuntimeOwnerFactoryBindings {
    ConfigRegistries& configuration;
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding hud; //43475A9C
    EntityObserverRegistry& entity_updates; //43475BA8
    engine::OwnerRegistryBinding effect_frames; //43475A84
    skins::SequenceOwnerBindings sequences;
    rage::OwnerBindings rage;
    rage::LifecycleOwner*& rage_active; //43475810
    legit::OwnerBindings legit;
    esp::EspOwnerBindings esp;
    esp::EspOwnerRuntime*& esp_active; //434758DC
    effects::BlurBindings blur;
    effects::BlurEffect*& blur_active; //43475A20
    misc::ConsoleOutputConfig& console_settings;
    misc::ColoredLogRing*& console_active; //434758CC
    const float& frame_time;
    misc::FovConfig& fov_settings;
    misc::FovOwnerBindings fov;
    misc::NormalLog& event_output;
    rage::TargetRelationshipBindings event_targets;
    script::ConsoleInputHook& purchase_console;
    void* const& console_container;
};

//43481F6A,4348DA25,4347FEBC,43481EAB,4347F6DB,43484E4E,4347DA60,43481F44.
// FOV/purchase OOM tails reach unresolved INT1; source throws bad_alloc.
// No captured native
// code is called. All registry/image/configuration aliases are checked before
// any binding is published or any factory is allowed to allocate an owner.
void bind_runtime_owner_factories(NativeStartupSources&, RuntimeOwnerFactoryBindings&);
} //namespace skeet::client
