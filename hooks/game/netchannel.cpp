#include "util/address.h"
// Game Hook: netchannel
#include "skeet/network/active_sound_retention.hpp"
#include "skeet/network/client_state_cvars.hpp"
#include "skeet/engine/executable_pattern.hpp"
#include <array>
#include <cstring>
#include "skeet/network/client_state_hook_bindings.hpp"
#include "skeet/engine/unload.hpp"
#include "skeet/network/client_state_hook_family.hpp"
#include "skeet/network/client_state_hooks.hpp"
#include "skeet/network/tick_update_dispatch.hpp"
#include "skeet/network/incoming_update.hpp"
#include "skeet/network/shared_esp.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/effects/world.hpp"
#include "skeet/events/event_hook.hpp"
#include "skeet/network/client_state_runtime_owner.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/client/history_runtime_owner.hpp"
#include "skeet/client/command_capture_runtime_owner.hpp"
#include "skeet/client/startup_bindings.hpp"
#include "skeet/engine/hook_runtime_owner.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/menu/root_menu.hpp"
#include "skeet/menu/startup_window.hpp"
#include "skeet/script/source_runtime_owner.hpp"
#include "skeet/script/owner.hpp"
#include <stdexcept>
#include <windows.h>
#include "skeet/script/cvar_api.hpp"
#include <xmmintrin.h>
#include "skeet/network/command_owner.hpp"
#include "skeet/network/packet_duck_live.hpp"
#include "skeet/movement/simulation_environment.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/client/target_refresh_runtime_owner.hpp"
#include <bit>
#include <intrin.h>
#include "skeet/network/data_update_observers.hpp"
#include "skeet/network/exploit_acknowledgement.hpp"
#include "skeet/client/entity_command_owner.hpp"
#include "skeet/client/prediction_history_owner.hpp"
#include "skeet/skins/native_agents.hpp"
#include <immintrin.h>
#include <limits>
#include "skeet/network/exploit_command_owner.hpp"
#include "skeet/network/exploit_phase1.hpp"
#include "skeet/network/exploit_phase7.hpp"
#include "skeet/network/exploit_message.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/network/exploit_observers.hpp"
#include "skeet/misc/viewmodel_lag.hpp"
#include "skeet/rage/shot_feedback.hpp"
#include "skeet/network/exploit_owner.hpp"
#include <new>
#include "skeet/network/exploit_controls.hpp"
#include "skeet/misc/grenade_owner.hpp"
#include <algorithm>
#include "skeet/network/exploit_phase_state.hpp"
#include "skeet/anti_aim/config.hpp"
#include "skeet/anti_aim/movement_capture.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/client/command_angle_retention.hpp"
#include "skeet/client/command_capture.hpp"
#include "skeet/client/command_recapture.hpp"
#include "skeet/client/command_transform.hpp"
#include "skeet/client/generated_command_owners.hpp"
#include "skeet/client/restricted_player_command.hpp"
#include "skeet/movement/rage_stop.hpp"
#include "skeet/prediction/observer_registry.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/script/lua_runtime_bindings.hpp"
#include "skeet/network/exploit_runtime.hpp"
#include "skeet/misc/ping_spike_channel.hpp"
#include "skeet/esp/owner_runtime.hpp"
#include "skeet/network/packet_control.hpp"
#include <cmath>
#include "skeet/network/packet_dispatch.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/esp/packet_tracers.hpp"
#include "skeet/prediction/history.hpp"
#include "packet_duck_body.hpp"
#include "skeet/network/packet_owner.hpp"
#include "skeet/network/packet_scheduler.hpp"
#include "skeet/network/packet_world.hpp"
#include "skeet/network/receive_float_proxy.hpp"
#include "skeet/engine/native_data_image.hpp"
#include <cstdint>
#include <emmintrin.h>
#include "skeet/esp/player_runtime.hpp"
#include "skeet/misc/runtime.hpp"
#include "skeet/movement/steering_math.hpp"
#include "skeet/network/startup_user_messages.hpp"
#include "skeet/network/user_message_runtime.hpp"
#include "skeet/rage/feedback_owner.hpp"
#include "skeet/script/say_text_message.hpp"
#include "skeet/esp/user_message_handlers.hpp"
#include "skeet/network/update_observers.hpp"


// ============================================================================
// Subcomponent: active_sound_retention.cpp
// ============================================================================

namespace skeet::network {
using skeet::field;
using skeet::store;
using skeet::add;
using skeet::subtract;

namespace {
using Tree = esp::ActiveSoundTree;
using Node = esp::ActiveSoundNode;
Node& node(Tree& tree, std::int32_t index) noexcept {
    return const_cast<Node&>(tree.nodes[index]);
}
std::int32_t left(const Tree& tree, std::int32_t index) noexcept {
    return index == -1 ? -1 : tree.nodes[index].left; //433B91A5, nil4347262C
}
std::int32_t right(const Tree& tree, std::int32_t index) noexcept {
    return index == -1 ? -1 : tree.nodes[index].right; //433C82B6
}
std::int32_t parent(const Tree& tree, std::int32_t index) noexcept {
    return index == -1 ? -1 : tree.nodes[index].parent; //4341A66B
}
std::uint32_t color(const Tree& tree, std::int32_t index) noexcept {
    return index == -1 ? 1u : tree.nodes[index].unused_0c; //43340117
}
bool black(const Tree& tree, std::int32_t index) noexcept {
    return color(tree, index) == 1u; //433F4A70 uses equality, not nonzero
}
void rotate_left(Tree& tree, std::int32_t index) noexcept { //433ED2B2
    const auto child = right(tree, index);
    node(tree, index).right = left(tree, child);
    if (left(tree, child) != -1)
        node(tree, left(tree, child)).parent = index;
    if (child != -1)
        node(tree, child).parent = parent(tree, index);
    if (index == tree.root)
        tree.root = child;
    else if (left(tree, parent(tree, index)) == index)
        node(tree, parent(tree, index)).left = child;
    else
        node(tree, parent(tree, index)).right = child;
    node(tree, child).left = index;
    if (index != -1)
        node(tree, index).parent = child;
}
void rotate_right(Tree& tree, std::int32_t index) noexcept { //433F1EEC
    const auto child = left(tree, index);
    node(tree, index).left = right(tree, child);
    if (right(tree, child) != -1)
        node(tree, right(tree, child)).parent = index;
    if (child != -1)
        node(tree, child).parent = parent(tree, index);
    if (index == tree.root)
        tree.root = child;
    else if (right(tree, parent(tree, index)) == index)
        node(tree, parent(tree, index)).right = child;
    else
        node(tree, parent(tree, index)).left = child;
    node(tree, child).right = index;
    if (index != -1)
        node(tree, index).parent = child;
}
void fix_removed_black(Tree& tree, std::int32_t index) noexcept { //4334025F
    while (index != tree.root && black(tree, index)) {
        auto p = parent(tree, index);
        if (index == left(tree, p)) {
            auto sibling = right(tree, p);
            if (color(tree, sibling) == 0) {
                node(tree, sibling).unused_0c = 1;
                node(tree, p).unused_0c = 0;
                rotate_left(tree, p);
                p = parent(tree, index);
                sibling = right(tree, p);
            }
            if (black(tree, left(tree, sibling)) && black(tree, right(tree, sibling))) {
                if (sibling != -1)
                    node(tree, sibling).unused_0c = 0;
                index = p;
            } else {
                if (black(tree, right(tree, sibling))) {
                    node(tree, left(tree, sibling)).unused_0c = 1;
                    node(tree, sibling).unused_0c = 0;
                    rotate_right(tree, sibling);
                    p = parent(tree, index);
                    sibling = right(tree, p);
                }
                node(tree, sibling).unused_0c = color(tree, p);
                node(tree, p).unused_0c = 1;
                node(tree, right(tree, sibling)).unused_0c = 1;
                rotate_left(tree, p);
                index = tree.root;
            }
        } else {
            auto sibling = left(tree, p);
            if (color(tree, sibling) == 0) {
                node(tree, sibling).unused_0c = 1;
                node(tree, p).unused_0c = 0;
                rotate_right(tree, p);
                p = parent(tree, index);
                sibling = left(tree, p);
            }
            if (black(tree, right(tree, sibling)) && black(tree, left(tree, sibling))) {
                if (sibling != -1)
                    node(tree, sibling).unused_0c = 0;
                index = p;
            } else {
                if (black(tree, left(tree, sibling))) {
                    node(tree, right(tree, sibling)).unused_0c = 1;
                    node(tree, sibling).unused_0c = 0;
                    rotate_left(tree, sibling);
                    p = parent(tree, index);
                    sibling = left(tree, p);
                }
                node(tree, sibling).unused_0c = color(tree, p);
                node(tree, p).unused_0c = 1;
                node(tree, left(tree, sibling)).unused_0c = 1;
                rotate_right(tree, p);
                index = tree.root;
            }
        }
    }
    node(tree, index).unused_0c = 1;
}
void unlink(Tree& tree, std::int32_t index) noexcept { //complete43359947
    if (index == -1)
        return;
    auto removed = index;
    if (left(tree, index) != -1 && right(tree, index) != -1) {
        removed = right(tree, index);
        while (left(tree, removed) != -1)
            removed = left(tree, removed);
    }
    const auto child = left(tree, removed) != -1 ? left(tree, removed) : right(tree, removed);
    if (child != -1)
        node(tree, child).parent = parent(tree, removed);
    if (removed == tree.root)
        tree.root = child;
    else if (left(tree, parent(tree, removed)) == removed)
        node(tree, parent(tree, removed)).left = child;
    else
        node(tree, parent(tree, removed)).right = child;
    const auto removed_color = color(tree, removed);
    if (removed != index) {
        node(tree, removed).parent = parent(tree, index);
        node(tree, removed).right = right(tree, index);
        node(tree, removed).left = left(tree, index);
        if (index == tree.root)
            tree.root = removed;
        else if (left(tree, parent(tree, index)) == index)
            node(tree, parent(tree, index)).left = removed;
        else
            node(tree, parent(tree, index)).right = removed;
        if (left(tree, removed) != -1)
            node(tree, left(tree, removed)).parent = removed;
        if (right(tree, removed) != -1)
            node(tree, right(tree, removed)).parent = removed;
        node(tree, removed).unused_0c = color(tree, index);
    }
    // Native does not run the fixup for a nil child. Preserve that branch.
    if (child != -1 && removed_color == 1)
        fix_removed_black(tree, child);
}
} //namespace
std::int32_t first_active_sound(const esp::ActiveSoundTree& tree) noexcept {
    auto index = tree.root;
    while (left(tree, index) != -1)
        index = left(tree, index);
    return index;
}
std::int32_t next_active_sound(const esp::ActiveSoundTree& tree, std::int32_t index) noexcept {
    if (std::uint32_t(index) >= tree.allocation_count || index > tree.last_allocated ||
        left(tree, index) == index)
        return -1;
    if (right(tree, index) != -1) {
        index = right(tree, index);
        while (left(tree, index) != -1)
            index = left(tree, index);
        return index;
    }
    auto p = parent(tree, index);
    while (p != -1 && right(tree, p) == index) {
        index = p;
        p = parent(tree, p);
    }
    return p;
}
void release_active_sound(esp::ActiveSoundTree& tree, std::int32_t index) noexcept {
    unlink(tree, index);
    node(tree, index).left = index;
    node(tree, index).right = static_cast<std::int32_t>(tree.unused_18);
    --tree.count;
    tree.unused_18 = static_cast<std::uint32_t>(index);
}
} //namespace skeet::network

// ============================================================================
// Subcomponent: client_state_cvars.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* [dedup field] */
struct CvarArguments {
    const char* name;
    const char* value;
    const ServerCvarEventNames& names;
};
int marshal_cvar(script::Bridge& bridge, void* pointer) { //complete4339C702
    const auto& args = *static_cast<const CvarArguments*>(pointer);
    const auto& api = *bridge.api;
    auto* const state = bridge.state;
    api.createtable(state, 0, 2);
    api.pushlstring(state, args.names.name.get(), 8);
    api.pushstring(state, args.name);
    api.settable(state, -3);
    api.pushlstring(state, args.names.value.get(), 9);
    api.pushstring(state, args.value);
    api.settable(state, -3);
    return 1;
}
}
void observe_server_cvars(ClientStateServerCvars& runtime, const void* message) {
    if (!runtime.bridge.state || !(field<std::uint8_t>(message, 0x10) & 1) ||
        !field<void*>(message, 8))
        return;
    if (!runtime.lookup_attempted && !runtime.name_getter) {
        runtime.lookup_attempted = 1;
        constexpr std::array<std::uint8_t, 6> pattern{0xF6, 0x41, 0x18, 0x01, 0x74, 0x10};
        runtime.name_getter = reinterpret_cast<ServerCvarNameGetter>(
            engine::executable_pattern(runtime.engine_module, pattern.data(), pattern.size()));
    }
    if (!runtime.name_getter)
        return;
    auto* const list = field<const void*>(message, 8);
    const auto count = field<std::int32_t>(list, 0xC);
    for (std::int32_t index = 0; index < count; ++index) {
        auto* entry =
            field<void*>(field<const void*>(list, 8), std::ptrdiff_t(index) * sizeof(void*));
        const auto* name = runtime.name_getter(entry);
        const void* value = nullptr;
        if (field<std::uint32_t>(entry, 0x18) & 2) {
            auto* alternate = field<const void*>(entry, 0xC);
            if (field<std::uint32_t>(alternate, 0x10))
                value = alternate;
        }
        if (!value)
            value = field<const void*>(entry, 8);
        const auto* bytes = field<std::uint32_t>(value, 0x14) < 16 ? static_cast<const char*>(value)
                                                                   : field<const char*>(value, 0);
        if (!name || !*name || !bytes || !*bytes)
            return;
        CvarArguments args{name, bytes, runtime.names};
        script::dispatch_callbacks(runtime.bridge, runtime.names.event.get(), marshal_cvar, &args);
        // The actual4347CA4C slot is reread on the next iteration.
    }
}
} //namespace skeet::network

// ============================================================================
// Subcomponent: client_state_hooks.cpp
// ============================================================================

namespace skeet::network {
namespace {
ClientStateHooksRuntime* client_state_runtime;
template <class Result, class... Args>
Result original(void* object, std::size_t slot, Args... args) {
    auto** table = *static_cast<void***>(object);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    return reinterpret_cast<engine::MemberFunction<Result, Args...>>(
        metadata->original_table[slot])(object, args...);
}
/* [dedup field] */
}
bool ClientStateHooksRuntime::slot2(void* object, std::uintptr_t incoming) {
    const bool result = original<bool>(object, 2, incoming);
    client::restore_channel_hook(bindings.engine_client, bindings.channel);
    return result;
}
std::uintptr_t ClientStateHooksRuntime::slot15(void* object, std::uintptr_t incoming) {
    observe_server_cvars(bindings.server_cvars, reinterpret_cast<const void*>(incoming));
    return original<std::uintptr_t>(object, 15, incoming);
}
std::uintptr_t ClientStateHooksRuntime::slot32(void* object, std::uintptr_t incoming) {
    const auto* message = reinterpret_cast<const void*>(incoming);
    const auto identifier =
        (field<std::uint8_t>(message, 0x18) & 1) ? field<std::int32_t>(message, 0xC) : -1;
    auto* patch = bindings.scoped_patches;
    while (patch && patch->identifier != identifier)
        patch = patch->next;
    if (patch && patch->target) {
        patch->saved = *patch->target;
        *patch->target = patch;
    }
    const auto result = original<std::uintptr_t>(object, 32, incoming);
    if (patch && patch->target)
        *patch->target = patch->saved;
    return result;
}
void bind_client_state_hooks_runtime(ClientStateHooksRuntime& value) noexcept {
    client_state_runtime = &value;
}
#if defined(_M_IX86) || defined(__i386__)
#define CLIENT_STATE_FAST __fastcall
#else
#define CLIENT_STATE_FAST
#endif
bool CLIENT_STATE_FAST client_state_slot2_hook(void* object, void*, std::uintptr_t incoming) {
    return client_state_runtime->slot2(object, incoming);
}
std::uintptr_t CLIENT_STATE_FAST client_state_slot12_hook(void* object, void*,
                                                          std::uintptr_t incoming) {
    return original<std::uintptr_t>(object, 12, incoming);
}
std::uintptr_t CLIENT_STATE_FAST client_state_slot15_hook(void* object, void*,
                                                          std::uintptr_t incoming) {
    return client_state_runtime->slot15(object, incoming);
}
std::uintptr_t CLIENT_STATE_FAST client_state_slot25_hook(void* object, void*,
                                                          std::uintptr_t incoming) {
    return original<std::uintptr_t>(object, 25, incoming);
}
std::uintptr_t CLIENT_STATE_FAST client_state_slot32_hook(void* object, void*,
                                                          std::uintptr_t incoming) {
    return client_state_runtime->slot32(object, incoming);
}
bool CLIENT_STATE_FAST client_state_slot47_hook(void* object, void*, std::int32_t state,
                                                std::uintptr_t second, std::uintptr_t third) {
    return client_state_runtime->slot47(object, state, second, third);
}
#undef CLIENT_STATE_FAST
} //namespace skeet::network

// ============================================================================
// Subcomponent: client_state_hook_family.cpp
// ============================================================================

namespace skeet::network {
std::span<const ClientStateSourceReplacement, 13> client_state_source_replacements() {
    static const std::array<ClientStateSourceReplacement, 13> entries{
        {{client_state_native_replacements[0], reinterpret_cast<void*>(&client_state_slot2_hook)},
         {client_state_native_replacements[1], reinterpret_cast<void*>(&incoming_tick_hook)},
         {client_state_native_replacements[2], reinterpret_cast<void*>(&incoming_update_hook)},
         {client_state_native_replacements[3], reinterpret_cast<void*>(&client_state_slot12_hook)},
         {client_state_native_replacements[4],
          reinterpret_cast<void*>(&effects::flash_effect_hook)},
         {client_state_native_replacements[5], reinterpret_cast<void*>(&client_state_slot15_hook)},
         {client_state_native_replacements[6],
          reinterpret_cast<void*>(&process_voice_message_hook)},
         {client_state_native_replacements[7], reinterpret_cast<void*>(&client_state_slot25_hook)},
         {client_state_native_replacements[8], reinterpret_cast<void*>(&client_state_slot32_hook)},
         {client_state_native_replacements[9], reinterpret_cast<void*>(&events::event_packet_hook)},
         {client_state_native_replacements[10], reinterpret_cast<void*>(&update_status_hook)},
         {client_state_native_replacements[11], reinterpret_cast<void*>(&client_state_slot47_hook)},
         {client_state_native_replacements[12],
          reinterpret_cast<void*>(&client::entity_update_hook)}}};
    return entries;
}

ClientStateHookFamily::ClientStateHookFamily(engine::HookDescriptorStorage& storage,
                                             engine::VtableHookRegistry& hooks,
                                             engine::NativeImageEnvironment images,
                                             engine::HookDescriptorSystem system,
                                             ClientStateHookRuntimes runtimes)
    : metadata_(storage.at(client_state_descriptor_identity)), hooks_(hooks),
      engine_module_(storage.image().native_cell<std::byte*>(client_state_module_identity)),
      images_(images), system_(system), runtimes_(runtimes) {}

void ClientStateHookFamily::prepare() {
    engine::prepare_rtti_vtable_hook(metadata_, engine_module_, client_state_rtti_hash,
                                     client_state_rtti_base_hash, images_, hooks_.memory, system_);
    bind_replacements();
}

void ClientStateHookFamily::bind_replacements() {
    // Publish the actual source owners before4331E834 can install their table,
    // including its observed cleanup-node allocation failure path.
    bind_client_state_hooks_runtime(runtimes_.core);
    bind_tick_update_runtime(runtimes_.ticks);
    bind_incoming_update_runtime(runtimes_.incoming);
    effects::bind_flash_effect_runtime(runtimes_.effects);
    bind_shared_esp_runtime(runtimes_.shared_esp);
    events::bind_event_packet_hook_runtime(runtimes_.events);
    client::bind_entity_update_runtime(runtimes_.entities);
    for (const auto& replacement : client_state_source_replacements())
        metadata_.replacement_table[replacement.native.slot] = replacement.source_entry;
}

engine::VtableHookNode* ClientStateHookFamily::install(void* actual_receiver) {
    return engine::install_vtable_hook(hooks_, actual_receiver, metadata_);
}
}

// ============================================================================
// Subcomponent: client_state_runtime_owner.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::network {
namespace {
template <class T> T& pointed(engine::NativeDataImage& data, std::uint32_t identity) {
    auto* pointer = data.native_cell<T*>(identity);
    if (!pointer)
        throw std::invalid_argument(
            "ClientState contexts require the initialized live engine alias");
    return *pointer;
}
engine::DecodedOffset rule_guard(client::ClientRuntimeStorage& storage) {
    return {storage.offsets().globals, 0x43468378, 0x43468244, 0xBF567619, 0};
}
misc::ErrorLog& errors(script::SourceScriptRuntimeOwner& scripts) {
    auto* log = scripts.module_bindings().client.error_log;
    if (!log)
        throw std::invalid_argument(
            "ClientState contexts require the actual shared script error log");
    return *log;
}
client::ClientRuntimeStorage&
checked_storage(client::ClientRuntimeStorage& storage, const client::EngineRuntimeBindings& engine,
                client::ConfigurationRuntimeOwner& configuration,
                client::HistoryRuntimeOwner& history, client::CommandCaptureRuntimeOwner& commands,
                engine::HookRuntimeOwner& hooks, script::SourceScriptRuntimeOwner& scripts,
                ui::MainMenuWindowOwner& window, ClientStateRuntimeImports imports) {
    auto& data = storage.data();
    const auto core = engine.interfaces.core();
    auto& modules = scripts.module_bindings();
    auto& menu = window.controller();
    if (&configuration.storage() != &storage || &history.storage() != &storage ||
        &commands.storage() != &storage || &hooks.storage() != &storage ||
        &scripts.storage() != &storage || &history.players().storage() != &storage ||
        &scripts.unload_state() != &hooks.unload_state() ||
        &modules.player_cache != &history.players().cache() ||
        &modules.players != &history.players().adjustments() ||
        &modules.native_offsets != &storage.offsets() || &engine.variables.data() != &data ||
        &menu.script_owner() != &scripts.require_owner() ||
        &scripts.require_owner() != &hooks.script_owner() || &menu.window() != &window.window() ||
        data.native_cell<script::ScriptOwner*>(0x43475B00) != &scripts.require_owner() ||
        core.engine_client != data.native_cell<void*>(0x4346A514) ||
        core.entity_list != data.native_cell<void*>(0x4346A890) ||
        &commands.sent_history().angles != &data.native_cell<std::array<float, 3>>(0x43476414) ||
        &commands.sent_history().records !=
            &data.native_cell<std::array<client::SentCommandRecord, 150>>(0x43476450))
        throw std::invalid_argument(
            "ClientState contexts must share the actual menu, script, history and hook owners");
    if (!imports.exchange || !core.engine_client || !core.entity_list ||
        !data.native_cell<void*>(0x43468E44) || !data.native_cell<std::byte*>(0x43468D20) ||
        !engine.variables.at(0x43469238).get())
        throw std::invalid_argument(
            "ClientState contexts require their genuine engine objects, ConVar and exchange import");
    (void)hooks.channel();
    (void)errors(scripts);
    (void)std::ptrdiff_t(rule_guard(storage));
    if (&history.entity_runtime().interval != &pointed<float>(data, 0x4346A338))
        throw std::invalid_argument(
            "ClientState and history must share the same live engine interval");
    return storage;
}
}
ClientStateRuntimeImports native_client_state_runtime_imports() {
    const auto module = GetModuleHandleW(L"kernel32.dll");
    const auto exchange = module ? GetProcAddress(module, "InterlockedExchange") : nullptr;
    if (!exchange)
        throw std::runtime_error(
            "ClientState contexts require the genuine InterlockedExchange export");
    return {reinterpret_cast<decltype(ClientStateRuntimeImports::exchange)>(exchange)};
}
struct ClientStateRuntimeOwner::Data {
    client::ClientRuntimeStorage& storage;
    client::ConfigurationRuntimeOwner& configuration;
    client::PostInitializationData startup;
    ClientStateServerCvars cvars;
    ClientStateHooksBindings core_bindings;
    ClientStateHooksRuntime core;
    TickUpdateRuntime ticks;
    IncomingUpdateRuntime incoming;
    SharedEspRuntime shared;
    events::EventPacketHookRuntime events;
    client::EntityUpdateRuntime entities;
    ClientStateHookFamily family;
    Data(client::ClientRuntimeStorage& source, const client::EngineRuntimeBindings& engine,
         client::ConfigurationRuntimeOwner& settings, client::HistoryRuntimeOwner& history,
         client::CommandCaptureRuntimeOwner& commands, engine::HookRuntimeOwner& hooks,
         script::SourceScriptRuntimeOwner& scripts, ui::MainMenuWindowOwner& window,
         ClientStateRuntimeImports imports)
        : storage(checked_storage(source, engine, settings, history, commands, hooks, scripts,
                                  window, imports)),
          configuration(settings), startup(source.data_owner()),
          cvars{scripts.bridge(), source.data().native_cell<std::byte*>(0x43468D20),
                source.data().native_cell<ServerCvarNameGetter>(0x4347CA4C),
                source.data().native_cell<std::uint8_t>(0x4347CA50),
                ServerCvarEventNames(source.event_names())},
          core_bindings{source.data().native_cell<void*>(0x4346A514),
                        hooks.channel_state(),
                        cvars,
                        source.data().native_cell<ClientStateScopedPatch*>(0x43476900),
                        engine.variables.at(0x43469238),
                        engine.variables.startup_variables(),
                        source.data().native_cell<float*>(0x4346A338),
                        source.data().native_cell<std::int32_t>(0x4347C9EC),
                        source.data().native_cell<float>(0x43469254),
                        startup.acknowledgements(),
                        startup.command_reset(),
                        source.owner_registries().at(0x43475B78),
                        settings.anti_untrusted().enabled.value,
                        source.data().native_cell<void**>(0x4346A3A4),
                        std::ptrdiff_t(rule_guard(source)),
                        window.controller().preset_manager(),
                        errors(scripts)},
          core(core_bindings),
          ticks{{source.entity_observers(),
                 source.data().native_cell<IncomingUpdatePatch*>(0x4347644C),
                 source.data().native_cell<std::int32_t>(0x4346D4B4),
                 source.data().native_cell<std::int32_t>(0x4346D4B8),
                 source.data().native_cell<std::int32_t>(0x4347C9EC),
                 source.data().native_cell<float>(0x43479924),
                 source.data().native_cell<float>(0x43476928),
                 source.data().native_cell<const std::int32_t*>(0x434758D8),
                 source.data().native_cell<const float*>(0x4346A334)},
                {source.data().native_cell<void*>(0x4346A514),
                 source.data().native_cell<void* const*>(0x4346A3A4), rule_guard(source),
                 startup.acknowledgements(), hooks.channel().bindings.history, imports.exchange}},
          incoming{{source.data().native_cell<void*>(0x43475808),
                    source.data().native_cell<const esp::ActiveSoundTree*>(0x43476420),
                    source.data().native_cell<void*>(0x4346A514),
                    source.data().native_cell<void*>(0x4346A890),
                    source.data().native_cell<void*>(0x43467B04),
                    source.data().native_cell<client::EntityCommandOwner*>(0x43475898),
                    source.data().native_cell<std::int32_t*>(0x43475BA4),
                    source.data().native_cell<const std::int32_t*>(0x43475794),
                    source.data().native_cell<const std::int32_t*>(0x434758D8),
                    source.data().native_cell<const std::int32_t*>(0x43475A60),
                    source.data().native_cell<const float*>(0x4346A338),
                    source.data().native_cell<std::int32_t>(0x4346D4B4),
                    source.data().native_cell<std::int32_t>(0x4346D4B8),
                    source.data().native_cell<std::int32_t>(0x4347C9EC),
                    source.data().native_cell<lagcomp::TickTimingSamples>(0x43478C70),
                    source.data().native_cell<float>(0x43476928), source.entity_observers(),
                    source.data().native_cell<IncomingUpdatePatch*>(0x4347644C)},
                   {engine.variables.at(0x43469238), engine.variables.at(0x4346A850),
                    commands.player_gates().rejection_bytes[0], commands.player_gates().tick_base,
                    commands.sent_history(), hooks.channel().bindings.history,
                    source.data().native_cell<esp::EspOwnerRuntime*>(0x434758DC),
                    imports.exchange}},
          shared{source.data().native_cell<client::RejectedPlayerUpdateOwner*>(0x434758D0),
                 source.data().native_cell<void*>(0x43475808),
                 source.data().native_cell<void*>(0x4346A514),
                 source.data().native_cell<void*>(0x4346A890),
                 source.data().native_cell<void**>(0x43468FF0),
                 source.data().native_cell<void**>(0x4346A3A4),
                 history.game_type(),
                 history.teammates_are_enemies(),
                 history.players().adjustments(),
                 history.players().cache(),
                 settings.other_esp_settings().shared.enabled.value,
                 settings.other_esp_settings().shared.restrict_updates.value,
                 pointed<float>(source.data(), 0x4346A338),
                 reinterpret_cast<const float*>(source.data().native_cell<void*>(0x43467B04))[8],
                 scripts.bridge(),
                 source.event_names().alias(events::EventName::voice),
                 shared_esp_offsets(source.offsets())},
          events(source.event_registry(), source.owner_registries().at(0x434758E0),
                 &source.data().native_cell<void*>(0x43475808)),
          entities{{source.data().native_cell<std::uint32_t>(0x43476904),
                    source.data().native_cell<client::EntityUpdatePatch*>(0x43476908),
                    source.data().native_cell<std::int32_t>(0x4346D4BC), source.entity_observers()},
                   {source.data().native_cell<void*>(0x4346A514),
                    source.data().native_cell<void*>(0x4346A890),
                    std::uint32_t(history.target_offsets().team), imports.exchange},
                   window.controller().skins_menu()},
          family(hooks.descriptor_storage(), hooks.registry(), hooks.descriptor_images(),
                 hooks.descriptor_system(),
                 {core, ticks, incoming, settings.effects_settings(), shared, events, entities}) {}
};
ClientStateRuntimeOwner::ClientStateRuntimeOwner(
    client::ClientRuntimeStorage& storage, const client::EngineRuntimeBindings& engine,
    client::ConfigurationRuntimeOwner& configuration, client::HistoryRuntimeOwner& history,
    client::CommandCaptureRuntimeOwner& commands, engine::HookRuntimeOwner& hooks,
    script::SourceScriptRuntimeOwner& scripts, ui::MainMenuWindowOwner& window)
    : ClientStateRuntimeOwner(storage, engine, configuration, history, commands, hooks, scripts,
                              window, native_client_state_runtime_imports()) {}
