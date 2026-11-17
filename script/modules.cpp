#include "skeet/script/modules.hpp"
#include "skeet/script/ui_type_storage.hpp"
#include "skeet/input/runtime.hpp"
#include "skeet/engine/unload.hpp"
#include "skeet/client/shutdown.hpp"
#include "skeet/menu/scripts_menu.hpp"
#include "skeet/misc/logs.hpp"
#include <stdexcept>
#include <utility>

namespace skeet::script {
namespace {
ShutdownState&
initialize_module_shutdown_state(ShutdownState& state,
                                 std::span<const std::byte> current_config) noexcept {
    // The late owner's current-config publication retains its original cut,
    // while the process bridge/UI/close state already has its stable address.
    state.current_config = current_config;
    return state;
}
}

void ScriptModulesOwner::validate_construction(ScriptRuntime& runtime,
                                               const ScriptModulesBindings& bindings) {
    (void)require_uninitialized(runtime, bindings);
}

ScriptRuntime& ScriptModulesOwner::require_uninitialized(ScriptRuntime& runtime,
                                                         const ScriptModulesBindings& bindings) {
    if (!bindings.config.baseline_b.is_unbound() ||
        !bindings.config.baseline_a.same_source(bindings.config.presets.baseline_view()))
        throw std::invalid_argument(
            "Lua owner construction requires its preset baseline and a deferred owner-local stream");
    const auto& image = bindings.prediction_callbacks.image();
    if (&bindings.command_callbacks.image() != &image ||
        &bindings.entity_callbacks.image() != &image || &bindings.ui_types.image() != &image ||
        &bindings.unload_state.owners != &image.native_cell<engine::OwnerRegistry>(0x43475b78u) ||
        &bindings.hud_callbacks.registry !=
            &image.native_cell<engine::OwnerRegistry>(0x43475a9cu) ||
        &bindings.network_callbacks.registry !=
            &image.native_cell<engine::OwnerRegistry>(0x43475b94u) ||
        &bindings.frame_callbacks.registry !=
            &image.native_cell<engine::OwnerRegistry>(0x43475ad8u) ||
        &bindings.update_callbacks.registry !=
            &image.native_cell<engine::OwnerRegistry>(0x43475988u) ||
        &bindings.event_callbacks.registry !=
            &image.native_cell<engine::OwnerRegistry>(0x434758e0u))
        throw std::invalid_argument(
            "Script observer registries must share their retained native image");
    if (runtime.bridge.state || runtime.extensions || runtime.ui_bindings || runtime.ui_module ||
        runtime.json_module || runtime.vector_module || runtime.compat_module)
        throw std::invalid_argument(
            "Lua module composition requires its uninitialized canonical runtime");
    if (!bindings.client.normal_log || !bindings.client.error_log)
        throw std::invalid_argument(
            "Lua modules require the client's concrete normal and error logs");
    if (!bindings.client.cvar)
        throw std::invalid_argument("Lua cvar module requires the resolved engine cvar interface");
    if (&bindings.extensions.process_bridge() != &runtime.bridge ||
        &bindings.extensions.browser_state().zero_callback_storage !=
            &bindings.unload_state.unloading)
        throw std::invalid_argument(
            "Lua process extensions require the same bridge and live unload flag");
    if (!bindings.client.reload_scripts)
        throw std::invalid_argument("Lua frame updates require the canonical reload-request byte");
    if (!bindings.cipher.query)
        throw std::invalid_argument(
            "Lua builtins require their concrete native process cipher inputs");
    if (&bindings.entity.targeting.player_adjustments != &bindings.players)
        throw std::invalid_argument(
            "Lua entity, plist and the PLAYERS menu require the same adjustment records");
    if (bindings.entity.local_player != &bindings.client.local_player ||
        &bindings.entity.game_rules != &bindings.client.game_rules)
        throw std::invalid_argument(
            "Lua client, entity and game events require the same live local-player and game-rules slots");
    if ((bindings.materials.unloading &&
         bindings.materials.unloading != &bindings.unload_state.unloading) ||
        (bindings.materials.activity && bindings.materials.activity != &bindings.input.disabled))
        throw std::invalid_argument(
            "Lua materials require the canonical unload flag and input shutdown word");
    bindings.materials.unloading = &bindings.unload_state.unloading;
    bindings.materials.activity = &bindings.input.disabled;
    if (bindings.client.unsafe_builtin != &bindings.command.marshalling ||
        bindings.globals.choked_commands != &bindings.command.choked_commands)
        throw std::invalid_argument(
            "Lua command and client modules require their shared native flag and choke word");
    if ((bindings.panorama.unsafe_4347582c &&
         bindings.panorama.unsafe_4347582c != &runtime.initializing) ||
        (bindings.panorama.unsafe_43475a5c &&
         bindings.panorama.unsafe_43475a5c != &bindings.command.marshalling))
        throw std::invalid_argument(
            "Lua panorama requires the canonical initializer and command marshalling flags");
    bindings.panorama.unsafe_4347582c = &runtime.initializing;
    bindings.panorama.unsafe_43475a5c = &bindings.command.marshalling;
    if (bindings.ui.window && bindings.ui.window != &bindings.config.window)
        throw std::invalid_argument("Lua UI and configuration must use the same menu window");
    if (bindings.process_shutdown &&
        (&bindings.process_shutdown->bridge != &runtime.bridge ||
         bindings.process_shutdown->ui != &bindings.ui ||
         !bindings.extensions.is_process_shutdown_state(*bindings.process_shutdown)))
        throw std::invalid_argument("Lua modules require the actual early process shutdown state");
    return runtime;
}

CjsonApi ScriptModulesOwner::bind_json(CjsonApi json, const LuaApi& lua) {
    json.lua = lua;
    return json;
}

EntityBindings& ScriptModulesOwner::bind_entity(ScriptModulesBindings& bindings) {
    bind_entity_offsets(bindings.entity, bindings.native_offsets);
    return bindings.entity;
}

ExtensionRegistry& ScriptModulesOwner::bind_registry(ExtensionRegistry& registry,
                                                     ScriptModulesOwner& owner) {
    registry.client = &owner.client_;
    registry.config = &owner.config_;
    registry.cvar = &owner.cvar_;
    registry.database = &owner.database_;
    registry.entity = &owner.entity_;
    registry.globals = &owner.globals_;
    registry.panorama = &owner.panorama_;
    registry.materialsystem = &owner.materialsystem_;
    registry.plist = &owner.plist_;
    registry.renderer = &owner.renderer_;
    registry.ui = &owner.ui_;
    registry.files = &owner.files_;
    return registry;
}

ScriptModulesOwner::ScriptModulesOwner(ScriptRuntime& runtime, ScriptModulesBindings bindings)
    : runtime_(require_uninitialized(runtime, bindings)), client_bindings_(bindings.client),
      command_snapshot_(bindings.command.snapshot), panorama_services_(bindings.panorama),
      configuration_(bindings.config.configuration), window_(bindings.config.window),
      presets_(bindings.config.presets), preset_services_(bindings.config.services),
      identities_(bindings.identities), players_(bindings.players), input_(bindings.input),
      unload_state_(bindings.unload_state), renderer_bindings_(bindings.renderer),
      ui_bindings_(bindings.ui), flags_(bindings.flags),
      material_references_(bindings.extensions.material_references()),
      browser_(bindings.extensions.browser_state()), material_registry_(bindings.material_registry),
      json_(runtime_.bridge, bind_json(bindings.json, bindings.lua)),
      vector_(runtime_.bridge, bindings.vector, runtime_.loading_builtin),
      compat_(runtime_.bridge, bindings.vector.loadbuffer, runtime_.loading_builtin),
      client_(bindings.client, flags_), config_(bindings.config),
      cvar_(bindings.client.cvar, runtime_.builtin_context_ref, bindings.unrestricted_cvars),
      database_(std::move(bindings.database_path)),
      entity_(bind_entity(bindings), bindings.player_cache), globals_(bindings.globals),
      panorama_(bindings.panorama, browser_, runtime_.builtin_context_ref),
      materialsystem_(bindings.materials, &bindings.model_renderer, material_references_,
                      runtime_.builtin_context_ref, material_registry_),
      plist_(bindings.players), renderer_(bindings.renderer),
      ui_(runtime_.bridge, bindings.ui, bindings.config.configuration, bindings.identities,
          bindings.input, bindings.ui_types),
      registry_(bind_registry(bindings.extensions, *this)),
      standalone_shutdown_(bindings.process_shutdown
                               ? std::nullopt
                               : std::optional<ShutdownState>(registry_.make_shutdown_state(
                                     runtime_.bridge, bindings.current_config))),
      shutdown_(initialize_module_shutdown_state(
          bindings.process_shutdown ? *bindings.process_shutdown : *standalone_shutdown_,
          bindings.current_config)),
      manager_(runtime_.bridge, shutdown_, bindings.ui, runtime_, runtime_.builtins, bindings.files,
               bindings.settings, &bindings.persistence),
      command_(runtime_.bridge, bindings.command, bindings.command_callbacks),
      paint_(runtime_.bridge, bindings.hud_callbacks),
      entity_observer_(runtime_.bridge, bindings.entity_callbacks),
      pre_send_(runtime_.bridge, client_, bindings.pre_send, bindings.network_callbacks),
      prediction_(runtime_.bridge, bindings.prediction_callbacks),
      frame_(manager_, *bindings.client.reload_scripts, bindings.frame_callbacks),
      view_(runtime_.bridge, bindings.update_callbacks),
      event_(client_, &client_bindings_.local_player) {
    config_.bind_current_config(shutdown_);
    config_.validate_streams(presets_.baseline_view(), shutdown_);
    runtime_.bridge.api = &bindings.lua;
    runtime_.cipher = bindings.cipher;
    bindings.frame_sequence.bind(runtime_.bridge);
    runtime_.bridge.error_context = bindings.client.error_log;
    runtime_.bridge.report_error = report_error;
    runtime_.extensions = &registry_;
    runtime_.json_module = &json_;
    runtime_.vector_module = &vector_;
    runtime_.compat_module = &compat_;
    runtime_.ui_bindings = &ui_bindings_;
    runtime_.ui_module = &ui_;
    client_bindings_.input = input_.bitmap();
    client_bindings_.initializing = &runtime_.initializing;
    client_bindings_.normal_log->script = &runtime_.bridge;
    manager_.preserve_hotkey_toggle = &bindings.config.presets.preserve_hotkey_toggle;
    events::register_event_observer(bindings.event_callbacks.registry, event_,
                                    bindings.event_callbacks.memory);
    bindings.prediction_callbacks.publish(prediction_); //4347FFC9, after4347FFBB registration
    registry_.attach_shutdown_state(shutdown_);
}

void ScriptModulesOwner::publish_command_hud_entity_tables(
    client::CommandPhaseRegistry& commands, client::EntityObserverRegistry& entities) noexcept {
    command_.publish_table(commands); //4347FFF6, normal43470834
    paint_.publish_table(); //4347FFFD, HUD4346FD9C
    entity_observer_.publish_table(entities); //43480004, entities434729C0
}

void ScriptModulesOwner::report_error(void* context, const char* text) {
    static_cast<misc::ErrorLog*>(context)->format("%s", text);
}

void ScriptModulesOwner::validate_menu(ui::MenuWindow& window, ConfigRegistries& configuration,
                                       ui::WidgetIdentities& identities, players::State& players,
                                       input::Runtime& input) const {
    if (&window != &window_ || &configuration != &configuration_ || &identities != &identities_ ||
        &players != &players_ || &input != &input_)
        throw std::invalid_argument(
            "Root menu and Lua modules require their canonical shared owners");
}

void ScriptModulesOwner::validate_config_binding(ConfigPresetsManager& presets,
                                                 ConfigPresetsServices& services) const {
    if (&presets != &presets_ || &services != &preset_services_)
        throw std::invalid_argument(
            "Lua configuration and the main menu require the same preset manager and services");
    config_.validate_streams(presets.baseline_view(), shutdown_);
}

void ScriptModulesOwner::validate_renderer_binding(ui::RendererContainer& renderer,
                                                   ui::GuiDpiState& dpi) const {
    if (!renderer_bindings_.renderer || *renderer_bindings_.renderer != &renderer ||
        renderer_bindings_.indicator_scale != &dpi.active_scale)
        throw std::invalid_argument(
            "Lua renderer and the main menu require their canonical renderer slot and DPI scale");
}

void ScriptModulesOwner::bind_script_settings(const ui::ScriptsMenuValues& settings) {
    if (client_bindings_.reload_scripts != &settings.reload_requested ||
        (client_bindings_.unsafe_access &&
         client_bindings_.unsafe_access != &settings.allow_unsafe) ||
        (panorama_services_.unsafe_43475798 &&
         panorama_services_.unsafe_43475798 != &settings.allow_unsafe))
        throw std::invalid_argument(
            "Lua modules and the scripts menu require their shared unsafe and reload bytes");
    client_bindings_.unsafe_access = &settings.allow_unsafe;
    panorama_services_.unsafe_43475798 = &settings.allow_unsafe;
}

void ScriptModulesOwner::bind_menu(ui::MenuWindow& window, ConfigRegistries& configuration,
                                   ui::WidgetIdentities& identities, players::State& players,
                                   input::Runtime& input, ui::PlayersMenu& player_menu) {
    validate_menu(window, configuration, identities, players, input);
    if (client_bindings_.players_menu && client_bindings_.players_menu != &player_menu)
        throw std::invalid_argument(
            "Lua client module is already bound to a different PLAYERS menu");
    ui_bindings_.window = &window;
    client_bindings_.players_menu = &player_menu;
}

void ScriptModulesOwner::unbind_menu(ui::MenuWindow& window) noexcept {
    if (&window != &window_)
        return;
    if (ui_bindings_.window == &window)
        ui_bindings_.window = nullptr;
    client_bindings_.players_menu = nullptr;
}

void ScriptModulesOwner::shutdown() {
    manager_.shutdown();
}

void ScriptModulesOwner::bind_shutdown(skeet::client::ShutdownRuntime& finalizer) const {
    if (&finalizer.scripts != &shutdown_ || &finalizer.input != &input_ ||
        &finalizer.scripting_enabled != &runtime_.enabled ||
        &finalizer.hooks.state != &unload_state_)
        throw std::invalid_argument(
            "Client finalization requires the canonical Lua, input and unload owners");
    skeet::client::bind_shutdown_runtime(finalizer);
}

ScriptModulesOwner::~ScriptModulesOwner() noexcept(false) {
    // The process registry survives primary slot0 and its later CRT finalizers.
    if (standalone_shutdown_)
        registry_.detach_shutdown_state(shutdown_);
    // Native43330CA0 frees the owner's callback containers without lua_close.
    // The separate client finalizer calls destroy_runtime while modules live.
    if (client_bindings_.normal_log->script == &runtime_.bridge)
        client_bindings_.normal_log->script = nullptr;
    if (runtime_.bridge.error_context == client_bindings_.error_log &&
        runtime_.bridge.report_error == report_error) {
        runtime_.bridge.report_error = nullptr;
        runtime_.bridge.error_context = nullptr;
    }
    runtime_.extensions = nullptr;
    runtime_.json_module = nullptr;
    runtime_.vector_module = nullptr;
    runtime_.compat_module = nullptr;
    runtime_.ui_bindings = nullptr;
    runtime_.ui_module = nullptr;
    if (registry_.client == &client_)
        registry_.client = nullptr;
    if (registry_.config == &config_)
        registry_.config = nullptr;
    if (registry_.cvar == &cvar_)
        registry_.cvar = nullptr;
    if (registry_.database == &database_)
        registry_.database = nullptr;
    if (registry_.entity == &entity_)
        registry_.entity = nullptr;
    if (registry_.globals == &globals_)
        registry_.globals = nullptr;
    if (registry_.panorama == &panorama_)
        registry_.panorama = nullptr;
    if (registry_.materialsystem == &materialsystem_)
        registry_.materialsystem = nullptr;
    if (registry_.plist == &plist_)
        registry_.plist = nullptr;
    if (registry_.renderer == &renderer_)
        registry_.renderer = nullptr;
    if (registry_.ui == &ui_)
        registry_.ui = nullptr;
    if (registry_.files == &files_)
        registry_.files = nullptr;
}

} // namespace skeet::script
