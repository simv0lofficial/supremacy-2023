#include "skeet/script/api_runtime_owner.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/client/history_runtime_owner.hpp"
#include "skeet/client/visual_runtime_owner.hpp"
#include "skeet/client/misc_runtime_owner.hpp"
#include "skeet/client/hitboxes.hpp"
#include "skeet/animation/runtime_owner.hpp"
#include "skeet/engine/hook_runtime_owner.hpp"
#include "skeet/engine/scene_runtime_owner.hpp"
#include "skeet/engine/netvar_initialization.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/effects/runtime_owner.hpp"
#include "skeet/effects/blur.hpp"
#include "skeet/esp/owner_runtime.hpp"
#include "skeet/misc/runtime.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/render/renderer_assets.hpp"
#include "skeet/render/startup_renderer.hpp"
#include "skeet/menu/startup_window.hpp"
#include "skeet/config_storage/owner.hpp"
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script {
namespace {
void require(bool condition, const char* message) {
    if (!condition)
        throw std::invalid_argument(message);
}
template <class T> T& pointed(engine::NativeDataImage& data, std::uint32_t cell) {
    auto* pointer = data.native_cell<T*>(cell);
    require(pointer != nullptr, "Lua API requires the initialized live engine alias");
    return *pointer;
}
std::ptrdiff_t offset(client::ClientRuntimeStorage& storage, std::uint32_t identity) {
    const auto& globals = storage.offsets().globals;
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.pointer_identity == identity)
            return engine::DecodedOffset(globals, identity, row.xor_key_identity,
                                         row.initial_xor_key, 0);
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.pointer_identity == identity)
            return engine::DecodedOffset(globals, identity, row.xor_key_identity,
                                         row.initial_xor_key, 0);
    struct Derived {
        std::uint32_t pointer, key, salt;
    };
    constexpr std::array derived{Derived{0x43469064u, 0x4346A484u, 0x17978090u},
                                 Derived{0x43467EBCu, 0x43467038u, 0x432B1F5Bu},
                                 Derived{0x43467A88u, 0x43467160u, 0x0D560FACu}};
    for (const auto& row : derived)
        if (row.pointer == identity)
            return engine::DecodedOffset(globals, identity, row.key, row.salt, 0);
    throw std::logic_error("Lua API field has no recovered receive/datamap/derived recipe");
}
std::uint32_t field32(client::ClientRuntimeStorage& storage, std::uint32_t identity) {
    return static_cast<std::uint32_t>(offset(storage, identity));
}
void require_v8(const PanoramaV8Api& v8) {
    require(v8.isolate_enter && v8.isolate_exit && v8.handle_scope_construct &&
                v8.handle_scope_destroy && v8.try_catch_construct && v8.try_catch_destroy &&
                v8.has_caught && v8.exception && v8.create_handle && v8.globalize_reference &&
                v8.dispose_global && v8.context_enter && v8.context_exit && v8.context_global &&
                v8.string_new && v8.string_maybe_new && v8.number_new && v8.object_new &&
                v8.array_new && v8.compile_unbound_script && v8.bind_to_current_context &&
                v8.script_run && v8.object_get && v8.object_set && v8.array_set && v8.array_get &&
                v8.array_length && v8.function_call && v8.object_call && v8.is_function &&
                v8.is_object && v8.is_array && v8.is_callable && v8.is_boolean &&
                v8.boolean_value && v8.is_int32 && v8.is_uint32 && v8.is_number && v8.to_int32 &&
                v8.to_uint32 && v8.to_number && v8.to_object && v8.to_string && v8.int32_value &&
                v8.uint32_value && v8.number_value && v8.utf8_construct && v8.utf8_destroy &&
                v8.cached_data_destroy && v8.process_heap && v8.heap_free,
            "Lua Panorama requires the complete genuine loaded V8 API");
}
client::ClientRuntimeStorage&
checked(client::ClientRuntimeStorage& storage, const client::EngineRuntimeBindings& engine,
        client::ConfigurationRuntimeOwner& configuration, client::PlayerRuntimeOwner& players,
        client::HistoryRuntimeOwner& history, engine::SceneQueryRuntimeOwner& scene,
        animation::RuntimeOwner& animation, client::VisualRuntimeOwner& visual,
        effects::EffectsRuntimeOwner& effects, engine::HookRuntimeOwner& hooks,
        ui::RendererAssets& assets, client::MiscRuntimeOwner& miscellaneous,
        SourceScriptRuntimeOwner& scripts, const ScriptApiRuntimeImports& imports) {
    auto& data = storage.data();
    require(&engine.variables.data() == &data && &configuration.storage() == &storage &&
                &players.storage() == &storage && &history.storage() == &storage &&
                &history.players() == &players && &scene.storage() == &storage &&
                &scene.players() == &players && &animation.storage() == &storage &&
                &animation.players() == &players.cache() && &visual.storage() == &storage &&
                &effects.storage() == &storage && &hooks.storage() == &storage &&
                &assets.storage() == &storage && &miscellaneous.storage() == &storage &&
                &scripts.storage() == &storage &&
                &hooks.unload_state() == &scripts.unload_state() &&
                &assets.startup_renderer() == &visual.startup_renderer(),
            "Lua API requires the canonical engine, feature, renderer, script and process owners");
    const auto recipes = engine::build_interface_manifest();
    for (const auto cell :
         {0x4346A514u, 0x4346A890u, 0x434683E0u, 0x4346A87Cu, 0x43468260u, 0x4346A614u, 0x43468E98u,
          0x4346A4E0u, 0x43468A4Cu, 0x434689C0u, 0x43468364u}) {
        bool found = false;
        for (std::size_t index = 0; index < recipes.size(); ++index) {
            if (recipes[index].destination_cell != cell)
                continue;
            require(data.native_cell<void*>(cell) &&
                        data.native_cell<void*>(cell) == engine.interfaces.objects[index],
                    "Lua API interface is absent from its canonical acquired bundle");
            found = true;
            break;
        }
        require(found, "Lua API interface has no supported-build acquisition recipe");
    }
    auto* globals = data.native_cell<const std::byte*>(0x43467B04u);
    require(globals && data.native_cell<const void*>(0x4346A334u) == globals &&
                data.native_cell<const void*>(0x43467EB8u) == globals + 0x10 &&
                data.native_cell<const void*>(0x4346A338u) == globals + 0x20 &&
                data.native_cell<const void*>(0x43469300u) == globals + 0x18 &&
                data.native_cell<const void*>(0x434678F8u) == globals + 4 &&
                data.native_cell<const void*>(0x43468D4Cu) == globals + 0x1C,
            "Lua API globals and hitbox clocks require their canonical member aliases");
    for (const auto cell :
         {0x43475B2Cu, 0x43475794u, 0x43475BA4u, 0x434757DCu, 0x43469304u, 0x4346A720u, 0x43467DBCu,
          0x43467A68u, 0x43475B90u, 0x43467550u, 0x4346D628u, 0x4346D62Cu})
        require(data.native_cell<void*>(cell) != nullptr,
                "Lua API requires the genuine field/function producers");
    require(storage.offsets().words && storage.offsets().property_proxy_slots[0] &&
                storage.offsets().property_proxy_slots[1],
            "Lua entity API requires the actual receive offsets and proxy fields");
    for (const auto cell : {0x4346747Cu, 0x43467A8Cu, 0x4346A568u})
        require(engine.variables.at(cell).get() != nullptr,
                "Lua API requires its acquired canonical ConVars");
    require(imports.modules.loader && imports.modules.acquire_peb_lock &&
                imports.modules.release_peb_lock && imports.random.seed && imports.random.integer &&
                imports.random.uniform && imports.clock.counter && imports.clock.system_time &&
                imports.clock.time_zone_bias && imports.clock.unix_seconds && imports.arc_tangent2,
            "Lua API requires its genuine loader, RNG, clock and SSE math imports");
    require_v8(imports.v8);
    require(imports.v8.process_heap == storage.memory().process_heap,
            "Lua Panorama and source owners must share the actual process heap");
    require(data.native_cell<ui::RendererContainer*>(0x4347690Cu) ==
                &visual.startup_renderer().renderer(),
            "Lua renderer requires the actual canonical renderer front");
    require(engine.update_bone_attachments == animation.bones().update_attachments &&
                engine.update_bone_attachments ==
                    data.native_cell<animation::BoneAttachmentUpdate>(0x43468F80u),
            "Lua hitboxes require the shared genuine animation attachment entry");
    return storage;
}
ClientOffsets client_fields(client::ClientRuntimeStorage& storage) {
    return {offset(storage, 0x43468E78u),        offset(storage, 0x43468BD4u),
            offset(storage, 0x4346A6CCu),        offset(storage, 0x43468F84u),
            offset(storage, 0x43468A34u),        offset(storage, 0x43467A74u),
            offset(storage, 0x4346908Cu),        offset(storage, 0x434683C8u),
            offset(storage, 0x43469344u) - 0x18, offset(storage, 0x43468378u)};
}
client::ClientHitboxOffsets hitbox_fields(client::ClientRuntimeStorage& storage,
                                          const animation::RuntimeOwner& animation) {
    const auto& fields = animation.bones().fields;
    return {field32(storage, 0x43469198u),
            static_cast<std::uint32_t>(fields.studio_wrapper_32e4daf1),
            static_cast<std::uint32_t>(fields.hitbox_set_c2b6fb10),
            field32(storage, 0x43469064u),
            static_cast<std::uint32_t>(fields.angles_3b1f6ecc),
            static_cast<std::uint32_t>(fields.origin_2a671b42),
            static_cast<std::uint32_t>(fields.origin_4e66bb34),
            static_cast<std::uint32_t>(fields.entity_flags_83a6cf2b),
            field32(storage, 0x43467EBCu),
            field32(storage, 0x43467A88u),
            static_cast<std::uint32_t>(fields.bone_accessor_54783141)};
}
}
ScriptApiRuntimeImports
native_script_api_runtime_imports(const client::EngineRuntimeBindings& engine) {
    const auto vstdlib = GetModuleHandleW(L"vstdlib.dll");
    require(vstdlib && vstdlib == engine.modules.at(u"vstdlib.dll"),
            "Lua RNG requires the already acquired vstdlib module");
    ClientRandom random;
    random.seed = reinterpret_cast<decltype(random.seed)>(GetProcAddress(vstdlib, "RandomSeed"));
    random.integer =
        reinterpret_cast<decltype(random.integer)>(GetProcAddress(vstdlib, "RandomInt"));
    random.uniform =
        reinterpret_cast<decltype(random.uniform)>(GetProcAddress(vstdlib, "RandomFloat"));
    require(random.seed && random.integer && random.uniform,
            "Lua RNG requires genuine vstdlib exports");
    return {random, client_clock_imports(), bind_loaded_panorama_v8_api(),
            engine::native_loaded_module_environment(), animation::native_sse_atan2};
}
struct ScriptApiRuntimeOwner::Data {
    client::ClientRuntimeStorage& storage;
    client::ConfigurationRuntimeOwner& configuration;
    client::PlayerRuntimeOwner& players;
    effects::EffectsRuntimeOwner& effects;
    SourceScriptRuntimeOwner& scripts;
    ScriptApiRuntimeImports imports;
    misc::NormalLog normal;
    misc::ErrorLog& errors;
    client::ClientHitboxRuntime hitbox;
    ClientBindings client;
    EntityBindings entity;
    GlobalsBindings globals;
    PanoramaServices panorama;
    MaterialInterfaces materials;
    RendererBindings renderer;
    Data(client::ClientRuntimeStorage& source, const client::EngineRuntimeBindings& engine,
         client::ConfigurationRuntimeOwner& settings, client::PlayerRuntimeOwner& player_state,
         client::HistoryRuntimeOwner& history, engine::SceneQueryRuntimeOwner& scene,
         animation::RuntimeOwner& animation, client::VisualRuntimeOwner& visual,
         effects::EffectsRuntimeOwner& effect_owner, engine::HookRuntimeOwner& hooks,
         ui::RendererAssets& assets, client::MiscRuntimeOwner& miscellaneous,
         SourceScriptRuntimeOwner& scripting, ScriptApiRuntimeImports services)
        : storage(checked(source, engine, settings, player_state, history, scene, animation, visual,
                          effect_owner, hooks, assets, miscellaneous, scripting, services)),
          configuration(settings), players(player_state), effects(effect_owner), scripts(scripting),
          imports(services), errors(miscellaneous.runtime().error_log),
          hitbox{player_state.cache(),
                 hitbox_fields(source, animation),
                 source.data().native_cell<void*>(0x43468260u),
                 animation.trigonometry(),
                 imports.arc_tangent2,
                 engine.update_bone_attachments,
                 pointed<float>(source.data(), 0x4346A338u),
                 pointed<std::int32_t>(source.data(), 0x43468D4Cu),
                 pointed<float>(source.data(), 0x43467EB8u)},
          client{.local_player = source.data().native_cell<void*>(0x43475808u),
                 .game_rules = source.data().native_cell<void**>(0x4346A3A4u)},
          entity{.game_rules = source.data().native_cell<void**>(0x4346A3A4u),
                 .player_resource = source.data().native_cell<void**>(0x43468FF0u),
                 .targeting = {nullptr, source.data().native_cell<void*>(0x4346A890u),
                               engine.variables.at(0x43467A8Cu), engine.variables.at(0x4346A568u),
                               player_state.adjustments(), history.target_offsets()},
                 .players = EntityPlayerSource::canonical(
                     source.data().native_cell<esp::EspOwnerRuntime*>(0x434758DCu))},
          globals{source.data().native_cell<const GlobalsVariables*>(0x43467B04u),
                  source.data().native_cell<void*>(0x4346A514u),
                  source.data().native_cell<const float*>(0x4346A338u),
                  source.data().native_cell<const std::int32_t*>(0x43475B2Cu),
                  source.data().native_cell<const std::int32_t*>(0x43475794u),
                  source.data().native_cell<const std::int32_t*>(0x43475BA4u),
                  source.data().native_cell<const std::int32_t*>(0x434757DCu)},
          panorama{source.data().native_cell<void*>(0x43468E98u),
                   &source.data().native_cell<std::uint8_t>(0x43475798u),
                   &source.data().native_cell<std::uint8_t>(0x43475A5Cu),
                   &scripting.runtime().initializing,
                   imports.v8,
                   source.data().native_cell<PanoramaBeginEngineCall>(0x4346D628u),
                   source.data().native_cell<PanoramaEndEngineCall>(0x4346D62Cu)},
          materials{source.data().native_cell<void*>(0x4346A514u),
                    source.data().native_cell<void*>(0x4346A4E0u),
                    source.data().native_cell<void*>(0x4346A890u),
                    source.data().native_cell<void*>(0x4346A614u),
                    source.data().native_cell<void*>(0x43468A4Cu),
                    &source.input_runtime().disabled,
                    &scripting.unload_state().unloading,
                    source.data().native_cell<MaterialVariableFactory>(0x43467A68u)},
          renderer{&source.data().native_cell<ui::RendererContainer*>(0x4347690Cu),
                   &assets.icon_cache().image_rasterizer(),
                   &visual.projection(),
                   &source.data().native_cell<effects::BlurEffect*>(0x43475A20u),
                   reinterpret_cast<const bool*>(&settings.hide_from_obs().enabled.value),
                   &visual.startup_renderer().dpi().active_scale} {
        auto& data = source.data();
        normal.cvar = data.native_cell<void*>(0x4346A87Cu);
        normal.script = &scripting.bridge();
        errors.cvar = normal.cvar;
        errors.surface = data.native_cell<void*>(0x434689C0u);
        errors.current_time = &pointed<float>(data, 0x4346A334u);
        misc::bind_log_configuration(settings.hide_from_obs(), errors, normal);
        misc::bind_log_hud(data.native_cell<misc::ColoredLogRing*>(0x434758CCu), errors, normal);
        client.engine = data.native_cell<void*>(0x4346A514u);
        client.entity_list = data.native_cell<void*>(0x4346A890u);
        client.cvar = data.native_cell<void*>(0x4346A87Cu);
        client.debug_overlay = data.native_cell<void*>(0x43468260u);
        client.model_info = data.native_cell<void*>(0x4346A614u);
        client.latency_rate_cvar = engine.variables.at(0x4346747Cu);
        client.offsets = client_fields(source);
        client.input = source.input_runtime().bitmap();
        client.camera_position = &data.native_cell<std::array<float, 3>>(0x43479CCCu);
        client.real_latency_incoming = &data.native_cell<double>(0x4347B118u);
        client.real_latency_outgoing = &data.native_cell<double>(0x4347A500u);
        client.frame_number = data.native_cell<const std::int32_t*>(0x43475B2Cu);
        client.interval_per_tick = data.native_cell<const float*>(0x4346A338u);
        client.engine_interval = &globals.variables->interval_per_tick;
        client.threat_handle = &data.native_cell<std::int32_t>(0x4346DB1Cu);
        client.reload_scripts = &data.native_cell<std::uint8_t>(0x4347CA28u);
        client.full_update = &data.native_cell<std::uint8_t>(0x43475BD2u);
        client.subscriptions_dirty =
            static_cast<std::uint8_t*>(data.native_cell<void*>(0x43468364u)) + 0xA0;
        client.angle_restriction_switch = &settings.anti_untrusted().enabled.value;
        client.console_input = &hooks.console_input();
        client.unsafe_access = panorama.unsafe_43475798;
        client.unsafe_builtin = panorama.unsafe_43475a5c;
        client.initializing = &scripting.runtime().initializing;
        client.loaded_modules = &imports.modules;
        client.random = imports.random;
        client.clock = imports.clock;
        client.set_clan_tag = data.native_cell<misc::SetClanTag>(0x43467550u);
        client.normal_log = &normal;
        client.error_log = &errors;
        client.hud_log =
            ClientHudLogSource::canonical(data.native_cell<misc::ColoredLogRing*>(0x434758CCu));
        client.ballistics = &scene.queries().ballistics();
        client.bullet_filter_vtable = data.native_cell<void**>(0x43475B90u);
        client.hitboxes = &hitbox;
        entity.engine = client.engine;
        entity.entity_list = client.entity_list;
        entity.base_client = data.native_cell<void*>(0x434683E0u);
        entity.local_player = &data.native_cell<void*>(0x43475808u);
        entity.game_rules_proxy_table = data.native_cell<void*>(0x4346A720u);
        entity.max_players = data.native_cell<const std::int32_t*>(0x43469300u);
        entity.projection = &visual.projection();
        entity.esp_activation = &data.native_cell<std::uint8_t>(0x4346A65Cu);
        entity.class_size_interface = data.native_cell<void*>(0x43469304u);
        entity.related_receive_class = data.native_cell<engine::ClientClass*>(0x43467DBCu);
        bind_entity_offsets(entity, source.offsets());
    }
};
ScriptApiRuntimeOwner::ScriptApiRuntimeOwner(
    client::ClientRuntimeStorage& storage, const client::EngineRuntimeBindings& engine,
    client::ConfigurationRuntimeOwner& configuration, client::PlayerRuntimeOwner& players,
    client::HistoryRuntimeOwner& history, engine::SceneQueryRuntimeOwner& scene,
    animation::RuntimeOwner& animation, client::VisualRuntimeOwner& visual,
    effects::EffectsRuntimeOwner& effects, engine::HookRuntimeOwner& hooks,
    ui::RendererAssets& assets, client::MiscRuntimeOwner& miscellaneous,
    SourceScriptRuntimeOwner& scripts)
    : ScriptApiRuntimeOwner(storage, engine, configuration, players, history, scene, animation,
                            visual, effects, hooks, assets, miscellaneous, scripts,
                            native_script_api_runtime_imports(engine)) {}
