#include "skeet/script/source_runtime_owner.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/source_terminal_cleanup.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/config_storage/owner.hpp"
#include "skeet/config_storage/preset_runtime.hpp"
#include "skeet/menu/startup_window.hpp"
#include "skeet/menu/startup_data.hpp"
#include "skeet/script/ui_type_storage.hpp"
#include <stdexcept>

namespace skeet::script {
namespace {
client::ClientRuntimeStorage& require_storage(client::ClientRuntimeStorage& storage,
                                              const engine::UnloadState& unload,
                                              const UiTypeStorage& ui_types) {
    const auto memory = storage.memory();
    const auto ui_memory = ui_types.memory();
    if (&unload.owners != &storage.owner_registries().at(0x43475B78u) ||
        &ui_types.image() != &storage.data() || ui_memory.allocate != memory.allocate ||
        ui_memory.free != memory.free || ui_memory.process_heap != memory.process_heap)
        throw std::invalid_argument(
            "Source Lua requires its canonical data, UI types and unload owners");
    return storage;
}
}

SourceScriptRuntimeOwner::SourceScriptRuntimeOwner(client::ClientRuntimeStorage& storage,
                                                   engine::UnloadState& unload,
                                                   UiTypeStorage& ui_types,
                                                   std::span<BuiltinModule> builtins,
                                                   std::span<RequiredModule> required)
    : storage_(require_storage(storage, unload, ui_types)), unload_(unload), ui_types_(ui_types),
      bridge_(storage_.event_names()),
      runtime_(bridge_, source_lua_runtime_bindings().initialization,
               storage_.data().native_cell<bool>(0x434688BDu)),
      browser_(unload_.unloading), material_registry_(native_material_registry_imports()),
      extensions_({bridge_, material_refs_, browser_, storage_.memory()}),
      shutdown_{bridge_, {}, &ui_, {}}, startup_{extensions_, storage_.exit_callbacks()} {
    extensions_.attach_process_shutdown_state(shutdown_, {ui_, flags_});
    bridge_.api = &source_lua_runtime_bindings().lua;
    runtime_.builtins = builtins;
    runtime_.required_modules = required;
    runtime_.cipher = BuiltinCipher::system();
    if (!runtime_.cipher.query)
        throw std::runtime_error("Source Lua requires the native process-times query");
}
SourceScriptRuntimeOwner::~SourceScriptRuntimeOwner() {
    extensions_.detach_shutdown_state(shutdown_);
}
client::SourceTerminalCleanupBindings&
SourceScriptRuntimeOwner::source_terminal_cleanup(client::PlayerRuntimeOwner& players) {
    if (&players.storage() != &storage_)
        throw std::invalid_argument(
            "Source terminal cleanup requires the Lua owner's canonical player storage");
    if (!terminal_)
        terminal_ =
            std::make_unique<client::SourceTerminalCleanupBindings>(storage_, *this, players);
    if (&terminal_->players != &players.cache() || &terminal_->records != &players.records())
        throw std::invalid_argument(
            "Source terminal cleanup was already bound to another player producer");
    return *terminal_;
}

ScriptOwner*& SourceScriptRuntimeOwner::current_owner() const noexcept {
    return storage_.data().native_cell<ScriptOwner*>(0x43475B00u);
}

void SourceScriptRuntimeOwner::bind_modules(SourceScriptModuleInputs source) {
    if (modules_ || factory_ || current_owner() || bridge_.state)
        throw std::logic_error(
            "Source Lua module dependencies have already been bound or constructed");
    auto& data = storage_.data();
    auto& config = source.configuration;
    auto& window = source.main_window;
    if (&config.native_storage() != &storage_.config_storage() ||
        &config.configuration() != &storage_.configuration() ||
        &window.menu_data() != &storage_.menu_startup_data())
        throw std::invalid_argument(
            "Source Lua and CONFIG require the canonical window and configuration storage");
    if (&source.client.local_player != &data.native_cell<void*>(0x43475808u) ||
        &source.client.game_rules != &data.native_cell<void**>(0x4346A3A4u) ||
        source.entity.local_player != &data.native_cell<void*>(0x43475808u) ||
        &source.entity.game_rules != &data.native_cell<void**>(0x4346A3A4u) ||
        &source.entity.player_resource != &data.native_cell<void**>(0x43468FF0u))
        throw std::invalid_argument(
            "Source Lua client and entity views require their live canonical pointer cells");
    const auto engine = data.native_cell<void*>(0x4346A514u);
    const auto entities = data.native_cell<void*>(0x4346A890u);
    const auto cvar = data.native_cell<void*>(0x4346A87Cu);
    if (!engine || !entities || !cvar || source.client.engine != engine ||
        source.client.entity_list != entities || source.client.cvar != cvar ||
        source.entity.engine != engine || source.entity.entity_list != entities ||
        source.globals.engine_client != engine)
        throw std::invalid_argument(
            "Source Lua requires the resolved canonical engine, entity and cvar interfaces");
    const auto choked = data.native_cell<const std::int32_t*>(0x434757DCu);
    const auto sequence = data.native_cell<const std::uint32_t*>(0x434678F8u);
    if (!choked || !sequence || source.globals.choked_commands != choked ||
        &source.frame_sequence.get() != sequence)
        throw std::invalid_argument(
            "Source Lua requires the actual shared choke and frame-sequence words");
    if (!source.globals.variables || !source.globals.server_tick_interval ||
        !source.globals.last_outgoing_command || !source.globals.old_command_ack ||
        !source.globals.command_ack ||
        source.globals.variables != data.native_cell<const GlobalsVariables*>(0x43467B04u) ||
        source.globals.server_tick_interval != data.native_cell<const float*>(0x4346A338u) ||
        source.globals.last_outgoing_command !=
            data.native_cell<const std::int32_t*>(0x43475B2Cu) ||
        source.globals.old_command_ack != data.native_cell<const std::int32_t*>(0x43475794u) ||
        source.globals.command_ack != data.native_cell<const std::int32_t*>(0x43475BA4u))
        throw std::invalid_argument(
            "Source Lua globals require their canonical engine and command pointer cells");
    if (source.command_environment.angle_guard != &data.native_cell<std::uint8_t>(0x434693ECu) ||
        source.command_environment.shared_player != data.native_cell<void**>(0x4346A3A4u))
        throw std::invalid_argument("Source Lua requires the canonical command restriction inputs");
    auto& reload = data.native_cell<std::uint8_t>(0x4347CA28u);
    auto& unsafe = data.native_cell<std::uint8_t>(0x43475798u);
    auto& marshalling = data.native_cell<std::uint8_t>(0x43475A5Cu);
    if ((source.client.reload_scripts && source.client.reload_scripts != &reload) ||
        (source.client.unsafe_access && source.client.unsafe_access != &unsafe) ||
        (source.client.unsafe_builtin && source.client.unsafe_builtin != &marshalling) ||
        (source.panorama.unsafe_43475798 && source.panorama.unsafe_43475798 != &unsafe) ||
        (source.renderer.renderer &&
         source.renderer.renderer != &data.native_cell<ui::RendererContainer*>(0x4347690Cu)) ||
        (source.renderer.indicator_scale &&
         source.renderer.indicator_scale != &storage_.menu_startup_data().dpi.active_scale))
        throw std::invalid_argument(
            "Source Lua requires the canonical script-menu controls, renderer slot and DPI");
    auto& bitmap = data.native_cell<ScriptExecutableBitmap>(0x4346D638u);
    const auto image_base = data.native_cell<std::uintptr_t>(0x43467E70u);
    const auto image_size = data.native_cell<std::uint32_t>(0x434679DCu);
    if (!image_base || image_size < 0x10000u || image_base > 0xffffffffu - image_size ||
        !bitmap.buffer || bitmap.bit_count != 0x8000u)
        throw std::invalid_argument(
            "Source script owner requires the real initialized executable range and bitmap");
    const auto system = native_script_owner_system();
    const auto memory = storage_.memory();
    const auto registry = [&](std::uint32_t address) {
        return engine::OwnerRegistryBinding{storage_.owner_registries().at(address), memory};
    };
    source.client.reload_scripts = &reload;
    source.client.unsafe_access = &unsafe;
    source.client.unsafe_builtin = &marshalling;
    source.panorama.unsafe_43475798 = &unsafe;
    source.renderer.renderer = &data.native_cell<ui::RendererContainer*>(0x4347690Cu);
    source.renderer.indicator_scale = &storage_.menu_startup_data().dpi.active_scale;
    const auto& lua = source_lua_runtime_bindings();
    ConfigBindings configuration{
        storage_.configuration(),
        config.named(),
        window.window(),
        config.runtime(),
        config.presets(),
        ConfigStreamView::native_vector(storage_.config_storage().baseline_cells()),
        ConfigStreamView::unbound(),
        engine,
        source.command_environment};
    ScriptCommandOwnerBindings command{data.native_cell<client::ScriptCommand*>(0x43479300u),
                                       *choked, marshalling, source.command_environment,
                                       storage_.command_phases().services().scheduler_flags};
    PreSendObserverBindings pre_send{
        storage_.event_registry(), data.native_cell<void*>(0x43468E44u),
        data.native_cell<std::uint8_t>(0x434691B8u), data.native_cell<void**>(0x43475B0Cu)};
    //43482538 produces the selected database path during ScriptOwner's real
    // extension process pass. Its pre-initialization UNICODE_STRING is empty.
    modules_.emplace(ScriptModulesBindings{lua.lua,
                                           lua.json,
                                           lua.vector,
                                           runtime_.cipher,
                                           source.client,
                                           flags_,
                                           extensions_,
                                           configuration,
                                           data.native_cell<std::uint8_t>(0x434758D4u),
                                           {},
                                           source.entity,
                                           storage_.offsets(),
                                           source.player_cache,
                                           source.globals,
                                           source.panorama,
                                           source.materials,
                                           source.model_renderer,
                                           material_registry_,
                                           source.players,
                                           source.renderer,
                                           ui_,
                                           ui_types_,
                                           source.identities,
                                           storage_.input_runtime(),
                                           unload_,
                                           source.frame_sequence,
                                           config.native_storage(),
                                           config.persistence(),
                                           system_script_files(),
                                           storage_.menu_startup_data().baseline_b(),
                                           command,
                                           storage_.command_phases(),
                                           registry(0x43475A9Cu),
                                           storage_.entity_observers(),
                                           pre_send,
                                           registry(0x43475B94u),
                                           storage_.prediction_observers(),
                                           registry(0x43475AD8u),
                                           registry(0x43475988u),
                                           registry(0x434758E0u),
                                           &shutdown_});
    ScriptOwnerBindings owner{current_owner(),
                              registry(0x43475B78u),
                              {image_base, image_size, bitmap},
                              system,
                              storage_.exit_callbacks()};
    factory_.emplace(client::ScriptRuntimeFactoryBindings{runtime_, owner, *modules_});
    configuration_ = &config;
}

client::ScriptRuntimeFactoryBindings& SourceScriptRuntimeOwner::factory_bindings() {
    if (!factory_)
        throw std::logic_error("Source Lua feature dependencies are not bound");
    return *factory_;
}
ScriptModulesBindings& SourceScriptRuntimeOwner::module_bindings() {
    if (!modules_)
        throw std::logic_error("Source Lua feature dependencies are not bound");
    return *modules_;
}
ScriptOwner* SourceScriptRuntimeOwner::create_owner() {
    if (creation_started_ || current_owner())
        throw std::logic_error("Source script owner creation has already started");
    auto& factory = factory_bindings();
    creation_started_ = true;
    auto* owner = client::create_script_runtime_owner(factory);
    if (owner)
        bind_manager();
    return owner;
}
ScriptOwner& SourceScriptRuntimeOwner::require_owner() const {
    auto* owner = current_owner();
    if (!owner || &owner->modules().runtime() != &runtime_)
        throw std::logic_error("Source Lua requires its constructed canonical script owner");
    return *owner;
}
ScriptModulesOwner& SourceScriptRuntimeOwner::modules() const {
    return require_owner().modules();
}
ScriptManager& SourceScriptRuntimeOwner::manager() const {
    return modules().manager();
}
void SourceScriptRuntimeOwner::bind_manager() {
    if (!configuration_)
        throw std::logic_error("Source Lua CONFIG dependencies are not bound");
    configuration_->runtime().bind_scripts(configuration_->presets(), manager());
}
} //namespace skeet::script
