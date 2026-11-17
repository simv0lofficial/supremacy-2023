#pragma once
#include "skeet/prediction/observer_registry.hpp"
#include "skeet/events/event_dispatch.hpp"
#include "skeet/script/cjson.hpp"
#include "skeet/script/client_api.hpp"
#include "skeet/script/command_owner.hpp"
#include "skeet/script/compat.hpp"
#include "skeet/script/config_api.hpp"
#include "skeet/script/cvar_api.hpp"
#include "skeet/script/database.hpp"
#include "skeet/script/entity_api.hpp"
#include "skeet/script/entity_observer.hpp"
#include "skeet/script/files.hpp"
#include "skeet/script/frame_sequence.hpp"
#include "skeet/script/frame_update.hpp"
#include "skeet/script/globals_api.hpp"
#include "skeet/script/manager.hpp"
#include "skeet/script/material_api.hpp"
#include "skeet/script/material_registry.hpp"
#include "skeet/script/paint.hpp"
#include "skeet/script/panorama_api.hpp"
#include "skeet/script/plist_api.hpp"
#include "skeet/script/pre_send_observer.hpp"
#include "skeet/script/registry.hpp"
#include "skeet/script/renderer_api.hpp"
#include "skeet/script/ui_module.hpp"
#include "skeet/script/view_filter.hpp"
#include <optional>

namespace skeet::engine {
struct UnloadState;
}
namespace skeet::client {
class ShutdownRuntime;
}
namespace skeet::ui {
struct ScriptsMenuValues;
}
namespace skeet::script {

// Engine objects and import tables come from the client's actual binding
// phase. Shared process data remains live through GUI unlinking and the last
// exit callback, including after primary slot0 has released this owner. The database
// path is the selected native4347BD48 path, rather than an owner-local default.
struct ScriptModulesBindings {
    const LuaApi& lua;
    CjsonApi json;
    VectorApi vector;
    BuiltinCipher cipher;
    ClientBindings& client;
    ReferenceStrings& flags; //process-global43479118/4346D720
    ExtensionRegistry& extensions; //actual43475944 constructor registrations
    ConfigBindings config;
    const std::uint8_t& unrestricted_cvars; //434758D4
    std::filesystem::path database_path;
    EntityBindings& entity;
    const engine::NativeOffsetStorage& native_offsets;
    skeet::client::PlayerStateCache& player_cache;
    GlobalsBindings globals;
    PanoramaServices& panorama;
    MaterialInterfaces& materials;
    effects::ModelRenderRuntime& model_renderer;
    MaterialMethodRegistry& material_registry; //process owner outlives primary cleanup
    players::State& players;
    RendererBindings renderer;
    UiBindings& ui;
    UiTypeStorage& ui_types; //same image-backed43479670 storage as CRT initialization
    skeet::ui::WidgetIdentities& identities;
    input::Runtime& input;
    engine::UnloadState& unload_state;
    FrameSequenceBinding frame_sequence; //actual word reached through434678F8
    SettingsStateBinding settings;
    SettingsPersistence& persistence;
    ScriptFileSystem& files;
    std::span<const std::byte> current_config;
    ScriptCommandOwnerBindings command;
    skeet::client::CommandPhaseRegistry& command_callbacks; //actual43345EBC registration order
    engine::OwnerRegistryBinding hud_callbacks; //43475A9C/A0/A4
    skeet::client::EntityObserverRegistry& entity_callbacks; //43475BA8/BAC
    PreSendObserverBindings pre_send;
    engine::OwnerRegistryBinding network_callbacks; //43475B94/B98/BC
    prediction::ObserverRegistry& prediction_callbacks; //43475AB0/AB4
    engine::OwnerRegistryBinding frame_callbacks; //43475AD8/ADC/AE0
    engine::OwnerRegistryBinding update_callbacks; //43475988/8C/90
    engine::OwnerRegistryBinding event_callbacks; //434758E0/E4/E8
    // Source production borrows the early process state also consumed by the
    // client finalizer. Explicit standalone module fixtures own their state.
    ShutdownState* process_shutdown = nullptr;
};

// Production lifetime for the eleven recovered modules and the global file
// exports. Every closure points into these owned objects. The shared runtime,
// configuration and UI/input objects are the same ones used by the main menu.
class ScriptModulesOwner {
public:
    ScriptModulesOwner(ScriptRuntime&, ScriptModulesBindings);
    ~ScriptModulesOwner() noexcept(false);
    ScriptModulesOwner(const ScriptModulesOwner&) = delete;
    ScriptModulesOwner& operator=(const ScriptModulesOwner&) = delete;
    static void validate_construction(ScriptRuntime&, const ScriptModulesBindings&);
    //4347FFF6/4347FFFD/43480004 follow the whole owner's primary-table store.
    // Constructors append these physical receivers without publishing them.
    void publish_command_hud_entity_tables(client::CommandPhaseRegistry&,
                                           client::EntityObserverRegistry&) noexcept;