ScriptApiRuntimeOwner::ScriptApiRuntimeOwner(
    client::ClientRuntimeStorage& storage, const client::EngineRuntimeBindings& engine,
    client::ConfigurationRuntimeOwner& configuration, client::PlayerRuntimeOwner& players,
    client::HistoryRuntimeOwner& history, engine::SceneQueryRuntimeOwner& scene,
    animation::RuntimeOwner& animation, client::VisualRuntimeOwner& visual,
    effects::EffectsRuntimeOwner& effects, engine::HookRuntimeOwner& hooks,
    ui::RendererAssets& assets, client::MiscRuntimeOwner& miscellaneous,
    SourceScriptRuntimeOwner& scripts, ScriptApiRuntimeImports imports)
    : data_(std::make_unique<Data>(storage, engine, configuration, players, history, scene,
                                   animation, visual, effects, hooks, assets, miscellaneous,
                                   scripts, imports)) {}
ScriptApiRuntimeOwner::~ScriptApiRuntimeOwner() = default;
client::ClientRuntimeStorage& ScriptApiRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
ClientBindings& ScriptApiRuntimeOwner::client() noexcept {
    return data_->client;
}
EntityBindings& ScriptApiRuntimeOwner::entity() noexcept {
    return data_->entity;
}
const GlobalsBindings& ScriptApiRuntimeOwner::globals() const noexcept {
    return data_->globals;
}
PanoramaServices& ScriptApiRuntimeOwner::panorama() noexcept {
    return data_->panorama;
}
MaterialInterfaces& ScriptApiRuntimeOwner::materials() noexcept {
    return data_->materials;
}
const RendererBindings& ScriptApiRuntimeOwner::renderer() const noexcept {
    return data_->renderer;
}
misc::NormalLog& ScriptApiRuntimeOwner::normal_log() noexcept {
    return data_->normal;
}
misc::ErrorLog& ScriptApiRuntimeOwner::error_log() noexcept {
    return data_->errors;
}
client::ClientHitboxRuntime& ScriptApiRuntimeOwner::hitboxes() noexcept {
    return data_->hitbox;
}
SourceScriptModuleInputs ScriptApiRuntimeOwner::module_inputs(ConfigStorageOwner& configuration,
                                                              ui::MainMenuWindowOwner& window) {
    auto& source = *data_;
    auto& data = source.storage.data();
    require(&configuration.native_storage() == &source.storage.config_storage() &&
                &configuration.configuration() == &source.storage.configuration() &&
                &window.menu_data() == &source.storage.menu_startup_data() &&
                source.storage.input_runtime().main_menu == &window.window(),
            "Lua API requires the canonical CONFIG and early main window owners");
    return {configuration,
            window,
            source.client,
            source.entity,
            source.players.cache(),
            source.globals,
            source.panorama,
            source.materials,
            source.effects.models(),
            source.players.adjustments(),
            source.renderer,
            source.configuration.widget_identities(),
            FrameSequenceBinding(pointed<std::uint32_t>(data, 0x434678F8u)),
            {&source.configuration.anti_untrusted().enabled.value,
             data.native_cell<void**>(0x4346A3A4u), source.client.offsets.valve_matchmaking}};
}
void ScriptApiRuntimeOwner::bind_modules(ConfigStorageOwner& configuration,
                                         ui::MainMenuWindowOwner& window) {
    data_->scripts.bind_modules(module_inputs(configuration, window));
}
}
