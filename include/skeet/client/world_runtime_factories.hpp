#pragma once
#include "skeet/effects/harmless_molotov_owner.hpp"
#include "skeet/effects/scope_owner.hpp"
#include "skeet/effects/smoke_owner.hpp"
#include "skeet/engine/game_system.hpp"
#include "skeet/events/hit_marker_owner.hpp"
#include "skeet/input/native_window_lifecycle.hpp"
#include "skeet/misc/auto_accept_owner.hpp"
#include "skeet/misc/disable_sv_pure_owner.hpp"
#include "skeet/misc/persistent_kill_feed.hpp"
#include "skeet/misc/reveal_overwatch.hpp"
#include "skeet/misc/reveal_ranks.hpp"

namespace skeet::client {
class NativeStartupSources;
class CommandPhaseRegistry;

struct GameSystemFactoryContext {
    engine::GameSystemManager* const& manager; //live43467FB0, captured by the constructor
    void* const* allocator; //address of live43467204
    engine::GameSystemRuntime& runtime;
};
struct RevealRanksFactoryContext {
    misc::RevealRanksConfig& settings;
    misc::RevealRanksRuntime*& active; //canonical434758F0 source projection
    misc::RevealRanksBindings engine;
};
struct RevealOverwatchFactoryContext {
    misc::RevealOverwatchConfig& settings;
    void* const& engine; //4346A514
};
struct DisableSvPureFactoryContext {
    misc::DisablePureConfig& settings; //same177AB88C value as GameSystemRuntime
    misc::DisableSvPureOwner*& active; //43475974
};
struct GameSystemRegistrationFactoryContext {
    engine::GameSystemRegistrationNode** const* head_slot; //address of43468D50
    const engine::OwnerRegistry& callbacks; //43475A84/88/8C
    engine::UnloadState& unload;
};
struct PersistentKillFeedFactoryContext {
    misc::PersistentKillFeedConfig& settings;
    misc::PersistentKillFeedBindings& engine;
};
struct HitMarkerFactoryContext {
    events::EventRegistry& events; //the existing43478DD0 permanent-listener registry
    events::HitMarkerOwnerBindings bindings;
};
struct NativeWindowFactoryContext {
    SettingsPersistence& persistence;
    input::NativeWindowLifecycleImports imports;
    engine::LoadedModuleEnvironment modules;
    engine::NativeImageEnvironment images;
};

// These are borrowed canonical registries/settings and existing complete
// constructor contexts. Keep this aggregate and its referents alive until
// lifecycle destruction has finished. Descriptor initialization belongs to
// each whole constructor; do not initialize a second feature configuration.
struct WorldRuntimeFactoryBindings {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding updates; //43475988
    engine::OwnerRegistryBinding hud; //43475A9C
    engine::OwnerRegistryBinding frames; //43475AD8
    engine::OwnerRegistryBinding event_observers; //434758E0
    engine::OwnerRegistryBinding messages; //43475B94
    CommandPhaseRegistry& command_owners; //canonical434757A0/A4/A8
    ConfigRegistries& configuration;
    effects::SmokeOwnerBindings smoke;
    effects::ScopeOwnerBindings scope;
    GameSystemFactoryContext game_system;
    misc::AutoAcceptOwnerBindings auto_accept;
    RevealRanksFactoryContext reveal_ranks;
    RevealOverwatchFactoryContext reveal_overwatch;
    DisableSvPureFactoryContext disable_sv_pure;
    GameSystemRegistrationFactoryContext game_system_registration;
    PersistentKillFeedFactoryContext persistent_kill_feed;
    HitMarkerFactoryContext hit_markers;
    NativeWindowFactoryContext window_lifecycle;
    effects::HarmlessMolotovOwnerBindings harmless_molotov;
};

// Twelve exact factory identities. Runtime-owner initialization supplies their
// order; this only binds ordinary source implementations. The game-window
// proxy at4347D94F has its separate concrete GameWindowProxyFactoryBindings.
void bind_world_runtime_factories(NativeStartupSources&, WorldRuntimeFactoryBindings&);
} //namespace skeet::client