ClientStateRuntimeOwner::ClientStateRuntimeOwner(
    client::ClientRuntimeStorage& storage, const client::EngineRuntimeBindings& engine,
    client::ConfigurationRuntimeOwner& configuration, client::HistoryRuntimeOwner& history,
    client::CommandCaptureRuntimeOwner& commands, engine::HookRuntimeOwner& hooks,
    script::SourceScriptRuntimeOwner& scripts, ui::MainMenuWindowOwner& window,
    ClientStateRuntimeImports imports)
    : data_(std::make_unique<Data>(storage, engine, configuration, history, commands, hooks,
                                   scripts, window, imports)) {}
ClientStateRuntimeOwner::~ClientStateRuntimeOwner() = default;
client::ClientRuntimeStorage& ClientStateRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
ClientStateHooksRuntime& ClientStateRuntimeOwner::core() noexcept {
    return data_->core;
}
TickUpdateRuntime& ClientStateRuntimeOwner::ticks() noexcept {
    return data_->ticks;
}
IncomingUpdateRuntime& ClientStateRuntimeOwner::incoming() noexcept {
    return data_->incoming;
}
SharedEspRuntime& ClientStateRuntimeOwner::shared_esp() noexcept {
    return data_->shared;
}
events::EventPacketHookRuntime& ClientStateRuntimeOwner::event_packets() noexcept {
    return data_->events;
}
client::EntityUpdateRuntime& ClientStateRuntimeOwner::entities() noexcept {
    return data_->entities;
}
const esp::EffectsSettings& ClientStateRuntimeOwner::effects() const noexcept {
    return data_->configuration.effects_settings();
}
ClientStateHookFamily& ClientStateRuntimeOwner::family() noexcept {
    return data_->family;
}
}

// ============================================================================
// Subcomponent: client_state_signon.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* [dedup field] */
/* [dedup store] */
void set_integer(void* variable, std::int32_t value, std::uint32_t clear_flags) { //43400F0D
    const auto callbacks = field<std::uint32_t>(variable, 0x60);
    store(variable, 0x60, std::uint32_t(0));
    auto* const parent = field<void*>(variable, 0x1C);
    const auto flags = field<std::uint32_t>(parent, 0x14);
    const auto lower = field<std::uint32_t>(parent, 8);
    store(parent, 0x14, flags & ~clear_flags);
    auto* const current_parent = field<void*>(variable, 0x1C);
    store(current_parent, 8, field<std::uint32_t>(current_parent, 8) & ~clear_flags);
    engine::invoke<void>(variable, 16, value);
    store(parent, 8, lower);
    store(parent, 0x14, flags);
    store(variable, 0x60, callbacks);
}
void set_interval(void* variable, std::uint32_t clear_flags, bool clear_lower) {
    const auto callbacks = field<std::uint32_t>(variable, 0x60);
    store(variable, 0x60, std::uint32_t(0));
    auto* const parent = field<void*>(variable, 0x1C);
    const auto flags = field<std::uint32_t>(parent, 0x14);
    const auto lower = field<std::uint32_t>(parent, 8);
    store(parent, 0x14, flags & ~clear_flags);
    if (clear_lower) {
        auto* const current_parent = field<void*>(variable, 0x1C);
        store(current_parent, 8, field<std::uint32_t>(current_parent, 8) & ~clear_flags);
    }
    engine::invoke<void>(variable, 15, 1.0f / 64.0f);
    store(parent, 8, lower);
    store(parent, 0x14, flags);
    store(variable, 0x60, callbacks);
}
std::int32_t tick_rate(const ClientStateHooksBindings& b) {
    return _mm_cvtt_ss2si(
        _mm_add_ss(_mm_div_ss(_mm_set_ss(1.0f), _mm_set_ss(*b.interval)), _mm_set_ss(0.5f)));
}
}
bool ClientStateHooksRuntime::slot47(void* object, std::int32_t state, std::uintptr_t second,
                                     std::uintptr_t third) {
    auto& b = bindings;
    b.transition_status = 0;
    if (script::cvar_int(b.convar_43469238) != 0)
        engine::invoke<void>(b.convar_43469238, 16, std::int32_t(0));
    if (state == 2) {
        client::restore_channel_hook(b.engine_client, b.channel);
        set_integer(b.variables.at_43467fbc, 0xC0000, 0x200);
        set_integer(b.variables.at_434671ac, 64, 0x200);
        set_integer(b.variables.at_4346747c, 64, 0x200);
        set_interval(b.variables.at_4346a748, 0x200, true);
        set_integer(b.variables.at_4346a330, 2, 0x200);
        set_integer(b.variables.at_43467130, 1200, 0x200);
        b.interpolation_floor = 1.0f / 64.0f;
    } else if (state == 3) {
        set_integer(b.variables.at_434671ac, tick_rate(b), 0);
        set_integer(b.variables.at_4346747c, tick_rate(b), 0);
        set_interval(b.variables.at_4346a748, 0, false);
        b.interpolation_floor = 1.0f / 64.0f;
    }
    auto** table = *static_cast<void***>(object);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    const auto result = reinterpret_cast<
        engine::MemberFunction<bool, std::int32_t, std::uintptr_t, std::uintptr_t>>(
        metadata->original_table[47])(object, state, second, third);
    if (state == 6) {
        client::reset_startup_acknowledgements(b.acknowledgements);
        client::reset_command_state(b.command_reset);
        // These are the reconstructed primary owner factories' real source
        // vtables, shared with PostInitializationRuntime; no payload body is
        // supplied as a replacement callback.
        const auto owners = b.owners.entries();
        for (auto* owner : owners)
            engine::invoke<void>(owner, 7);
        if (!b.allow_untrusted && b.shared_player && *b.shared_player &&
            field<std::uint8_t>(*b.shared_player, b.valve_matchmaking_bf567619)) {
            // The canonical manager owns the actual registry/baseline and
            // implements hash0 ->4337ECFE ->433971CD in this exact order.
            b.presets.reset();
            b.errors.format("%s", "Attempted to use untrusted config in a valve server.");
        }
    }
    return result;
}
} //namespace skeet::network

// ============================================================================
// Subcomponent: command_owner.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* [dedup field] */
Vector command_frame_vector(const std::array<float, 3>& value) noexcept {
    return {value[0], value[1], value[2]};
}
void initialize_packet_state_prefix(FakeLagState& state) noexcept {
    state.tracked_target = -1;
    state.value_8c = -1;
    state.flags |= 1U;
}
void initialize_packet_state_tail(FakeLagState& state, std::uint32_t timestamp_low) noexcept {
    const auto cycle = std::int32_t(timestamp_low & 0x7fffffffU);
    state.target = 0;
    state.cycle = cycle;
    state.selected = 14;
}
float convar_float(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1c);
        if (parent == object)
            break;
        object = parent;
    }
    return std::bit_cast<float>(field<std::uint32_t>(object, 0x2c) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
FakeLagActivation activation(const PacketCommandOwner& owner) noexcept {
    return {owner.settings.enabled.value, owner.settings.hotkey.value.binding, owner.globals.input};
}
void* rules(const PacketCommandOwner& owner) noexcept {
    return owner.globals.shared_player ? *owner.globals.shared_player : nullptr;
}
bool freeze_period(const PacketCommandOwner& owner) noexcept {
    const auto player = rules(owner);
    return player && field<std::uint8_t>(player, owner.offsets.rule_d720aea6) != 0;
}
bool rule_gate(const PacketCommandOwner& owner) noexcept {
    const auto player = rules(owner);
    return !player || field<std::uint8_t>(player, owner.offsets.rule_bf567619) != 0;
}
void refresh_and_trigger(PacketCommandOwner& owner, const client::CommandFrame& frame, void* player,
                         client::Command& command, const PacketFrame& packet,
                         std::uint32_t& flags) {
    auto& world = owner.bindings.world;
    world.local_player = player;
    world.entity_list = owner.globals.entity_list;
    owner.bindings.target_refresh.refresh(owner.target, player, command,
                                          frame.previous_command.movement.buttons,
                                          frame.angles_118[1]);

    // These reads occur after the shared refresh, at the same boundary as
    // 433A0914. A preceding selector/engine call may have changed live globals.
    world.local_player = player;
    world.entity_list = owner.globals.entity_list;
    auto targeting = owner.bindings.targeting;
    targeting.local_player = player;
    targeting.entity_list = owner.globals.entity_list;
    auto weapons = owner.bindings.weapons;
    weapons.entity_list = owner.globals.entity_list;
    const FakeLagTriggerFrame trigger{packet,
                                      command_frame_vector(frame.origin_168),
                                      frame.scalar_c0,
                                      frame.tick_base_c8,
                                      frame.weapon_kind_124,
                                      reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)),
                                      command.movement.buttons,
                                      std::uint32_t(command.weapon_select)};
    FakeLagTriggerEnvironment trigger_environment{
        world,
        targeting,
        weapons,
        owner.globals.engine_client,
        *owner.globals.maximum_clients,
        targeting.offsets.active_weapon,
        rule_gate(owner),
        *owner.globals.last_outgoing,
        *owner.globals.real_time,
        *owner.globals.interval,
        convar_float(owner.bindings.movement.convars.gravity)};
    apply_fake_lag_triggers(owner.state, trigger, owner.target, trigger_environment, flags,
                            owner.globals.collision_result);
}
}

PacketFrame capture_packet_frame(const client::CommandFrame& frame) noexcept {
    return {frame.flags,
            frame.choke_limit_fc,
            frame.choked_15c,
            command_frame_vector(frame.velocity_150),
            command_frame_vector(frame.previous_velocity_180),
            command_frame_vector(frame.origin_10c),
            frame.previous_speed_190};
}
void initialize_packet_command_state(FakeLagState& state) noexcept {
    initialize_packet_state_prefix(state);
    const auto timestamp_low = static_cast<std::uint32_t>(__rdtsc());
    initialize_packet_state_tail(state, timestamp_low);
}
void initialize_packet_command_state(FakeLagState& state, std::uint32_t timestamp_low) noexcept {
    initialize_packet_state_prefix(state);
    initialize_packet_state_tail(state, timestamp_low);
}
void process_packet_phase2(PacketCommandOwner& owner, void* player, void* weapon,
                           client::Command& command, const std::array<float, 3>& incoming_angles,
                           std::uint8_t& send_packet, std::uint32_t& flags) {
    static_cast<void>(weapon);
    static_cast<void>(incoming_angles);
    static_cast<void>(send_packet);
    const auto& offsets = owner.offsets;
    const auto choked = *owner.globals.choked_commands;
    if (choked == 0) {
        const PacketCommandContext context{choked,
                                           owner.globals.force_on_next_command != 0,
                                           owner.globals.last_command,
                                           command.number,
                                           field<Vector>(player, offsets.origin_a9ee6062),
                                           field<std::uint32_t>(player, offsets.duck_3309c0cf)};
        prepare_fake_lag_command(owner.state, context, flags);
    } else
        owner.state.recorded_choked = choked;
    update_packet_duck_live(owner, player, command.movement.buttons, flags);
}
void process_packet_phase4(PacketCommandOwner& owner, client::CommandFrame& frame, void* player,
                           client::Command& command, std::uint8_t& send_packet,
                           std::uint32_t& flags) {
    const auto packet = capture_packet_frame(frame);
    begin_fake_lag_tick(owner.state, packet, owner.globals.collision_result);
    if (freeze_period(owner))
        return;
    if (fake_lag_active(activation(owner))) {
        if (select_fake_lag_limit(owner.state, packet, *owner.globals.interval,
                                  owner.bindings.random))
            refresh_and_trigger(owner, frame, player, command, packet, flags);
        apply_fake_lag_send_gate(owner.state, packet, flags, send_packet);
    }
    finish_fake_lag_tick(owner.state, packet, owner.target.handle,
                         std::bit_cast<std::int32_t>(owner.target.fields_18[0]));
}
void process_packet_phase6(PacketCommandOwner& owner, client::CommandFrame& frame, void* player,
                           client::Command& command, std::uint8_t& send_packet,
                           std::uint32_t& flags) {
    static_cast<void>(player);
    static_cast<void>(send_packet);
    record_fake_lag_attack(owner.state, capture_packet_frame(frame), frame.weapon_id_d8,
                           frame.weapon_kind_124, command.movement.buttons,
                           *owner.globals.last_outgoing, owner.bindings.weapon_classification,
                           flags);
}
std::int32_t query_packet_command_limit(PacketCommandOwner& owner, std::int32_t limit) noexcept {
    return cap_fake_lag_commands(owner.state, limit);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: data_update_observers.cpp
// ============================================================================

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_DATA_UPDATE_FASTCALL __fastcall
#else
#define SKEET_DATA_UPDATE_FASTCALL
#endif

namespace skeet::network {
namespace {
constexpr std::array<DataUpdateObserverTable, 4> tables{{
    {DataUpdateObserverId::prediction_history, {{0x433C95B7U, 0x433F4628U}}},
    {DataUpdateObserverId::agents, {{0x433C95B7U, 0x433466F7U}}},
    {DataUpdateObserverId::exploit, {{0x433CCC20U, 0x433EEA67U}}},
    {DataUpdateObserverId::entity_commands, {{0x433D67DFU, 0x43403681U}}},
}};

// Native433C95B7 is the shared, actual RET4 capture entry for these two owners.
void SKEET_DATA_UPDATE_FASTCALL empty_capture(NativeDataUpdateReceiver*, void*, std::int32_t) {}
void SKEET_DATA_UPDATE_FASTCALL finish_prediction(NativeDataUpdateReceiver* receiver, void* player,
                                                  std::int32_t update) {
    client::finish_prediction_data_update(
        *static_cast<client::PredictionHistoryOwner*>(receiver->source), player, update);
}
void SKEET_DATA_UPDATE_FASTCALL finish_agents(NativeDataUpdateReceiver* receiver, void* player,
                                              std::int32_t update) {
    static_cast<skins::NativeAgentRuntime*>(receiver->source)->process(player, update);
}
void SKEET_DATA_UPDATE_FASTCALL capture_exploit(NativeDataUpdateReceiver* receiver, void* player,
                                                std::int32_t) {
    capture_exploit_acknowledgement(*static_cast<ExploitAcknowledgementOwner*>(receiver->source),
                                    player);
}
void SKEET_DATA_UPDATE_FASTCALL finish_exploit(NativeDataUpdateReceiver* receiver, void* player,
                                               std::int32_t) {
    finish_exploit_acknowledgement(*static_cast<ExploitAcknowledgementOwner*>(receiver->source),
                                   player);
}
void SKEET_DATA_UPDATE_FASTCALL capture_entity_commands(NativeDataUpdateReceiver* receiver,
                                                        void* player, std::int32_t update) {
    client::process_entity_data_update_capture(
        *static_cast<client::EntityCommandOwner*>(receiver->source), player, update);
}
void SKEET_DATA_UPDATE_FASTCALL finish_entity_commands(NativeDataUpdateReceiver* receiver,
                                                       void* player, std::int32_t update) {
    client::process_entity_data_update_finish(
        *static_cast<client::EntityCommandOwner*>(receiver->source), player, update);
}
const NativeDataUpdateVtable data_update_prediction_entries{empty_capture, finish_prediction};
const NativeDataUpdateVtable data_update_agent_entries{empty_capture, finish_agents};
const NativeDataUpdateVtable data_update_exploit_entries{capture_exploit, finish_exploit};
const NativeDataUpdateVtable data_update_entity_entries{capture_entity_commands, finish_entity_commands};

NativeDataUpdateReceiver& data_update_source_receiver(void* object) {
    auto* const receiver = static_cast<NativeDataUpdateReceiver*>(object);
    if (!receiver ||
        (receiver->vtable != &data_update_prediction_entries && receiver->vtable != &data_update_agent_entries &&
         receiver->vtable != &data_update_exploit_entries && receiver->vtable != &data_update_entity_entries))
        throw std::logic_error("Native data-update entry has no concrete source receiver binding");
    // Captured native vtable addresses are identifiers only. An unbridged
    // image receiver must not execute captured code as a dispatch substitute.
    return *receiver;
}
}
std::span<const DataUpdateObserverTable> native_data_update_observer_tables() noexcept {
    return tables;
}
std::optional<DataUpdateObserverId> identify_data_update_observer(std::uint32_t address) noexcept {
    for (const auto& table : tables)
        if (std::uint32_t(table.id) == address)
            return table.id;
    return std::nullopt;
}
DataUpdateObserverRegistry::DataUpdateObserverRegistry(
    engine::PreservedInitialization& initialization, engine::JobMemory memory)
    : DataUpdateObserverRegistry(initialization.image_owner(), memory) {}
DataUpdateObserverRegistry::DataUpdateObserverRegistry(
    std::shared_ptr<engine::NativeDataImage> image, engine::JobMemory memory)
    : image_(image),
      owners_(engine::require_native_data_image(image_).native_cell<engine::OwnerRegistry>(
          0x43475760u)),
      memory_(memory) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument(
            "Native data-update storage requires the canonical process heap");
}

void DataUpdateObserverRegistry::append_receiver(NativeDataUpdateReceiver& receiver, void* source) {
    receiver.source = source;
    // Exact4337EFD9: register_owner retains its -4 sentinel handling and the
    //433F1665/4334DC99 rereads after allocation. Final secondary tables are
    //published separately at their actual later constructor write sites.
    engine::register_owner(owners_, &receiver, memory_);
}
void DataUpdateObserverRegistry::append(client::PredictionHistoryOwner& owner) {
    append_receiver(owner.data_update_receiver, &owner);
}
void DataUpdateObserverRegistry::append(skins::NativeAgentRuntime& owner) {
    append_receiver(owner.data_update_receiver_, &owner);
}
void DataUpdateObserverRegistry::append(ExploitAcknowledgementOwner& owner) {
    append_receiver(owner.data_update_receiver, &owner);
}
void DataUpdateObserverRegistry::append(client::EntityCommandOwner& owner) {
    append_receiver(owner.data_update_receiver, &owner);
}
void DataUpdateObserverRegistry::publish(client::PredictionHistoryOwner& owner) const noexcept {
    owner.data_update_receiver.vtable = &data_update_prediction_entries;
}
void DataUpdateObserverRegistry::publish(skins::NativeAgentRuntime& owner) const noexcept {
    owner.data_update_receiver_.vtable = &data_update_agent_entries;
}
void DataUpdateObserverRegistry::publish(ExploitAcknowledgementOwner& owner) const noexcept {
    owner.data_update_receiver.vtable = &data_update_exploit_entries;
}
void DataUpdateObserverRegistry::publish(client::EntityCommandOwner& owner) const noexcept {
    owner.data_update_receiver.vtable = &data_update_entity_entries;
}

void DataUpdateObserverRegistry::capture_forward(void* player, std::int32_t update) const {
    //433B7680 reads begin, then433B7687 reads end. Do not reread either cell
    //after a receiver call; insertion during dispatch is outside this range.
    auto** cursor = owners_.begin;
    auto** const end = owners_.end;
    for (; cursor != end; ++cursor) {
        auto& receiver = data_update_source_receiver(*cursor);
        receiver.vtable->capture(&receiver, player, update);
    }
}
void DataUpdateObserverRegistry::finish_reverse(void* player, std::int32_t update) const {
    //433B711D/433B7123 capture begin then end; decrement precedes slot1.
    auto** const begin = owners_.begin;
    auto** cursor = owners_.end;
    while (cursor != begin) {
        auto& receiver = data_update_source_receiver(*--cursor);
        receiver.vtable->finish(&receiver, player, update);
    }
}
void DataUpdateObserverRegistry::finalize_storage() const {
    engine::UnloadSystemFunctions functions;
    functions.free_heap = memory_.free;
    functions.process_heap = memory_.process_heap;
    // Complete433F4A1D: load/null-test actual43475760, free its aligned native
    //allocation, leave begin/end/limit untouched. No automatic second release.
    engine::free_native_allocation(owners_.begin, functions);
}
} //namespace skeet::network

#undef SKEET_DATA_UPDATE_FASTCALL

