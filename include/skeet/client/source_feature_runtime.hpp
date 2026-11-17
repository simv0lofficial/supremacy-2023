#pragma once
#include "native_startup.hpp"
#include "skeet/engine/build_material_bindings.hpp"
#include "skeet/engine/build_skin_bindings.hpp"
#include <memory>

namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::animation {
class FixAnimationBackupCleanupContext;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::ui {
class RendererAssets;
class StartupRenderer;
class MainMenuWindowOwner;
class RootMenu;
}
namespace skeet::engine {
class HookRuntimeOwner;
class ClientClassFactoryRuntime;
}
namespace skeet::effects {
class VisualReceiveOwner;
class CameraRuntimeOwner;
}
namespace skeet::skins {
class SkinsRuntimeOwner;
}
namespace skeet::misc {
class SettingsRuntimeOwner;
class ViewmodelRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class CrtRuntimeOwner;
class ConfigurationRuntimeOwner;
class StaticCrtRuntimeOwner;
class WindowRuntimeOwner;
class PlayerRuntimeOwner;
class ClientShutdownOwner;
class HistoryRuntimeOwner;
class LocalAnimationRuntimeOwner;
class EntityCommandRuntimeOwner;
class CommandRuntimeOwner;
class VisualRuntimeOwner;
class MiscRuntimeOwner;
class PlayerReceiveOwner;
class WorldRuntimeOwner;
struct EngineRuntimeBindings;
struct SourceFeatureRuntimeImports;

// Already established process and engine lifetimes at the feature-factory cut.
// No retained executable client, captured callback or alternate data image is
// an input. The exact final initializer remains owned by the startup caller.
struct SourceFeatureInputs {
    ClientRuntimeStorage& storage;
    const EngineRuntimeBindings& engine;
    CrtRuntimeOwner& crt;
    ConfigurationRuntimeOwner& configuration;
    StaticCrtRuntimeOwner& static_crt;
    WindowRuntimeOwner& window;
    PlayerRuntimeOwner& players;
    script::SourceScriptRuntimeOwner& scripts;
    animation::FixAnimationBackupCleanupContext& animation_cleanup;
    ui::RendererAssets& assets;
    ui::StartupRenderer& renderer;
    ui::MainMenuWindowOwner& main_window;
    ConfigStorageOwner& presets;
    ClientShutdownOwner& shutdown;
    engine::BuildMaterialBindings materials;
    engine::BuildSkinBindings skin_engine;
    NativeInitializerCall final_initializer;
};
// One complete, stable assembly of all57 feature factory contexts and the
// shared command/hook adapters. Construction runs none of those factories.
// It is kept alive until hooks and lifecycle owners have been destroyed and
// source terminal cleanup has completed; borrowed early owners outlive it.
class SourceFeatureRuntime final {
public:
    explicit SourceFeatureRuntime(SourceFeatureInputs);
    SourceFeatureRuntime(SourceFeatureInputs, const SourceFeatureRuntimeImports&);
    ~SourceFeatureRuntime();
    SourceFeatureRuntime(const SourceFeatureRuntime&) = delete;
    SourceFeatureRuntime& operator=(const SourceFeatureRuntime&) = delete;
    SourceFeatureRuntime(SourceFeatureRuntime&&) = delete;
    SourceFeatureRuntime& operator=(SourceFeatureRuntime&&) = delete;
    NativeStartupSourceGraph& graph() noexcept;
    void bind_factories(NativeStartupSources&);
    void initialize_features(NativeStartupSources&);
    // After initialize_features and compose_menu, bind the94 entries to their
    // actual ScriptOwner/RootMenu and physical feature contexts.
    void bind_hook_contexts();
    ui::RootMenu& compose_menu();
    engine::HookRuntimeOwner& hooks() noexcept;
    engine::ClientClassFactoryRuntime& class_factories() noexcept;
    HistoryRuntimeOwner& history() noexcept;
    LocalAnimationRuntimeOwner& local_animation() noexcept;
    EntityCommandRuntimeOwner& entity_commands() noexcept;
    CommandRuntimeOwner& commands() noexcept;
    VisualRuntimeOwner& visual() noexcept;
    MiscRuntimeOwner& miscellaneous() noexcept;
    WorldRuntimeOwner& world() noexcept;
    skins::SkinsRuntimeOwner& skins() noexcept;
    misc::SettingsRuntimeOwner& menu_settings() noexcept;
    misc::ViewmodelRuntimeOwner& viewmodel() noexcept;
    effects::CameraRuntimeOwner& camera() noexcept;
    PlayerReceiveOwner& player_receives() noexcept;
    effects::VisualReceiveOwner& visual_receives() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