    Bridge& bridge() noexcept {
        return runtime_.bridge;
    }
    ScriptRuntime& runtime() noexcept {
        return runtime_;
    }
    engine::UnloadState& unload_state() noexcept {
        return unload_state_;
    }
    ScriptManager& manager() noexcept {
        return manager_;
    }
    ShutdownState& shutdown_state() noexcept {
        return shutdown_;
    }
    UiBindings& ui_bindings() noexcept {
        return ui_bindings_;
    }
    ReferenceStrings& flags() noexcept {
        return flags_;
    }
    MaterialRefs& material_references() noexcept {
        return material_references_;
    }
    BrowserState& browser_state() noexcept {
        return browser_;
    }
    ClientModule& client() noexcept {
        return client_;
    }
    MaterialMethodRegistry& material_registry() noexcept {
        return material_registry_;
    }
    DatabaseModule& database() noexcept {
        return database_;
    }
    skeet::client::ScriptCommand*& command_snapshot() noexcept {
        return command_snapshot_;
    }
    EntityModule& entity() noexcept {
        return entity_;
    }
    RendererModule& renderer() noexcept {
        return renderer_;
    }
    ExtensionRegistry& extensions() noexcept {
        return registry_;
    }

    // The PLAYERS menu exists only after RootMenu constructs its concrete tab.
    // The remaining identities must match those supplied at module creation.
    void validate_menu(ui::MenuWindow&, ConfigRegistries&, ui::WidgetIdentities&, players::State&,
                       input::Runtime&) const;
    void validate_config_binding(ConfigPresetsManager&, ConfigPresetsServices&) const;
    void validate_renderer_binding(ui::RendererContainer&, ui::GuiDpiState&) const;
    void bind_script_settings(const ui::ScriptsMenuValues&);
    void bind_menu(ui::MenuWindow&, ConfigRegistries&, ui::WidgetIdentities&, players::State&,
                   input::Runtime&, ui::PlayersMenu&);
    void unbind_menu(ui::MenuWindow&) noexcept;
    void bind_shutdown(skeet::client::ShutdownRuntime&) const;
    void shutdown();

private:
    ScriptRuntime& runtime_;
    ClientBindings& client_bindings_;
    skeet::client::ScriptCommand*& command_snapshot_;
    PanoramaServices& panorama_services_;
    ConfigRegistries& configuration_;
    ui::MenuWindow& window_;
    ConfigPresetsManager& presets_;
    ConfigPresetsServices& preset_services_;
    ui::WidgetIdentities& identities_;
    players::State& players_;
    input::Runtime& input_;
    engine::UnloadState& unload_state_;
    RendererBindings renderer_bindings_;
    UiBindings& ui_bindings_;
    ReferenceStrings& flags_;
    MaterialRefs& material_references_;
    BrowserState& browser_;
    MaterialMethodRegistry& material_registry_;
    JsonModule json_;
    VectorModule vector_;
    CompatModule compat_;
    ClientModule client_;
    ConfigModule config_;
    CvarModule cvar_;
    DatabaseModule database_;
    EntityModule entity_;
    GlobalsModule globals_;
    PanoramaModule panorama_;
    MaterialModule materialsystem_;
    PlistModule plist_;
    RendererModule renderer_;
    UiModule ui_;
    FileModule files_;
    ExtensionRegistry& registry_;
    std::optional<ShutdownState> standalone_shutdown_;
    ShutdownState& shutdown_;
    ScriptManager manager_;
    ScriptCommandOwner command_;
    PaintHudHandler paint_;
    ScriptEntityObserver entity_observer_;
    PreSendObserver pre_send_;
    prediction::ScriptPredictionObserver prediction_;
    ScriptFrameUpdate frame_;
    ScriptViewObserver view_;
    events::LuaEventObserver event_;

    static ScriptRuntime& require_uninitialized(ScriptRuntime&, const ScriptModulesBindings&);
    static CjsonApi bind_json(CjsonApi, const LuaApi&);
    static EntityBindings& bind_entity(ScriptModulesBindings&);
    static ExtensionRegistry& bind_registry(ExtensionRegistry&, ScriptModulesOwner&);
    static void report_error(void*, const char*);
};

} // namespace skeet::script