// ============================================================================
// Subcomponent: exploit_acknowledgement.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
/* [dedup add] */
/* [dedup subtract] */
bool acknowledgement_enabled(const void* object) noexcept {
    for (;;) {
        const auto* parent = field<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    const auto bits = field<std::uint32_t>(object, 0x2C) ^
                      static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object));
    return std::bit_cast<float>(bits) != 0.0f; //43407DE2 admits unordered values
}
long double current_engine_time(void* engine_client) {
    return engine::invoke<long double>(engine_client, 14);
}
std::int32_t receive_tick(const ExploitAcknowledgementBindings& bindings) {
    const auto time = current_engine_time(bindings.engine_client);
    const auto interval = field<float>(bindings.engine_globals, 0x20);
    // Native FDIV/FADD operate in ST0, then FSTP rounds once to float32
    // before CVTTSS2SI. Keep the original extended-return path on x86.
    volatile float rounded = static_cast<float>(time / static_cast<long double>(interval) + 0.5L);
    return _mm_cvttss_si32(_mm_set_ss(rounded));
}
std::int32_t process_ticks(const ExploitAcknowledgementBindings& bindings) noexcept {
    auto duration = _mm_cvtepi32_ps(_mm_cvtsi32_si128(bindings.process_milliseconds));
    duration = _mm_div_ss(duration, _mm_set_ss(1000.0f));
    duration = _mm_min_ss(_mm_max_ss(duration, _mm_setzero_ps()), _mm_set_ss(1.0f));
    duration = _mm_div_ss(duration, _mm_set_ss(*bindings.interval));
    return _mm_cvttss_si32(_mm_add_ss(duration, _mm_set_ss(0.5f)));
}
}
void capture_exploit_acknowledgement(ExploitAcknowledgementOwner& owner, void* player) {
    owner.captured_tick_base_118 = field<std::int32_t>(player, owner.bindings.tick_base_3d10b94c);
}
void finish_exploit_acknowledgement(ExploitAcknowledgementOwner& owner, void* player) {
    auto& runtime = owner.runtime;
    auto& state = runtime.network;
    const auto& bindings = owner.bindings;
    if (!state.mode)
        return;
    const auto acknowledged = *bindings.acknowledged;
    if (acknowledged <= *bindings.old_acknowledged)
        return;
    if (field<std::uint8_t>(player, bindings.rejected_9d920e35))
        return;
    if (!acknowledgement_enabled(bindings.acknowledgement_convar.get()))
        return;
    if (field<std::int32_t>(bindings.engine_globals, 0x18) == 1)
        return;

    if (acknowledged >= runtime.transition_command_110) {
        runtime.transition_command_110 = (std::numeric_limits<std::int32_t>::max)();
        const auto threshold = subtract(receive_tick(bindings), 1);
        if (field<std::int32_t>(player, bindings.tick_base_3d10b94c) >= threshold)
            state.shot_tick = subtract(field<std::int32_t>(player, bindings.tick_base_3d10b94c), 1);
    }
    const auto command = engine::invoke<std::int32_t>(bindings.engine_client, 205);
    if (command < owner.phases.recharge_ready_command_11c || state.available < state.limit)
        return;
    const auto processing = process_ticks(bindings);
    const auto current = receive_tick(bindings);
    const auto difference = subtract(add(current, processing),
                                     field<std::int32_t>(player, bindings.tick_base_3d10b94c));
    if (std::uint32_t(difference) > 15u)
        return;
    if (runtime.generated_commands[std::uint32_t(command) % 150u] != command)
        return;

    if (state.history_count != 8)
        state.history_count = add(state.history_count, 1);
    state.history_first = (state.history_first - 1u) & 7u;
    state.history[state.history_first] = add(difference, 1);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: exploit_command_owner.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
/* [dedup add] */
void* frame_weapon(const client::CommandFrame& frame) noexcept {
    return reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
}
ExploitRuntimeSettings current_settings(const ExploitCommandOwner& owner) {
    const auto& base = owner.settings;
    return {owner.controls,
            base.command_block,
            base.special_weapon_43467560,
            base.configured_budget_43469274,
            base.process_milliseconds_43468290,
            base.health_divisor_4346db30};
}
void refresh_timing(ExploitCommandOwner& owner) {
    auto& bindings = owner.bindings;
    auto& timing = bindings.timing;
    timing.choked = *bindings.choked_commands;
    timing.last_outgoing = *bindings.last_outgoing;
    timing.process_milliseconds = owner.settings.process_milliseconds_43468290;
    timing.interval = *bindings.tick_interval;
    timing.current_time = *bindings.current_time;
    timing.latency = owner.timing.latency;
    timing.command_tick = owner.timing.command_tick;
    const auto acknowledged = owner.timing.acknowledged;
    const auto sent = owner.timing.sent;
    timing.have_ack = acknowledged != nullptr;
    timing.acknowledged = acknowledged ? *acknowledged : 0;
    timing.have_sent = sent != nullptr;
    timing.sent = sent ? *sent : 0;
}
}

void reset_exploit_command_owner(ExploitCommandOwner& owner) noexcept {
    auto& runtime = owner.runtime;
    auto& state = runtime.network;
    state.available = 0;
    runtime.low_health_120 = 0;
    runtime.generated_commands.fill(-1);
    state.shot_tick = -1;
    runtime.transition_command_110 = 0x7FFFFFFF;
    owner.phases.recharge_ready_command_11c = 0x7FFFFFFF;
    state.shot_command = -1;
    owner.weapon_command_12c = -1;
    for (auto& entry : runtime.command_ticks)
        entry.command = -1;
    runtime.field_130 = 0;
    runtime.last_flush_command_134 = -1;
}

void exploit_owner_phase1(ExploitCommandOwner& owner, void* player, void*, client::Command& command,
                          std::uint8_t& send_packet) {
    const auto settings = current_settings(owner);
    process_exploit_phase1(owner.runtime, owner.phases, player, command, send_packet, settings,
                           owner.bindings, owner.phase1, owner.command_flags);
}

void exploit_owner_phase2(ExploitCommandOwner& owner, void*, void* weapon, client::Command& command,
                          std::uint8_t&, std::array<float, 3>&) {
    auto& state = owner.runtime.network;
    auto& bindings = owner.bindings;
    state.flags &= ~0x80U;
    if (state.mode == 0)
        return;
    if (!owner.settings.special_weapon_43467560) {
        if (weapon && engine::invoke<std::int32_t>(weapon, 455) == 9) {
            if ((command.movement.buttons & 0x801U) ||
                field<std::uint8_t>(weapon, bindings.offsets.weapon_byte_e1df7e9b) ||
                field<float>(weapon, bindings.offsets.weapon_time_3b93667b) > 0.0f ||
                owner.weapon_command_12c == *bindings.last_outgoing) {
                state.flags = (state.flags & ~2U) | 0x80U;
                owner.weapon_command_12c = *bindings.last_outgoing;
                return;
            }
        } else if (owner.weapon_command_12c == *bindings.last_outgoing) {
            state.flags = (state.flags & ~2U) | 0x80U;
            return;
        }
    }
    if ((state.flags & 2U) || !weapon)
        return;
    const auto identity = engine::invoke<std::int32_t>(static_cast<std::byte*>(weapon) + 8, 10);
    if (command.weapon_select && command.weapon_select != identity) {
        state.flags |= 2U;
        return;
    }
    auto number = *bindings.last_outgoing;
    for (std::uint32_t count = 0; count < 150; ++count) {
        number = add(number, 1);
        if (command.number <= number)
            return;
        const auto& historical = bindings.commands[std::uint32_t(number) % 150];
        if (historical.number == number && historical.weapon_select &&
            historical.weapon_select != identity) {
            state.flags |= 2U;
            return;
        }
    }
}

void exploit_owner_phase3(ExploitCommandOwner& owner, client::CommandFrame& frame, void*,
                          client::Command&, std::uint8_t& send_packet) {
    apply_exploit_weapon_gate(owner.runtime.network, *owner.bindings.last_outgoing,
                              owner.settings.special_weapon_43467560, frame.weapon_kind_124,
                              frame.choked_15c, frame.secondary_flags, owner.command_flags,
                              send_packet);
}

void exploit_owner_phase4(ExploitCommandOwner& owner, client::CommandFrame& frame, void* player,
                          client::Command& command, std::uint8_t&) {
    refresh_timing(owner);
    auto& timing = owner.bindings.timing;
    timing.player_next_attack = timing.weapon_next_attack = 0.0f;
    const auto& state = owner.runtime.network;
    if (state.mode != 0 && state.available >= state.limit && frame.weapon_address) {
        timing.player_next_attack =
            field<float>(player, owner.bindings.firing.offsets.player_next_attack);
        timing.weapon_next_attack =
            field<float>(frame_weapon(frame), owner.bindings.firing.offsets.next_primary);
    }
    timing.freeze_period = false;
    if (state.mode == 3 && state.available >= state.limit) {
        const auto rules = owner.bindings.game_rules ? *owner.bindings.game_rules : nullptr;
        timing.freeze_period =
            rules && field<std::uint8_t>(rules, owner.bindings.firing.offsets.rule_d720aea6);
    }
    ExploitFrame projected{frame.flags,
                           frame.previous_command.movement.buttons,
                           frame.weapon_address != 0,
                           frame.command_number_ac,
                           frame.tick_base_c8,
                           std::bit_cast<std::int32_t>(frame.field_cc),
                           frame.weapon_id_d8,
                           frame.weapon_kind_124,
                           std::bit_cast<std::int32_t>(frame.field_1d4),
                           std::bit_cast<std::int32_t>(frame.field_1d8),
                           std::bit_cast<std::int32_t>(frame.field_1e0)};
    ExploitCommand input{command.number, command.angles[0], command.angles[1],
                         command.movement.buttons};
    update_exploit_command(owner.runtime.network, projected, input, owner.bindings.timing,
                           owner.command_flags);
    frame.flags = projected.flags;
    frame.field_cc = std::uint32_t(projected.saved_tick_base);
    frame.field_1d4 = std::uint32_t(projected.exploit_mode);
    frame.field_1d8 = std::uint32_t(projected.next_tick);
    frame.field_1e0 = std::uint32_t(projected.choke_end);
    command.angles[0] = input.pitch;
    command.angles[1] = input.yaw;
    command.movement.buttons = input.buttons;
}

void exploit_owner_phase5(ExploitCommandOwner& owner, client::CommandFrame& frame, void* player,
                          client::Command& command, std::uint8_t& send_packet) {
    refresh_timing(owner);
    const auto settings = current_settings(owner);
    process_exploit_runtime(owner.runtime, frame, player, frame_weapon(frame), command, send_packet,
                            settings, owner.bindings, owner.command_flags);
}

void exploit_owner_phase7(ExploitCommandOwner& owner, client::CommandFrame& frame, void* player,
                          client::Command& command, std::uint8_t& send_packet) {
    refresh_timing(owner);
    const auto settings = current_settings(owner);
    process_exploit_phase7(owner.runtime, owner.phases, frame, player, command, send_packet,
                           settings, owner.bindings, owner.phase7, owner.command_flags);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: exploit_message.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
/* [dedup write] */
/* [dedup add] */
/* [dedup subtract] */
struct MessageBytes {
    union {
        std::array<std::byte, 16> local;
        std::byte* pointer;
    } storage;
    std::uint32_t size, capacity;
};
static_assert(sizeof(MessageBytes) == 0x18);
#if defined(_M_IX86) || defined(__i386__)
void __fastcall message_entry(void* object, misc::NetworkMessageCall* call) {
#else
void message_entry(void* object, misc::NetworkMessageCall* call) {
#endif
    auto& endpoint = *static_cast<ExploitMessageHandler*>(object);
    rewrite_exploit_command_message(endpoint.state, endpoint.bindings, *call);
}
const std::array<misc::NetworkMessageEntry, 1> message_table{message_entry};
}
ExploitMessageHandler::ExploitMessageHandler(ExploitState& state_value,
                                             const ExploitMessageBindings& current) noexcept
    : NetworkMessageCallback{nullptr}, state(state_value), bindings(current) {}
void ExploitMessageHandler::publish_table() noexcept {
    vtable = message_table.data();
}

void rewrite_exploit_command_message(const ExploitState& state,
                                     const ExploitMessageBindings& bindings,
                                     misc::NetworkMessageCall& call) {
    if (engine::invoke<std::int32_t>(call.message, 7) != 9)
        return;
    void* const message = call.message;
    if ((field<std::uint32_t>(message, 0x1C) & 6U) != 6U)
        return;
    const auto saved_count = field<std::int32_t>(message, 0x10);
    if (saved_count == 0 || !(field<std::uint8_t>(message, 0x1C) & 1U) ||
        field<std::int32_t>(message, 0xC) != 2)
        return;
    const auto choked = *bindings.choked;
    if (choked == 0 && saved_count == 1) {
        const auto next = add(*bindings.last_outgoing, 1);
        const auto* ring = field<const client::Command*>(bindings.input, 0xF0);
        const auto& command = ring[std::uint32_t(next) % 150U];
        if (command.number == next && std::bit_cast<std::uint32_t>(command.tick) == 0x7F7FFFFFU) {
            call.cancelled = 1;
            return;
        }
    }
    if (state.budget <= 15)
        return;
    auto requested = add(choked, 1);
    if (requested < 0)
        requested = 0;
    if (subtract(requested, 15) <= 0)
        return;

    const auto saved_data = field<void*>(message, 0x14);
    void* const channel = call.channel;
    MessageBytes encoded;
    encoded.storage.local.fill(std::byte{0});
    encoded.size = 0;
    encoded.capacity = 15;
    std::array<std::byte, 4001> bytes;
    ExploitCommandWriter writer{bytes.data(), 4000, 32000, 0, 0, 0, 0, nullptr};
    write(message, 0xC, std::int32_t(2));
    write(message, 0x14, &encoded);
    const auto count = requested < 60 ? requested : 60;
    write(message, 0x10, count);
    const auto last = add(add(*bindings.choked, 1), *bindings.last_outgoing);
    auto to = add(subtract(last, count), -1);
    std::int32_t from = -1;
    bool complete = true;
    while (to <= last) {
        const auto first_new = add(subtract(last, field<std::int32_t>(message, 0x10)), 1);
        if (!engine::invoke<std::uint8_t>(bindings.client, 24, std::int32_t(0), &writer, from, to,
                                          std::uint8_t(to >= first_new))) {
            complete = false;
            break;
        }
        from = to;
        to = add(to, 1);
    }
    if (complete) {
        encoded.size = std::uint32_t(add(writer.current_bit, 7) >> 3);
        if (encoded.size < 16)
            std::memmove(encoded.storage.local.data(), bytes.data(), encoded.size);
        else {
            encoded.storage.pointer = bytes.data();
            encoded.capacity = 4000;
            bytes[encoded.size] = std::byte{0};
        }
        using Send = engine::MemberFunction<std::uint8_t, void*, std::uint8_t, std::uint8_t>;
        reinterpret_cast<Send>(bindings.original_channel_vtable[40])(
            channel, message, std::uint8_t(0), std::uint8_t(0));
    }
    call.cancelled = 1;
    write(message, 0x14, saved_data);
    write(message, 0x10, saved_count);
}
} //namespace skeet::network

// ============================================================================
// Subcomponent: exploit_observers.cpp
// ============================================================================

#if defined(_M_IX86) || defined(__i386__)
#define EXPLOIT_RECHARGE_FAST __fastcall
#define EXPLOIT_RECHARGE_MEMBER __thiscall
#else
#define EXPLOIT_RECHARGE_FAST
#define EXPLOIT_RECHARGE_MEMBER
#endif

namespace skeet::network {
namespace {
void EXPLOIT_RECHARGE_FAST empty_generated(NativeExploitRechargeReceiver*, void*, std::int32_t,
                                           std::int32_t) {} //4334A269
void EXPLOIT_RECHARGE_FAST empty_recharge(NativeExploitRechargeReceiver*, void*, std::int32_t) {
} //433C95B7
void EXPLOIT_RECHARGE_MEMBER empty_cloned(NativeExploitRechargeReceiver*, void*, client::Command*) {
} //4334A269
void EXPLOIT_RECHARGE_FAST recharge_prediction(NativeExploitRechargeReceiver* receiver, void*,
                                               std::int32_t number) {
    client::clone_previous_prediction_snapshot(
        *static_cast<client::PredictionHistoryOwner*>(receiver->source), number);
}
void EXPLOIT_RECHARGE_FAST recharge_entity_commands(NativeExploitRechargeReceiver* receiver, void*,
                                                    std::int32_t number) {
    client::observe_entity_command_recharge(
        *static_cast<client::EntityCommandOwner*>(receiver->source), number);
}
void EXPLOIT_RECHARGE_FAST recharge_viewmodel(NativeExploitRechargeReceiver* receiver, void*,
                                              std::int32_t) {
    auto& owner = *static_cast<ExploitViewmodelRecharge*>(receiver->source);
    misc::advance_viewmodel_prediction(owner.state, owner.bindings);
}
void EXPLOIT_RECHARGE_FAST generated_feedback(NativeExploitRechargeReceiver* receiver, void*,
                                              std::int32_t final_command, std::int32_t) {
    auto& owner = *static_cast<ExploitShotFeedbackObserver*>(receiver->source);
    // Complete433C86CE: word1 is retained; only mutable word3 changes.
    for (auto& record : owner.state.pending) {
        const auto command = std::bit_cast<std::int32_t>(record.words[1]);
        if (owner.last_outgoing < command && command < final_command)
            record.words[3] = std::uint32_t(final_command);
    }
}
void EXPLOIT_RECHARGE_FAST generated_viewmodel(NativeExploitRechargeReceiver* receiver, void*,
                                               std::int32_t final_command, std::int32_t count) {
    auto& owner = *static_cast<ExploitViewmodelRecharge*>(receiver->source);
    misc::record_viewmodel_command_range(owner.state, owner.bindings, final_command, count);
}
const NativeExploitRechargeVtable recharge_prediction_entries{empty_generated, recharge_prediction,
                                                     empty_cloned};
const NativeExploitRechargeVtable recharge_noop_entries{empty_generated, empty_recharge, empty_cloned};
const NativeExploitRechargeVtable recharge_feedback_entries{generated_feedback, empty_recharge,
                                                   empty_cloned};
const NativeExploitRechargeVtable recharge_entity_entries{empty_generated, recharge_entity_commands,
                                                 empty_cloned};
const NativeExploitRechargeVtable recharge_viewmodel_entries{generated_viewmodel, recharge_viewmodel,
                                                    empty_cloned};

NativeExploitRechargeReceiver& recharge_source_receiver(void* endpoint) {
    auto* receiver = static_cast<NativeExploitRechargeReceiver*>(endpoint);
    if (!receiver ||
        (receiver->vtable != &recharge_prediction_entries && receiver->vtable != &recharge_noop_entries &&
         receiver->vtable != &recharge_feedback_entries && receiver->vtable != &recharge_entity_entries &&
         receiver->vtable != &recharge_viewmodel_entries))
        throw std::logic_error("Native recharge entry has no concrete source receiver binding");
    return *receiver;
}
}
ExploitRechargeObserverRegistry::ExploitRechargeObserverRegistry(
    engine::PreservedInitialization& initialization, engine::JobMemory memory)
    : ExploitRechargeObserverRegistry(initialization.image_owner(), memory) {}
ExploitRechargeObserverRegistry::ExploitRechargeObserverRegistry(
    std::shared_ptr<engine::NativeDataImage> image, engine::JobMemory memory)
    : image_(image),
      owners_(engine::require_native_data_image(image_).native_cell<engine::OwnerRegistry>(
          0x43475784u)),
      memory_(memory) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument("Native recharge storage requires the canonical process heap");
}
void ExploitRechargeObserverRegistry::append_receiver(NativeExploitRechargeReceiver& receiver,
                                                      void* source) {
    receiver.source = source;
    engine::register_owner(owners_, &receiver, memory_);
}
void ExploitRechargeObserverRegistry::append(client::PredictionHistoryOwner& owner) {
    append_receiver(owner.recharge_receiver, &owner);
}
void ExploitRechargeObserverRegistry::append(client::EntityCommandOwner& owner) {
    append_receiver(owner.recharge_receiver, &owner);
}
void ExploitRechargeObserverRegistry::append(ExploitShotFeedbackObserver& owner) {
    append_receiver(owner.receiver, &owner);
}
void ExploitRechargeObserverRegistry::append(ExploitViewmodelRecharge& owner) {
    append_receiver(owner.receiver, &owner);
}
void ExploitRechargeObserverRegistry::append_noop(NativeExploitRechargeReceiver& receiver,
                                                  void* source) {
    append_receiver(receiver, source);
}
void ExploitRechargeObserverRegistry::publish(
    client::PredictionHistoryOwner& owner) const noexcept {
    owner.recharge_receiver.vtable = &recharge_prediction_entries;
}
void ExploitRechargeObserverRegistry::publish(client::EntityCommandOwner& owner) const noexcept {
    owner.recharge_receiver.vtable = &recharge_entity_entries;
}
void ExploitRechargeObserverRegistry::publish(ExploitShotFeedbackObserver& owner) const noexcept {
    owner.receiver.vtable = &recharge_feedback_entries;
}
void ExploitRechargeObserverRegistry::publish(ExploitViewmodelRecharge& owner) const noexcept {
    owner.receiver.vtable = &recharge_viewmodel_entries;
}
void ExploitRechargeObserverRegistry::publish_noop(
    NativeExploitRechargeReceiver& receiver) const noexcept {
    receiver.vtable = &recharge_noop_entries;
}

void ExploitRechargeObserverRegistry::notify_recharge(void* player, std::int32_t number) const {
    //433AC27B/281: capture end, then begin, once before any slot1 call.
    auto** const end = owners_.end;
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = recharge_source_receiver(*cursor);
        receiver.vtable->recharge(&receiver, player, number);
    }
}
void ExploitRechargeObserverRegistry::notify_generated(void* player, std::int32_t final_command,
                                                       std::int32_t count) const {
    //4335B693/6A0: end and begin are captured before the first slot0 call.
    auto** const end = owners_.end;
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = recharge_source_receiver(*cursor);
        receiver.vtable->generated(&receiver, player, final_command, count);
    }
}
void ExploitRechargeObserverRegistry::notify_cloned(void* player, client::Command& command) const {
    //433674CF/D5 retains actual slot2 traversal, including genuine RET8 entries.
    auto** const end = owners_.end;
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = recharge_source_receiver(*cursor);
        receiver.vtable->cloned(&receiver, player, &command);
    }
}
void ExploitRechargeObserverRegistry::finalize_storage() const {
    engine::UnloadSystemFunctions memory;
    memory.free_heap = memory_.free;
    memory.process_heap = memory_.process_heap;
    engine::free_native_allocation(owners_.begin, memory);
}
void notify_exploit_recharge_observers(const ExploitRechargeObserverRegistry& registry,
                                       void* player, std::int32_t number) {
    registry.notify_recharge(player, number);
}
void notify_exploit_generated_observers(const ExploitRechargeObserverRegistry& registry,
                                        void* player, std::int32_t final_command,
                                        std::int32_t count) {
    registry.notify_generated(player, final_command, count);
}
} //namespace skeet::network

#undef EXPLOIT_RECHARGE_FAST
#undef EXPLOIT_RECHARGE_MEMBER

// ============================================================================
// Subcomponent: exploit_owner.cpp
// ============================================================================

namespace skeet::network {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define EXPLOIT_MEMBER __thiscall
#define EXPLOIT_FAST __fastcall
#else
#define EXPLOIT_MEMBER
#define EXPLOIT_FAST
#endif
using PrimaryEntry = void(EXPLOIT_MEMBER*)(void*);
struct ExploitInstance;
struct HudEndpoint : hud::HudFrameCallback {
    ExploitInstance* owner;
};
void EXPLOIT_MEMBER exploit_destroy_entry(void*);
void EXPLOIT_MEMBER exploit_unchanged(void*) {} //actual433CCE8E
void EXPLOIT_MEMBER exploit_reset_entry(void*);
void EXPLOIT_MEMBER exploit_refresh_entry(void*);
void EXPLOIT_FAST hud_entry(void*, ui::RenderQueue*, void*, std::int32_t*);
const std::array<PrimaryEntry, 9> exploit_primary_table{exploit_destroy_entry, exploit_unchanged,     exploit_unchanged,
                                                exploit_unchanged,     exploit_unchanged,     exploit_reset_entry,
                                                exploit_unchanged,     exploit_refresh_entry, exploit_unchanged};
const std::array<hud::HudFrameEntry, 1> hud_table{hud_entry};

void require_source_bindings(const ExploitOwnerRegistries& registries,
                             const ExploitOwnerBindings& bindings) {
    const auto* const image = &registries.data_updates.image();
    const auto& command = bindings.command;
    if (&registries.prediction.image() != image ||
        &command.phase7.prediction_observers.image() != image ||
        &command.bindings.recharge_observers.image() != image ||
        &command.phase1.recharge_observers.image() != image ||
        &command.phase7.generated_observers.image() != image)
        throw std::invalid_argument("Exploit phases must share the retained observer registries");
    auto& snapshots = image->native_cell<client::PredictionHistoryOwner*>(0x434759b4u);
    auto& history = image->native_cell<client::EntityCommandOwner*>(0x43475898u);
    if (&command.bindings.prediction_snapshots_owner != &snapshots ||
        &command.phase7.restricted_owner != &snapshots ||
        &command.bindings.prediction_history_owner != &history)
        throw std::invalid_argument("Exploit phases must read the native history owner slots");
    if (&registries.primary.registry != &image->native_cell<engine::OwnerRegistry>(0x43475b78u) ||
        &registries.hud.registry != &image->native_cell<engine::OwnerRegistry>(0x43475a9cu) ||
        &registries.messages.registry != &image->native_cell<engine::OwnerRegistry>(0x43475b94u))
        throw std::invalid_argument(
            "Exploit construction requires the native lifecycle, HUD and message registries");
}

struct ExploitInstance {
    const PrimaryEntry* vtable = nullptr;
    engine::JobMemory memory;
    ExploitOwnerBindings bindings;
    HudEndpoint hud{{nullptr}, this};
    ExploitMessageHandler message;
    client::NativeCommandPhaseReceiver command_receiver;
    ExploitAcknowledgementOwner acknowledgement;

