#pragma once
#include "skeet/client/combat_runtime_factories.hpp"
#include "skeet/script/lua_runtime_bindings.hpp"
#include "skeet/script/startup_initializers.hpp"
#include <memory>
#include <optional>

namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class PlayerRuntimeOwner;
class SourceTerminalCleanupBindings;
}
namespace skeet::ui {
class MainMenuWindowOwner;
}
namespace skeet::script {

// Already constructed game/feature owners borrowed by the eleven Lua modules.
// Canonical storage, Lua APIs, process extensions, UI records, configuration,
// command state and observer registries are supplied by SourceScriptRuntimeOwner.
struct SourceScriptModuleInputs {
    ConfigStorageOwner& configuration;
    ui::MainMenuWindowOwner& main_window;
    ClientBindings& client;
    EntityBindings& entity;
    client::PlayerStateCache& player_cache;
    GlobalsBindings globals;
    PanoramaServices& panorama;
    MaterialInterfaces& materials;
    effects::ModelRenderRuntime& model_renderer;
    players::State& players;
    RendererBindings renderer;
    ui::WidgetIdentities& identities;
    FrameSequenceBinding frame_sequence;
    client::ScriptCommandEnvironment command_environment;
};

// Stable process lifetime around the actual4348E608 ScriptOwner factory.
// Construct this before CONFIG, which borrows bridge()/ui_bindings(). Bind its
// module inputs after CONFIG and feature owners exist, then create the owner
// at its real startup cut. This object must outlive ScriptOwner, its Lua state,
// RootMenu and all registered process exit callbacks. Its terminal container
// context can be acquired before CRT, independently of later module bindings.
// No CRT initializer, VM,
// handler or feature callback is executed by this context's constructor.
class SourceScriptRuntimeOwner {
public:
    SourceScriptRuntimeOwner(client::ClientRuntimeStorage&, engine::UnloadState&, UiTypeStorage&,
                             std::span<BuiltinModule>, std::span<RequiredModule>);
    ~SourceScriptRuntimeOwner();
    SourceScriptRuntimeOwner(const SourceScriptRuntimeOwner&) = delete;
    SourceScriptRuntimeOwner& operator=(const SourceScriptRuntimeOwner&) = delete;
    SourceScriptRuntimeOwner(SourceScriptRuntimeOwner&&) = delete;
    SourceScriptRuntimeOwner& operator=(SourceScriptRuntimeOwner&&) = delete;

    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    Bridge& bridge() noexcept {
        return bridge_;
    }
    ScriptRuntime& runtime() noexcept {
        return runtime_;
    }
    UiBindings& ui_bindings() noexcept {
        return ui_;
    }
    ReferenceStrings& flags() noexcept {
        return flags_;
    }
    MaterialRefs& material_references() noexcept {
        return material_refs_;
    }
    BrowserState& browser_state() noexcept {
        return browser_;
    }
    MaterialMethodRegistry& material_registry() noexcept {
        return material_registry_;
    }
    ExtensionRegistry& extensions() noexcept {
        return extensions_;
    }
    StartupInitializerBindings& startup_initializers() noexcept {
        return startup_;
    }
    engine::UnloadState& unload_state() const noexcept {
        return unload_;
    }
    UiTypeStorage& ui_types() const noexcept {
        return ui_types_;
    }
    ScriptOwner*& current_owner() const noexcept;
    ShutdownState& shutdown_state() noexcept {
        return shutdown_;
    }
    // Construct once before CRT registration. History and later Lua modules
    // borrow these same process containers after genuine engine acquisition.
    client::SourceTerminalCleanupBindings& source_terminal_cleanup(client::PlayerRuntimeOwner&);

    // Validates identity before publishing the complete typed factory context.
    // A selected database path is produced later by the actual43482538 process
    // initializer; construction does not select or open an alternative file.
    void bind_modules(SourceScriptModuleInputs);
    client::ScriptRuntimeFactoryBindings& factory_bindings();
    ScriptModulesBindings& module_bindings();
    ScriptOwner* create_owner();
    ScriptOwner& require_owner() const;
    ScriptModulesOwner& modules() const;
    ScriptManager& manager() const;
    // If the common combat dispatcher constructs the same factory context,
    // bind CONFIG's script-default view before using presets outside RootMenu.
    void bind_manager();

private:
    client::ClientRuntimeStorage& storage_;
    engine::UnloadState& unload_;
    UiTypeStorage& ui_types_;
    Bridge bridge_;
    ScriptRuntime runtime_;
    UiBindings ui_;
    ReferenceStrings flags_;
    MaterialRefs material_refs_;
    BrowserState browser_;
    MaterialMethodRegistry material_registry_;
    ExtensionRegistry extensions_;
    ShutdownState shutdown_;
    StartupInitializerBindings startup_;
    ConfigStorageOwner* configuration_ = nullptr;
    std::optional<ScriptModulesBindings> modules_;
    std::optional<client::ScriptRuntimeFactoryBindings> factory_;
    std::unique_ptr<client::SourceTerminalCleanupBindings> terminal_;
    bool creation_started_ = false;
};
} //namespace skeet::script