    ExploitInstance(const ExploitOwnerRegistries& registries, ConfigRegistries& config,
                    ExploitOwnerBindings context)
        : memory(registries.primary.memory), bindings(context),
          message(context.command.runtime.network, context.messages),
          acknowledgement{context.command.runtime, context.command.phases,
                          context.acknowledgement} {
        registries.primary.append(this); //433676A0, full+0
        hud::register_hud_frame(registries.hud, &hud); //433D20B5, full+8
        registries.messages.append(&message); //4336F73C, full+10
        registries.commands.append(command_receiver, &bindings.command); //4347DF11,+18
        registries.data_updates.append(acknowledgement); //4337EFD9,+20
        registries.prediction.append(bindings.command.runtime); //4341CEE4,+28
        message.publish_table(); //native4346F240
        registries.commands.publish(command_receiver, client::CommandOwnerId::exploit); //4347DF31
        registries.data_updates.publish(acknowledgement); //4347DF38, after prediction registration
        registries.prediction.publish(bindings.command.runtime); //4347DF3F, table4346F7D0
        vtable = exploit_primary_table.data(); //native434707D0
        hud.vtable = hud_table.data(); //native43472688
        rage::initialize_exploit_settings(config, bindings.command.controls.settings);
        auto& runtime = bindings.command.runtime;
        auto& state = runtime.network;
        state.mode = 0;
        state.last_command = -1;
        bindings.command.phases.observed_ack_10c = -1;
        acknowledgement.captured_tick_base_118 = -1;
        state.timer_124 = 0.0f;
        runtime.transition_command_110 = 0x7FFFFFFF;
        state.shot_tick = 0x7FFFFFFF;
        bindings.command.phases.recharge_ready_command_11c = 0x7FFFFFFF;
        state.budget = 16;
        state.available = 0;
        runtime.low_health_120 = 0;
        state.random.seed = 0;
        state.random.previous = 0;
        state.history_first = 0;
        state.history_count = 0;
        //4347DEEB leaves flags/limits/rings/random-table/other scalar words.
    }
};
static_assert(sizeof(ExploitInstance) <= native_exploit_owner_allocation_size);
void EXPLOIT_MEMBER exploit_destroy_entry(void* storage) {
    destroy_exploit_owner(storage);
}
void EXPLOIT_MEMBER exploit_reset_entry(void* storage) {
    reset_exploit_command_owner(static_cast<ExploitInstance*>(storage)->bindings.command);
}
void EXPLOIT_MEMBER exploit_refresh_entry(void* storage) {
    refresh_exploit_owner(storage);
}
void EXPLOIT_FAST hud_entry(void* endpoint, ui::RenderQueue* queue, void* player,
                            std::int32_t* cursor) {
    auto& owner = *static_cast<HudEndpoint*>(static_cast<hud::HudFrameCallback*>(endpoint))->owner;
    const auto& b = owner.bindings.hud;
    hud::draw_exploit_indicators(*queue, b.scripts, owner.bindings.command.runtime.network,
                                 b.shared_timer, b.indicators, player, b.interfaces, b.frame_time,
                                 *cursor, b.layout);
}
void* allocate(engine::JobMemory memory, std::size_t size) {
    auto* const raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    const auto total = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}
}
std::size_t exploit_owner_size() noexcept {
    return sizeof(ExploitInstance);
}
void construct_exploit_owner(void* storage, const ExploitOwnerRegistries& registries,
                             ConfigRegistries& config, ExploitOwnerBindings bindings) {
    require_source_bindings(registries, bindings);
    ::new (storage) ExploitInstance(registries, config, bindings);
}
void* allocate_exploit_owner_storage(engine::JobMemory memory) {
    return allocate(memory, native_exploit_owner_allocation_size);
}
void construct_allocated_exploit_owner(void* storage, const ExploitOwnerRegistries& registries,
                                       ConfigRegistries& config, ExploitOwnerBindings bindings) {
    require_source_bindings(registries, bindings);
    //43387448 uses HEAP_ZERO_MEMORY. Canonical value fields live outside the
    // semantic endpoint allocation, so project those zeroed native words here.
    // Placement construction itself deliberately preserves untouched words.
    auto& command = bindings.command;
    command.controls.settings = rage::ExploitSettings{};
    auto& runtime = command.runtime;
    runtime.network = ExploitState{};
    runtime.field_f0 = 0;
    runtime.transition_command_110 = 0;
    runtime.low_health_120 = 0;
    runtime.field_130 = 0;
    runtime.last_flush_command_134 = 0;
    runtime.command_ticks.fill(ExploitCommandTick{0, 0});
    runtime.generated_commands.fill(0);
    command.phases.observed_ack_10c = 0;
    command.phases.recharge_ready_command_11c = 0;
    command.weapon_command_12c = 0;
    construct_exploit_owner(storage, registries, config, bindings);
}
void destroy_exploit_owner(void* storage) {
    if (!storage)
        return; //actual433837D0 null guard
    auto* const owner = static_cast<ExploitInstance*>(storage);
    const auto memory = owner->memory;
    owner->~ExploitInstance();
    auto* const bytes = static_cast<std::byte*>(storage);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void refresh_exploit_owner(void* storage) {
    auto& owner = *static_cast<ExploitInstance*>(storage);
    owner.vtable[5](storage);
}
#undef EXPLOIT_MEMBER
#undef EXPLOIT_FAST
} //namespace skeet::network

// ============================================================================
// Subcomponent: exploit_phase1.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
/* deduplicated helper */
std::int32_t sub(std::int32_t first, std::int32_t second) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(first) - std::uint32_t(second));
}
void force_send(std::uint32_t& flags) noexcept {
    flags = (flags & ~0x400U) | 0x200U;
}
std::int32_t time_ticks(float current, float interval) noexcept {
    return _mm_cvttss_si32(
        _mm_add_ss(_mm_div_ss(_mm_set_ss(current), _mm_set_ss(interval)), _mm_set_ss(0.5f)));
}
void select_mode(ExploitState& state, void* player, const ExploitRuntimeSettings& settings,
                 const ExploitRuntimeBindings& bindings, const std::uint32_t& command_flags) {
    const auto rules = bindings.game_rules ? *bindings.game_rules : nullptr;
    const auto& offsets = bindings.offsets;
    const bool rule0 = rules && field<std::uint8_t>(rules, offsets.rule_bf567619);
    const bool rule1 = rule0 && field<std::uint8_t>(rules, offsets.rule_dda99ab5);
    const auto rule_mode = rule1 ? field<std::int32_t>(rules, offsets.rule_b7ea8fda) : 0;
    state.budget = network_command_budget(rules != nullptr, rule0, rule1, rule_mode,
                                          settings.configured_budget_43469274);
    state.mode = 0;
    state.minimum = sub(state.budget, 1);
    if (!engine::invoke<bool>(bindings.engine_client, 26) || !bindings.checksum || state.budget < 7)
        return;
    const auto freeze_rules = bindings.game_rules ? *bindings.game_rules : nullptr;
    if (freeze_rules && field<std::uint8_t>(freeze_rules, bindings.firing.offsets.rule_d720aea6) &&
        bindings.freeze_entity_43475b74 &&
        field<std::int32_t>(bindings.freeze_entity_43475b74, 0x20) > 1)
        return;
    if (field<std::uint8_t>(player, offsets.player_block_9d920e35))
        return;
    if ((command_flags & 0x40U) ||
        hotkey_active(settings.command_block, exploit_control_keys(settings)) ||
        (command_flags & 0x100U))
        return;
    if (exploit_control_active(settings, ExploitControl::fake_peek))
        state.mode = 3;
    else if (exploit_control_active(settings, ExploitControl::double_tap))
        state.mode = 1;
    else if (exploit_control_active(settings, ExploitControl::on_shot))
        state.mode = 2;
    else
        return;
    state.minimum = (std::max)(1, exploit_fake_lag_setting(settings));
    state.limit = std::clamp(sub(sub(state.budget, exploit_fake_lag_setting(settings)), 1), 1, 15);
    if (state.mode == 2)
        state.flags &= ~1U;
}
void* active_weapon(void* player, const ExploitPhase1Bindings& bindings) {
    const auto handle = field<std::uint32_t>(player, bindings.active_weapon_cd812654);
    return handle == 0xFFFFFFFFU ? nullptr : engine::invoke<void*>(bindings.entity_list, 4, handle);
}
}

bool exploit_phase1_record_pending(const ExploitPhase1Bindings& bindings, float interval) {
    const auto records = misc::grenade_owner_warning_records(bindings.record_owner_43475aac);
    for (const auto& record : records) {
        if (record.type != 0 || !(record.damage > 0.0f) || record.handle == UINT32_MAX)
            continue;
        const auto tick_time = _mm_mul_ss(
            _mm_cvtepi32_ps(_mm_cvtsi32_si128(bindings.tick_43475a60)), _mm_set_ss(interval));
        const auto remaining = _mm_sub_ss(_mm_set_ss(record.end_time), tick_time);
        if (0.5f >= _mm_cvtss_f32(remaining))
            return true;
    }
    return false;
}

void notify_exploit_recharge_observers(const ExploitPhase1Bindings& bindings, void* player,
                                       std::int32_t command_number) {
    notify_exploit_recharge_observers(bindings.recharge_observers, player, command_number);
}

void process_exploit_phase1(ExploitRuntimeState& runtime, ExploitPhaseState& phases, void* player,
                            client::Command& command, std::uint8_t& send_packet,
                            const ExploitRuntimeSettings& settings,
                            const ExploitRuntimeBindings& native,
                            const ExploitPhase1Bindings& bindings, std::uint32_t& command_flags) {
    auto& state = runtime.network;
    if (*native.choked_commands == 0)
        state.flags = (state.flags & ~0x54U) | ((state.flags & 2U) << 1) | 3U;
    command_flags &= ~0x800U;
    state.flags &= ~0x20U;
    select_mode(state, player, settings, native, command_flags);
    if (state.mode == 0) {
        if (state.available > 0)
            state.minimum = (std::max)(1, exploit_fake_lag_setting(settings));
        state.timer_124 = 0.0f;
        if (state.available > 0)
            force_send(command_flags);
        return;
    }
    const auto now = time_ticks(*native.current_time, *native.tick_interval);
    if (now < state.shot_tick && sub(state.shot_tick, now) < 16) {
        state.flags |= 0x20U;
        force_send(command_flags);
    }
    if (state.available >= state.limit) {
        const auto remaining = (std::max)(0, sub(state.budget, *native.choked_commands));
        if (state.available > remaining)
            state.available = remaining;
    }
    if (*native.choked_commands != 0)
        return;
    if (command_flags & 0x200000U) {
        command_flags &= ~0x200000U;
        state.timer_124 = 0.0f;
    }
    if (command.movement.buttons & 0x801U) {
        auto* weapon = active_weapon(player, bindings);
        if (!weapon || engine::invoke<std::int32_t>(weapon, 455) != 9)
            return;
    }
    if (exploit_phase1_record_pending(bindings, *native.tick_interval))
        return;
    // The working 433AC04B..433AC067 path retains the hotkey query, then
    // continues at 433AC16D without the command-flag or target veto.
    if (bindings.rage.enabled.value)
        (void)hotkey_active(bindings.rage.activation.value.binding, exploit_control_keys(settings));
    if (state.timer_124 > 0.0f)
        return;
    const auto acknowledged = *native.command_ack;
    if (acknowledged != phases.observed_ack_10c) {
        phases.observed_ack_10c = acknowledged;
        if (state.available >= state.limit && acknowledged >= phases.recharge_ready_command_11c &&
            state.history_count == 8 && !(state.flags & 8U)) {
            std::int32_t maximum = 0;
            for (std::int32_t index = 0; index < state.history_count; ++index)
                maximum =
                    (std::max)(maximum,
                               state.history[(state.history_first + std::uint32_t(index)) & 7U]);
            if (state.available > maximum && sub(state.available, maximum) >= 2) {
                state.history_count = 0;
                state.history_first = 0;
                state.available = maximum;
                phases.recharge_ready_command_11c = (std::numeric_limits<std::int32_t>::max)();
            }
        }
    }
    if (state.available >= state.limit)
        return;
    // Working 433AC224 -> 434230A7 preserves weapon changes as real commands.
    if (command.weapon_select != 0)
        return;
    command.tick = 0x7F7FFFFF;
    state.available = add(state.available, 1);
    state.history_count = 0;
    state.history_first = 0;
    if (state.available >= state.limit) {
        phases.recharge_ready_command_11c = add(command.number, 1);
        state.last_command = command.number;
    }
    force_send(command_flags);
    send_packet = 1;
    notify_exploit_recharge_observers(bindings, player, command.number);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: exploit_phase7.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* [dedup read] */
/* deduplicated helper */
/* [dedup add] */
/* [dedup subtract] */
std::uint32_t address(const void* object) noexcept {
    return std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
float multiply(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
float sum(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
MovementVector phase7_movement_vector(const std::array<float, 3>& v) noexcept {
    return {v[0], v[1], v[2]};
}
MovementAngles angles(const std::array<float, 3>& a) noexcept {
    return {a[0], a[1], a[2]};
}
/* [dedup frame_weapon] */
std::int32_t process_ticks(const ExploitRuntimeSettings& settings, float interval) noexcept {
    auto time =
        _mm_div_ss(_mm_cvtepi32_ps(_mm_cvtsi32_si128(settings.process_milliseconds_43468290)),
                   _mm_set_ss(1000.0f));
    time = _mm_min_ss(_mm_max_ss(time, _mm_setzero_ps()), _mm_set_ss(1.0f));
    return _mm_cvttss_si32(_mm_add_ss(_mm_div_ss(time, _mm_set_ss(interval)), _mm_set_ss(0.5f)));
}
void apply_buttons(void* player, const client::Command& command,
                   const ExploitPhase7Offsets& o) noexcept {
    const auto old = read<std::uint32_t>(player, o.buttons_4746d093);
    write(player, o.previous_buttons_e765ed03, old);
    write(player, o.buttons_4746d093, command.movement.buttons);
    const auto changed = read<std::uint32_t>(player, o.previous_buttons_e765ed03) ^
                         read<std::uint32_t>(player, o.buttons_4746d093);
    write(player, o.pressed_buttons_3db8eb39,
          changed & read<std::uint32_t>(player, o.buttons_4746d093));
    write(player, o.released_buttons_356b64e8,
          changed & ~read<std::uint32_t>(player, o.buttons_4746d093));
}
struct WeaponTime {
    float* target = nullptr;
    std::uint32_t saved = 0;
    WeaponTime(void* weapon, const client::CommandCaptureOffsets& o, float now) noexcept {
        if (!weapon)
            return;
        target =
            reinterpret_cast<float*>(static_cast<std::byte*>(weapon) + o.weapon_field_d2563274 - 8);
        std::memcpy(&saved, target, 4);
        std::memcpy(target, &now, 4);
    }
    void restore() noexcept {
        if (target) {
            std::memcpy(target, &saved, 4);
            target = nullptr;
        }
    }
};

// The two identical433B0D5C/CE0FE/D7606/849B5/CC0F2 sequences occur before
// cloning and after each generated movement/AA pass. Every tick reads current
// ConVars, player properties and collision interfaces through canonical bodies.
bool predictive_attack_clear(const client::CommandFrame& frame, void* player,
                             ExploitRuntimeBindings& runtime, ExploitPhase7Bindings& bindings) {
    if (!(frame.flags & 0x400U) || !frame.weapon_address || !(frame.flags & 0x40000U))
        return false;
    auto* weapon = frame_weapon(frame);
    if (!classified_packet_weapon(
            read<std::uint16_t>(weapon, runtime.firing.offsets.weapon_definition),
            runtime.weapon_classification))
        return false;
    if (!hotkey_active(bindings.predictive_attack_key, bindings.input) ||
        bindings.target.handle == -1)
        return false;
    auto* target =
        engine::invoke<void*>(bindings.capture.globals().entity_list, 4, bindings.target.handle);
    if (!target)
        return false;
    const auto speed = rage_stop_speed(weapon);
    const auto threshold = multiply(speed, speed);
    MovementSimulation simulation;
    const MovementCaptureInput movement{bindings.movement.offsets, bindings.movement.convars,
                                        *runtime.tick_interval, &bindings.movement_environment};
    initialize_live_movement_simulation(simulation, player, phase7_movement_vector(frame.origin_10c),
                                        phase7_movement_vector(frame.velocity_150), frame.player_flags_7c,
                                        std::uint32_t(add(frame.tick_base_c8, 1)), movement);
    for (std::int32_t i = 0; i < 14; ++i) {
        const auto square = sum(multiply(simulation.velocity.x, simulation.velocity.x),
                                multiply(simulation.velocity.y, simulation.velocity.y));
        if (threshold >= square)
            break;
        auto command = frame.previous_command;
        prepare_rage_stop_command(frame, player, command, simulation, bindings.stopping);
        simulate_live_movement_tick(simulation, bindings.collision, bindings.movement_environment,
                                    player, command.movement, angles(command.angles));
    }
    const auto eye = rage::target_eye_position(bindings.targeting, target);
    const MovementVector start{simulation.position.x, simulation.position.y,
                               sum(simulation.position.z, simulation.view_offset_z)};
    const auto ray = make_movement_hull_ray(start, {eye[0], eye[1], eye[2]}, {0, 0, 0}, {0, 0, 0});
    MovementCollisionTrace trace;
    trace.fraction = 1.0f;
    trace.all_solid = trace.start_solid = 0;
    trace.hitgroup = -1;
    trace.entity = 0;
    trace.hitbox = -1;
    trace_movement_world(bindings.collision, ray, 0x600400BU, trace, bindings.transform_override);
    return !(1.0f > trace.fraction) && !trace.all_solid && !trace.start_solid;
}

void capture_generated_frame(client::CommandFrame& frame, void* player, client::Command& command,
                             ExploitRuntimeBindings& runtime, ExploitPhase7Bindings& bindings) {
    const auto capture = bindings.capture.recapture(player);
    const auto& common = capture.offsets.common;
    const auto& o = bindings.capture_offsets;
    frame.angles_118 = command.angles;
    frame.input_side_80 = command.movement.side_move;
    frame.input_forward_78 = command.movement.forward_move;
    write(&frame, 0xF8, read<std::uint32_t>(player, common.player_flags));
    const auto tick = read<std::int32_t>(player, runtime.offsets.tick_base_3d10b94c);
    write(&frame, 0x88, tick);
    frame.tick_base_c8 = tick;
    const auto duck = read<float>(player, common.duck_amount);
    write(&frame, 0x1A4, duck);
    frame.duck_amount_98 = duck;
    write(&frame, 0x8C, read<std::array<float, 3>>(player, o.vector_48442721));
    const auto vy = read<float>(player, common.velocity + 4),
               vx = read<float>(player, common.velocity);
    frame.previous_speed_190 =
        _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(sum(multiply(vx, vx), multiply(vy, vy)))));
    frame.origin_168 = *engine::invoke<const std::array<float, 3>*>(player, 10);
    frame.previous_velocity_180 = read<std::array<float, 3>>(player, common.velocity);
    const auto handle = read<std::uint32_t>(player, capture.offsets.active_weapon);
    auto* weapon =
        handle == UINT32_MAX ? nullptr : engine::invoke<void*>(capture.entity_list, 4, handle);
    frame.weapon_address = address(weapon);
    if (weapon) {
        write(&frame, 0x194, read<float>(weapon, common.weapon_field_6777c9ce));
        write(&frame, 0xF4, read<float>(weapon, o.weapon_field_d2563274));
    }
    frame.flags &= ~0x400000U;
    write(&frame, 0x1B4, 0U);
    client::capture_command_physical_frame(frame, command, capture,
                                           client::CommandCapturePass::initial);
    frame.field_108 = frame.field_c4;
    frame.field_1d0 = read<std::uint32_t>(player, o.team);
    frame.flags |= 4U;
}

void run_generated_player(void* player, client::Command& command,
                          const client::CommandFrame& original, ExploitPhase7Bindings& bindings,
                          float current_time) {
    WeaponTime weapon_time(frame_weapon(original), bindings.capture_offsets, current_time);
    if (bindings.prediction_bindings.flag_head)
        prediction::set_prediction_flags(*bindings.prediction_bindings.flag_head, 1);
    client::save_final_command(command);
    auto* copy = static_cast<std::byte*>(player) + bindings.offsets.command_copy_5df1a4bc;
    write(copy, 0, std::uint8_t(1));
    if (copy + 4 != reinterpret_cast<std::byte*>(&command))
        std::memcpy(copy + 4, &command, sizeof command);
    write(copy, 0x68, command.number);
    write(player, bindings.offsets.command_marker_a249061a, -1);
    client::CommandCopyOverrideScope guard(bindings.transform_override);
    engine::invoke<void>(player, 155);
    engine::invoke<const void*>(player, 10);
    guard.restore();
    if (bindings.prediction_bindings.flag_head)
        prediction::set_prediction_flags(*bindings.prediction_bindings.flag_head, 0);
    weapon_time.restore();
}
}

void process_exploit_phase7(ExploitRuntimeState& runtime, ExploitPhaseState& phase,
                            client::CommandFrame& frame, void* player, client::Command& command,
                            std::uint8_t& send, const ExploitRuntimeSettings& settings,
                            ExploitRuntimeBindings& live, ExploitPhase7Bindings& bindings,
                            std::uint32_t& flags) {
    auto& state = runtime.network;
    if (!send || read<std::uint8_t>(player, live.offsets.player_block_9d920e35))
        return;
    if ((state.flags & 9U) != 9U) {
        if (state.mode == 2 && bindings.last_attack_command > *live.last_outgoing &&
            bindings.last_attack_command <= command.number)
            frame.flags |= 0x10000000U;
        return;
    }
    auto count =
        (std::min)(subtract(subtract(state.budget, *live.choked_commands), 1), state.available);
    for (std::int32_t i = 0; i < state.history_count; ++i)
        count = (std::min)(count, state.history[(state.history_first + std::uint32_t(i)) & 7U]);
    if (count <= 0)
        return;
    state.flags &= ~8U;
    state.timer_124 = 0.5f;
    const auto saved_tick = read<std::int32_t>(player, live.offsets.tick_base_3d10b94c);
    auto* const saved_time_target = live.current_time;
    const auto saved_time = *saved_time_target;
    const auto prediction =
        prediction::bind_command_prediction(bindings.prediction_bindings, live.prediction, player,
                                            command, bindings.prediction_player_offsets);
    auto targets = prediction.targets;
    lagcomp::PredictionScope outer(targets, address(player), address(&command),
                                   std::uint32_t(command.random_seed));
    if (std::uint32_t(command.tick) != 0x7F7FFFFFU) {
        if (state.last_command != *live.last_outgoing) {
            const auto process = process_ticks(settings, *live.tick_interval);
            const auto latency = average_latency_ticks(live.timing.latency);
            const auto adjusted = adjusted_command_tick(
                live.timing.command_tick, live.timing.have_ack, live.timing.acknowledged,
                live.timing.have_sent, live.timing.sent);
            const auto tick = add(add(add(subtract(process, count), 1), latency), adjusted);
            write(player, live.offsets.tick_base_3d10b94c, tick);
        }
        engine::invoke<void>(bindings.prediction_bindings.move_helper, 1, player);
        *live.current_time =
            multiply(float(read<std::int32_t>(player, live.offsets.tick_base_3d10b94c)),
                     *live.tick_interval);
        apply_buttons(player, command, bindings.offsets);
        if (auto* weapon = frame_weapon(frame)) {
            write(weapon, bindings.capture_offsets.weapon_field_d2563274,
                  read<float>(&frame, 0xF4));
            write(weapon, bindings.capture.physical_offsets().common.weapon_field_6777c9ce,
                  read<float>(&frame, 0x194));
        }
        WeaponTime weapon_time(frame_weapon(frame), bindings.capture_offsets, *live.current_time);
        engine::invoke<void>(player, 319);
        write(player, live.offsets.tick_base_3d10b94c,
              add(read<std::int32_t>(player, live.offsets.tick_base_3d10b94c), 1));
        engine::invoke<void>(bindings.prediction_bindings.move_helper, 1,
                             static_cast<void*>(nullptr));
        bindings.prediction_observers.finish_forward(player, command);
        weapon_time.restore();
    }
    bool initial_clear = false;
    if (command.movement.buttons & 1U) {
        bindings.attack_mask = ~frame.field_1dc & bindings.attack_policy;
        if (flags & 0x20000U)
            bindings.attack_mask &= ~4U;
    } else if (!(flags & 0x20000U) && bindings.predictive_attack)
        initial_clear = predictive_attack_clear(frame, player, live, bindings);
    bool clear = false;
    auto number = frame.previous_command.number;
    auto clone_choked = frame.choked_15c;
    for (std::int32_t i = 0; i < count; ++i) {
        const auto previous_clear = clear;
        clone_choked = add(clone_choked, 1);
        number = add(number, 1);
        const auto index = std::uint32_t(number) % 150U;
        auto& generated = live.commands[index];
        const bool last = i == subtract(count, 1);
        std::uint8_t generated_send = last;
        if (&generated != &frame.previous_command)
            generated = frame.previous_command;
        generated.number = number;
        generated.tick = command.tick;
        generated.movement.buttons |= command.movement.buttons & 0x400000U;
        generated.predicted = 1;
        *targets.player_current_command = address(&generated);
        generated.random_seed = std::int32_t(bindings.seed_generator(command.number) & 0x7FFFFFFFU);
        *bindings.prediction_seed = generated.random_seed;
        *live.current_time =
            multiply(float(read<std::int32_t>(player, live.offsets.tick_base_3d10b94c)),
                     *live.tick_interval);
        client::CommandFrame cloned;
        client::initialize_command_frame(cloned);
        cloned = frame;
        cloned.previous_command.number = generated.number;
        cloned.previous_command.tick = generated.tick;
        cloned.command_number_ac = generated.number;
        cloned.send_packet_address = address(&generated_send);
        cloned.choked_15c = clone_choked;
        capture_generated_frame(cloned, player, generated, live, bindings);
        if (last)
            cloned.choke_limit_fc = clone_choked;
        auto& script_data = bindings.scripts.storage().data();
        auto environment = bindings.angle_environment;
        environment.shared_player = script_data.native_cell<void**>(0x4346A3A4u);
        const client::ScriptCommandBindings script{
            &bindings.scripts.bridge(),
            script_data.native_cell<client::ScriptCommand*>(0x43479300u),
            "setup_command",
            script_data.native_cell<const std::int32_t*>(0x434757DCu),
            &script_data.native_cell<std::uint8_t>(0x43475A5Cu),
            script::source_lua_runtime_bindings().lua.pushlightuserdata,
            environment};
        client::dispatch_command_script(script, generated, cloned.angles_118[1], generated_send,
                                        flags);
        client::process_generated_movement_command(bindings.owners, cloned, player, generated,
                                                   generated_send);
        const auto saved_limit = cloned.choke_limit_fc;
        cloned.choke_limit_fc = clone_choked;
        const auto requested_send = generated_send;
        generated_send = 1;
        client::process_generated_angle_command(bindings.owners, cloned, player, generated,
                                                generated_send);
        generated_send = requested_send;
        cloned.choke_limit_fc = saved_limit;
        if (!(flags & 0x20000U)) {
            if (command.movement.buttons & 3U)
                clear = previous_clear;
            else if (!clear && !initial_clear && bindings.predictive_attack)
                clear = predictive_attack_clear(cloned, player, live, bindings);
            if (clear) {
                flags |= 8U;
                bindings.attack_mask |= 4U;
            }
        }
        const client::CommandButtonPolicy policy{bindings.inverted_commands,
                                                 bindings.anti_aim_other.leg_movement.value,
                                                 *live.choked_commands};
        client::reorient_command_movement(generated, cloned.angles_118, cloned.state_104,
                                          (cloned.flags & 0x40000U) != 0, !last, policy,
                                          bindings.trigonometry);
        environment.shared_player = script_data.native_cell<void**>(0x4346A3A4u);
        client::sanitize_command(generated, (cloned.flags & 8U) != 0, environment);
        run_generated_player(player, generated, frame, bindings, *live.current_time);
        write(player, live.offsets.tick_base_3d10b94c,
              subtract(read<std::int32_t>(player, live.offsets.tick_base_3d10b94c), 1));
        generated.predicted = 0;
        client::save_final_command(generated);
        auto& verified = live.verified_commands[index];
        if (&verified.command != &generated)
            verified.command = generated;
        verified.checksum = live.checksum ? live.checksum(&generated) : 0;
        if (auto* channel = engine::invoke<void*>(live.engine_client, 78))
            engine::invoke<void>(channel, 45);
        *live.choked_commands = add(*live.choked_commands, 1);
        generated_send = last;
        client::process_restricted_player_command(
            client::prediction_history_shared(bindings.restricted_owner), player, &generated,
            &generated_send, false, bindings.restricted);
        write(player, live.offsets.tick_base_3d10b94c,
              add(read<std::int32_t>(player, live.offsets.tick_base_3d10b94c), 1));
    }
    bindings.attack_mask = UINT32_MAX;
    notify_exploit_generated_observers(bindings.generated_observers, player, number, count);
    state.available = state.mode ? (std::max)(subtract(state.available, count), 0) : 0;
    if (auto* const history = live.prediction_history_owner)
        history->state.prediction_stamp = *live.history_stamp_4346a334;
    bindings.next_generated_command = add(number, 1);
    outer.restore();
    *saved_time_target = saved_time;
    write(player, live.offsets.tick_base_3d10b94c, saved_tick);
    send = 1;
    state.history_count = 0;
    state.history_first = 0;
    phase.recharge_ready_command_11c = (std::numeric_limits<std::int32_t>::max)();
    flags = (flags & ~0x400U) | 0x200U;
}
} //namespace skeet::network

// ============================================================================
// Subcomponent: exploit_runtime.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
/* [dedup store] */
/* deduplicated helper */
/* [dedup sub] */
/* [dedup force_send] */
/* [dedup time_ticks] */
/* [dedup process_ticks] */
std::int32_t available_commands(const ExploitState& state, std::int32_t choked) noexcept {
    auto available = (std::min)(sub(sub(state.budget, choked), 1), state.available);
    for (std::int32_t i = 0; i < state.history_count; ++i)
        available =
            (std::min)(available, state.history[(state.history_first + std::uint32_t(i)) & 7U]);
    return available;
}
std::int32_t history_available(const ExploitState& state) noexcept {
    auto available = state.available;
    for (std::int32_t i = 0; i < state.history_count; ++i)
        available =
            (std::min)(available, state.history[(state.history_first + std::uint32_t(i)) & 7U]);
    return available;
}
bool classified_weapon(const ExploitRuntimeBindings& bindings, void* weapon) noexcept {
    return weapon && classified_packet_weapon(
                         field<std::uint16_t>(weapon, bindings.firing.offsets.weapon_definition),
                         bindings.weapon_classification);
}
bool shot_window(const ExploitState& state, const ExploitRuntimeBindings& bindings) noexcept {
    const auto now = time_ticks(*bindings.current_time, *bindings.tick_interval);
    return add(now, 2) <= state.shot_tick && sub(state.shot_tick, now) < 16;
}
void flush(ExploitRuntimeState& runtime, const client::Command& command,
           std::uint32_t& flags) noexcept {
    runtime.network.flags &= ~0x12U;
    runtime.last_flush_command_134 = command.number;
    force_send(flags);
}
/* [dedup select_mode] */
void copy_snapshot(ExploitRuntimeBindings& bindings, std::uint32_t previous, std::uint32_t next) {
    //433671B7 reads the owner after the checksum callback on each clone.
    if (auto* const owner = bindings.prediction_snapshots_owner)
        client::copy_prediction_snapshot(*owner, std::bit_cast<std::int32_t>(previous),
                                         std::bit_cast<std::int32_t>(next));
}
void adjust_prediction_history(ExploitRuntimeBindings& bindings,
                               client::EntityCommandOwner* stamp_owner, std::int32_t command,
                               std::int32_t tick) {
    if (stamp_owner)
        stamp_owner->state.prediction_stamp = *bindings.history_stamp_4346a334;
    const auto last_command = add(add(*bindings.last_outgoing, 1), *bindings.choked_commands);
    for (std::int32_t count = 0; count < 150; ++count, command = add(command, 1)) {
        if (last_command < command)
            break;
        //433952F0 reloads43475898 for every lookup; the stamp owner above is
        //separately captured by its caller and need not be that current owner.
        auto* const owner = bindings.prediction_history_owner;
        if (!owner)
            break;
        auto& entry = owner->state.history[command % 150];
        if (entry.command_number != command)
            break;
        entry.state.flags |= 8U;
        store(&entry.state, 0, tick);
        const auto& input = bindings.commands[std::uint32_t(command) % 150];
        if (input.number != command)
            break;
        if (input.tick != 0x7F7FFFFF) {
            tick = add(tick, 1);
            store(&entry.state, 0, tick);
        }
    }
    store<std::int32_t>(bindings.prediction, 0xC, -1);
    store<std::int32_t>(bindings.prediction, 0x1C, 0);
    store<std::int32_t>(bindings.prediction, 0x24, 1);
}
void transition(ExploitRuntimeState& runtime, client::CommandFrame& frame, void* player,
                client::Command& command, bool shifting, std::int32_t pending,
                const ExploitRuntimeSettings& settings, ExploitRuntimeBindings& bindings) {
    auto& state = runtime.network;
    const auto latency = average_latency_ticks(bindings.timing.latency);
    const auto adjusted = adjusted_command_tick(
        bindings.timing.command_tick, bindings.timing.have_ack, bindings.timing.acknowledged,
        bindings.timing.have_sent, bindings.timing.sent);
    const auto process = process_ticks(settings, *bindings.tick_interval);
    auto number = add(*bindings.last_outgoing, 1);
    if (shifting) {
        const auto tick = add(add(add(sub(process, pending), 1), latency), adjusted);
        auto& stored = runtime.command_ticks[number % 150];
        if (stored.command != number) {
            auto* const stamp_owner = bindings.prediction_history_owner;
            stored = {number, tick};
            adjust_prediction_history(bindings, stamp_owner, number, tick);
            const auto tick_base = add(*bindings.choked_commands, tick);
            store(&frame, 0xC8, tick_base);
            store(player, bindings.offsets.tick_base_3d10b94c, tick_base);
        }
    } else {
        const auto tick = add(add(latency, adjusted), sub(process, *bindings.choked_commands));
        //4336727E saves this owner for both stamp writes, including433673E4.
        auto* const stamp_owner = bindings.prediction_history_owner;
        runtime.command_ticks[number % 150] = {number, tick};
        adjust_prediction_history(bindings, stamp_owner, number, tick);
        if (!(frame.flags & 0x20000000U)) {
            state.shot_tick = add(*bindings.choked_commands, tick);
            runtime.transition_command_110 = command.number;
        }
        const auto tick_base = add(*bindings.choked_commands, tick);
        store(&frame, 0xC8, tick_base);
        store(player, bindings.offsets.tick_base_3d10b94c, tick_base);
        if (state.mode != 0 && !(state.flags & 0x80U)) {
            const auto restored_tick = add(field<std::int32_t>(&frame, 0xCC), 1);
            number = add(command.number, 1);
            runtime.command_ticks[number % 150] = {number, restored_tick};
            adjust_prediction_history(bindings, stamp_owner, number, restored_tick);
        }
    }
    *bindings.current_time =
        float(field<std::int32_t>(player, bindings.offsets.tick_base_3d10b94c)) *
        *bindings.tick_interval;
    //433674CF..433674ED captures the actual registry and calls every slot2.
    //Their bodies are genuine RET8, but the native traversal remains present.
    bindings.recharge_observers.notify_cloned(player, command);
}
}

void process_exploit_runtime(ExploitRuntimeState& runtime, client::CommandFrame& frame,
                             void* player, void* weapon, client::Command& command,
                             std::uint8_t& send_packet, const ExploitRuntimeSettings& settings,
                             ExploitRuntimeBindings& bindings, std::uint32_t& flags) {
    auto& state = runtime.network;
    const bool previous_shift = state.last_command == *bindings.last_outgoing;
    auto pending = pending_exploit_commands(state, *bindings.choked_commands);
    select_mode(state, player, settings, bindings, flags);
    bool shifting = false;
    if (state.mode == 0) {
        if (state.available > 0) {
            state.minimum = (std::max)(1, exploit_fake_lag_setting(settings));
            state.flags |= 9U;
            shifting = true;
        }
    } else {
        if (state.available < state.limit)
            flags &= ~0x100000U;
        else if (flags & 0x100000U) {
            flags &= ~0x100000U;
            state.flags |= 8U;
        }
        if (settings.special_weapon_43467560 && state.available >= state.limit) {
            if (command.weapon_select == 0 && weapon && frame.weapon_kind_124 == 9 &&
                !(command.movement.buttons & 0x801U) &&
                !field<std::uint8_t>(weapon, bindings.offsets.weapon_byte_e1df7e9b) &&
                field<float>(weapon, bindings.offsets.weapon_time_3b93667b) > 0.0f &&
                field<std::uint8_t>(weapon, bindings.offsets.weapon_byte_e865a877)) {
                state.flags |= 0x40U;
                force_send(flags);
            }
            if ((state.flags & 0x40U) && *bindings.choked_commands != 0) {
                send_packet = 1;
                flush(runtime, command, flags);
            }
        }
        const auto previous_low_health = runtime.low_health_120;
        auto health_steps = (std::numeric_limits<std::int32_t>::max)();
        if (bindings.shared_player_43475808 && settings.health_divisor_4346db30 >= 1) {
            const auto health = field<std::int32_t>(bindings.shared_player_43475808,
                                                    bindings.offsets.player_health_9bb80163);
            health_steps = (std::max)(1, add(sub(health, 1), settings.health_divisor_4346db30) /
                                             settings.health_divisor_4346db30);
        }
        const auto& move = frame.previous_command.movement;
        const float magnitude =
            (move.forward_move * move.forward_move + move.side_move * move.side_move) +
            move.up_move * move.up_move;
        if (state.mode == 1 && state.available >= state.limit &&
            exploit_double_tap_setting(settings) == 1 &&
            (classified_weapon(bindings, weapon) || frame.weapon_kind_124 == 9 ||
             frame.weapon_kind_124 == 0) &&
            frame.weapon_id_d8 != 64 && !previous_low_health && health_steps < 6 &&
            !(state.flags & 8U) && !(command.movement.buttons & 0x801U) &&
            (frame.scalar_c0 > 2.0f || magnitude > 16.0f) &&
            runtime.last_flush_command_134 < *bindings.command_ack && !shot_window(state, bindings))
            flush(runtime, command, flags);
        runtime.low_health_120 = health_steps < 6;
        if (state.mode == 1) {
            const bool selected_matches =
                !command.weapon_select || !weapon ||
                command.weapon_select ==
                    engine::invoke<std::int32_t>(static_cast<std::byte*>(weapon) + 8, 10);
            if (selected_matches &&
                (classified_weapon(bindings, weapon) || frame.weapon_kind_124 == 0)) {
                auto firing = bindings.firing;
                firing.game_rules = bindings.game_rules;
                firing.current_time = bindings.current_time;
                const auto fire =
                    skeet::weapon::evaluate_fire_state(firing, player, weapon, command);
                if (fire.burst_ready || (fire.primary_ready && (command.movement.buttons & 1U)) ||
                    (fire.secondary_ready && (command.movement.buttons & 0x800U) &&
                     (frame.weapon_id_d8 == 64 || frame.weapon_kind_124 == 0))) {
                    if (!(frame.flags & 0x100000U) && frame.weapon_id_d8 != 31 &&
                        state.available >= state.limit) {
                        state.shot_command =
                            sub(add(frame.command_number_ac,
                                    available_commands(state, *bindings.choked_commands)),
                                1);
                        state.flags |= 8U;
                        if (frame.flags & 0x20000000U) {
                            state.flags = (state.flags & ~1U) | 8U;
                            force_send(flags);
                        } else if ((frame.weapon_id_d8 == 64 && (command.movement.buttons & 1U)) ||
                                   (exploit_double_tap_setting(settings) == 1 &&
                                    shot_window(state, bindings))) {
                            state.flags = (state.flags & ~1U) | 8U;
                            send_packet = 1;
                            flags = (flags & ~0x200U) | 0x400U;
                        }
                    }
                    state.timer_124 = 0.5f;
                }
            }
            if (state.available >= state.limit && exploit_double_tap_setting(settings) == 1 &&
                frame.weapon_id_d8 != 64 && (state.flags & 8U) && (flags & 0x20000U) &&
                shot_window(state, bindings)) {
                state.flags &= ~1U;
                force_send(flags);
            }
        }
        if (state.mode != 0 && state.available >= state.limit && (flags & 0x80000U)) {
            flags &= ~0x80000U;
            state.flags |= 0x10U;
        }
        if (state.available >= state.limit && send_packet && (state.flags & 0x10U) &&
            !(state.flags & 0x20U) && (state.flags & 4U) && frame.weapon_id_d8 != 64 &&
            frame.weapon_kind_124 != 9) {
            if (!(frame.flags & 0x20000000U) &&
                runtime.last_flush_command_134 < *bindings.command_ack)
                flush(runtime, command, flags);
        }
        if (frame.flags & 0x20000000U)
            state.flags = (state.flags & ~0x13U) | 8U;
        if (state.available >= state.limit && send_packet) {
            if ((state.flags & 9U) == 9U)
                shifting = true;
            else if (state.flags & 2U) {
                const auto available = history_available(state);
                const auto maximum =
                    (std::min)(state.budget, add(add(available, 1), *bindings.choked_commands));
                const auto count = sub(state.budget, *bindings.choked_commands);
                auto number = std::uint32_t(command.number);
                for (std::int32_t i = 0; i < count; ++i) {
                    ++number;
                    auto& output = bindings.commands[number % 150];
                    if (&output != &command)
                        output = command;
                    output.number = std::bit_cast<std::int32_t>(number);
                    output.tick = 0x7F7FFFFF;
                    output.movement.buttons |= command.movement.buttons & 0x420024U;
                    output.predicted = 1;
                    client::save_final_command(output);
                    auto& verified = bindings.verified_commands[number % 150];
                    if (&verified.command != &output)
                        verified.command = output;
                    verified.checksum = bindings.checksum ? bindings.checksum(&output) : 0;
                    *bindings.choked_commands = add(*bindings.choked_commands, 1);
                    copy_snapshot(bindings, std::uint32_t(command.number), number);
                }
                runtime.generated_commands[std::uint32_t(command.number) % 150] = command.number;
                state.last_command = command.number;
                send_packet = 1;
                pending = (std::min)(add(*bindings.choked_commands, 1), maximum);
                shifting = true;
            }
        }
    }
    if (send_packet && shifting != previous_shift)
        transition(runtime, frame, player, command, shifting, pending, settings, bindings);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: incoming_acknowledgement.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* [dedup field] */
std::int32_t engine_tick(IncomingUpdateGlobals& g) {
    const auto time = engine::invoke<float>(g.engine_client, 14);
    const auto interval = field<float>(g.engine_globals, 0x20);
    return _mm_cvtt_ss2si(_mm_set_ss(float(static_cast<long double>(time) / interval + 0.5L)));
}
std::int32_t history_tag(IncomingUpdateGlobals& g, std::int32_t command) { //43340A83
    const auto* state =
        prediction::CommandHistoryView::from_owner(g.prediction_owner).find(command);
    return state ? field<std::int32_t>(state, 0) : -1;
}
}
void reconcile_incoming_acknowledgement(void* player, bool& changed, IncomingUpdateGlobals& g,
                                        const IncomingUpdateBindings& b) {
    if (player && !engine::invoke<bool>(g.engine_client, 82) &&
        !engine::invoke<bool>(g.engine_client, 93) && *g.sequence > 0) {
        const auto tick_now = engine_tick(g);
        if (tick_now != *g.sequence)
            *g.acknowledged = *g.old_acknowledged;
        else {
            const auto command = engine::invoke<std::int32_t>(g.engine_client, 205);
            if (command != g.previous_command) {
                if (g.transition_status == 2 || g.transition_status == 1) {
                    *g.acknowledged = *g.old_acknowledged;
                    changed = true;
                }
            } else if (g.transition_status == 2) {
                const auto processed = engine::invoke<std::int32_t>(g.engine_client, 205);
                if (processed == g.previous_processed_command) {
                    const auto tick = field<std::int32_t>(player, b.tick_base_3d10b94c);
                    //433F9367 makes this call separately before its150-step loop.
                    (void)engine::invoke<std::int32_t>(g.engine_client, 205);
                    for (std::uint32_t age = 0; age < 150; ++age) {
                        const auto current = engine::invoke<std::int32_t>(g.engine_client, 205);
                        const auto candidate =
                            std::bit_cast<std::int32_t>(std::uint32_t(current) - age);
                        if (history_tag(g, candidate) == tick) {
                            *g.acknowledged = candidate;
                            break;
                        }
                    }
                }
            }
        }
    }
    g.previous_command = -1;
}
} //namespace skeet::network

// ============================================================================
// Subcomponent: incoming_update.cpp
// ============================================================================

namespace skeet::network {
namespace {
IncomingUpdateRuntime* incoming_update_runtime;
/* [dedup field] */
std::uint32_t convar_bits(const void* object, std::ptrdiff_t offset) {
    auto* owner = object;
    while (field<const void*>(owner, 0x1C) != owner)
        owner = field<const void*>(owner, 0x1C);
    return field<std::uint32_t>(owner, offset) ^
           std::uint32_t(reinterpret_cast<std::uintptr_t>(owner));
}
/* [dedup subtract] */
bool recent_sound_history(const prediction::CommandHistoryState& state, float interval) {
    const auto scaled = _mm_div_ss(_mm_set_ss(field<float>(&state, 0x68)), _mm_set_ss(interval));
    const auto tick = _mm_cvtt_ss2si(_mm_add_ss(scaled, _mm_set_ss(0.5f)));
    const auto delta = subtract(subtract(field<std::int32_t>(&state, 0), tick), 1);
    const auto sign = std::uint32_t(delta) >> 31;
    const auto magnitude = (std::uint32_t(delta) ^ (0u - sign)) + sign;
    // Native ABS uses wrapped32-bit arithmetic then a signed JGE20.
    return std::bit_cast<std::int32_t>(magnitude) < 20;
}
void retain_active_sounds(IncomingUpdateGlobals& g, const IncomingUpdateBindings& b) {
    if (!g.sounds || convar_bits(b.convar_43469238.get(), 0x30) != 0 ||
        std::bit_cast<float>(convar_bits(b.convar_4346a850.get(), 0x2C)) == 0.0f ||
        !engine::invoke<bool>(g.engine_client, 26) || engine::invoke<bool>(g.engine_client, 82) ||
        engine::invoke<bool>(g.engine_client, 93))
        return;
    const auto index = engine::invoke<std::int32_t>(g.engine_client, 12);
    auto* player = engine::invoke<void*, std::int32_t>(g.entity_list, 3, index);
    if (!player || field<std::uint8_t>(player, b.rejected_9d920e35))
        return;
    const auto number = engine::invoke<std::int32_t>(g.engine_client, 205);
    const auto* state = prediction::CommandHistoryView::from_owner(g.prediction_owner).find(number);
    if (!state || state->weapon_identity == UINT32_MAX)
        return;
    const auto identity = std::int32_t(std::uint16_t(state->weapon_identity));
    std::int32_t skipped = 0;
    auto* tree = const_cast<esp::ActiveSoundTree*>(g.sounds);
    for (;;) {
        auto ordinal = std::int32_t(0);
        auto sound = first_active_sound(*g.sounds);
        for (; sound != -1; sound = next_active_sound(*g.sounds, sound)) {
            if (tree->nodes[sound].value.source_index == identity && ++ordinal > skipped)
                break;
        }
        if (sound == -1)
            return;
        if (recent_sound_history(*state, *g.interval)) {
            release_active_sound(*tree, sound);
            tree = const_cast<esp::ActiveSoundTree*>(g.sounds);
        } else
            ++skipped;
    }
}
void record_acknowledgement_timing(std::int32_t delta, IncomingUpdateGlobals& g,
                                   const IncomingUpdateBindings& b) {
    if (delta == 0)
        return;
    const auto number = *g.acknowledged;
    const auto& sent = b.sent_commands.records[number % 150];
    if (sent.command_number != number)
        return;
    const auto adjusted = sent.adjusted_tick;
    const auto time = engine::invoke<float>(g.engine_client, 14);
    const auto interval = field<float>(g.engine_globals, 0x20);
    const auto current =
        _mm_cvtt_ss2si(_mm_set_ss(float(static_cast<long double>(time) / interval + 0.5L)));
    auto elapsed = subtract(current, adjusted);
    if (elapsed < 0)
        elapsed = 0;
    else if (elapsed > 64)
        elapsed = 64;
    if (g.timing.count != 16)
        ++g.timing.count;
    g.timing.first = (g.timing.first - 1u) & 15u;
    g.timing.values[g.timing.first] = float(elapsed);
}
}
void process_incoming_update(void* object, IncomingUpdateGlobals& g,
                             const IncomingUpdateBindings& b) {
    auto* const player = g.local_player;
    if (auto* owner = b.sound_owner) {
        const auto& context = owner->sound_context();
        esp::process_sound_overlay(context.players.identity(player), owner->state().sounds,
                                   g.sounds, context);
    }
    retain_active_sounds(g, b);
    bool changed = false;
    reconcile_incoming_acknowledgement(player, changed, g, b);
    const std::int32_t* saved_sequence = nullptr;
    std::int32_t previous_sequence = 0;
    if (changed) {
        saved_sequence = g.sequence;
        previous_sequence = *saved_sequence;
        *const_cast<std::int32_t*>(saved_sequence) = -1;
    }
    const auto delta = subtract(*g.acknowledged, *g.old_acknowledged);
    record_acknowledgement_timing(delta, g, b);
    auto** table = *static_cast<void***>(object);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    reinterpret_cast<engine::MemberFunction<void>>(metadata->original_table[6])(object);
    g.interpolation = misc::query_channel_interpolation(g.engine_client, b.latency);
    if (!changed && player) {
        const auto clock = *g.update_clock;
        if (clock == *g.sequence)
            g.receivers.incoming_complete(player, delta);
    }
    for (auto* patch = g.patches; patch; patch = patch->next)
        if (patch->enabled && patch->target)
            b.exchange(patch->target, patch->replacement);
    if (saved_sequence)
        *const_cast<std::int32_t*>(saved_sequence) = previous_sequence;
}
void bind_incoming_update_runtime(IncomingUpdateRuntime& value) noexcept {
    incoming_update_runtime = &value;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall incoming_update_hook(void* object, void*) {
#else
void incoming_update_hook(void* object, void*) {
#endif
    incoming_update_runtime->process(object);
}
} //namespace skeet::network

// ============================================================================
// Subcomponent: packet_control.cpp
// ============================================================================

namespace skeet::network {
namespace {
std::int32_t signed_bits(std::uint32_t value) noexcept {
    return std::bit_cast<std::int32_t>(value);
}
/* [dedup add] */
/* [dedup sub] */
std::int32_t absolute(std::int32_t value) noexcept {
    return value < 0 ? sub(0, value) : value;
}
float absolute(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) & 0x7fffffffU);
}
std::int32_t truncate(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
/* [dedup force_send] */
}

void reset_fake_lag(FakeLagState& state) noexcept {
    state.tracked_target = -1;
    state.last_origin = {};
    state.saved_duck_amount_bits = 0;
    state.previous_speed_delta = 0;
    state.value_7c = -1;
    state.value_8c = -1;
    state.flags = 0;
    state.target = 0;
    state.recorded_choked = 0;
    state.value_58 = 0;
    state.previous_selected = 0;
}
void prepare_fake_lag_command(FakeLagState& state, const PacketCommandContext& context,
                              std::uint32_t& flags) noexcept {
    if (context.force_on_next_command && context.engine_choked == 0 &&
        context.command_number == add(context.last_command, 1))
        force_send(flags);
    if (context.engine_choked == 0) {
        state.previous_selected = state.recorded_choked;
        state.last_origin = context.player_origin;
        state.saved_duck_amount_bits = context.player_3309c0cf;
    }
    state.recorded_choked = context.engine_choked;
}
void begin_fake_lag_tick(FakeLagState& state, const PacketFrame& frame,
                         bool& trace_result) noexcept {
    trace_result = false;
    if (frame.choked == 0) {
        state.cycle = add(state.cycle, 1);
        state.target = 0;
    }
    state.acceleration = {frame.velocity.x - frame.previous_velocity.x,
                          frame.velocity.y - frame.previous_velocity.y, 0.0F};
}

bool select_fake_lag_limit(FakeLagState& state, const PacketFrame& frame, float interval,
                           const RandomInterface& random) {
    if (state.mode == 2) {
        state.target = frame.limit;
        return false;
    }
    const auto product = signed_bits(std::uint32_t(state.variance) * std::uint32_t(frame.limit));
    const auto variance = add(product, 99) / 100;
    auto upper = frame.limit;
    auto selected = std::max(0, sub(upper, variance));
    if (state.mode == 3 && (std::uint32_t(state.cycle) & 1U)) {
        if (variance == 0)
            selected = 0;
        else {
            random.seed(random.context, state.cycle);
            // The native arithmetic shift rounds negative results down.
            selected = random.integer(random.context, 0, variance) >> 1;
        }
    } else {
        if (state.mode == 0 &&
            frame.velocity.x * frame.velocity.x + frame.velocity.y * frame.velocity.y > 4.0F) {
            float vx = frame.velocity.x, vy = frame.velocity.y;
            const float ax = (vx - frame.previous_velocity.x) * 0.5F;
            const float ay = (vy - frame.previous_velocity.y) * 0.5F;
            float x = frame.origin.x, y = frame.origin.y;
            auto step = frame.choked;
            if (step <= upper)
                do {
                    const auto next = add(step, 1);
                    const float dx = x - state.last_origin.x;
                    const float dy = y - state.last_origin.y;
                    if (dy * dy + dx * dx > 4096.0F) {
                        upper = std::min(next, upper);
                        selected = step;
                        break;
                    }
                    vx = vx + ax;
                    vy = vy + ay;
                    x = x + vx * interval;
                    y = y + vy * interval;
                    step = next;
                } while (step <= upper);
        }
        if (selected != upper) {
            random.seed(random.context, state.cycle);
            selected = random.integer(random.context, selected, upper);
            if (selected == state.previous_selected) {
                if (selected < frame.limit &&
                    (selected <= 0 || state.mode == 0 || random.integer(random.context, 0, 1) != 0))
                    selected = add(selected, 1);
                else
                    selected = sub(selected, 1);
            }
        }
    }
    state.selected = selected;
    return true;
}

void apply_fake_lag_send_gate(const FakeLagState& state, const PacketFrame& frame,
                              std::uint32_t flags, std::uint8_t& send_packet) noexcept {
    if (!(flags & 0x600U) && frame.choked < frame.limit && frame.choked < state.target)
        send_packet = 0;
}
bool fake_lag_pending(std::int32_t configured_limit, const PacketFrame& frame,
                      std::uint32_t flags) noexcept {
    return !(flags & 0x600U) && frame.choked < frame.limit &&
           (configured_limit < 1 || frame.choked < configured_limit);
}
bool velocity_trigger(const FakeLagState& state, const PacketFrame& frame) noexcept {
    const auto& v = frame.velocity;
    const auto& p = frame.previous_velocity;
    const float current =
        1.0F / (std::sqrt((v.x * v.x + v.y * v.y) + v.z * v.z) + 1.1920928955078125e-7F);
    const float previous =
        1.0F / (std::sqrt((p.x * p.x + p.y * p.y) + p.z * p.z) + 1.1920928955078125e-7F);
    const float dot = ((p.y * previous) * (v.y * current) + (p.x * previous) * (v.x * current)) +
                      (p.z * previous) * (v.z * current);
    if (!(dot >= 0.9998477101325989F))
        return true;
    return absolute((std::sqrt(v.x * v.x + v.y * v.y) - frame.previous_speed) -
                    state.previous_speed_delta) > 2.0F;
}
bool displacement_trigger(const FakeLagState& state, const PacketFrame& frame, float speed,
                          std::uint32_t buttons, float interval, float gravity) noexcept {
    if (!(speed >= 80.0F) || (frame.limit <= 7 && !(frame.flags & 0x4000U)) ||
        ((frame.flags & 0x40000U) && !(buttons & 2U)))
        return false;
    float vx = frame.velocity.x, vy = frame.velocity.y, vz = frame.velocity.z;
    float x = frame.origin.x, y = frame.origin.y, z = frame.origin.z;
    const float half_gravity = (interval * gravity) * 0.5F;
    auto step = add(frame.choked, 1);
    if (step <= frame.limit)
        do {
            vx += state.acceleration.x;
            vy += state.acceleration.y;
            const float intermediate_z = (vz + state.acceleration.z) - half_gravity;
            x = vx * interval + x;
            y = vy * interval + y;
            vz = intermediate_z - half_gravity;
            z = z + intermediate_z * interval;
            const float dx = x - state.last_origin.x, dy = y - state.last_origin.y,
                        dz = z - state.last_origin.z;
            if ((dy * dy + dx * dx) + dz * dz > 4096.0F)
                return true;
            step = add(step, 1);
        } while (step <= frame.limit);
    return false;
}
void finish_fake_lag_tick(FakeLagState& state, const PacketFrame& frame, std::int32_t target,
                          std::int32_t target_state) noexcept {
    state.previous_speed_delta =
        std::sqrt(frame.velocity.x * frame.velocity.x + frame.velocity.y * frame.velocity.y) -
        frame.previous_speed;
    state.tracked_target = target;
    state.flags = (state.flags & ~2U) | (target != -1 && target_state != 0 ? 2U : 0U);
}
bool trigger_target_change(LagTriggerState& state, float speed, std::int32_t target,
                           std::int32_t target_state, float time, std::uint32_t& flags) noexcept {
    if (!(speed > 2.0F) || target == -1 || target != state.target || (state.flags & 2U) ||
        target_state == 0 || !(absolute(state.last_time - time) > 0.2F))
        return false;
    force_send(flags);
    state.flags |= 4U;
    state.last_time = time;
    return true;
}
std::int32_t average_latency_ticks(const LatencySamples& samples) noexcept {
    if (samples.count <= 0)
        return 1;
    float sum = 0;
    for (std::int32_t i = 0; i < samples.count; ++i)
        sum += samples.values[(samples.first + std::uint32_t(i)) & 15U];
    return _mm_cvtss_si32(_mm_set_ss(sum / static_cast<float>(samples.count)));
}
std::int32_t adjusted_command_tick(std::int32_t base, bool have_ack, std::int32_t ack,
                                   bool have_sent, std::int32_t sent) noexcept {
    const auto delta = sub(sent, ack);
    if (have_ack && have_sent && std::uint32_t(sub(delta, 1)) < 9U)
        return add(base, delta);
    return base;
}
std::int32_t network_command_budget(bool have_rules, bool rule_0, bool rule_1, std::int32_t mode,
                                    std::int32_t configured) noexcept {
    if (have_rules) {
        if (!rule_0)
            return configured;
        if (rule_1 && mode == 1)
            return 3;
    }
    return 7;
}
std::int32_t next_shuffle_random(ShuffleRandom& state) noexcept {
    auto step = [&state]() {
        const auto quotient = state.seed / 127773;
        state.seed = signed_bits(std::uint32_t(state.seed) * 16807U +
                                 std::uint32_t(quotient) * std::uint32_t(-2147483647));
        if (state.seed < 0)
            state.seed = add(state.seed, 2147483647);
    };
    if (state.seed < 1 || state.previous == 0) {
        state.seed = sub(0, state.seed);
        if (state.seed < 1)
            state.seed = 1;
        for (std::int32_t j = 39; j >= 0; --j) {
            step();
            if (j < 32)
                state.table[std::uint32_t(j)] = state.seed;
        }
        state.previous = state.table[0];
    }
    step();
    // x86 computes signed division by 2^26, with its correction for negatives.
    auto index = state.previous / 67108864;
    if (index < 0 || index > 31)
        index = signed_bits(std::uint32_t(index) & 31U);
    state.previous = state.table[std::uint32_t(index)];
    state.table[std::uint32_t(index)] = state.seed;
    return state.previous;
}
std::int32_t pending_exploit_commands(const ExploitState& state, std::int32_t choked) noexcept {
    auto result = std::min(sub(sub(state.budget, choked), 1), state.available);
    for (std::int32_t i = 0; i < state.history_count; ++i)
        result = std::min(result, state.history[(state.history_first + std::uint32_t(i)) & 7U]);
    return add(add(choked, 1), result);
}
void select_exploit_mode(ExploitState& state, std::int32_t budget,
                         const ExploitGates& gates) noexcept {
    state.mode = 0;
    state.budget = budget;
    state.minimum = sub(budget, 1);
    if (!gates.connected || !gates.checksum_available || budget < 7 ||
        (gates.freeze_period && gates.freeze_entity_present && gates.freeze_entity_state > 1) ||
        gates.player_blocked || gates.command_blocked)
        return;
    if (gates.fake_peek_active)
        state.mode = 3;
    else if (gates.double_tap_active)
        state.mode = 1;
    else if (gates.on_shot_active)
        state.mode = 2;
    else
        return;
    state.minimum = std::max(1, state.fake_lag_limit);
    state.limit = std::clamp(sub(sub(budget, state.fake_lag_limit), 1), 1, 15);
    if (state.mode == 2)
        state.flags &= ~1U;
}
std::int32_t cap_fake_lag_commands(const FakeLagState& state, std::int32_t limit) noexcept {
    return std::min(state.configured_cap, limit);
}
std::int32_t cap_exploit_commands(const ExploitState& state, std::int32_t limit) noexcept {
    return state.mode != 0 || state.available > 0 ? std::min(state.minimum, limit) : limit;
}
void update_exploit_command(ExploitState& state, ExploitFrame& frame, ExploitCommand& command,
                            const ExploitTiming& timing, std::uint32_t& flags) noexcept {
    frame.choke_end = add(add(timing.choked, state.available), 1);
    frame.exploit_mode = state.available < state.limit ? 0 : state.mode;
    frame.next_tick = frame.tick_base;
    frame.saved_tick_base = frame.tick_base;
    frame.flags = (frame.flags & ~0x40000000U) | ((state.flags << 25) & 0x40000000U);
    if (absolute(sub(frame.tick, state.shot_command)) < 10)
        frame.flags |= 0x08000000U;
    if (state.mode != 0) {
        float process = static_cast<float>(timing.process_milliseconds) / 1000.0F;
        // Native MAXSS/MINSS return the second operand for unordered comparisons.
        if (!(process > 0.0F))
            process = 0.0F;
        if (!(process < 1.0F))
            process = 1.0F;
        const auto latency = average_latency_ticks(timing.latency);
        const auto tick = adjusted_command_tick(timing.command_tick, timing.have_ack,
                                                timing.acknowledged, timing.have_sent, timing.sent);
        const auto process_ticks = truncate(process / timing.interval + 0.5F);
        const auto base = add(latency, tick);
        frame.next_tick = add(base, process_ticks);
        if (state.available >= state.limit && frame.has_weapon) {
            const float upper =
                static_cast<float>(sub(add(base, process_ticks), 1)) * timing.interval;
            const float lower = static_cast<float>(add(add(add(tick, 1), latency),
                                                       sub(process_ticks, state.limit))) *
                                timing.interval;
            if (upper >= timing.player_next_attack && lower < timing.player_next_attack &&
                upper >= timing.weapon_next_attack)
                frame.flags |= 0x04000000U;
        }
        if (state.mode == 2 && state.available >= state.limit && !(frame.flags & 0x20000000U) &&
            frame.weapon_id != 0x31 && frame.weapon_id != 0x40 && frame.weapon_type != 9) {
            state.flags |= 0x10U;
            const auto now = truncate(timing.current_time / timing.interval + 0.5F);
            if (add(now, 2) > state.shot_tick || sub(state.shot_tick, now) >= 16 ||
                state.last_command != timing.last_outgoing || !(state.flags & 2U)) {
                force_send(flags);
                if (frame.movement_flags & 1U)
                    command.buttons &= ~1U;
            }
        }
        if (state.mode == 3 && state.available >= state.limit && !timing.freeze_period) {
            if (!(state.flags & 0x20U) && !(frame.flags & 0x20000000U)) {
                state.flags |= 0x10U;
                force_send(flags);
            } else {
                command.pitch = 89.0F;
                state.random.seed = sub(0, absolute(command.number));
                state.random.previous = 0;
                float value = static_cast<float>(
                    static_cast<double>(next_shuffle_random(state.random)) * 4.656612875245797e-10);
                if (static_cast<double>(value) > 0.99999988)
                    value = 0.9999998807907104F;
                command.yaw = (value + 0.0F) * 360.0F - 180.0F;
            }
        }
    }
    if (frame.exploit_mode == 2 &&
        (state.available < state.limit || (frame.flags & 0x20000000U) || frame.weapon_id == 0x31 ||
         frame.weapon_id == 0x40 || frame.weapon_type == 9))
        frame.exploit_mode = 0;
}
void apply_exploit_weapon_gate(const ExploitState& state, std::int32_t last_outgoing, bool enabled,
                               std::uint8_t weapon_type, std::int32_t choked,
                               std::uint32_t& frame_flags, std::uint32_t& command_flags,
                               std::uint8_t& send_packet) noexcept {
    if (state.mode != 0 && state.limit <= state.available && weapon_type == 9 && enabled) {
        frame_flags |= 1U;
        force_send(command_flags);
        if (state.last_command == last_outgoing)
            frame_flags |= 2U;
        if (choked == 0 && send_packet != 0)
            send_packet = 0;
    }
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: packet_dispatch.cpp
// ============================================================================

namespace skeet::network {
namespace {
PacketDispatchRuntime* active_runtime = nullptr;
/* deduplicated helper */
void original_update(void* receiver, std::int32_t update_type) {
    const auto table = *static_cast<void***>(receiver);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    using Original = engine::MemberFunction<void, std::int32_t>;
    reinterpret_cast<Original>(metadata->original_table[7])(receiver, update_type);
}
}
PacketDispatchOffsets packet_dispatch_offsets(const engine::NativeOffsetStorage& storage) {
    const auto decode = [&](std::uint32_t pointer, std::uint32_t key, std::uint32_t mask) {
        return engine::DecodedOffset(storage.globals, pointer, key, mask, 0);
    };
    return {decode(0x43467ec0, 0x434678bc, 0xb6f85c08), decode(0x43468490, 0x43467290, 0xb3832000),
            decode(0x4346a60c, 0x434678a8, 0xca5ec698)};
}
void dispatch_packet_update(PacketDispatchRuntime& runtime, void* receiver,
                            std::int32_t update_type) {
    auto* packet = receiver ? static_cast<std::byte*>(receiver) - 4 : nullptr;
    if (engine::invoke<bool>(runtime.engine, 82) || engine::invoke<bool>(runtime.engine, 93)) {
        original_update(receiver, update_type);
        return;
    }
    const auto captured = runtime.callbacks.entries();
    for (auto* object : captured) {
        auto* endpoint = static_cast<esp::EspPacketEndpoint*>(object);
        endpoint->vtable[0](endpoint, packet);
    }
    const auto player = read<std::uint32_t>(packet, runtime.offsets.zero_based_player);
    const auto one_based = std::bit_cast<std::int32_t>(player + 1u);
    if (one_based != engine::invoke<std::int32_t>(runtime.engine, 12)) {
        original_update(receiver, update_type);
        return;
    }
    // The native captures this address before the engine's command-number
    // query, but reads the pointed-to handle only after the history lookup.
    const auto* weapon_cell = packet + runtime.offsets.weapon_identity;
    const auto command = engine::invoke<std::int32_t>(runtime.engine, 205);
    const auto history = prediction::CommandHistoryView::from_owner(runtime.prediction_owner);
    const auto* saved = history.find(command);
    if (!saved)
        return;
    const auto handle = read<std::uint32_t>(weapon_cell, 0);
    if (saved->weapon_identity != handle || handle == UINT32_MAX)
        return;
    auto* weapon = engine::invoke<void*>(runtime.entity_list, 4, handle);
    if (!weapon)
        return;
    if (read<std::int32_t>(saved, 0x5c) <=
        read<std::int32_t>(weapon, runtime.offsets.weapon_ammunition))
        return;
    original_update(receiver, update_type);
}
void bind_packet_dispatch_runtime(PacketDispatchRuntime& runtime) noexcept {
    active_runtime = &runtime;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall packet_update_hook(void* receiver, void*, std::int32_t update_type) {
#else
void packet_update_hook(void* receiver, void*, std::int32_t update_type) {
#endif
    dispatch_packet_update(*active_runtime, receiver, update_type);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: packet_duck.cpp
// ============================================================================

namespace skeet::network {
namespace {
struct CapturedDuckInput {
    const DuckPacketInput& input;
    std::uint8_t movement_type() const noexcept {
        return input.player_state_7501cb91;
    }
    std::uint8_t player_state() const noexcept {
        return input.player_state_ac7f2913;
    }
    std::uint8_t duck_state() const noexcept {
        return input.player_state_f3c10e80;
    }
    std::uint32_t player_flags() const noexcept {
        return input.player_flags;
    }
    std::int32_t mode() const noexcept {
        return input.mode;
    }
    std::int32_t lag_mode(const FakeLagState& state) const noexcept {
        return state.mode;
    }
    bool active() const {
        return fake_lag_active(input.activation);
    }
    float interval() const noexcept {
        return input.interval;
    }
    Vector velocity() const noexcept {
        return input.velocity;
    }
    const Vector& absolute_origin() const noexcept {
        return input.absolute_origin;
    }
    std::int32_t engine_choked() const noexcept {
        return input.engine_choked;
    }
};
}
bool fake_lag_active(const FakeLagActivation& activation) {
    return activation.enabled && hotkey_active(activation.key, activation.input);
}
void update_packet_duck(FakeLagState& state, const DuckPacketInput& input, std::uint32_t& buttons,
                        std::uint32_t& flags) {
    packet_duck_detail::update(state, CapturedDuckInput{input}, buttons, flags);
}
void process_fake_lag_command(FakeLagState& state, const PacketCommandContext& context,
                              const DuckPacketInput& input, std::uint32_t& buttons,
                              std::uint32_t& flags) {
    prepare_fake_lag_command(state, context, flags);
    update_packet_duck(state, input, buttons, flags);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: packet_duck_live.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
struct LiveDuckInput {
    PacketCommandOwner& owner;
    void* player;

    std::uint8_t movement_type() const noexcept {
        return field<std::uint8_t>(player, owner.offsets.state_7501cb91);
    }
    std::uint8_t player_state() const noexcept {
        return field<std::uint8_t>(player, owner.offsets.state_ac7f2913);
    }
    std::uint8_t duck_state() const noexcept {
        return field<std::uint8_t>(player, owner.offsets.state_f3c10e80);
    }
    std::uint32_t player_flags() const noexcept {
        return field<std::uint32_t>(player, owner.offsets.player_flags_e2fbe695);
    }
    std::int32_t mode() const noexcept {
        return owner.globals.duck_mode;
    }
    std::int32_t lag_mode(const FakeLagState&) const noexcept {
        return owner.settings.amount.value;
    }
    bool active() const {
        return owner.settings.enabled.value &&
               hotkey_active(owner.settings.hotkey.value.binding, owner.globals.input);
    }
    float interval() const noexcept {
        return *owner.globals.interval;
    }
    Vector velocity() const noexcept {
        return field<Vector>(player, owner.offsets.velocity_36375c23);
    }
    const Vector& absolute_origin() const {
        return *engine::invoke<const Vector*>(player, 10);
    }
    std::int32_t engine_choked() const noexcept {
        return *owner.globals.choked_commands;
    }
};
}

void update_packet_duck_live(PacketCommandOwner& owner, void* player, std::uint32_t& buttons,
                             std::uint32_t& flags) {
    packet_duck_detail::update(owner.state, LiveDuckInput{owner, player}, buttons, flags);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: packet_owner.cpp
// ============================================================================

namespace skeet::network {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define PACKET_MEMBER __thiscall
#else
#define PACKET_MEMBER
#endif
using PrimaryEntry = void(PACKET_MEMBER*)(void*);
void PACKET_MEMBER packet_destroy_entry(void*);
void PACKET_MEMBER packet_unchanged(void*) {} //433CCE8E is exactly RET.
void PACKET_MEMBER packet_reset_entry(void*);
void PACKET_MEMBER packet_refresh_entry(void*);
const std::array<PrimaryEntry, 9> packet_primary_table{packet_destroy_entry, packet_unchanged,     packet_unchanged,
                                                packet_unchanged,     packet_unchanged,     packet_reset_entry,
                                                packet_unchanged,     packet_refresh_entry, packet_unchanged}; //4347290C
}

class PacketLifecycleOwner {
public:
    const PrimaryEntry* vtable = nullptr; //434901E1 clears the primary word.
    engine::JobMemory memory;
    PacketCommandOwner& command;
    client::NativeCommandPhaseReceiver command_receiver;

    PacketLifecycleOwner(engine::OwnerRegistryBinding primary,
                         client::CommandPhaseRegistry& commands, ConfigRegistries& registries,
                         anti_aim::FakeLagSettings& settings, PacketCommandOwner& source)
        : memory(primary.memory), command(source) {
        primary.append(this); //433676A0, full+0
        commands.append(command_receiver, &command); //434901EB, full+8
        vtable = packet_primary_table.data(); //434901F2, primary4347290C
        commands.publish(command_receiver, client::CommandOwnerId::fake_lag); //434901FC
        initialize_integer_config(registries.descriptors, settings.amount, 0x2A71D5FCu, 0);
        initialize_integer_config(registries.descriptors, settings.variance, 0xCA9768E6u, 0);
        initialize_integer_config(registries.descriptors, settings.limit, 0xF17FEF11u, 13);
        initialize_packet_command_state(command.state);
    }
};

namespace {
void PACKET_MEMBER packet_destroy_entry(void* storage) {
    destroy_packet_owner(storage);
}
void PACKET_MEMBER packet_reset_entry(void* storage) { //43369ACB
    reset_fake_lag(static_cast<PacketLifecycleOwner*>(storage)->command.state);
}
void PACKET_MEMBER packet_refresh_entry(void* storage) { //433CF2F5
    const auto* owner = static_cast<PacketLifecycleOwner*>(storage);
    owner->vtable[5](storage);
}
}

std::size_t packet_owner_size() noexcept {
    return sizeof(PacketLifecycleOwner);
}
PacketLifecycleOwner& construct_packet_owner(void* storage, engine::OwnerRegistryBinding primary,
                                             client::CommandPhaseRegistry& commands,
                                             ConfigRegistries& registries,
                                             anti_aim::FakeLagSettings& settings,
                                             PacketCommandOwner& command) {
    return *::new (storage) PacketLifecycleOwner(primary, commands, registries, settings, command);
}
void destroy_packet_owner(void* storage) {
    if (!storage)
        return; //433837D0 null guard
    auto* const owner = static_cast<PacketLifecycleOwner*>(storage);
    const auto memory = owner->memory;
    owner->~PacketLifecycleOwner();
    auto* const bytes = static_cast<std::byte*>(storage);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
#undef PACKET_MEMBER
} //namespace skeet::network

// ============================================================================
// Subcomponent: packet_scheduler.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
/* [dedup absolute] */
/* [dedup force_send] */
std::int32_t increment(std::int32_t value) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(value) + 1U);
}
}
void* nearest_enemy(const rage::TargetBindings& bindings, std::int32_t maximum_clients) {
    void* selected = nullptr;
    float distance = std::numeric_limits<float>::max();
    for (std::int32_t i = 1; i <= maximum_clients; i = increment(i)) {
        const auto candidate = engine::invoke<void*, std::int32_t>(bindings.entity_list, 3, i);
        if (!candidate || !engine::invoke<bool>(candidate, 158) ||
            candidate == bindings.local_player ||
            field<std::uint8_t>(candidate, bindings.offsets.rejection_bytes[0]) != 0 ||
            engine::invoke<std::int32_t>(candidate, 122) <= 0 ||
            engine::invoke<bool>(static_cast<std::byte*>(candidate) + 8, 9) ||
            field<std::uint8_t>(candidate, bindings.offsets.rejection_bytes[1]) != 0)
            continue;
        auto relationship = bindings;
        relationship.local_player = candidate;
        if (!rage::target_is_enemy(relationship, bindings.local_player))
            continue;
        const auto local = field<Vector>(bindings.local_player, bindings.offsets.origin);
        const auto other = field<Vector>(candidate, bindings.offsets.origin);
        const float dx = local.x - other.x, dy = local.y - other.y, dz = local.z - other.z;
        const float candidate_distance = (dy * dy + dx * dx) + dz * dz;
        if (candidate_distance < distance) {
            selected = candidate;
            distance = candidate_distance;
        }
    }
    return selected;
}
void apply_fake_lag_triggers(FakeLagState& state, const FakeLagTriggerFrame& frame,
                             const anti_aim::TargetState& target,
                             FakeLagTriggerEnvironment& environment, std::uint32_t& flags,
                             bool& collision_result) {
    const auto target_state = std::bit_cast<std::int32_t>(target.fields_18[0]);
    if (state.tracked_target != target.handle)
        state.flags = (state.flags & ~2U) | (target.handle != -1 && target_state != 0 ? 2U : 0U);
    LagTriggerState trigger{state.flags, state.target_trigger_time, state.tracked_target};
    const bool changed = trigger_target_change(trigger, frame.value_c0, target.handle, target_state,
                                               environment.current_time, flags);
    state.flags = trigger.flags;
    state.target_trigger_time = trigger.last_time;
    if (!changed && (state.flags & 4U) && frame.packet.choked == 0) {
        state.flags &= ~4U;
        state.target = frame.packet.limit;
    }
    if (!(frame.packet.flags & 8U) && frame.weapon_type == 0 && frame.value_c0 > 0.1F &&
        frame.value_c0 < 260.0F && (frame.packet.flags & 0x40000U)) {
        const auto enemy = nearest_enemy(environment.targeting, environment.maximum_clients);
        if (enemy) {
            const auto handle = field<std::int32_t>(enemy, environment.active_weapon);
            if (handle != -1) {
                const auto weapon =
                    engine::invoke<void*, std::int32_t>(environment.world.entity_list, 4, handle);
                if (weapon && engine::invoke<std::int32_t>(weapon, 455) == 0) {
                    const auto origin = field<Vector>(enemy, environment.world.origin);
                    const float dx = origin.x - frame.packet.origin.x,
                                dy = origin.y - frame.packet.origin.y;
                    const float distance = std::sqrt(dy * dy + dx * dx);
                    if (distance < 500.0F) {
                        const float position_scale = 1.0F / (distance + 1.1920928955078125e-7F);
                        const auto& velocity = frame.packet.velocity;
                        const float velocity_scale =
                            1.0F / (std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y) +
                                    1.1920928955078125e-7F);
                        const float dot =
                            (((position_scale * dy) * distance) * velocity_scale) * velocity.y +
                            (((dx * position_scale) * distance) * velocity_scale) * velocity.x;
                        if (dot > 0.0F)
                            state.target = state.selected;
                        else if (dot < 0.0F)
                            force_send(flags);
                    }
                }
            }
        }
    }
    if (!(flags & 0x600U) && ((environment.rule_bf567619 && increment(frame.packet.choked) > 2 &&
                               engine::invoke<bool>(environment.engine_client, 224)) ||
                              state.value_7c == environment.last_outgoing))
        force_send(flags);
    if (target_collision_trigger(state, frame.packet, frame.predicted_origin, target.handle,
                                 environment.world, collision_result)) {
        state.target = frame.packet.limit;
        state.last_trigger_time = environment.current_time;
    } else if (absolute(environment.current_time - state.last_trigger_time) < 0.75F)
        state.target = frame.packet.limit;
    if (frame.tick_base % 100 == 0 ||
        selected_weapon_trigger(environment.weapons, frame.weapon_type, frame.selected_weapon) ||
        secondary_attack_trigger(environment.weapons, frame.weapon, frame.buttons,
                                 environment.current_time) ||
        reload_trigger(environment.weapons, frame.weapon, frame.buttons) ||
        (frame.value_c0 > 2.0F &&
         (velocity_trigger(state, frame.packet) ||
          displacement_trigger(state, frame.packet, frame.value_c0, frame.buttons,
                               environment.interval, environment.gravity))))
        state.target = std::max(state.target, state.selected);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: packet_tick.cpp
// ============================================================================

namespace skeet::network {
void schedule_fake_lag(FakeLagState& state, const FakeLagTriggerFrame& frame,
                       const FakeLagActivation& activation, bool freeze_period,
                       anti_aim::TargetState& target, const anti_aim::TargetRefreshContext& refresh,
                       FakeLagTriggerEnvironment& environment, const RandomInterface& random,
                       std::uint32_t& flags, std::uint8_t& send_packet, bool& collision_result) {
    begin_fake_lag_tick(state, frame.packet, collision_result);
    if (freeze_period)
        return;
    if (fake_lag_active(activation)) {
        if (select_fake_lag_limit(state, frame.packet, environment.interval, random)) {
            anti_aim::refresh_yaw_target(target, refresh);
            apply_fake_lag_triggers(state, frame, target, environment, flags, collision_result);
        }
        apply_fake_lag_send_gate(state, frame.packet, flags, send_packet);
    }
    finish_fake_lag_tick(state, frame.packet, target.handle,
                         std::bit_cast<std::int32_t>(target.fields_18[0]));
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: packet_triggers.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
}
bool reload_trigger(const PacketWeaponBindings& bindings, void* weapon, std::uint32_t buttons) {
    if (!(buttons & 0x2000U))
        return false;
    const auto identifier = field<std::uint32_t>(weapon, bindings.reload_identifier);
    if (identifier == 0xffffffffU || identifier == engine::invoke<std::uint32_t>(weapon, 375))
        return false;
    return field<std::int32_t>(weapon, bindings.reload_counter) > 0;
}
bool secondary_attack_trigger(const PacketWeaponBindings& bindings, void* weapon,
                              std::uint32_t buttons, float time) {
    if (!(buttons & 0x800U) || !weapon || engine::invoke<std::int32_t>(weapon, 443) <= 0 ||
        engine::invoke<std::int32_t>(weapon, 462) != 0)
        return false;
    return time >= field<float>(weapon, bindings.next_secondary);
}
bool selected_weapon_trigger(const PacketWeaponBindings& bindings, std::uint8_t type,
                             std::uint32_t selected) {
    if (selected == 0)
        return false;
    if (type == 0 || type == 7 || type == 9)
        return true;
    if (selected & 0xffff0000U)
        return false;
    const auto entity = engine::invoke<void*, std::uint32_t>(bindings.entity_list, 3, selected);
    if (!entity)
        return false;
    const auto selected_type = engine::invoke<std::int32_t>(entity, 455);
    return selected_type == 8 || selected_type == 9;
}
bool classified_packet_weapon(std::uint16_t weapon,
                              std::span<const std::uint32_t, 3> classification) noexcept {
    return weapon < 65 && (classification[weapon >> 5] & (1U << (weapon & 31))) != 0;
}
void record_fake_lag_attack(FakeLagState& state, const PacketFrame& frame, std::int32_t weapon,
                            std::uint8_t type, std::uint32_t buttons, std::int32_t outgoing,
                            std::span<const std::uint32_t, 3> classification,
                            std::uint32_t& flags) noexcept {
    if (!classified_packet_weapon(std::uint16_t(weapon), classification))
        return;
    if ((frame.flags & 0x100000U) || ((frame.flags & 0x400U) && (buttons & 1U)) ||
        ((frame.flags & 2U) && (buttons & 0x800U) && (weapon == 0x40 || type == 0))) {
        state.value_7c = outgoing;
        flags = (flags & ~0x400U) | 0x200U;
    }
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: packet_world.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
const void* convar_parent(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1c);
        if (parent == object)
            return object;
        object = parent;
    }
}
/* [dedup convar_bits] */
/* [dedup convar_float] */
std::int32_t convar_integer(const void* object) noexcept {
    return std::bit_cast<std::int32_t>(convar_bits(object, 0x30));
}
/* [dedup absolute] */
Vector shifted(Vector a, Vector b) noexcept {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
}
Bounds transformed_bounds(const Matrix3x4& matrix, Vector minimum, Vector maximum) noexcept {
    const Vector center{(minimum.x + maximum.x) * 0.5F, (minimum.y + maximum.y) * 0.5F,
                        (minimum.z + maximum.z) * 0.5F};
    const Vector extent{maximum.x - center.x, maximum.y - center.y, maximum.z - center.z};
    Vector out_center{}, out_extent{};
    float* c = &out_center.x;
    float* e = &out_extent.x;
    for (unsigned i = 0; i < 3; ++i) {
        const auto& row = matrix.rows[i];
        c[i] = ((row[0] * center.x + row[1] * center.y) + row[2] * center.z) + row[3];
        e[i] = (absolute(row[0] * extent.x) + absolute(row[1] * extent.y)) +
               absolute(row[2] * extent.z);
    }
    return {
        {out_center.x - out_extent.x, out_center.y - out_extent.y, out_center.z - out_extent.z},
        {out_extent.x + out_center.x, out_extent.y + out_center.y, out_extent.z + out_center.z}};
}
Bounds entity_bounds(void* entity) {
    const auto collision = engine::invoke<void*>(entity, 3);
    if (!(engine::invoke<std::uint32_t>(collision, 12) & 0x40U) &&
        engine::invoke<std::int32_t>(collision, 11) != 2 &&
        engine::invoke<std::int32_t>(collision, 11) != 0) {
        const auto angles = engine::invoke<const Vector*>(collision, 9);
        if (angles->x != 0.0F || angles->y != 0.0F || angles->z != 0.0F) {
            const auto maximum = engine::invoke<const Vector*>(collision, 2);
            const auto minimum = engine::invoke<const Vector*>(collision, 1);
            const auto matrix = engine::invoke<const Matrix3x4*>(collision, 10);
            return transformed_bounds(*matrix, *minimum, *maximum);
        }
    }
    const auto origin = engine::invoke<const Vector*>(entity, 10);
    const auto minimum = engine::invoke<const Vector*>(collision, 1);
    Bounds out;
    out.minimum = shifted(*origin, *minimum);
    const auto maximum = engine::invoke<const Vector*>(collision, 2);
    out.maximum = shifted(*origin, *maximum);
    return out;
}
Bounds standing_bounds(Vector origin, float radius, float height) noexcept {
    return {{(origin.x + 0.0F) - radius, (origin.y + 0.0F) - radius, (origin.z + 46.0F) - 0.0F},
            {(origin.x + 0.0F) + radius, (origin.y + 0.0F) + radius, (origin.z + 64.0F) + height}};
}
bool target_collision_trigger(FakeLagState& state, const PacketFrame& frame,
                              Vector predicted_origin, std::int32_t handle, PacketWorld& world,
                              bool& result) {
    if (handle == -1)
        return false;
    const auto target = engine::invoke<void*, std::int32_t>(world.entity_list, 4, handle);
    if (!target || field<std::uint8_t>(target, world.rejection_byte) != 0)
        return false;
    const auto networkable = static_cast<std::byte*>(target) + 8;
    if (engine::invoke<bool>(networkable, 9)) {
        const auto index = engine::invoke<std::int32_t>(networkable, 10);
        if (world.player_geometry[std::uint32_t(index) & 63U].alpha == 0.0F)
            return false;
    }
    const auto current = entity_bounds(world.local_player);
    const Vector displacement{predicted_origin.x - frame.origin.x,
                              predicted_origin.y - frame.origin.y,
                              predicted_origin.z - frame.origin.z};
    Bounds predicted{shifted(current.minimum, displacement),
                     shifted(current.maximum, displacement)};
    const auto local =
        standing_bounds(field<Vector>(target, world.origin), convar_float(world.hull_radius.get()),
                        convar_float(world.standing_height.get()));
    void* wind = nullptr;
    if (state.value_8c != -1)
        wind = engine::invoke<void*, std::int32_t>(world.entity_list, 4, state.value_8c);
    if (!wind && !(state.flags & 8U)) {
        state.value_8c = -1;
        state.flags |= 8U;
        const auto end = field<std::int32_t>(world.entity_list, 0x24);
        for (std::uint32_t i = 65; std::bit_cast<std::int32_t>(i) < end; ++i) {
            if (i & 0xffff0000U)
                continue;
            const auto candidate = engine::invoke<void*, std::uint32_t>(world.entity_list, 3, i);
            if (candidate && engine::invoke<const void*>(static_cast<std::byte*>(candidate) + 8,
                                                         2) == world.wind_class) {
                state.value_8c = *engine::invoke<const std::int32_t*>(candidate, 2);
                wind = candidate;
                break;
            }
        }
    }
    Vector movement{};
    if (wind) {
        const auto velocity = field<Vector>(wind, world.wind_velocity);
        const auto scale = convar_float(world.wind_scale.get());
        movement = {scale * velocity.x, velocity.y * scale, velocity.z * scale};
    }
    const auto type = convar_integer(world.game_type.get());
    const auto mode = convar_integer(world.game_mode.get());
    const float expansion = (type == 0 && mode == 0) || (type == 1 && mode == 2) ? 8.25F : 0.0F;
    bool first = false;
    if (world.box_query.function) {
        first = world.box_query.function(world.box_query.context, predicted, local, movement,
                                         expansion);
        if (world.box_query.function) {
            result = world.box_query.function(world.box_query.context, current, local, movement,
                                              expansion);
            return first && !result;
        }
    }
    result = false;
    return first;
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: receive_float_proxy.cpp
// ============================================================================

namespace skeet::network {
namespace {
ReceiveFloatProxyFunction* original_cell = nullptr;
/* [dedup read] */
/* [dedup write] */
std::uint32_t shift_left(std::uint32_t value, std::uint32_t count) {
    return value << (count & 31);
}
std::uint32_t shift_right(std::uint32_t value, std::uint32_t count) {
    return value >> (count & 31);
}
std::uint32_t mask(std::uint32_t bits) {
    return bits == 32 ? 0xffffffffu : shift_left(1, bits) - 1;
}
float rounded_float(double value) {
    return _mm_cvtss_f32(_mm_cvtpd_ps(_mm_set_sd(value)));
}
double unsigned_double(std::uint32_t value) {
    // Native signed CVTDQ2PD plus the unsigned2^32 correction also preserves
    // positive zero under MXCSR round-down. C++'s generic uint32 conversion
    // may instead lower to an equal-double subtraction producing minus zero.
    const auto integer = _mm_cvtsi32_si128(std::bit_cast<std::int32_t>(value));
    return _mm_cvtsd_f64(
        _mm_add_sd(_mm_cvtepi32_pd(integer), _mm_set_sd(value >> 31 ? 4294967296.0 : 0.0)));
}
/* [dedup multiply] */
//433EC30F truncates a signed64-bit integer and this caller consumes only
// EAX. Its nonfinite/out-of-range indefinite result has a zero low dword.
std::uint32_t truncated_low_word(double value) {
    const auto bits = std::bit_cast<std::uint64_t>(value);
    const auto exponent = std::int32_t((bits >> 52) & 0x7ff) - 1023;
    if (exponent < 0 || exponent >= 63)
        return 0;
    const auto significand = (bits & 0x000fffffffffffffull) | 0x0010000000000000ull;
    const auto magnitude = exponent >= 52 ? significand << unsigned(exponent - 52)
                                          : significand >> unsigned(52 - exponent);
    return std::uint32_t(bits >> 63 ? 0 - magnitude : magnitude);
}
bool scale_fits(float scale, double extent, std::uint32_t maximum) {
    const double product = double(scale) * extent;
    // Native COMISD/JBE accepts an unordered product in this exact gate.
    return truncated_low_word(product) <= maximum && !(product > unsigned_double(maximum));
}
void initialize_scale(void* descriptor) {
    if (read<float>(descriptor, 0x34) != 0.0f)
        return;
    const auto high = read<float>(descriptor, 0x14), low = read<float>(descriptor, 0x10);
    const auto width = _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(high), _mm_set_ss(low)));
    const auto bits = read<std::uint32_t>(descriptor, 0x0c);
    const auto maximum = bits == 32 ? 0xfffffffeu : shift_left(1, bits) - 1;
    const double extent = double(width), upper = unsigned_double(maximum);
    const float absolute = std::bit_cast<float>(std::bit_cast<std::uint32_t>(width) & 0x7fffffffu);
    const double ratio = absolute <= 0.0010000000474974513f ? upper : upper / extent;
    auto scale = rounded_float(ratio);
    if (!scale_fits(scale, extent, maximum)) {
        constexpr std::array factors{0.9998999834060669f, 0.9900000095367432f, 0.8999999761581421f,
                                     0.800000011920929f, 0.699999988079071f};
        bool selected = false;
        const auto initial = rounded_float(upper / extent);
        for (const auto factor : factors) {
            scale = multiply(factor, initial);
            if (scale_fits(scale, extent, maximum)) {
                selected = true;
                break;
            }
        }
        if (!selected)
            scale = 0.0f;
    }
    write(descriptor, 0x34, scale);
}
std::uint32_t read_bits(void* reader, std::uint32_t bits, bool& failed) {
    const auto available = read<std::int32_t>(reader, 0x14);
    const auto cache = read<std::uint32_t>(reader, 0x10);
    auto* next = read<const std::uint32_t*>(reader, 0x18);
    const auto* end = read<const std::uint32_t*>(reader, 0x1c);
    const auto next_address = reinterpret_cast<std::uintptr_t>(next);
    const auto end_address = reinterpret_cast<std::uintptr_t>(end);
    const auto advance = [&] {
        write(reader, 0x18, reinterpret_cast<const std::uint32_t*>(next_address + 4));
    };
    if (available >= std::int32_t(bits)) {
        const auto result = cache & mask(bits);
        const auto remaining = std::uint32_t(available) - bits;
        write(reader, 0x14, std::bit_cast<std::int32_t>(remaining));
        if (remaining)
            write(reader, 0x10, shift_right(cache, bits));
        else {
            write(reader, 0x14, std::int32_t(32));
            std::uint32_t replacement = 0;
            if (next_address == end_address) {
                advance();
                write(reader, 0x14, std::int32_t(1));
            } else if (next_address > end_address)
                write(reader, 4, std::uint8_t(1));
            else {
                replacement = read<std::uint32_t>(next, 0);
                advance();
            }
            write(reader, 0x10, replacement);
        }
        return result;
    }
    const auto missing = bits - std::uint32_t(available);
    std::uint32_t replacement = 0;
    if (next_address == end_address) {
        advance();
        write(reader, 0x14, std::int32_t(1));
        write(reader, 4, std::uint8_t(1));
        failed = true;
    } else if (next_address > end_address) {
        write(reader, 4, std::uint8_t(1));
        failed = true;
    } else {
        replacement = read<std::uint32_t>(next, 0);
        advance();
        failed = read<std::uint8_t>(reader, 4) != 0;
    }
    write(reader, 0x10, replacement);
    if (failed)
        return 0;
    write(reader, 0x14, std::int32_t(32 - missing));
    const auto result = shift_left(replacement & mask(missing), std::uint32_t(available)) | cache;
    write(reader, 0x10, shift_right(replacement, missing));
    return result;
}
}
void receive_float_value(void* argument, ReceiveFloatProxyFunction const& original) {
    auto* const descriptor = read<void*>(argument, 0x30);
    if (read<std::uint32_t>(descriptor, 0x3c) & 0x3f026u) {
        original(argument);
        return;
    }
    initialize_scale(descriptor);
    auto* const reader = read<void*>(argument, 0x34);
    const auto bits = read<std::uint32_t>(descriptor, 0x0c);
    bool failed = false;
    const auto encoded = read_bits(reader, bits, failed);
    float value;
    if (failed || encoded == 0)
        value = read<float>(descriptor, 0x10);
    else if (encoded >= shift_left(1, bits) - 1)
        value = read<float>(descriptor, 0x14);
    else {
        const auto converted = rounded_float(unsigned_double(encoded));
        const auto scaled =
            _mm_div_ss(_mm_set_ss(converted), _mm_set_ss(read<float>(descriptor, 0x34)));
        value = _mm_cvtss_f32(_mm_add_ss(scaled, _mm_set_ss(read<float>(descriptor, 0x10))));
    }
    write(argument, 8, value);
    if (auto* const property = read<const void*>(argument, 0)) {
        using Proxy = void(__cdecl*)(void*, std::uintptr_t, std::uintptr_t);
        const auto fourth = read<std::uintptr_t>(argument, 0x2c);
        const auto third = read<std::uintptr_t>(argument, 0x28);
        read<Proxy>(property, 0x20)(argument, third, fourth);
    }
}
void bind_receive_float_proxy(engine::NativeDataImage& data) noexcept {
    original_cell = &data.native_cell<ReceiveFloatProxyFunction>(0x43476A44u);
}
void __cdecl receive_float_proxy(void* argument) {
    receive_float_value(argument, *original_cell);
}
} //namespace skeet::network

// ============================================================================
// Subcomponent: shared_esp.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* deduplicated helper */
/* deduplicated helper */
std::int32_t integer(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
void* networkable(void* player) noexcept {
    return static_cast<std::byte*>(player) + 8;
}
std::int32_t index(void* player) {
    return engine::invoke<std::int32_t>(networkable(player), 10);
}
bool dormant(void* player) {
    return engine::invoke<bool>(networkable(player), 9);
}

bool alive(const SharedEspRuntime& runtime, void* player) { //433F1E5F
    if (!player)
        return false;
    if (!dormant(player))
        return read<std::uint8_t>(player, runtime.offsets.life_state) == 0;
    if (!runtime.player_resource || !*runtime.player_resource)
        return false;
    const auto resource = *runtime.player_resource;
    const auto player_index = index(player);
    if (!read<std::uint8_t>(resource, runtime.offsets.connected + player_index))
        return false;
    return read<std::uint8_t>(resource, runtime.offsets.alive + player_index) != 0;
}
bool enemy(const SharedEspRuntime& runtime, void* local, void* player) { //433E4E4E
    if (!local || !player || local == player)
        return false;
    if (script::cvar_int(runtime.game_type.get()) == 6) {
        const auto squad = read<std::int32_t>(local, runtime.offsets.survival_team);
        return squad == -1 || squad != read<std::int32_t>(player, runtime.offsets.survival_team);
    }
    if (!runtime.player_resource || !*runtime.player_resource)
        return false;
    const auto resource = *runtime.player_resource;
    const auto first = index(local), second = index(player);
    if (!read<std::uint8_t>(resource, runtime.offsets.connected + first) ||
        !read<std::uint8_t>(resource, runtime.offsets.connected + second))
        return false;
    const auto other_team =
        read<std::int32_t>(resource, runtime.offsets.resource_team + second * 4);
    const auto local_team = read<std::int32_t>(resource, runtime.offsets.resource_team + first * 4);
    if (local_team == 2) {
        if (other_team == 3)
            return true;
    } else if (local_team == 3) {
        if (other_team == 2)
            return true;
    } else
        return false;
    return script::cvar_int(runtime.teammates_are_enemies.get()) != 0;
}
void geometry(esp::PlayerGeometry& output, void* entity, esp::Vector3 origin,
              const SharedEspOffsets& offsets) {
    //4341AF4A retains pose reads after the first two engine bounds calls.
    output.origin = origin;
    output.center = origin;
    const auto& minimum = engine::invoke<const esp::Vector3&>(entity, 76);
    const auto& maximum = engine::invoke<const esp::Vector3&>(entity, 77);
    output.center.z += (minimum.z + maximum.z) * 0.5f;
    output.eye_height = read<float>(entity, offsets.pose_parameters + 4) * 18.0f + 58.0f;
    output.pose_adjustment = read<float>(entity, offsets.pose_parameters + 0x30) * 6.0f;
    const auto& second_maximum = engine::invoke<const esp::Vector3&>(entity, 77);
    output.lower = {output.center.x, output.center.y, output.center.z - second_maximum.z * 0.6f};
    output.upper = {output.center.x, output.center.y,
                    (output.origin.z + output.eye_height) - output.pose_adjustment};
}
bool valid_origin(esp::Vector3 origin) noexcept {
    for (const auto value : {origin.x, origin.y, origin.z})
        if (!std::isfinite(value) || !(value > -16384.0f) || !(value < 16384.0f))
            return false;
    return true;
}
float rule_time(const SharedEspRuntime& runtime) noexcept { //433FC87C
    return runtime.game_rules && *runtime.game_rules
               ? read<float>(*runtime.game_rules, runtime.offsets.rule_time)
               : 0.0f;
}
}
SharedEspOffsets shared_esp_offsets(const engine::NativeOffsetStorage& storage) {
    const auto decode = [&](std::uint32_t pointer, std::uint32_t key, std::uint32_t mask) {
        return static_cast<std::ptrdiff_t>(*storage.globals.pointers.at(pointer) ^
                                           storage.globals.keys.at(key) ^ mask);
    };
    return {decode(0x43468e78, 0x434684e4, 0x9d920e35), decode(0x43467cb8, 0x4346a814, 0x9571d5f5),
            decode(0x4346a724, 0x4346857c, 0xd266b537), decode(0x434684f0, 0x4346a558, 0xc1c49a34),
            decode(0x434678ac, 0x43467b7c, 0x430c8993), decode(0x43467d78, 0x43467b80, 0x1caaf0a8),
            decode(0x43468d6c, 0x43468428, 0x9f8e7f71)};
}

bool receive_shared_esp(SharedEspRuntime& runtime, void* message) { //433D5BCF
    auto* owner = runtime.owner;
    if (!owner)
        return false;
    auto* local = runtime.local_player;
    if (!local || !owner->construct_message || !owner->destroy_message ||
        !owner->construct_receiver || !owner->destroy_receiver)
        return false;
    if (engine::invoke<bool>(runtime.engine, 82) || engine::invoke<bool>(runtime.engine, 93))
        return false;
    if (script::cvar_int(const_cast<void*>(owner->game_type)) == 6)
        return false;
    if (!script::cvar_int(const_cast<void*>(owner->send_gate_a)) &&
        !script::cvar_int(const_cast<void*>(owner->send_gate_b)))
        return false;
    const auto flags = read<std::uint32_t>(message, 0x34);
    if ((flags & 0x10u) && read<std::uint32_t>(read<const void*>(message, 0x18), 0x10) != 0)
        return false;
    if (!(flags & 0x40u) || read<std::uint32_t>(message, 0x20) != 0 || (flags & 0x185u) != 0x185u)
        return false;
    const auto raw_sender = read<std::uint32_t>(message, 8);
    if (raw_sender >= 64u)
        return false;
    const auto sender = static_cast<std::int32_t>(raw_sender + 1u);
    if (sender == engine::invoke<std::int32_t>(owner->engine_client, 12))
        return true;

    const std::array<std::uint32_t, 5> wire{
        read<std::uint32_t>(message, 0x10), read<std::uint32_t>(message, 0x14),
        read<std::uint32_t>(message, 0x28), read<std::uint32_t>(message, 0x24),
        read<std::uint32_t>(message, 0x2c)};
    SharedEspPacket packet;
    std::memcpy(packet.bytes.data(), wire.data(), packet.bytes.size());
    packet.sender_index = sender;
    misc::PlayerInfo info{};
    SharedEspFields decoded{};
    if (!engine::invoke<bool>(owner->engine_client, 8, sender, &info) ||
        !decode_shared_esp_packet(packet, *owner, read<std::uint32_t>(info.bytes.data(), 8),
                                 read<std::uint32_t>(info.bytes.data(), 0x90), decoded)) {
        dispatch_voice_message(runtime, message);
        return false;
    }
    if (!runtime.receive_enabled)
        return false;
    if (runtime.permission_gate &&
        !runtime.adjustments.records[std::uint32_t(sender)].adjustments.allow_shared_esp)
        return true;
    if (decoded.entity_index < 1 || decoded.entity_index > 64 || !valid_origin(decoded.origin))
        return true;
    const auto now = engine::invoke<float>(owner->engine_client, 14);
    const auto current_tick = lagcomp::receive_time_to_tick(now, runtime.engine_interval);
    if (decoded.tick > current_tick)
        return true;
    const auto second = integer(1.0f / runtime.tick_interval + 0.5f);
    const auto oldest =
        std::bit_cast<std::int32_t>(std::uint32_t(current_tick) - std::uint32_t(second));
    if (decoded.tick < oldest || (decoded.yaw > 360 && decoded.yaw < 511))
        return true;
    auto* player = engine::invoke<void*>(runtime.entity_list, 3, decoded.entity_index);
    auto* cached = runtime.cache.get(decoded.entity_index);
    if (!player || !engine::invoke<bool>(player, 158) || !alive(runtime, player) ||
        !enemy(runtime, local, player))
        return true;
    if (dormant(player)) {
        if (decoded.tick <= cached->geometry.last_live_tick)
            return true;
        geometry(cached->geometry, player, decoded.origin, runtime.offsets);
        cached->geometry.alpha = 1.0f;
        cached->geometry.last_live_tick = decoded.tick;
    } else {
        if (decoded.yaw == 511 || decoded.tick <= cached->native.shared_yaw_tick)
            return true;
        cached->native.shared_rule_time = rule_time(runtime);
        cached->native.shared_yaw = normalize_movement_angle(float(decoded.yaw));
        cached->native.shared_yaw_tick = current_tick;
        cached->native.flags = (cached->native.flags & ~0x40u) | 0x80u;
    }
    return true;
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: shared_esp_hook.cpp
// ============================================================================

namespace skeet::network {
namespace {
SharedEspRuntime* active_shared_esp_runtime = nullptr;
int marshal_voice(script::Bridge& bridge, void* context) { //433BA24E
    const auto& api = *bridge.api;
    api.createtable(bridge.state, 0, 0);
    api.pushstring(bridge.state, "data");
    api.pushlightuserdata(bridge.state, context);
    api.settable(bridge.state, -3);
    return 1;
}
}
void dispatch_voice_message(SharedEspRuntime& runtime, void* message) { //433E90B7
    if (!runtime.script.state)
        return;
    script::dispatch_callbacks(runtime.script, runtime.voice_event_name.get(), marshal_voice,
                               message);
}
bool process_voice_message(SharedEspRuntime& runtime, void* receiver, void* message) { //433FD2C4
    if (receive_shared_esp(runtime, message))
        return true;
    const auto table = *static_cast<void***>(receiver);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    using Original = engine::MemberFunction<bool, void*>;
    return reinterpret_cast<Original>(metadata->original_table[24])(receiver, message);
}
void bind_shared_esp_runtime(SharedEspRuntime& runtime) noexcept {
    active_shared_esp_runtime = &runtime;
}
#if defined(_M_IX86) || defined(__i386__)
bool __fastcall process_voice_message_hook(void* receiver, void*, void* message) {
#else
bool process_voice_message_hook(void* receiver, void*, void* message) {
#endif
    return process_voice_message(*active_shared_esp_runtime, receiver, message);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: shared_esp_packet.cpp
// ============================================================================

namespace skeet::network {
namespace {
float coordinate(std::uint32_t value) noexcept {
    const auto whole = (value & 1u) ? ((value >> 3) & 0x3fffu) + 1u : 0u;
    const auto fraction = (value & 2u) ? (value >> 17) & 31u : 0u;
    const auto result = float(whole) + float(fraction) * 0.03125f;
    return (value & 4u) ? -result : result;
}

std::uint16_t encrypt_tail(std::uint16_t left, std::uint16_t right,
                           std::uint16_t previous_left, std::uint16_t previous_right,
                           std::uint32_t key) noexcept {
    left = static_cast<std::uint16_t>((left ^ previous_left) + key);
    right = static_cast<std::uint16_t>((right ^ previous_right) + std::rotr(key, 1));
    auto round_key = std::rotr(key, 2);
    for (unsigned round = 0; round < 15; ++round) {
        left = static_cast<std::uint16_t>(
            std::rotl(static_cast<std::uint16_t>(right ^ left), right & 15u) + round_key);
        round_key = std::rotr(round_key, 1);
        right = static_cast<std::uint16_t>(
            std::rotl(static_cast<std::uint16_t>(right ^ left), left & 15u) + round_key);
        round_key = std::rotr(round_key, 1);
    }
    return left;
}
}

bool decode_shared_esp_packet(SharedEspPacket packet,
                             const client::RejectedPlayerUpdateOwner& owner,
                             std::uint32_t sender_account, std::uint32_t sender_user_id,
                             SharedEspFields& decoded) noexcept {
    if (packet.sender_index < 1 || packet.sender_index > 64)
        return false;
    client::transform_shared_esp_packet(client::SharedEspPacketCipher::rc4_drop128, owner,
                                       packet.bytes);
    std::array<std::uint16_t, 10> words{};
    std::memcpy(words.data(), packet.bytes.data(), packet.bytes.size());
    const auto encrypted_tail = words[9];
    std::uint16_t previous_left = 0, previous_right = 0;
    for (std::size_t index = 1; index < 9; index += 2) {
        const auto saved_left = words[index], saved_right = words[index + 1];
        auto left = saved_left, right = saved_right;
        auto round_key = std::rotl(owner.mixing_key, 1);
        for (unsigned round = 0; round < 15; ++round) {
            right = static_cast<std::uint16_t>(
                std::rotr(static_cast<std::uint16_t>(right - round_key), left & 15u) ^ left);
            round_key = std::rotl(round_key, 1);
            left = static_cast<std::uint16_t>(
                std::rotr(static_cast<std::uint16_t>(left - round_key), right & 15u) ^ right);
            round_key = std::rotl(round_key, 1);
        }
        words[index] = static_cast<std::uint16_t>(
            (left - std::rotl(round_key, 1)) ^ previous_left);
        words[index + 1] = static_cast<std::uint16_t>((right - round_key) ^ previous_right);
        previous_left = saved_left;
        previous_right = saved_right;
    }
    std::array<std::uint32_t, 5> fields{};
    std::memcpy(fields.data(), words.data(), packet.bytes.size());
    fields[0] ^= sender_account;
    fields[1] ^= sender_user_id;
    fields[2] ^= std::uint32_t(packet.sender_index);
    if (fields[0] >> 16 != owner.marker)
        return false;
    const auto entity_index = (fields[2] & 0x7fu) + 1u;
    if (entity_index > 64)
        return false;

    // The wire omits the final encrypted right half. Its plaintext is the
    // selected player index; the left half contains only two Z fraction bits.
    // Recover those four possibilities exactly, without decrypting the sender
    // index as if it were two more bytes received from the network.
    unsigned matches = 0;
    std::uint32_t tail = 0;
    for (std::uint16_t candidate = 0; candidate < 4; ++candidate) {
        if (encrypt_tail(candidate, static_cast<std::uint16_t>(entity_index), previous_left,
                         previous_right, owner.mixing_key) == encrypted_tail) {
            tail = candidate;
            ++matches;
        }
    }
    if (matches != 1)
        return false;
    fields[4] = (fields[4] & 0xffffu) | (tail << 16);
    const auto x = (fields[2] >> 16) | ((fields[3] & 63u) << 16);
    const auto y = (fields[3] >> 6) & 0x3fffffu;
    const auto z = (fields[3] >> 28) | ((fields[4] & 0x3ffffu) << 4);
    decoded = {static_cast<std::uint16_t>(fields[0] >> 16),
               std::bit_cast<std::int32_t>(fields[1]), entity_index,
               (fields[2] >> 7) & 0x1ffu, {coordinate(x), coordinate(y), coordinate(z)}};
    return true;
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: startup_user_messages.cpp
// ============================================================================

namespace skeet::network {
namespace {
/* [dedup read] */
void require(bool value, const char* message) {
    if (!value)
        throw std::runtime_error(message);
}
#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
extern "C" __attribute__((naked)) void skeet_user_message_destroy() {
    __asm__("andl $0,8(%ecx)\n\tmovl 4(%ecx),%ecx\n\tmovl (%ecx),%eax\n\tjmp *(%eax)");
}
#define MESSAGE_FORWARD(slot, offset)                                                              \
    extern "C" __attribute__((naked)) void skeet_user_message_forward_##slot() {                   \
        __asm__("movl 4(%ecx),%ecx\n\tmovl (%ecx),%eax\n\tjmp *" #offset "(%eax)");                \
    }
#elif defined(_M_IX86) && defined(_MSC_VER)
extern "C" __declspec(naked) void skeet_user_message_destroy(){
    __asm and dword ptr[ecx + 8], 0 __asm mov ecx, dword ptr[ecx + 4] __asm mov eax,
    dword ptr[ecx] __asm jmp dword ptr[eax]}
#define MESSAGE_FORWARD(slot, offset)                                                              \
    extern "C" __declspec(naked) void skeet_user_message_forward_##slot() {                        \
        __asm mov ecx, dword ptr[ecx + 4] __asm mov eax,                                           \
            dword ptr[ecx] __asm jmp dword ptr[eax + offset]                                       \
    }
#else
#error Startup user-message wrappers require the actual x86 member ABI
#endif
MESSAGE_FORWARD(1, 4)
MESSAGE_FORWARD(3, 12)
MESSAGE_FORWARD(4, 16)
#undef MESSAGE_FORWARD

void* __thiscall decode(ClientStateScopedPatch* wrapper, std::uintptr_t first,
                        std::uintptr_t second, std::uintptr_t third,
                        std::uintptr_t fourth) { //433D26FB
    using Decode =
        void*(__thiscall*)(void*, std::uintptr_t, std::uintptr_t, std::uintptr_t, std::uintptr_t);
    auto* const original = wrapper->saved;
    const auto method = (*reinterpret_cast<Decode**>(original))[2];
    auto* const result = method(original, first, second, third, fourth);
    if (result && wrapper->field_0c)
        reinterpret_cast<void(__cdecl*)(const void*)>(std::uintptr_t(wrapper->field_0c))(result);
    return result;
}
const std::array<std::uintptr_t, 5> table{
    reinterpret_cast<std::uintptr_t>(&skeet_user_message_destroy),
    reinterpret_cast<std::uintptr_t>(&skeet_user_message_forward_1),
    reinterpret_cast<std::uintptr_t>(&decode),
    reinterpret_cast<std::uintptr_t>(&skeet_user_message_forward_3),
    reinterpret_cast<std::uintptr_t>(&skeet_user_message_forward_4)};
}
std::span<const StartupUserMessageDescriptor, 4> startup_user_message_descriptors() {
    static const std::array<StartupUserMessageDescriptor, 4> descriptors{
        {{64, 0x433A00C4u, 0x43469034u, rage::feedback_position_hook},
         {6, 0x433C05B4u, 0x4346A6E0u, script::say_text_message_hook},
         {69, 0x433E7234u, 0x4346A5C4u, esp::weapon_sound_user_message_hook},
         {25, 0x433FCBD6u, 0x43468394u, esp::spotted_user_message_hook}}};
    return descriptors;
}
std::span<const std::uintptr_t, 5> startup_user_message_wrapper_table() {
    return table;
}

StartupUserMessageTargets discover_startup_user_message_targets(engine::NativeDataImage& data) {
    const auto* const module = data.native_cell<const std::byte*>(0x4346A55Cu);
    auto* const client = data.native_cell<void*>(0x434683E0u);
    require(module && client, "Startup user messages require actual client.dll and VClient018");
    const auto base = reinterpret_cast<std::uintptr_t>(module);
    require(read<std::uint16_t>(module) == 0x5A4D && read<std::uint32_t>(module, 0x3C) == 0x150,
            "Unsupported startup user-message DOS header");
    const auto* const nt = module + 0x150;
    require(
        read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt, 4) == 0x14C &&
            read<std::uint16_t>(nt, 0x18) == 0x10B && read<std::uint32_t>(nt, 8) == 0x65272025 &&
            read<std::uint32_t>(nt, 0x50) == 0x5509000 && read<std::uint32_t>(nt, 0x58) == 0xFA635F,
        "Unsupported startup user-message client build");
    const auto* const dispatch = (*reinterpret_cast<const std::byte* const* const*>(client))[38];
    require(dispatch == module + 0x262130, "VClient018 user-message dispatch method differs");
    const auto address = [&](std::size_t offset) { return read<std::uint32_t>(dispatch, offset); };
    const auto relative = [&](std::size_t offset) {
        return reinterpret_cast<std::uintptr_t>(dispatch) + offset + 5 +
               read<std::int32_t>(dispatch, offset + 1);
    };
    const unsigned char prefix[]{0x55, 0x8B, 0xEC, 0xA1};
    const unsigned char gate[]{0xA8, 0x01, 0x75, 0x1A, 0x83, 0xC8, 0x01, 0xA3};
    const unsigned char ending[]{0x83, 0xC4, 0x04, 0xB9};
    require(!std::memcmp(dispatch, prefix, sizeof prefix) && address(4) == base + 0x5362700 &&
                !std::memcmp(dispatch + 8, gate, sizeof gate) &&
                address(0x10) == base + 0x5362700 && dispatch[0x14] == std::byte{0xE8} &&
                relative(0x14) == base + 0x377D50 && dispatch[0x19] == std::byte{0x68} &&
                address(0x1A) == base + 0xB1CF10 && dispatch[0x1E] == std::byte{0xE8} &&
                relative(0x1E) == base + 0xAC74D7 &&
                !std::memcmp(dispatch + 0x23, ending, sizeof ending) &&
                address(0x27) == base + 0x5362704 && dispatch[0x2B] == std::byte{0x5D} &&
                dispatch[0x2C] == std::byte{0xE9} && relative(0x2C) == base + 0x377DA0,
            "Startup SDK message map producer operands differ");
    const auto* const map = reinterpret_cast<const std::byte*>(std::uintptr_t(address(0x27)));
    using Less = bool(__cdecl*)(const std::int32_t*, const std::int32_t*);
    const auto less = read<Less>(map);
    constexpr unsigned char comparison[]{0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x08, 0x8B, 0x08, 0x8B,
                                         0x45, 0x0C, 0x3B, 0x08, 0x0F, 0x9C, 0xC0, 0x5D, 0xC3};
    require(reinterpret_cast<std::uintptr_t>(less) == base + 0x197570 &&
                !std::memcmp(reinterpret_cast<const void*>(less), comparison, sizeof comparison),
            "Startup SDK message map comparator differs");
    const auto* const nodes = read<const std::byte*>(map, 4);
    const auto capacity = read<std::int32_t>(map, 8);
    require(nodes && capacity > 0 && capacity <= 0xFFFF &&
                read<const std::byte*>(map, 0x18) == nodes,
            "Startup SDK message map pool is invalid");
    StartupUserMessageTargets targets{};
    const auto descriptors = startup_user_message_descriptors();
    for (std::size_t index = 0; index < targets.size(); ++index) {
        const auto identifier = descriptors[index].identifier;
        auto node = read<std::uint16_t>(map, 0x10);
        std::size_t visited = 0;
        for (;;) {
            require(node != 0xFFFF && node < capacity && visited++ < std::size_t(capacity),
                    "A required SDK user-message tree node is missing or cyclic");
            const auto* const item = nodes + std::size_t(node) * 0x30;
            const auto key = read<std::int32_t>(item, 8);
            // The independently verified SDK comparator above is precisely a
            // signed integer comparison; its ordinary semantics are source.
            if (identifier < key) {
                node = read<std::uint16_t>(item);
                continue;
            }
            if (key < identifier) {
                node = read<std::uint16_t>(item, 2);
                continue;
            }
            targets[index] = read<ClientStateScopedPatch**>(item, 0x14);
            require(targets[index] && read<std::int32_t>(item, 0x20) > 0 && *targets[index],
                    "A required SDK user-message binding vector is empty");
            break;
        }
    }
    return targets;
}
void prepare_startup_user_message_patches(engine::NativeDataImage& data,
                                          const StartupUserMessageTargets& targets) {
    auto*& head = data.native_cell<ClientStateScopedPatch*>(0x43476900u);
    if (head)
        throw std::logic_error("Startup user-message patch list is already prepared");
    const auto descriptors = startup_user_message_descriptors();
    for (std::size_t index = 0; index < targets.size(); ++index) {
        const auto& patch =
            data.native_cell<ClientStateScopedPatch>(descriptors[index].patch_address);
        if (patch.field_00 || patch.saved || patch.target || patch.field_0c || patch.identifier ||
            patch.next || !targets[index] || !*targets[index])
            throw std::logic_error("Startup user-message patch or actual SDK binding is invalid");
    }
    for (std::size_t index = 0; index < targets.size(); ++index) {
        auto& patch = data.native_cell<ClientStateScopedPatch>(descriptors[index].patch_address);
        patch.field_00 = std::uint32_t(reinterpret_cast<std::uintptr_t>(table.data()));
        patch.target = targets[index];
        patch.field_0c =
            std::uint32_t(reinterpret_cast<std::uintptr_t>(descriptors[index].callback));
        patch.identifier = descriptors[index].identifier;
        patch.next = head;
        head = &patch;
    }
}
void prepare_startup_user_message_patches(engine::NativeDataImage& data) {
    require(user_message_contexts_ready(data),
            "SDK user-message callbacks require their initialized source contexts");
    prepare_startup_user_message_patches(data, discover_startup_user_message_targets(data));
}
static_assert(sizeof(ClientStateScopedPatch) == 24);
} //namespace skeet::network

// ============================================================================
// Subcomponent: tick_update_dispatch.cpp
// ============================================================================

namespace skeet::network {
namespace {
TickUpdateRuntime* tick_update_runtime = nullptr;
/* [dedup field] */
void** original_table(void* object) {
    const auto table = *static_cast<void***>(object);
    return static_cast<const engine::VtableHookMetadata*>(table[-2])->original_table;
}
bool retain_incoming_tick(const TickUpdateBindings& bindings) { //433240A6
    auto* holder = bindings.game_rules;
    if (!holder || !*holder)
        return true;
    const auto offset = std::ptrdiff_t(bindings.game_rule_guard);
    return field<std::uint8_t>(*holder, offset) != 0;
}
}

void update_tick_status(const void* status, TickUpdateGlobals& globals,
                        const TickUpdateBindings& bindings) { //433841D8
    if ((field<std::uint32_t>(status, 0x24) & 4u) && !field<std::uint8_t>(status, 0x10)) {
        if (*globals.sequence == -1)
            globals.transition_status = 0;
        else if (globals.transition_status == 1) {
            globals.transition_status = 2;
            globals.previous_processed_command =
                engine::invoke<std::int32_t>(bindings.engine_client, 205);
            // The engine can replace the pointer during virtual205; native
            // code reloads434758D8 for the write after that call.
            *const_cast<std::int32_t*>(globals.sequence) = -1;
        }
    } else {
        if (globals.transition_status == 2)
            globals.transition_status = 0;
        else {
            const auto delta = globals.acknowledged_at - *globals.current_time;
            const auto absolute =
                std::bit_cast<float>(std::bit_cast<std::uint32_t>(delta) & 0x7FFFFFFFu);
            if (absolute > 2.0f)
                globals.transition_status = 0;
        }
    }
    globals.previous_command = engine::invoke<std::int32_t>(bindings.engine_client, 205);
}
void publish_update_status(void* object, const void* status, TickUpdateGlobals& globals,
                           const TickUpdateBindings& bindings) { //4333CA14
    update_tick_status(status, globals, bindings);
    globals.receivers.update_status(status);
    using Original = engine::MemberFunction<void, const void*>;
    reinterpret_cast<Original>(original_table(object)[37])(object, status);
}
void publish_incoming_tick(void* object, std::uintptr_t first, std::int32_t incoming,
                           TickUpdateGlobals& globals,
                           const TickUpdateBindings& bindings) { //4341EFED
    const auto selected =
        retain_incoming_tick(bindings)
            ? incoming
            : misc::acknowledge_datagram_sequence(bindings.acknowledgements, incoming);
    globals.previous_command = -1;
    globals.interpolation =
        misc::query_channel_interpolation(bindings.engine_client, bindings.latency);
    globals.receivers.reset_update_state(selected);
    for (auto* patch = globals.patches; patch; patch = patch->next) {
        if (patch->enabled)
            bindings.exchange(patch->target, std::bit_cast<std::int32_t>(patch->enabled));
    }
    using Original = engine::MemberFunction<void, std::uintptr_t, std::int32_t>;
    reinterpret_cast<Original>(original_table(object)[5])(object, first, selected);
}
void bind_tick_update_runtime(TickUpdateRuntime& value) noexcept {
    tick_update_runtime = &value;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall update_status_hook(void* object, void*, const void* status) {
#else
void update_status_hook(void* object, void*, const void* status) {
#endif
    tick_update_runtime->status(object, status);
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall incoming_tick_hook(void* object, void*, std::uintptr_t first,
                                   std::int32_t incoming) {
#else
void incoming_tick_hook(void* object, void*, std::uintptr_t first, std::int32_t incoming) {
#endif
    tick_update_runtime->incoming(object, first, incoming);
}
} //namespace skeet::network

// ============================================================================
// Subcomponent: update_observers.cpp
// ============================================================================

namespace skeet::network {
namespace {
void dispatch(std::span<void* const> entries, std::size_t phase) {
    for (auto* object : entries) {
        auto* observer = static_cast<UpdateObserver*>(object);
        observer->vtable->phases[phase](object);
    }
}
}
void* register_update_observer(engine::OwnerRegistryBinding registry, UpdateObserver& observer) {
    return registry.append(&observer);
}
void dispatch_update_phase0(const engine::OwnerRegistry& registry, std::int32_t& stage) {
    const auto entries = registry.entries();
    stage = 5;
    dispatch(entries, 0);
}
void dispatch_update_phase1(const engine::OwnerRegistry& registry, std::int32_t& stage) {
    const auto entries = registry.entries();
    stage = 6;
    dispatch(entries, 1);
}
void dispatch_update_phase2(const engine::OwnerRegistry& registry) {
    dispatch(registry.entries(), 2);
}
void dispatch_update_phase3(const engine::OwnerRegistry& registry) {
    dispatch(registry.entries(), 3);
}
void dispatch_update_phase4(const engine::OwnerRegistry& registry) {
    dispatch(registry.entries(), 4);
}
} // namespace skeet::network

// ============================================================================
// Subcomponent: user_message_runtime.cpp
// ============================================================================

namespace skeet::network {
namespace {
UserMessageRuntime* active = nullptr;
}
struct UserMessageRuntime::Data {
    client::ClientRuntimeStorage& storage;
    esp::UserMessageHandlers esp;
    rage::FeedbackPositionRuntime feedback;
    script::SayTextMessageRuntime chat;
    bool bound = false;
    Data(client::ClientRuntimeStorage& s, const players::State& players, script::Bridge& bridge)
        : storage(s),
          esp(s.data_owner(), {s.data().native_cell<esp::EspOwnerRuntime*>(0x434758DCu),
                               s.data().native_cell<std::uint8_t>(0x4346754Cu), players}),
          feedback{s.owner_registries().at(0x43475930u)},
          chat{bridge, script::SayTextMessageNames(s.event_names())} {}
};
UserMessageRuntime::UserMessageRuntime(client::ClientRuntimeStorage& storage,
                                       const players::State& players, script::Bridge& bridge)
    : data_(std::make_unique<Data>(storage, players, bridge)) {}
UserMessageRuntime::~UserMessageRuntime() {
    if (!data_->bound)
        return;
    esp::unbind_user_message_handlers(data_->esp);
    rage::unbind_feedback_position_runtime(data_->feedback);
    script::unbind_say_text_message_runtime(data_->chat);
    if (active == this)
        active = nullptr;
}
void UserMessageRuntime::bind() {
    if (active || data_->bound)
        throw std::logic_error("SDK user-message contexts are already bound");
    esp::bind_user_message_handlers(data_->esp);
    rage::bind_feedback_position_runtime(data_->feedback);
    script::bind_say_text_message_runtime(data_->chat);
    data_->bound = true;
    active = this;
}
bool UserMessageRuntime::bound_to(const engine::NativeDataImage& image) const noexcept {
    return data_->bound && &data_->storage.data() == &image;
}
bool user_message_contexts_ready(const engine::NativeDataImage& image) noexcept {
    return active && active->bound_to(image);
}
}
