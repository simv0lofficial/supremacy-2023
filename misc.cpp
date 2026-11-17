// Miscellaneous Features Subsystem

#include "util/address.h"
using skeet::field;
using skeet::store;
using skeet::read;
using skeet::write;
using skeet::add;
using skeet::subtract;

#include "skeet/misc/auto_accept_hook.hpp"
#include "skeet/misc/native_text.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/engine/unload.hpp"
#include <array>
#include <cstring>
#include <windows.h>
#include "skeet/misc/auto_accept_owner.hpp"
#include <cstddef>
#include <new>
#include <type_traits>
#include "skeet/misc/automatic_weapons.hpp"
#include "skeet/lagcomp/update_timing.hpp"
#include <bit>
#include "skeet/misc/automatic_weapons_owner.hpp"
#include "skeet/misc/command_owner.hpp"
#include "skeet/misc/clan_tag.hpp"
#include <xmmintrin.h>
#include "skeet/misc/clan_tag_owner.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/misc/console_output_owner.hpp"
#include "skeet/misc/disable_sv_pure_owner.hpp"
#include "skeet/misc/fov.hpp"
#include <emmintrin.h>
#include "skeet/misc/fov_owner.hpp"
#include "skeet/misc/logs.hpp"
#include "skeet/script/bridge.hpp"
#include <cmath>
#include <cstdio>
#include "skeet/misc/low_fps_runtime.hpp"
#include "skeet/hud/composite_handlers.hpp"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <intrin.h>
#include "skeet/misc/network_console_handlers.hpp"
#include "skeet/font_metrics.hpp"
#include <limits>
#include "skeet/misc/network_fix.hpp"
#include "skeet/client/prediction_history_owner.hpp"
#include "skeet/misc/no_fall_damage.hpp"
#include "skeet/movement/steering_math.hpp"
#include <immintrin.h>
#include "skeet/misc/persistent_kill_feed.hpp"
#include "skeet/core.hpp"
#include "skeet/events/event_dispatch.hpp"
#include "skeet/misc/ping_spike.hpp"
#include "skeet/misc/ping_spike_channel.hpp"
#include "skeet/misc/purchase_log.hpp"
#include "skeet/misc/purchase_log_owner.hpp"
#include "skeet/script/client_api.hpp"
#include "skeet/misc/quick_switch.hpp"
#include "skeet/misc/quick_switch_owner.hpp"
#include "skeet/misc/rebuy_fix.hpp"
#include "skeet/events/engine_event.hpp"
#include "skeet/misc/rebuy_fix_owner.hpp"
#include "skeet/misc/runtime.hpp"
#include "skeet/engine/owner_allocation.hpp"
#include "skeet/misc/reveal_overwatch.hpp"
#include "skeet/network/update_observers.hpp"
#include "skeet/misc/reveal_ranks.hpp"
#include "skeet/misc/settings_runtime.hpp"
#include "skeet/config_storage/settings_persistence.hpp"
#include "skeet/engine/native_data_image.hpp"
#include "skeet/misc/settings_runtime_owner.hpp"
#include "skeet/misc/unload.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/config_storage/owner.hpp"
#include "skeet/menu/startup_window.hpp"
#include "skeet/menu/startup_data.hpp"
#include "skeet/menu/root_menu.hpp"
#include "skeet/script/manager.hpp"
#include "skeet/misc/viewmodel_hooks.hpp"
#include "skeet/misc/viewmodel_lag_owner.hpp"
#include "skeet/menu/misc_menu.hpp"
#include "skeet/effects/view.hpp"
#include "skeet/misc/viewmodel_lag.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/misc/viewmodel_runtime_owner.hpp"
#include "skeet/engine/hook_runtime_owner.hpp"
#include "skeet/engine/netvar_initialization.hpp"
#include "skeet/effects/runtime_owner.hpp"
#include "skeet/skins/runtime_owner.hpp"
#include "skeet/skins/config.hpp"
#include "skeet/menu/settings_menu_storage.hpp"


// ============================================================================
// Subcomponent: auto_accept_hook.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
using AcceptCallback = void(__stdcall*)(const char*);
#else
using AcceptCallback = void (*)(const char*);
#endif
// deduplicated field
const engine::VtableHookMetadata& auto_accept_hook_metadata(void* receiver) {
    auto** table = field<void**>(receiver, 0);
    return *static_cast<const engine::VtableHookMetadata*>(table[-2]);
}
AutoAcceptPatchState* restore_patch = nullptr;
}

std::uint32_t process_auto_accept_event(void* receiver, const char* event_name,
                                        std::uintptr_t second, std::uintptr_t third,
                                        std::uintptr_t fourth, std::uintptr_t fifth,
                                        std::uintptr_t sixth, std::uintptr_t seventh) {
    if (event_name) {
        auto* owner = static_cast<AutoAcceptLifecycleOwner*>(auto_accept_hook_metadata(receiver).owner);
        const auto data = auto_accept_handler_data(*owner);
        if (data.configuration.enabled.value &&
            engine::netvar_name_hash(event_name) == data.name_hash) {
            //43320459..67 captures both encrypted bytes/key and callback before
            // copying. The local buffer is the native pair of stack words.
            const NativeEncryptedText encoded{data.name.bytes, data.name.key, {}};
            const auto callback = reinterpret_cast<AcceptCallback>(data.callback);
            std::array<char, 8> decoded;
            decode_native_text(encoded, decoded.data());
            // One pushed string argument, no caller ADD ESP: the genuine
            // constructor callback4346D554 uses stdcall/RET4; result unused.
            callback(decoded.data());
        }
    }
    // The callback may change the receiver's table. Native reloads it and its
    // metadata AFTER the callback, then selects original byte offset4/slot1.
    auto** original = auto_accept_hook_metadata(receiver).original_table;
    using Original =
        engine::MemberFunction<std::uint32_t, const char*, std::uintptr_t, std::uintptr_t,
                               std::uintptr_t, std::uintptr_t, std::uintptr_t, std::uintptr_t>;
    return reinterpret_cast<Original>(original[1])(receiver, event_name, second, third, fourth,
                                                   fifth, sixth, seventh);
}
#if defined(_M_IX86) || defined(__i386__)
std::uint32_t __fastcall auto_accept_event_hook(void* receiver, void*, const char* event_name,
#else
std::uint32_t auto_accept_event_hook(void* receiver, void*, const char* event_name,
#endif
                                                std::uintptr_t second, std::uintptr_t third,
                                                std::uintptr_t fourth, std::uintptr_t fifth,
                                                std::uintptr_t sixth, std::uintptr_t seventh) {
    return process_auto_accept_event(receiver, event_name, second, third, fourth, fifth, sixth,
                                     seventh);
}

std::uint32_t restore_auto_accept_receiver(AutoAcceptPatchState& patch, void* receiver,
                                           std::uintptr_t argument) {
    auto** original = auto_accept_hook_metadata(receiver).original_table;
    using Entry = engine::MemberFunction<std::uint32_t, std::uintptr_t>;
    //4338A9E9 saves original slot0 before exchanging the receiver's table.
    const auto entry = reinterpret_cast<Entry>(original[0]);
    InterlockedExchangePointer(static_cast<void* volatile*>(receiver),
                               patch.metadata.original_table);
    patch.metadata.target = nullptr;
    return entry(receiver, argument);
}
void bind_auto_accept_restore_hook(AutoAcceptPatchState& patch) noexcept {
    restore_patch = &patch;
}
#if defined(_M_IX86) || defined(__i386__)
std::uint32_t __fastcall auto_accept_restore_hook(void* receiver, void*, std::uintptr_t argument) {
#else
std::uint32_t auto_accept_restore_hook(void* receiver, void*, std::uintptr_t argument) {
#endif
    return restore_auto_accept_receiver(*restore_patch, receiver, argument);
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: auto_accept_owner.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define ACCEPT_MEMBER __thiscall
#else
#define ACCEPT_MEMBER
#endif
using OwnerEntry = void(ACCEPT_MEMBER*)(void*);
void ACCEPT_MEMBER auto_accept_owner_destroy_owner(void*);
void ACCEPT_MEMBER auto_accept_owner_install_owner(void*);
void ACCEPT_MEMBER auto_accept_owner_restore_owner(void*);
void ACCEPT_MEMBER auto_accept_owner_unchanged_owner(void*) {} //433CCE8E RET
const std::array<OwnerEntry, 9> auto_accept_owner_lifecycle_table{auto_accept_owner_destroy_owner,   auto_accept_owner_unchanged_owner, auto_accept_owner_unchanged_owner,
                                                auto_accept_owner_install_owner,   auto_accept_owner_restore_owner,   auto_accept_owner_unchanged_owner,
                                                auto_accept_owner_unchanged_owner, auto_accept_owner_unchanged_owner, auto_accept_owner_unchanged_owner};
struct AutoAcceptPrefix {
    const OwnerEntry* vtable;
    AutoAcceptConfig configuration;
    NativeEncryptedText name;
    std::uint32_t name_hash;
    void* callback;
};
static_assert(std::is_trivially_copyable_v<AutoAcceptPrefix>);
#if UINTPTR_MAX == UINT32_MAX
// Clang's x86 layout extension verifies the native inherited-config prefix.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(AutoAcceptPrefix) == 0x28);
static_assert(sizeof(AutoAcceptConfig) == 0x14);
static_assert(offsetof(BooleanConfig, value) == 0x10);
static_assert(offsetof(AutoAcceptPrefix, configuration) == 4);
static_assert(offsetof(AutoAcceptPrefix, name) == 0x18);
static_assert(offsetof(AutoAcceptPrefix, name_hash) == 0x20);
static_assert(offsetof(AutoAcceptPrefix, callback) == 0x24);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif
// deduplicated field
void free_aligned(engine::JobMemory memory, void* object) { //433837D0
    if (!object)
        return;
    auto* bytes = static_cast<std::byte*>(object);
    memory.free(memory.process_heap, 0, bytes - 16 - field<std::uint32_t>(bytes, -8));
}
AutoAcceptLifecycleOwner** active_owner_slot = nullptr;
}

class AutoAcceptLifecycleOwner {
public:
    AutoAcceptPrefix prefix;
    engine::JobMemory memory;
    AutoAcceptOwnerBindings bindings;

    AutoAcceptLifecycleOwner(engine::OwnerRegistryBinding lifecycle, ConfigRegistries& registries,
                             AutoAcceptOwnerBindings shared, const AutoAcceptPrefix& previous)
        : memory(lifecycle.memory), bindings(shared) {
        std::memcpy(&prefix, &previous, sizeof prefix);
        prefix.vtable = nullptr;
        lifecycle.append(this);
        prefix.vtable = auto_accept_owner_lifecycle_table.data(); //4346EDFC
        initialize_auto_accept_config(registries, prefix.configuration);
        construct_native_encrypted_text(prefix.name, "accept", 6, memory);
        prefix.name_hash = 0xE8BB3240;
        prefix.callback = bindings.constructor_callback;
    }
};
#if UINTPTR_MAX == UINT32_MAX
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(AutoAcceptLifecycleOwner, prefix) == 0);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif

namespace {
void ACCEPT_MEMBER auto_accept_owner_install_owner(void* object) {
    install_auto_accept(*static_cast<AutoAcceptLifecycleOwner*>(object));
}
void ACCEPT_MEMBER auto_accept_owner_restore_owner(void* object) {
    restore_auto_accept(*static_cast<AutoAcceptLifecycleOwner*>(object));
}
void ACCEPT_MEMBER auto_accept_owner_destroy_owner(void* object) { //433E3DAC
    *active_owner_slot = nullptr;
    auto* owner = static_cast<AutoAcceptLifecycleOwner*>(object);
    if (!owner)
        return;
    const auto memory = owner->memory;
    destroy_native_encrypted_text(owner->prefix.name, memory);
    owner->~AutoAcceptLifecycleOwner();
    free_aligned(memory, owner);
}
}

std::size_t auto_accept_owner_size() noexcept {
    return sizeof(AutoAcceptLifecycleOwner);
}
AutoAcceptLifecycleOwner& construct_auto_accept(void* storage,
                                                engine::OwnerRegistryBinding lifecycle,
                                                ConfigRegistries& registries,
                                                AutoAcceptOwnerBindings bindings) {
    active_owner_slot = &bindings.active_owner;
    AutoAcceptPrefix previous;
    std::memcpy(&previous, storage, sizeof previous);
    auto* owner =
        ::new (storage) AutoAcceptLifecycleOwner(lifecycle, registries, bindings, previous);
    bindings.active_owner = owner;
    return *owner;
}
AutoAcceptConfig& auto_accept_configuration(AutoAcceptLifecycleOwner& owner) noexcept {
    return owner.prefix.configuration;
}
AutoAcceptHandlerData auto_accept_handler_data(AutoAcceptLifecycleOwner& owner) noexcept {
    return {owner.prefix.configuration, owner.prefix.name, owner.prefix.name_hash,
            owner.prefix.callback};
}
void install_auto_accept(AutoAcceptLifecycleOwner& owner) {
    if (!owner.prefix.callback || !owner.bindings.source_module)
        return;
    auto* ui_engine = engine::invoke<void*>(owner.bindings.panorama, 11);
    if (!ui_engine)
        return;
    auto* target = engine::invoke<void*>(ui_engine, 25);
    if (!target)
        return;
    owner.bindings.patch.metadata.target = target;
    owner.bindings.patch.metadata.owner = &owner;
}
void restore_auto_accept(AutoAcceptLifecycleOwner& owner) {
    auto& descriptor = owner.bindings.patch.metadata;
    if (!descriptor.replacement_table || !descriptor.target)
        return;
    InterlockedExchangePointer(reinterpret_cast<void* volatile*>(descriptor.target),
                               descriptor.original_table);
}
std::span<const AutoAcceptHookReplacement, 2> auto_accept_hook_replacements() {
    static const std::array<AutoAcceptHookReplacement, 2> replacements{
        {{0, 0x4338A9E9u, reinterpret_cast<void*>(&auto_accept_restore_hook)},
         {1, 0x43320430u, reinterpret_cast<void*>(&auto_accept_event_hook)}}};
    return replacements;
}
AutoAcceptHookFamily::AutoAcceptHookFamily(AutoAcceptPatchState& patch,
                                           engine::VtableHookRegistry& hooks,
                                           std::byte* const& source_module,
                                           engine::NativeImageEnvironment images,
                                           engine::HookDescriptorSystem system) noexcept
    : patch_(patch), hooks_(hooks), source_module_(source_module), images_(images),
      system_(system) {}
void AutoAcceptHookFamily::prepare() {
    engine::prepare_rtti_vtable_hook(patch_.metadata, source_module_, auto_accept_hook_class_hash,
                                     auto_accept_hook_base_hash, images_, hooks_.memory, system_);
    bind_replacements();
}
void AutoAcceptHookFamily::bind_replacements() noexcept {
    // The slot0 callback reads this same descriptor view even if cleanup-node
    // allocation fails after install_vtable_hook has performed its atomic swap.
    bind_auto_accept_restore_hook(patch_);
    for (const auto& replacement : auto_accept_hook_replacements())
        patch_.metadata.replacement_table[replacement.slot] = replacement.source_entry;
}
engine::VtableHookNode* AutoAcceptHookFamily::install(void* actual_receiver) {
    return engine::install_vtable_hook(hooks_, actual_receiver, patch_.metadata);
}
#undef ACCEPT_MEMBER
} //namespace skeet::misc

// ============================================================================
// Subcomponent: automatic_weapons.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// deduplicated field

void* frame_weapon(const client::CommandFrame& frame) noexcept {
    return reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
}

// deduplicated add

bool first_bomb_nearby(const AutomaticWeaponsBindings& bindings, void* player) {
    // Complete 4340AEDA. Retain both returned references until after the local
    // origin query: an engine call can change the candidate's origin storage.
    const auto* list = *bindings.planted_bombs;
    if (!list || list->count == 0)
        return false;
    auto* bomb = list->entries[0];
    if (!bomb || !field<std::uint8_t>(bomb, bindings.offsets.bomb_ticking))
        return false;
    const auto* bomb_origin = engine::invoke<const std::array<float, 3>*>(bomb, 10);
    const auto* player_origin = engine::invoke<const std::array<float, 3>*>(player, 10);
    const auto x = (*bomb_origin)[0] - (*player_origin)[0];
    const auto y = (*bomb_origin)[1] - (*player_origin)[1];
    const auto z = (*bomb_origin)[2] - (*player_origin)[2];
    return ((y * y) + (x * x)) + (z * z) < 10000.0f;
}

void process_last_second_defuse(const AutomaticWeaponsConfig& config,
                                const AutomaticWeaponsBindings& bindings,
                                const client::CommandFrame& frame, void* player,
                                client::Command& command, InputBitmap input) {
    if (!hotkey_active(config.last_second_defuse.value.binding, input) || !*bindings.planted_bombs)
        return;
    if (field<std::int32_t>(player, bindings.offsets.team) != 3 || !(frame.flags & 0x200000u) ||
        !first_bomb_nearby(bindings, player))
        return;

    // The native margin is 350ms on top of the actual 5/10 second defuse time.
    const auto duration = field<std::uint8_t>(player, bindings.offsets.has_defuser)
                              ? 5.349999904632568f
                              : 10.350000381469727f;
    // Reload the live list after both engine origin queries, as the owner does.
    const auto* bomb = (*bindings.planted_bombs)->entries[0];
    const auto remaining =
        field<float>(bomb, bindings.offsets.bomb_detonation_time) - *bindings.current_time;
    if (remaining <= duration)
        command.movement.buttons |= 0x20u;
}

void release_attack(const client::CommandFrame& frame, client::Command& command) noexcept {
    command.movement.buttons &= ~1u;
    if (frame.weapon_id_d8 == 64)
        command.movement.buttons &= ~0x800u;
}
}

void process_automatic_weapons(AutomaticWeaponsState& state, const AutomaticWeaponsConfig& config,
                               const AutomaticWeaponsBindings& bindings,
                               const client::CommandFrame& frame, void* player,
                               client::Command& command, InputBitmap input) {
    process_last_second_defuse(config, bindings, frame, player, command, input);
    if (!config.enabled.value)
        return;

    const auto& offsets = bindings.offsets;
    if (field<std::uint8_t>(player, offsets.player_blocks_attack))
        command.movement.buttons &= ~0x801u;

    auto* weapon = frame_weapon(frame);
    if (!weapon || !lagcomp::history_weapon_is_firearm(
                       field<std::uint16_t>(weapon, offsets.weapon_definition)))
        return;
    // Revolver mode is handled regardless of its automatic-fire virtual. Other
    // automatic weapons keep the engine's held-attack behavior.
    if (frame.weapon_id_d8 != 64 && engine::invoke<bool>(weapon, 430))
        return;

    if ((frame.flags & 8u) && field<std::int32_t>(player, offsets.shots_fired) > 0) {
        release_attack(frame, command);
        return;
    }

    const auto configured_delay = config.delay.value;
    weapon = frame_weapon(frame);
    const auto last_shot = field<float>(weapon, offsets.weapon_last_shot);
    if (state.last_shot_time != last_shot) {
        state.last_shot_time = last_shot;
        if (configured_delay) {
            auto minimum = std::bit_cast<std::int32_t>(1u - std::uint32_t(configured_delay));
            if (minimum < -30)
                minimum = -30;
            state.delay_adjustment = bindings.random_integer(minimum, 30);
        }
    }
    if (field<std::int32_t>(player, offsets.shots_fired) <= 0)
        return;

    // Both the active weapon and its shot timestamp are read again after the
    // random import. Capture elapsed time before the fire-cycle query.
    weapon = frame_weapon(frame);
    const auto elapsed = *bindings.current_time - field<float>(weapon, offsets.weapon_last_shot);
    if (configured_delay) {
        const auto delay = add(configured_delay, state.delay_adjustment);
        if (delay > 0) {
            const auto cycle = engine::invoke<float>(weapon, 433, std::int32_t(0));
            const auto extra = float(delay) * 0.0010000000474974513f;
            const auto release_after = float(static_cast<long double>(cycle) + extra);
            if (release_after > elapsed)
                return;
        }
    }
    release_attack(frame, command);
}

} // namespace skeet::misc

// ============================================================================
// Subcomponent: automatic_weapons_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define AUTOMATIC_MEMBER __thiscall
#else
#define AUTOMATIC_MEMBER
#endif
using OwnerEntry = void(AUTOMATIC_MEMBER*)(void*);
void AUTOMATIC_MEMBER destroy_automatic_weapons(void*);
void AUTOMATIC_MEMBER automatic_weapons_owner_unchanged_automatic_weapons(void*) {} //actual433CCE8E RET
const std::array<OwnerEntry, 9> automatic_weapons_owner_lifecycle_table{
    destroy_automatic_weapons,   automatic_weapons_owner_unchanged_automatic_weapons, automatic_weapons_owner_unchanged_automatic_weapons,
    automatic_weapons_owner_unchanged_automatic_weapons, automatic_weapons_owner_unchanged_automatic_weapons, automatic_weapons_owner_unchanged_automatic_weapons,
    automatic_weapons_owner_unchanged_automatic_weapons, automatic_weapons_owner_unchanged_automatic_weapons, automatic_weapons_owner_unchanged_automatic_weapons};
}

class AutomaticWeaponsLifecycleOwner {
public:
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    CommandOwner command;
    client::NativeCommandPhaseReceiver command_receiver{};

    AutomaticWeaponsLifecycleOwner(engine::OwnerRegistryBinding lifecycle,
                                   client::CommandPhaseRegistry& commands,
                                   ConfigRegistries& registries, AutomaticWeaponsConfig& settings,
                                   AutomaticWeaponsState& state,
                                   const AutomaticWeaponsBindings& bindings)
        : memory(lifecycle.memory),
          command(AutomaticWeaponsCommandOwner{state, settings, bindings}) {
        lifecycle.append(this); //4347D999 ->433676A0, before primary table assignment
        commands.append(command_receiver, &command); //4347D9A1 ->43345EBC
        vtable = automatic_weapons_owner_lifecycle_table.data(); //4347D9A9, table4346F358
        commands.publish(command_receiver, client::CommandOwnerId::automatic_weapons); //4347D9B1
        initialize_automatic_weapons_config(registries, settings);
        state.delay_adjustment = 0;
        state.last_shot_time = 0.0f;
    }
};

namespace {
void AUTOMATIC_MEMBER destroy_automatic_weapons(void* object) {
    // Native433B1A7A jumps directly to433837D0. The configuration and command
    // registries are not edited here; their common shutdown owns their ranges.
    auto* owner = static_cast<AutomaticWeaponsLifecycleOwner*>(object);
    const auto memory = owner->memory;
    owner->~AutomaticWeaponsLifecycleOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}
std::size_t automatic_weapons_owner_size() noexcept {
    return sizeof(AutomaticWeaponsLifecycleOwner);
}
AutomaticWeaponsLifecycleOwner&
construct_automatic_weapons(void* storage, engine::OwnerRegistryBinding lifecycle,
                            client::CommandPhaseRegistry& commands, ConfigRegistries& registries,
                            AutomaticWeaponsConfig& settings, AutomaticWeaponsState& state,
                            const AutomaticWeaponsBindings& bindings) {
    return *::new (storage)
        AutomaticWeaponsLifecycleOwner(lifecycle, commands, registries, settings, state, bindings);
}
#undef AUTOMATIC_MEMBER
} //namespace skeet::misc

// ============================================================================
// Subcomponent: clan_tag.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// clan_tag_read
template <class T> T clan_tag_read(void* object, std::uint32_t offset) { T val; std::memcpy(&val, static_cast<std::byte*>(object) + offset, sizeof val); return val; }
std::int32_t clan_tag_signed_bits(std::uint32_t value) { return std::bit_cast<std::int32_t>(value); }
}
std::int32_t ClanTagClock::mean_latency_ticks() const {
    return lagcomp::mean_history_timing_ticks(latency);
}
std::int32_t ClanTagClock::corrected_client_tick() const {
    auto result = client_tick;
    if (acknowledged_tick && outgoing_tick) {
        const auto difference = std::uint32_t(*outgoing_tick) - std::uint32_t(*acknowledged_tick);
        if (difference - 1 < 9)
            result = clan_tag_signed_bits(std::uint32_t(result) + difference);
    }
    return result;
}
void ClanTagRuntime::reset() noexcept {
    was_enabled = 0;
}
void ClanTagRuntime::update() {
    if (!set_tag)
        return;
    if (!*enabled) {
        if (!was_enabled)
            return;
        was_enabled = 0;
        last_frame = -2;
        if (!engine::invoke<bool>(engine, 26) || !clan_id_cvar ||
            !clan_tag_read<std::uint32_t>(clan_id_cvar, 0x60))
            return;
        const auto callbacks = clan_tag_read<void**>(clan_id_cvar, 0x54);
        using Callback = void (*)(void*, const char*, float);
        reinterpret_cast<Callback>(callbacks[0])(static_cast<std::byte*>(clan_id_cvar) + 0x18, "",
                                                 0.0f);
        return;
    }
    auto* rules = game_rules ? *game_rules : nullptr;
    const auto phase = rules ? clan_tag_read<std::int32_t>(rules, game_phase_offset) : 0;
    if (phase == 4 || phase == 5) {
        if (last_frame == -1)
            return;
        last_frame = -1;
        changed = 1;
        set_tag("supremacy.su", "supremacy.su");
        was_enabled = 1;
        changed = 0;
        return;
    }
    const auto tick = clan_tag_signed_bits(std::uint32_t(clock->mean_latency_ticks()) +
                                  std::uint32_t(clock->corrected_client_tick()));
    const auto period = _mm_cvttss_si32(_mm_set_ss(0.3f / clock->interval_per_tick + 0.5f));
    const auto step = std::uint32_t(tick / period) % 30;
    static constexpr std::array<std::int32_t, 30> frames{
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 11, 11, 11,
        11, 11, 11, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
    const auto frame = frames[step];
    if (frame == last_frame)
        return;
    last_frame = frame;
    std::array<char, 44> text{};
    constexpr char animation[] = "             supremacy                ";
    std::memcpy(text.data(), animation, sizeof(animation));
    changed = 1;
    auto* tag = text.data() + frame;
    tag[15] = '\0';
    set_tag(tag, tag);
    was_enabled = 1;
    changed = 0; // native final WORD store at class +0x28
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: clan_tag_owner.cpp
// ============================================================================

namespace skeet::misc {
void process_clan_tag_message(ClanTagRuntime& runtime,
                              const client::ScriptCommandEnvironment& environment,
                              NetworkMessageCall& call) {
    if (!runtime.changed)
        return;
    if (engine::invoke<std::int32_t>(call.message, 7) != 18)
        return;
    if (client::command_roll_blocked(environment))
        return;
    const std::uint8_t value = 0;
    std::memcpy(static_cast<std::byte*>(call.message) + 0x18, &value, sizeof value);
    *call.voice = 1;
}
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define CLAN_MEMBER __thiscall
#define CLAN_FAST __fastcall
#else
#define CLAN_MEMBER
#define CLAN_FAST
#endif
using OwnerEntry = void(CLAN_MEMBER*)(void*);
struct ClanTagInstance;
struct MessageEndpoint : NetworkMessageCallback {
    ClanTagInstance* owner;
};
void CLAN_MEMBER clan_tag_owner_destroy(void*);
void CLAN_MEMBER clan_tag_owner_unchanged(void*) {} //433CCE8E
void CLAN_MEMBER clan_tag_owner_reset(void*);
void CLAN_MEMBER clan_tag_owner_reset_forwarder(void*);
void CLAN_FAST message_receiver(void*, NetworkMessageCall*);
const std::array<OwnerEntry, 9> clan_tag_owner_lifecycle_table{clan_tag_owner_destroy,   clan_tag_owner_unchanged,       clan_tag_owner_unchanged,
                                                clan_tag_owner_unchanged, clan_tag_owner_unchanged,       clan_tag_owner_reset,
                                                clan_tag_owner_unchanged, clan_tag_owner_reset_forwarder, clan_tag_owner_unchanged};
const std::array<NetworkMessageEntry, 1> message_table{message_receiver};

struct ClanTagInstance {
    const OwnerEntry* vtable = nullptr;
    client::NativeEntityObserverReceiver entity_receiver{};
    MessageEndpoint message{{nullptr}, this};
    ClanTagRuntime& runtime;
    const client::ScriptCommandEnvironment& environment;
    engine::JobMemory memory;
    ClanTagInstance(engine::OwnerRegistryBinding owners, client::EntityObserverRegistry& updates,
                    engine::OwnerRegistryBinding messages, ConfigRegistries& registry,
                    ClanTagConfig& settings, ClanTagRuntime& state,
                    const client::ScriptCommandEnvironment& context)
        : runtime(state), environment(context), memory(owners.memory) {
        owners.append(this); //4347E09A
        updates.append_clan_tag(entity_receiver, runtime); //4347E0A2
        messages.append(&message); //4347E0AA
        vtable = clan_tag_owner_lifecycle_table.data(); //4347E0B2, table4346F728
        updates.publish_clan_tag(entity_receiver); //4347E0BA, table4346EEA0
        message.vtable = message_table.data(); //4347E0C2
        initialize_clan_tag_config(registry, settings);
        runtime.enabled = &settings.enabled.value;
        runtime.was_enabled = 0;
        runtime.changed = 0;
        runtime.last_frame = 0;
    }
};
void CLAN_MEMBER clan_tag_owner_destroy(void* object) { //433B1A7A ->433837D0
    auto* owner = static_cast<ClanTagInstance*>(object);
    const auto memory = owner->memory;
    owner->~ClanTagInstance();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void CLAN_MEMBER clan_tag_owner_reset(void* object) {
    static_cast<ClanTagInstance*>(object)->runtime.reset();
}
void CLAN_MEMBER clan_tag_owner_reset_forwarder(void* object) { //433CF2F5 calls current primaryslot5
    static_cast<ClanTagInstance*>(object)->vtable[5](object);
}
void CLAN_FAST message_receiver(void* object, NetworkMessageCall* call) { //4336E573
    auto* endpoint = static_cast<MessageEndpoint*>(static_cast<NetworkMessageCallback*>(object));
    process_clan_tag_message(endpoint->owner->runtime, endpoint->owner->environment, *call);
}
#undef CLAN_MEMBER
#undef CLAN_FAST
}
std::size_t clan_tag_owner_size() noexcept {
    return sizeof(ClanTagInstance);
}
void construct_clan_tag_owner(void* storage, engine::OwnerRegistryBinding owners,
                              client::EntityObserverRegistry& updates,
                              engine::OwnerRegistryBinding messages, ConfigRegistries& registry,
                              ClanTagConfig& settings, ClanTagRuntime& runtime,
                              const client::ScriptCommandEnvironment& context) {
    ::new (storage)
        ClanTagInstance(owners, updates, messages, registry, settings, runtime, context);
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: command_owner.cpp
// ============================================================================

namespace skeet::misc {
void KnifeCommandBindings::process(const KnifeConfig& settings, client::CommandFrame& frame,
                                   client::Command& command, void* player, std::uint8_t& packet,
                                   std::uint32_t& flags) const {
    std::visit(
        [&](auto* source) {
            using Source = std::remove_cv_t<std::remove_pointer_t<decltype(source)>>;
            if constexpr (std::is_same_v<Source, KnifeRuntimeBindings>)
                process_knife_command(settings, frame, command, player, packet, flags, *source);
            else
                source->process(settings, frame, command, player, packet, flags);
        },
        source_);
}
void dispatch_command_owner(CommandOwner& value, std::uint32_t phase, client::CommandFrame* frame,
                            void* player, void* weapon, client::Command& command,
                            std::uint8_t& send_packet, std::array<float, 3>& incoming_angles,
                            InputBitmap keys) {
    std::visit(
        [&](auto& owner) {
            using Owner = std::remove_cvref_t<decltype(owner)>;
            if constexpr (std::is_same_v<Owner, QuickSwitchRuntime*>) {
                if (phase == 2)
                    owner->process_phase2(player, weapon, command, send_packet);
            } else if constexpr (std::is_same_v<Owner, RebuyFixRuntime*>) {
                if (phase == 2) {
                    owner->local_player = player;
                    owner->update(command.movement);
                }
            } else if constexpr (std::is_same_v<Owner, AutomaticWeaponsCommandOwner>) {
                if (phase == 4)
                    process_automatic_weapons(owner.state, owner.settings, owner.bindings, *frame,
                                              player, command, keys);
            } else if constexpr (std::is_same_v<Owner, KnifeCommandOwner>) {
                if (phase == 4)
                    owner.bindings.process(owner.settings, *frame, command, player, send_packet,
                                           owner.scheduler_flags);
            } else if constexpr (std::is_same_v<Owner, ZeusCommandOwner>) {
                if (phase == 4)
                    process_zeus(owner.state, owner.settings, owner.bindings, *frame, player,
                                 command);
            } else if constexpr (std::is_same_v<Owner, BlockbotCommandOwner>) {
                if (phase == 2) {
                    MovementAngles angles{incoming_angles[0], incoming_angles[1],
                                          incoming_angles[2]};
                    process_blockbot(owner.state, owner.enabled.value != 0,
                                     owner.hotkey.value.binding, keys, player, command, angles,
                                     owner.bindings);
                    incoming_angles = {angles.pitch, angles.yaw, angles.roll};
                }
            }
        },
        value);
}
std::int32_t command_owner_budget(const CommandOwner&, std::int32_t incoming) noexcept {
    return incoming;
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: config.cpp
// ============================================================================

namespace skeet::misc {
void initialize_fov_config(ConfigRegistries& r, FovConfig& c) noexcept {
    initialize_integer_config(r.descriptors, c.fov, 0xFC225A4Fu, 90);
    initialize_integer_config(r.descriptors, c.zoom_fov, 0xCA80F9EDu, 100);
}
void initialize_knife_config(ConfigRegistries& r, KnifeConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x7A5909EEu);
    initialize_integer_config(r.descriptors, c.mode, 0xA7C81D9Bu, 3);
}
void initialize_zeus_config(ConfigRegistries& r, ZeusConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x2E314CE2u);
}
void initialize_automatic_weapons_config(ConfigRegistries& r, AutomaticWeaponsConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x3D5DE026u);
    initialize_integer_config(r.descriptors, c.delay, 0x3952AAA6u, c.delay.value);
    initialize_hotkey_config(r, c.last_second_defuse, 0x473CC2DEu, 0x52, HotkeyMode::hold);
}
void initialize_quick_switch_config(ConfigRegistries& r, QuickSwitchConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x0D634908u);
}
void initialize_reveal_ranks_config(ConfigRegistries& r, RevealRanksConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x30E0CFE7u);
}
void initialize_reveal_overwatch_config(ConfigRegistries& r, RevealOverwatchConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0xD351D519u);
}
void initialize_auto_accept_config(ConfigRegistries& r, AutoAcceptConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x141F80B7u);
    c.enabled.value = 1;
}
void initialize_clan_tag_config(ConfigRegistries& r, ClanTagConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x9F6C4676u);
}
void initialize_event_log_config(ConfigRegistries& r, EventLogConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.purchases, 0x5086BCF5u);
    initialize_boolean_config(r.descriptors, c.damage, 0x0CE14F5Au);
}
void initialize_grenade_prediction_config(ConfigRegistries& r, GrenadePredictionConfig& c,
                                          esp::GrenadeEspConfig& visual) noexcept {
    initialize_boolean_config(r.descriptors, c.super_toss, 0x37A61386u);
    initialize_boolean_config(r.descriptors, visual.trajectory, 0xDC4C1BB9u);
    initialize_color_config(r.descriptors, visual.trajectory_color, 0x8428F8F9u, 0xB33232FFu);
    initialize_color_config(r.descriptors, visual.trajectory_secondary_color, 0xC2611C1Fu,
                            0xFF3BC896u);
    initialize_boolean_config(r.descriptors, visual.proximity_warning, 0x85BDA341u);
    initialize_boolean_config(r.descriptors, c.automatic_release, 0x969EEF1Fu);
    initialize_hotkey_config(r, c.release_key, 0xDB791DB8u, 0x1B, HotkeyMode::always);
    initialize_integer_config(r.descriptors, c.minimum_damage, 0x510F682Du, 50);
}
void initialize_persistent_kill_feed_config(ConfigRegistries& r,
                                            PersistentKillFeedConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x245CEDB5u);
}
void initialize_disable_pure_config(ConfigRegistries& r, DisablePureConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x177AB88Cu);
}
void initialize_rebuy_config(ConfigRegistries& r, RebuyConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x6ED248E1u);
    c.enabled.value = 0;
}
void initialize_console_output_config(ConfigRegistries& r, ConsoleOutputConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x52EADF91u);
}
void initialize_ping_spike_enabled(ConfigRegistries& r, PingSpikeConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x916F47DAu);
}
void initialize_ping_spike_key(ConfigRegistries& r, PingSpikeConfig& c) noexcept {
    initialize_hotkey_config(r, c.key, 0x3C4135FFu, 0x1B, HotkeyMode::hold);
}
void initialize_ping_spike_milliseconds(ConfigRegistries& r, PingSpikeConfig& c) noexcept {
    initialize_integer_config(r.descriptors, c.milliseconds, 0xC5290E81u, 100);
}
void initialize_free_look_config(ConfigRegistries& r, FreeLookConfig& c) noexcept {
    initialize_hotkey_config(r, c.key, 0x77D31850u, 0x1B, HotkeyMode::hold);
}
void initialize_anti_untrusted_config(ConfigRegistries& r, AntiUntrustedConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x851E6ABEu);
    c.enabled.value = 1;
}
void initialize_custom_events_config(ConfigRegistries& r, CustomEventsConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x526A2BF4u);
    c.enabled.value = 1;
}
void initialize_hide_from_obs_config(ConfigRegistries& r, HideFromObsConfig& c) noexcept {
    initialize_boolean_config(r.descriptors, c.enabled, 0x643364E6u);
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: console_output_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define CONSOLE_MEMBER __thiscall
#else
#define CONSOLE_MEMBER
#endif
using OwnerEntry = void(CONSOLE_MEMBER*)(void*);
// This points to the canonical434758CC slot. Keeping its address also
// preserves4332503F's unconditional clear when the receiver itself is null.
ColoredLogRing** console_active_slot = nullptr;
void CONSOLE_MEMBER console_output_owner_destroy(void*);
void CONSOLE_MEMBER console_output_owner_unchanged(void*) {} //433CCE8E, all eight remaining entries.
const std::array<OwnerEntry, 9> console_output_owner_lifecycle_table{console_output_owner_destroy,   console_output_owner_unchanged, console_output_owner_unchanged,
                                                console_output_owner_unchanged, console_output_owner_unchanged, console_output_owner_unchanged,
                                                console_output_owner_unchanged, console_output_owner_unchanged, console_output_owner_unchanged}; //4346F418

void* console_output_owner_allocate_owner(engine::JobMemory memory, std::size_t size) { //43387448
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    const auto allocation_size = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void console_output_owner_free_owner(engine::JobMemory memory, void* object) { //433837D0
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}

class ConsoleOutputOwner {
public:
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    ColoredLogRing ring;
    ConsoleOutputHandler frame;

    ConsoleOutputOwner(engine::OwnerRegistryBinding lifecycle,
                       engine::OwnerRegistryBinding callbacks, ConfigRegistries& registries,
                       ConsoleOutputConfig& settings, ColoredLogRing*& active,
                       const float& frame_time)
        : memory(lifecycle.memory), frame(settings, ring, frame_time) {
        console_active_slot = &active;
        lifecycle.append(this); //433676A0
        hud::register_hud_frame(callbacks, &frame); //433D20B5
        vtable = console_output_owner_lifecycle_table.data();
        frame.initialize_vtable();
        initialize_console_output_config(registries, settings); //preserves value
        ring.head = 0;
        ring.count = 0;
        for (auto& line : ring.lines)
            line.text.clear(); //eight native UTF16 vectors
        ring.hex = {u'0', u'1', u'2', u'3', u'4', u'5', u'6', u'7',
                    u'8', u'9', u'A', u'B', u'C', u'D', u'E', u'F'};
    }
};
namespace {
void CONSOLE_MEMBER console_output_owner_destroy(void* object) { //4332503F
    *console_active_slot = nullptr;
    if (!object)
        return;
    auto* owner = static_cast<ConsoleOutputOwner*>(object);
    const auto memory = owner->memory;
    owner->~ConsoleOutputOwner(); //43369B05: console_output_owner_destroy all eight UTF16 strings
    console_output_owner_free_owner(memory, object);
}
#undef CONSOLE_MEMBER
}
std::size_t console_output_owner_size() noexcept {
    return sizeof(ConsoleOutputOwner);
}
ConsoleOutputOwner&
construct_console_output_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                               engine::OwnerRegistryBinding callbacks, ConfigRegistries& registries,
                               ConsoleOutputConfig& settings, ColoredLogRing*& active,
                               const float& frame_time) {
    auto* owner = ::new (storage)
        ConsoleOutputOwner(lifecycle, callbacks, registries, settings, active, frame_time);
    active = &owner->ring;
    return *owner;
}
ColoredLogRing& console_output_ring(ConsoleOutputOwner& owner) noexcept {
    return owner.ring;
}
ColoredLogRing* initialize_console_output_owner(engine::OwnerRegistryBinding lifecycle,
                                                engine::OwnerRegistryBinding callbacks,
                                                ConfigRegistries& registries,
                                                ConsoleOutputConfig& settings,
                                                ColoredLogRing*& active,
                                                const float& frame_time) { //43484E4E
    console_active_slot = &active;
    auto* storage = console_output_owner_allocate_owner(lifecycle.memory, sizeof(ConsoleOutputOwner));
    if (!storage) {
        active = nullptr;
        return nullptr;
    } //43484E65..E6C, restored-source path
    construct_console_output_owner(storage, lifecycle, callbacks, registries, settings, active,
                                   frame_time);
    return active;
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: disable_sv_pure_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define PURE_MEMBER __thiscall
#else
#define PURE_MEMBER
#endif
using OwnerEntry = void(PURE_MEMBER*)(void*);
DisableSvPureOwner** pure_active_slot = nullptr; //address of canonical43475974
void PURE_MEMBER disable_sv_pure_owner_destroy(void*);
void PURE_MEMBER disable_sv_pure_owner_unchanged_startup(void*) {} //4347D93C: RET
void PURE_MEMBER disable_sv_pure_owner_unchanged(void*) {} //433CCE8E: RET0
const std::array<OwnerEntry, 9> disable_sv_pure_owner_lifecycle_table{disable_sv_pure_owner_destroy,   disable_sv_pure_owner_unchanged_startup, disable_sv_pure_owner_unchanged, disable_sv_pure_owner_unchanged,
                                                disable_sv_pure_owner_unchanged, disable_sv_pure_owner_unchanged,         disable_sv_pure_owner_unchanged, disable_sv_pure_owner_unchanged,
                                                disable_sv_pure_owner_unchanged}; //raw4346F534
}

class DisableSvPureOwner {
public:
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    DisablePureConfig& configuration;
    DisableSvPureHandler message;

    DisableSvPureOwner(engine::OwnerRegistryBinding lifecycle,
                       engine::OwnerRegistryBinding messages, ConfigRegistries& registries,
                       DisablePureConfig& settings, DisableSvPureOwner*& active)
        : memory(lifecycle.memory), configuration(settings), message(settings) {
        pure_active_slot = &active;
        lifecycle.append(this); //433676A0
        messages.append(&message); //4336F73C
        vtable = disable_sv_pure_owner_lifecycle_table.data(); //4346F534
        message.initialize_vtable(); //43472C74 ->4335C233
        initialize_disable_pure_config(registries, settings); //existing byte preserved
    }
};
namespace {
void PURE_MEMBER disable_sv_pure_owner_destroy(void* object) { //433ECC98 ->433837D0
    *pure_active_slot = nullptr;
    if (!object)
        return;
    auto* owner = static_cast<DisableSvPureOwner*>(object);
    const auto memory = owner->memory;
    owner->~DisableSvPureOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
#undef PURE_MEMBER
}
std::size_t disable_sv_pure_owner_size() noexcept {
    return sizeof(DisableSvPureOwner);
}
DisableSvPureOwner&
construct_disable_sv_pure_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                                engine::OwnerRegistryBinding messages, ConfigRegistries& registries,
                                DisablePureConfig& settings, DisableSvPureOwner*& active) {
    return *::new (storage) DisableSvPureOwner(lifecycle, messages, registries, settings, active);
}
const DisablePureConfig& disable_sv_pure_configuration(const DisableSvPureOwner& owner) noexcept {
    return owner.configuration;
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: fov.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// fov_read
template <class T> T fov_read(void* object, std::uint32_t offset) { T val; std::memcpy(&val, static_cast<std::byte*>(object) + offset, sizeof val); return val; }
std::int32_t fov_signed_bits(std::uint32_t value) { return std::bit_cast<std::int32_t>(value); }
void write_override(void* context, float value) {
    std::memcpy(static_cast<std::byte*>(context) + 8, &value, sizeof value);
}
}
void ZoomSensitivityRuntime::reset() {
    if (const auto context = bindings.override_context)
        write_override(context, 0.0f);
}
void ZoomSensitivityRuntime::update() {
    const auto initial_context = bindings.override_context;
    if (!initial_context)
        return;
    write_override(initial_context, 0.0f);
    auto* player = bindings.local_player;
    if (!player || settings.zoom_fov.value == 100 ||
        fov_read<std::int32_t>(player, bindings.fov_offset) == 45)
        return;
    if (settings.zoom_fov.value == 0) {
        write_override(initial_context, 1.0f);
        return;
    }
    auto current = _mm_set_ss(engine::invoke<float>(player, 332));
    auto base = engine::invoke<std::int32_t>(player, 333);
    const auto current_integer = _mm_cvttss_si32(current);
    if (current_integer == base)
        return;
    if (!base)
        base = 1;
    const auto difference = fov_signed_bits(std::uint32_t(current_integer) - std::uint32_t(base));
    if (difference) {
        auto scaled = _mm_cvtepi32_ps(_mm_cvtsi32_si128(settings.zoom_fov.value));
        scaled = _mm_mul_ss(scaled, _mm_set_ss(0.01f));
        scaled = _mm_mul_ss(scaled, _mm_cvtepi32_ps(_mm_cvtsi32_si128(difference)));
        scaled = _mm_add_ss(scaled, _mm_set_ss(0.5f));
        const auto interpolated =
            fov_signed_bits(std::uint32_t(_mm_cvttss_si32(scaled)) + std::uint32_t(base));
        current = _mm_cvtepi32_ps(_mm_cvtsi32_si128(interpolated));
    }
    auto* parent = reinterpret_cast<void*>(
        std::uintptr_t(bindings.encoded_mouse_sensitivity_cvar ^ 0x1EEEF408u));
    for (;;) {
        auto* next = fov_read<void*>(parent, 0x1C);
        if (next == parent)
            break;
        parent = next;
    }
    const auto encoded =
        fov_read<std::uint32_t>(parent, 0x2C) ^ std::uint32_t(reinterpret_cast<std::uintptr_t>(parent));
    auto ratio = _mm_div_ss(current, _mm_cvtepi32_ps(_mm_cvtsi32_si128(base)));
    ratio = _mm_mul_ss(ratio, _mm_set_ss(std::bit_cast<float>(encoded)));
    // Native reloads43468994 after both player virtuals; they may replace it.
    write_override(bindings.override_context, _mm_cvtss_f32(ratio));
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: fov_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define FOV_MEMBER __thiscall
#else
#define FOV_MEMBER
#endif
using OwnerEntry = void(FOV_MEMBER*)(void*);
void FOV_MEMBER fov_owner_destroy(void*);
void FOV_MEMBER fov_owner_unchanged(void*) {} //433CCE8E
void FOV_MEMBER fov_owner_reset(void*);
const std::array<OwnerEntry, 9> fov_owner_owner_table{fov_owner_destroy,   fov_owner_unchanged, fov_owner_unchanged, fov_owner_unchanged, fov_owner_reset,
                                            fov_owner_unchanged, fov_owner_unchanged, fov_owner_unchanged, fov_owner_unchanged}; //43471E7C
}
class FovOwner {
public:
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    ZoomSensitivityRuntime sensitivity;
    effects::ViewFovRuntime view;
    effects::FovViewObserver update;
    FovOwner(engine::OwnerRegistryBinding lifecycle, engine::OwnerRegistryBinding updates,
             ConfigRegistries& registries, FovConfig& config, FovOwnerBindings bindings)
        : memory(lifecycle.memory), sensitivity(config, bindings.sensitivity),
          view{config, bindings.entity_list, &bindings.sensitivity.local_player,
               bindings.view_offsets},
          update(sensitivity, view) {
        lifecycle.append(this);
        network::register_update_observer(updates, update.endpoint);
        vtable = fov_owner_owner_table.data();
        update.initialize_vtable();
        initialize_fov_config(registries, config);
    }
};
namespace {
void FOV_MEMBER fov_owner_destroy(void* object) { //433B1A7A tail433837D0
    if (!object)
        return;
    auto* owner = static_cast<FovOwner*>(object);
    const auto memory = owner->memory;
    owner->~FovOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void FOV_MEMBER fov_owner_reset(void* object) {
    static_cast<FovOwner*>(object)->sensitivity.reset(); //43416A53
}
#undef FOV_MEMBER
}
std::size_t fov_owner_size() noexcept {
    return sizeof(FovOwner);
}
FovOwner& construct_fov_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                              engine::OwnerRegistryBinding updates, ConfigRegistries& registries,
                              FovConfig& config, FovOwnerBindings bindings) {
    return *::new (storage) FovOwner(lifecycle, updates, registries, config, bindings);
}
ZoomSensitivityRuntime& fov_sensitivity_runtime(FovOwner& owner) noexcept {
    return owner.sensitivity;
}
effects::ViewFovRuntime& fov_view_runtime(FovOwner& owner) noexcept {
    return owner.view;
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: logs.cpp
// ============================================================================

namespace skeet::misc {
void ColoredLogRing::append_segment(ColoredLogLine& line, std::u16string_view text,
                                    std::uint32_t color) {
    if (line.text.size() + text.size() >= 512)
        return; // native checks before adding its color prefix
    if (text.size() != 1 || text.front() != u'\n') {
        line.text += u'\x07';
        for (unsigned byte = 0; byte < 4; ++byte) {
            const auto component = std::uint8_t(color >> (byte * 8));
            line.text += hex[component >> 4];
            line.text += hex[component & 15];
        }
    }
    line.text.append(text);
}
void ColoredLogRing::append(const char* utf8, std::uint32_t color) {
    if (!*enabled)
        return;
    if (count == 0) {
        if (count != 8)
            ++count;
        head = (head - 1) & 7;
        lines[head].text.clear();
        lines[head].lifetime = 8.0f;
    }
    auto* line = &lines[head & 7];
    std::array<wchar_t, 1024> wide{};
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide.data(), static_cast<int>(wide.size()));
    wide.back() = 0;
    auto* current = reinterpret_cast<const char16_t*>(wide.data());
    while (*current) {
        std::u16string_view remaining(current);
        const auto newline = remaining.find(u'\n');
        if (newline == remaining.npos) {
            append_segment(*line, remaining, color);
            line->lifetime = 8.0f;
            return;
        }
        const auto length = newline + 1;
        append_segment(*line, remaining.substr(0, length), color);
        if (!line->text.empty() && line->text.front() != u'\n') {
            if (count != 8)
                ++count;
            head = (head - 1) & 7;
            line = &lines[head];
        }
        line->text.clear();
        line->lifetime = 8.0f;
        current += length;
    }
}
void ErrorLog::write(std::string_view text) {
    format("%.*s", static_cast<int>(text.size()), text.data());
}
void ErrorLog::format(const char* format, ...) {
    std::va_list arguments;
    va_start(arguments, format);
    vformat(format, arguments);
    va_end(arguments);
}
void ErrorLog::vformat(const char* format, std::va_list arguments) {
    if (*disabled)
        return;
    std::array<char, 8192> message{};
    const auto length = vsprintf_s(message.data(), message.size(), format, arguments);
    if (length <= 0 || length >= 0x1FFF)
        return;
    message[length] = '\n';
    message[length + 1] = '\0';
    using Print = void (*)(void*, const std::uint32_t*, const char*, ...);
    const std::uint32_t prefix_color = 0xFF1DE6B5, text_color = 0xFF2828FF;
    reinterpret_cast<Print>((*static_cast<void***>(cvar))[25])(cvar, &prefix_color, "%s",
                                                               "[supremacy] ");
    reinterpret_cast<Print>((*static_cast<void***>(cvar))[25])(cvar, &text_color, "%s",
                                                               message.data());
    if (auto* hud = *hud_slot)
        hud->append(message.data(), text_color);
    const auto delta = *current_time - last_sound;
    const auto absolute = std::bit_cast<float>(std::bit_cast<std::uint32_t>(delta) & 0x7FFFFFFFu);
    if (absolute > 5.0f) {
        engine::invoke<void>(surface, 82, "ui/weapon_cant_buy.wav");
        last_sound = *current_time;
    }
}
void NormalLog::write(std::string_view text) {
    format("%.*s", static_cast<int>(text.size()), text.data());
}
void NormalLog::console_format(const char* format, ...) {
    if (*disabled)
        return;
    std::array<char, 1024> message{};
    std::va_list arguments;
    va_start(arguments, format);
    const auto count = vsnprintf(message.data(), message.size() - 2, format, arguments);
    va_end(arguments);
    if (count <= 0 || count >= std::int32_t(message.size() - 2))
        return;
    message[std::size_t(count)] = '\n';
    using Print = void (*)(void*, const char*, ...);
    reinterpret_cast<Print>((*static_cast<void***>(cvar))[26])(cvar, "[supremacy] %s",
                                                               message.data());
}
void NormalLog::format(const char* format, ...) {
    std::va_list arguments;
    va_start(arguments, format);
    vformat(format, arguments);
    va_end(arguments);
}
void NormalLog::vformat(const char* format, std::va_list arguments) {
    if (*disabled)
        return;
    std::array<char, 8192> message{};
    const auto length = vsprintf_s(message.data(), message.size(), format, arguments);
    if (length <= 0 || length >= 0x1FFF)
        return;
    message[length] = '\n';
    message[length + 1] = '\0';
    using ColorPrint = void (*)(void*, const std::uint32_t*, const char*, ...);
    using Print = void (*)(void*, const char*, ...);
    const std::uint32_t prefix_color = 0xFF1DE6B5;
    reinterpret_cast<ColorPrint>((*static_cast<void***>(cvar))[25])(cvar, &prefix_color, "%s",
                                                                    "[supremacy] ");
    reinterpret_cast<Print>((*static_cast<void***>(cvar))[26])(cvar, "%s", message.data());
    if (script && script::has_output(*script)) {
        message[length] = '\0';
        script::dispatch_output(*script, std::string_view(message.data(), length), 0xFFFFFFFFu);
    } else if (auto* hud = *hud_slot) {
        hud->append(message.data(), 0xC8FFFFFFu);
    }
}
void bind_log_configuration(const HideFromObsConfig& config, ErrorLog& error,
                            NormalLog& normal) noexcept {
    error.disabled = &config.enabled.value;
    normal.disabled = &config.enabled.value;
}
void bind_log_hud(ColoredLogRing* const& slot, ErrorLog& error, NormalLog& normal) noexcept {
    error.hud_slot = &slot;
    normal.hud_slot = &slot;
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: low_fps_runtime.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::misc {
namespace {
std::uint32_t startup_word(const void* record) {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::byte*>(record) + 4, sizeof value);
    return value;
}
#if defined(_M_IX86) || defined(__i386__)
#define LOW_FPS_MEMBER __thiscall
#else
#define LOW_FPS_MEMBER
#endif
using LifecycleEntry = void(LOW_FPS_MEMBER*)(void*);
void LOW_FPS_MEMBER low_fps_runtime_destroy_owner(void*);
void LOW_FPS_MEMBER low_fps_runtime_bind_owner(void*);
void LOW_FPS_MEMBER low_fps_runtime_reset_owner(void*);
void LOW_FPS_MEMBER low_fps_runtime_unchanged_owner(void*) {} //433CCE8E is RET.
const std::array<LifecycleEntry, 9> low_fps_runtime_lifecycle_table{
    low_fps_runtime_destroy_owner, low_fps_runtime_bind_owner,      low_fps_runtime_unchanged_owner, low_fps_runtime_unchanged_owner, low_fps_runtime_unchanged_owner,
    low_fps_runtime_reset_owner,   low_fps_runtime_unchanged_owner, low_fps_runtime_reset_owner,     low_fps_runtime_unchanged_owner};

void* low_fps_runtime_allocate_owner(engine::JobMemory memory, std::size_t size) { //43387448
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    const auto allocation_size = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void low_fps_runtime_free_owner(engine::JobMemory memory, void* object) { //433837D0
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}

class LowFpsLifecycleOwner {
public:
    const LifecycleEntry* vtable = nullptr;
    engine::JobMemory memory;
    LowFpsRuntime** active;
    LowFpsRuntime runtime;
    client::NativeCommandPhaseReceiver command_receiver{};
    alignas(hud::PerformanceHudHandler)
        std::array<std::byte, sizeof(hud::PerformanceHudHandler)> hud_storage;

    LowFpsLifecycleOwner(engine::OwnerRegistryBinding lifecycle,
                         client::CommandPhaseRegistry& commands,
                         engine::OwnerRegistryBinding hud_owners, ConfigRegistries& registry,
                         rage::PerformanceSettings& settings, const rage::GlobalSettings& rage,
                         LowFpsBindings bindings, script::Bridge& scripts, const InputBitmap& input,
                         const esp::IndicatorLayout& layout, const std::uint32_t& life_offset,
                         LowFpsRuntime*& current)
        : memory(lifecycle.memory), active(&current), runtime(settings, rage, bindings) {
        lifecycle.append(this); //4347E6FF ->433676A0
        commands.append(command_receiver, &runtime); //4347E70C ->43345EBC, secondary+8
        auto* const callback = ::new (hud_storage.data()) hud::PerformanceHudHandler(
            make_low_fps_hud_binding(runtime, scripts, input, layout, life_offset));
        hud::register_hud_frame(hud_owners, callback); //4347E719 ->433D20B5
        vtable = low_fps_runtime_lifecycle_table.data(); //4347E721, table4346EF74
        commands.publish(command_receiver, client::CommandOwnerId::owner_43472148); //4347E72A
        callback->publish(); //4347E730, table4346F010
        skeet::rage::initialize_performance_settings(registry, settings);
        runtime.initialize();
    }
    ~LowFpsLifecycleOwner() {
        std::destroy_at(
            std::launder(reinterpret_cast<hud::PerformanceHudHandler*>(hud_storage.data())));
    }
};

namespace {
void LOW_FPS_MEMBER low_fps_runtime_destroy_owner(void* object) { //433B1A7A ->433837D0
    auto* owner = static_cast<LowFpsLifecycleOwner*>(object);
    const auto memory = owner->memory;
    *owner->active = nullptr;
    owner->~LowFpsLifecycleOwner();
    low_fps_runtime_free_owner(memory, owner);
}
void LOW_FPS_MEMBER low_fps_runtime_bind_owner(void* object) {
    static_cast<LowFpsLifecycleOwner*>(object)->runtime.bind_dependencies();
}
void LOW_FPS_MEMBER low_fps_runtime_reset_owner(void* object) {
    static_cast<LowFpsLifecycleOwner*>(object)->runtime.reset();
}
#undef LOW_FPS_MEMBER
}

LowFpsClock native_low_fps_clock() {
    const auto library = GetModuleHandleW(L"ntdll.dll");
    LowFpsClock result{
        reinterpret_cast<LowFpsClockQuery>(GetProcAddress(library, "RtlQueryPerformanceCounter")),
        reinterpret_cast<LowFpsClockQuery>(
            GetProcAddress(library, "RtlQueryPerformanceFrequency"))};
    if (!result.counter || !result.frequency)
        throw std::logic_error("Low FPS requires the native ntdll performance clock");
    return result;
}

LowFpsRuntime::LowFpsRuntime(rage::PerformanceSettings& settings,
                             const rage::GlobalSettings& global, LowFpsBindings engine)
    : config(settings), rage(global), bindings(engine) {}

void LowFpsRuntime::initialize() {
    state.level = 0.0f;
    state.rage_enabled = nullptr;
    state.low_fps = 0;
    std::int64_t frequency = 0;
    bindings.clock.frequency(&frequency); //native return value is ignored
    state.counter_frequency = static_cast<double>(frequency); //433EC47E, signed64
}

void LowFpsRuntime::bind_dependencies() {
    //4347E0FB resolves23060EC5 and takes its+10 value. This is that same live
    //descriptor, supplied by the typed aggregate instead of a second lookup.
    state.rage_enabled = &rage.enabled.value;
    if (bindings.startup_first)
        state.startup_first_word = startup_word(bindings.startup_first);
    if (bindings.startup_second)
        state.startup_second_word = startup_word(bindings.startup_second);
}

void LowFpsRuntime::reset() noexcept {
    state.level = 0.0f;
    state.frame_number = -1;
    state.low_fps = 0;
    state.accumulated_seconds = 0.0;
}

bool LowFpsRuntime::active(InputBitmap input) const {
    return config.mitigations.value != 0 && *state.rage_enabled != 0 &&
           hotkey_active(rage.activation.value.binding, input);
}

void LowFpsRuntime::phase1(InputBitmap input) {
    if (!active(input))
        return;
    std::int64_t counter = 0;
    bindings.clock.counter(&counter);
    state.started_counter = static_cast<std::uint64_t>(counter);
}

void LowFpsRuntime::phase4(client::CommandFrame& frame) {
    std::uint32_t mitigations = 0;
    if (state.level > 0.10000000149011612f) {
        const auto selected = static_cast<std::uint32_t>(config.mitigations.value);
        if (selected & 0x80u) {
            frame.flags |= 0x20000u;
            mitigations = 0x80u;
        }
        mitigations |= selected & 1u;
        if (state.level > 0.30000001192092896f)
            mitigations |= selected & 0x1eu;
    }
    frame.field_1a0 = mitigations;
    state.low_fps = mitigations != 0;
}

void LowFpsRuntime::phase7(const client::CommandFrame& frame, InputBitmap input) {
    if (!active(input)) {
        reset();
        return;
    }
    const auto x = frame.origin_10c[0] - state.previous_origin[0];
    const auto y = frame.origin_10c[1] - state.previous_origin[1];
    const auto z = frame.origin_10c[2] - state.previous_origin[2];
    if ((y * y + x * x) + z * z > 64.0f) {
        const auto decay = bindings.tick_interval * 0.33000001311302185f;
        state.level = maximum_number(state.level - decay, 0.0f);
    }
    if (bindings.frame_number != state.frame_number) {
        state.accumulated_seconds = 0.0;
        state.frame_number = bindings.frame_number;
    }
    std::int64_t counter = 0;
    bindings.clock.counter(&counter);
    const auto elapsed = static_cast<std::uint64_t>(counter) - state.started_counter;
    state.accumulated_seconds =
        static_cast<double>(elapsed) / state.counter_frequency +
        state.accumulated_seconds; //433EC43E converts the wrapped unsigned64 delta
    if (state.accumulated_seconds >= static_cast<double>(bindings.tick_interval)) {
        state.level = minimum_number(state.level + 0.33000001311302185f, 1.0f);
        state.previous_origin = frame.origin_10c;
    }
}

hud::PerformanceHudBinding make_low_fps_hud_binding(LowFpsRuntime& runtime, script::Bridge& scripts,
                                                    const InputBitmap& input,
                                                    const esp::IndicatorLayout& layout,
                                                    const std::uint32_t& life_offset) {
    return {scripts,
            runtime.config,
            runtime.state.low_fps,
            runtime.state.rage_enabled,
            runtime.rage,
            input,
            layout,
            life_offset};
}

LowFpsLifecycleOwner* initialize_low_fps_owner(
    engine::OwnerRegistryBinding lifecycle, client::CommandPhaseRegistry& commands,
    engine::OwnerRegistryBinding hud_owners, ConfigRegistries& registry,
    rage::PerformanceSettings& settings, const rage::GlobalSettings& rage, LowFpsBindings bindings,
    script::Bridge& scripts, const InputBitmap& input, const esp::IndicatorLayout& layout,
    const std::uint32_t& life_offset, LowFpsRuntime*& active) {
    auto* storage =
        low_fps_runtime_allocate_owner(lifecycle.memory, std::max(std::size_t(0x78), sizeof(LowFpsLifecycleOwner)));
    if (!storage) {
        // The original allocation-failure edge reaches unresolved INT1; source allocation failure terminates this factory with bad_alloc.
        throw std::bad_alloc{};
    }
    auto* owner = ::new (storage)
        LowFpsLifecycleOwner(lifecycle, commands, hud_owners, registry, settings, rage, bindings,
                             scripts, input, layout, life_offset, active);
    active = &owner->runtime;
    return owner;
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: native_text.cpp
// ============================================================================

namespace skeet::misc {
namespace {
char empty_text[2]{}; //native434759AC/AD sentinel
void* native_text_allocate(engine::JobMemory memory, std::uint32_t length) { //43387448
    if (!length)
        return nullptr;
    const auto allocation_size = length + 0x20u;
    auto* raw = memory.allocate(memory.process_heap, 8, allocation_size);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void free(engine::JobMemory memory, void* value) { //433837D0
    auto* bytes = static_cast<std::byte*>(value);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}
void construct_native_text(NativeText& text, const char* value, engine::JobMemory memory) {
    text.begin = nullptr;
    text.end = nullptr;
    text.limit = nullptr;
    const char* end = value;
    while (*end)
        ++end;
    construct_native_text_range(text, value, end, memory);
}
void construct_native_text_range(NativeText& text, const char* first, const char* last,
                                 engine::JobMemory memory) {
    const auto length = std::uint32_t(reinterpret_cast<std::uintptr_t>(last) -
                                      reinterpret_cast<std::uintptr_t>(first));
    const auto capacity = length + 1u;
    auto* begin = capacity < 2 ? empty_text : static_cast<char*>(native_text_allocate(memory, capacity));
    text.begin = begin;
    text.end = begin;
    text.limit = capacity < 2 ? empty_text + 1 : begin + capacity;
    std::memmove(begin, first, length);
    text.end = begin + length;
    begin[length] = 0;
}
void destroy_native_text(NativeText& text, engine::JobMemory memory) {
    const auto capacity =
        std::bit_cast<std::int32_t>(std::uint32_t(reinterpret_cast<std::uintptr_t>(text.limit)) -
                                    std::uint32_t(reinterpret_cast<std::uintptr_t>(text.begin)));
    if (capacity >= 2 && text.begin)
        free(memory, text.begin);
}
void construct_native_encrypted_text(NativeEncryptedText& text, const char* value,
                                     std::uint32_t length, engine::JobMemory memory) {
    text.bytes = static_cast<char*>(native_text_allocate(memory, length + 1u));
    text.bytes[length] = 0;
    text.key = 0;
    std::uint8_t key;
    do
        key = std::uint8_t(0u - std::uint32_t(__rdtsc() & 0x7Fu));
    while (!key);
    text.key = key;
    for (std::uint32_t i = 0; i < length; ++i)
        text.bytes[i] = char(std::uint8_t(value[i]) ^ key);
}
void destroy_native_encrypted_text(NativeEncryptedText& text, engine::JobMemory memory) {
    if (!text.bytes)
        return;
    free(memory, text.bytes);
    text.bytes = nullptr;
}
void decode_native_text(const NativeEncryptedText& text, char* destination) noexcept {
    auto* encoded = reinterpret_cast<const std::uint8_t*>(text.bytes);
    while (*encoded)
        *destination++ = char(*encoded++ ^ text.key);
    *destination = 0;
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: network_console_handlers.cpp
// ============================================================================

namespace skeet::misc {
namespace {
NetworkMessageRuntime* active_network_message_runtime = nullptr;
template <class T> T read(const void* object, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}
template <class T> void network_console_handlers_write(void* object, std::ptrdiff_t offset, T value) { std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(T)); }
// deduplicated add
std::int32_t truncate(float value) noexcept {
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}
bool contains_payload(const std::byte* source, std::uint32_t length, const std::byte* pattern,
                      std::uint32_t pattern_length) {
    // 433DF260 compares each candidate backwards. Both native callers use a
    // positive pattern length (4 or13), and retain the explicit null guards.
    if (!source || length < pattern_length || !pattern)
        return false;
    const auto maximum = std::numeric_limits<std::uintptr_t>::max() -
                         reinterpret_cast<std::uintptr_t>(source) - pattern_length;
    const auto bounded = std::min<std::uintptr_t>(length, maximum);
    if (bounded < pattern_length)
        return false;
    for (std::uintptr_t offset = 0; offset <= bounded - pattern_length; ++offset) {
        auto index = pattern_length;
        while (index && source[offset + index - 1] == pattern[index - 1])
            --index;
        if (!index)
            return true;
    }
    return false;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall disable_pure_entry(void* endpoint, NetworkMessageCall* call) {
#else
void disable_pure_entry(void* endpoint, NetworkMessageCall* call) {
#endif
    apply_disable_sv_pure(static_cast<DisableSvPureHandler*>(endpoint)->config, *call);
}
const std::array<NetworkMessageEntry, 1> disable_pure_table{disable_pure_entry};
#if defined(_M_IX86) || defined(__i386__)
void __fastcall console_output_entry(void* endpoint, ui::RenderQueue* queue, void*, std::int32_t*) {
#else
void console_output_entry(void* endpoint, ui::RenderQueue* queue, void*, std::int32_t*) {
#endif
    auto& handler = *static_cast<ConsoleOutputHandler*>(endpoint);
    draw_console_output(*queue, handler.config, handler.ring, handler.frame_time);
}
const std::array<hud::HudFrameEntry, 1> console_output_table{console_output_entry};
}

DisableSvPureHandler::DisableSvPureHandler(const DisablePureConfig& canonical) noexcept
    : NetworkMessageCallback{nullptr}, config(canonical) {}
DisableSvPureHandler::DisableSvPureHandler(engine::OwnerRegistryBinding callbacks,
                                           const DisablePureConfig& canonical)
    : DisableSvPureHandler(canonical) {
    callbacks.append(this); //4336F73C precedes the secondary-vtable write.
    initialize_vtable();
}
void DisableSvPureHandler::initialize_vtable() noexcept {
    vtable = disable_pure_table.data();
}
void apply_disable_sv_pure(const DisablePureConfig& config, NetworkMessageCall& call) {
    if (config.enabled.value && engine::invoke<std::int32_t>(call.message, 7) == 14)
        call.cancelled = 1;
}
void update_network_message_transition(NetworkMessageTransition& state, void* message) {
    if (!state.active)
        return;
    if (engine::invoke<std::int32_t>(message, 7) == 4) {
        if (read<std::int32_t>(message, 0xC) == *state.sequence) {
            network_console_handlers_write<std::int32_t>(message, 0xC, -1);
            state.active = 0;
            state.mismatched = 0;
            state.status = 1;
            state.acknowledged_at = *state.current_time;
            return;
        }
        state.mismatched = 1;
    }
    if (state.active && state.mismatched && engine::invoke<std::int32_t>(message, 7) == 9) {
        if (state.reset)
            state.reset();
        state.active = 0;
        state.mismatched = 0;
        state.status = 0;
    }
}
bool send_network_message(NetworkMessageRuntime& runtime, void* channel, void* message,
                          std::uint8_t force_reliable, std::uint8_t voice) {
    update_network_message_transition(runtime.transition, message);
    if (engine::invoke<std::int32_t>(message, 7) == 23 &&
        (!(read<std::uint8_t>(message, 0x1C) & 1) ||
         (read<std::int32_t>(message, 0x10) != 0x23 && read<std::int32_t>(message, 0x10) != 0x44)))
        return true;
    if (engine::invoke<std::int32_t>(message, 7) == 18 && (read<std::uint8_t>(message, 0x14) & 1)) {
        const auto* string = read<const void*>(message, 0xC);
        const auto size = read<std::uint32_t>(string, 0x10);
        if (size) {
            const auto* bytes = read<std::uint32_t>(string, 0x14) < 16
                                    ? static_cast<const std::byte*>(string)
                                    : read<const std::byte*>(string, 0);
            const auto module = runtime.client_module;
            const auto header = module ? reinterpret_cast<std::uintptr_t>(module) +
                                             read<std::uint32_t>(module, 0x3C)
                                       : 0;
            if (contains_payload(bytes, size, reinterpret_cast<const std::byte*>(header + 8), 4) ||
                contains_payload(bytes, size, reinterpret_cast<const std::byte*>("UserExtraData"),
                                 13))
                return true;
        }
    }
    NetworkMessageCall call{channel, message, &force_reliable, &voice, 0};
    const auto callbacks = runtime.callbacks.entries();
    for (auto* object : callbacks) {
        auto* callback = static_cast<NetworkMessageCallback*>(object);
        callback->vtable[0](object, &call);
        if (call.cancelled)
            return true;
    }
    const auto table = *static_cast<void***>(channel);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    using Send = engine::MemberFunction<bool, void*, std::uint8_t, std::uint8_t>;
    return reinterpret_cast<Send>(metadata->original_table[40])(channel, message, force_reliable,
                                                                voice);
}
void bind_network_message_runtime(NetworkMessageRuntime& runtime) noexcept {
    active_network_message_runtime = &runtime;
}
#if defined(_M_IX86) || defined(__i386__)
bool __fastcall send_network_message_hook(void* channel, void*, void* message,
                                          std::uint8_t force_reliable, std::uint8_t voice) {
#else
bool send_network_message_hook(void* channel, void*, void* message, std::uint8_t force_reliable,
                               std::uint8_t voice) {
#endif
    return send_network_message(*active_network_message_runtime, channel, message, force_reliable,
                                voice);
}

ConsoleOutputHandler::ConsoleOutputHandler(const ConsoleOutputConfig& canonical,
                                           ColoredLogRing& shared_ring, const float& delta) noexcept
    : config(canonical), ring(shared_ring), frame_time(delta) {}
ConsoleOutputHandler::ConsoleOutputHandler(engine::OwnerRegistryBinding callbacks,
                                           const ConsoleOutputConfig& canonical,
                                           ColoredLogRing& shared_ring, const float& delta)
    : ConsoleOutputHandler(canonical, shared_ring, delta) {
    hud::register_hud_frame(callbacks, this);
    initialize_vtable();
}
void ConsoleOutputHandler::initialize_vtable() noexcept {
    vtable = console_output_table.data();
    ring.enabled = &config.enabled.value;
}

void draw_console_output(ui::RenderQueue& queue, const ConsoleOutputConfig& config,
                         ColoredLogRing& ring, float frame_time) {
    if (!config.enabled.value) {
        ring.count = 0;
        ring.head = 0;
        return;
    }
    while (ring.count && ring.lines[(ring.head - 1 + ring.count) & 7].lifetime <= 0.0f)
        --ring.count;
    if (!queue.fonts)
        throw std::logic_error("Console output requires the font backend");
    const auto step = add(ui::font_height(*queue.fonts, 1), 1);
    std::int32_t y = 5;
    for (auto index = static_cast<std::int32_t>(ring.count) - 1; index >= 0; --index) {
        auto& line = ring.lines[(ring.head + std::uint32_t(index)) & 7];
        line.lifetime -= frame_time;
        std::uint32_t color = 0xFFFFFFFFu;
        if (0.5f > line.lifetime) {
            // Native MAXSS/MINSS select their second operand when unordered.
            auto fade = line.lifetime > 0.0f ? line.lifetime : 0.0f;
            fade = fade < 0.5f ? fade : 0.5f;
            fade *= 2.0f;
            color = (std::uint32_t(truncate(fade * 255.0f)) << 24) | 0x00FFFFFFu;
            if (index == 0 && fade < 0.2f)
                y = truncate(static_cast<float>(y) -
                             (1.0f - fade / 0.2f) * static_cast<float>(step));
        }
        queue.text(8, y, color, 9, 0, line.text);
        y = add(y, step);
    }
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: network_fix.cpp
// ============================================================================

namespace skeet::misc {
namespace {
const NetworkFixBindings* active_network_fix;
const NetworkFixProxyRuntime* active_proxy_runtime;
struct ProxyThreadState {
    const NetworkFixProxyRuntime* owner = nullptr;
    std::uint32_t continuation = 0;
    NetworkFixProxyTransfer transfer{};
};
thread_local ProxyThreadState proxy_thread;
// deduplicated field
template <class T> void put(void* object, std::ptrdiff_t offset, const T& value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
}
extern "C" const NetworkFixProxyTransfer* __cdecl skeet_network_fix_proxy_prepare() {
    if (!proxy_thread.owner)
        throw std::logic_error("Network-fix return proxy has no recorded source owner");
    return proxy_thread.owner->prepare_proxy_transfer();
}
extern "C" std::uint32_t __cdecl skeet_network_fix_proxy_guard(std::uintptr_t outer_return_site) {
    if (!active_proxy_runtime)
        throw std::logic_error("Network-fix singlestep guard has no source owner");
    return active_proxy_runtime->finish_singlestep(outer_return_site) ? 1u : 0u;
}
#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
extern "C" __attribute__((naked)) void skeet_network_fix_proxy_entry() {
    __asm__("call _skeet_network_fix_proxy_prepare\n\t"
            "pushl (%eax)\n\t"
            "movl 8(%eax),%ecx\n\t"
            "movl 4(%eax),%eax\n\t"
            "testl %eax,%eax\n\t"
            "ret");
}
extern "C" __attribute__((naked)) void skeet_network_fix_singlestep_entry() {
    __asm__("pushl 4(%esp)\n\t"
            "call _skeet_network_fix_proxy_guard\n\t"
            "addl $4,%esp\n\t"
            "testl %eax,%eax\n\t"
            "jz 1f\n\t"
            "popl %eax\n\t"
            "xorl %eax,%eax\n\t"
            "1: ret");
}
#elif defined(_M_IX86) && defined(_MSC_VER)
extern "C" __declspec(naked) void skeet_network_fix_proxy_entry() {
    __asm {
        call skeet_network_fix_proxy_prepare
        push dword ptr [eax]
        mov ecx,dword ptr [eax+8]
        mov eax,dword ptr [eax+4]
        test eax,eax
        ret
    }
}
extern "C" __declspec(naked) void skeet_network_fix_singlestep_entry() {
    __asm {
        push dword ptr [esp+4]
        call skeet_network_fix_proxy_guard
        add esp,4
        test eax,eax
        jz unchanged_return
        pop eax
        xor eax,eax
    unchanged_return:
        ret
    }
}
#endif

void NetworkFixProxyRuntime::record_original_proxy(std::uint32_t raw_entry) const {
    // MOVD XMM0 followed by MOVAPS XMM14 only preserves these32 raw bits;
    // the other96 bits are zero and the getter consumes the low word alone.
    proxy_thread.owner = this;
    proxy_thread.continuation = raw_entry;
}
void* NetworkFixProxyRuntime::proxy_entry() const {
#if defined(_M_IX86) || defined(__i386__)
    return reinterpret_cast<void*>(&skeet_network_fix_proxy_entry);
#else
    throw std::logic_error("Network-fix return transfers require their source x86 ABI");
#endif
}
const NetworkFixProxyTransfer* NetworkFixProxyRuntime::prepare_proxy_transfer() const {
#if defined(_M_IX86) || defined(__i386__)
    static_assert(sizeof(void*) == 4 && sizeof(NetworkFixProxyTransfer) == 12);
    if (proxy_thread.owner != this || !proxy_thread.continuation)
        throw std::logic_error("Network-fix return proxy requires its recorded continuation");
    const auto continuation =
        proxy_thread.continuation; //the native getter runs before the engine call
    const auto process = bindings_.process;
    if (!process)
        throw std::logic_error("Network-fix engine entry has not been initialized");
    process(0); //433D6D38, CL0 before any clone lookup/publication
    if (!active_clone_) {
        if (active_proxy_runtime && active_proxy_runtime != this)
            throw std::logic_error(
                "Network-fix singlestep clone already belongs to another source owner");
        auto* variable = reinterpret_cast<std::byte*>(
            std::uintptr_t(bindings_.encoded_singlestep ^ 0x66e946dbu));
        active_clone_ = clone_.data(); //43476410 publication precedes the native memcpy
        std::memcpy(clone_.data(), variable, clone_.size());
        variable = reinterpret_cast<std::byte*>(
            std::uintptr_t(bindings_.encoded_singlestep ^ 0x66e946dbu));
        put(active_clone_, 0x1c, field<void*>(variable, 0x1c));
        variable = reinterpret_cast<std::byte*>(
            std::uintptr_t(bindings_.encoded_singlestep ^ 0x66e946dbu));
        std::memcpy(table_.data(), field<const std::uintptr_t*>(variable, 0) - 1, sizeof table_);
        put(active_clone_, 0, table_.data() + 1); //one RTTI word precedes the21-entry table
        table_[14] =
            reinterpret_cast<std::uintptr_t>(&skeet_network_fix_singlestep_entry); //virtual13
        active_proxy_runtime = this;
    }
    auto* variable =
        reinterpret_cast<std::byte*>(std::uintptr_t(bindings_.encoded_singlestep ^ 0x66e946dbu));
    put(variable, 0x1c, active_clone_);
    proxy_thread.transfer = {
        continuation, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(variable)),
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(active_clone_))};
    return &proxy_thread.transfer;
#else
    throw std::logic_error("Network-fix return transfers require their source x86 ABI");
#endif
}
bool NetworkFixProxyRuntime::finish_singlestep(std::uintptr_t outer_return_site) const {
    if (outer_return_site != bindings_.guarded_return_site)
        return false;
    if (!active_clone_)
        throw std::logic_error("Network-fix singlestep guard requires its initialized clone");
    auto* variable =
        reinterpret_cast<std::byte*>(std::uintptr_t(bindings_.encoded_singlestep ^ 0x66e946dbu));
    put(variable, 0x1c, field<void*>(active_clone_, 0x1c));
    return true;
}
std::uintptr_t process_network_fix(void* receiver, void* supplied_object, std::uintptr_t caller,
                                   const NetworkFixBindings& bindings) {
    if (caller == bindings.proxy_call_site && bindings.proxy_context && bindings.enabled) {
        if (!engine::invoke<bool>(bindings.engine, 82) &&
            !engine::invoke<bool>(bindings.engine, 93) &&
            !engine::invoke<bool>(bindings.engine, 197)) {
            //The veneer supplies the caller's EBP. Its first word is the
            //previous frame, whose+4 slot contains the original continuation.
            auto* const previous_frame = field<std::byte*>(supplied_object, 0);
            const auto previous = field<std::uint32_t>(previous_frame, 4);
            bindings.proxy_runtime.record_original_proxy(previous);
            put(previous_frame, 4, bindings.proxy_runtime.proxy_entry());
        }
    } else if (caller == bindings.sequence_call_site) {
        const auto& offsets = bindings.offsets;
        if (!field<std::uint8_t>(receiver, offsets.byte_9d920e35) &&
            _mm_comige_ss(_mm_set_ss(field<float>(receiver, offsets.scalar_7e9ba7bc)),
                          _mm_set_ss(580.0f)) &&
            !field<std::uint8_t>(receiver, offsets.byte_ac7f2913)) {
            const auto* command = field<void*>(receiver, offsets.pointer_f58e6118);
            if (command) {
                const auto number = field<std::uint32_t>(command, 4);
                client::prediction_history_storage(*bindings.sequence_state)
                    .landing_tags[number % 150u] = number;
            }
        }
    }
    const auto table = *static_cast<void***>(receiver);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    return reinterpret_cast<engine::MemberFunction<std::uintptr_t>>(metadata->original_table[268])(
        receiver);
}
void bind_network_fix(const NetworkFixBindings& bindings) noexcept {
    active_network_fix = &bindings;
}
#if defined(_M_IX86) || defined(__i386__)
std::uintptr_t __fastcall network_fix_hook(void* receiver, void* supplied_object) {
#else
std::uintptr_t network_fix_hook(void* receiver, void* supplied_object) {
#endif
    return process_network_fix(receiver, supplied_object,
                               reinterpret_cast<std::uintptr_t>(_ReturnAddress()),
                               *active_network_fix);
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: no_fall_damage.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// deduplicated field
float convar(const void* object) noexcept {
    for (;;) {
        const auto* next = field<const void*>(object, 0x1c);
        if (next == object)
            break;
        object = next;
    }
    return std::bit_cast<float>(field<std::uint32_t>(object, 0x2c) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
MovementVector origin(void* player) {
    return *engine::invoke<const MovementVector*>(player, 10);
}
MovementCollisionTrace empty_trace() noexcept {
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = -1;
    trace.hitbox = -1;
    return trace;
}
MovementCollisionTrace trace_down(const NoFallDamageBindings& b, void* player,
                                  MovementVector position) {
    const auto maximum = *engine::invoke<const MovementVector*>(player, 77);
    const auto minimum = *engine::invoke<const MovementVector*>(player, 76);
    const MovementVector end{position.x, position.y, position.z - 1000.0f};
    auto trace = empty_trace();
    b.collision.trace_ray(
        make_movement_hull_ray(position, end, minimum, {maximum.x, maximum.y, 54.0f}), 1u,
        {MovementCollisionFilterKind::reject_entities, 0, 0, 0}, trace);
    return trace;
}
bool acceptable_floor(const MovementCollisionTrace& trace) noexcept {
    return trace.fraction < 1.0f && !trace.start_solid && !trace.all_solid &&
           trace.plane_normal.z >= 0.699999988079071f;
}
void accelerate_forecast(const NoFallDamageRuntime& runtime, MovementVector& velocity, float yaw,
                         bool& alternating) {
    const auto speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    float forward = 450.0f, side = 0.0f, heading = yaw;
    if (speed != 0.0f) {
        forward = 0.0f;
        const auto change = 845.5f / speed;
        if (alternating) {
            heading = yaw - change;
            side = -450.0f;
        } else {
            heading = change + yaw;
            side = 450.0f;
        }
        alternating = !alternating;
    }
    MovementVector axis, right;
    movement_angle_vectors({0.0f, heading, 0.0f}, &axis, &right, nullptr);
    MovementVector direction{right.x * side + axis.x * forward, right.y * side + axis.y * forward,
                             0.0f};
    auto wish_speed = std::sqrt((direction.x * direction.x + direction.y * direction.y) +
                                direction.z * direction.z);
    const auto scale = 1.0f / (wish_speed + std::numeric_limits<float>::epsilon());
    direction.x *= scale;
    direction.y *= scale;
    direction.z *= scale;
    if (wish_speed != 0.0f && wish_speed > 250.0f)
        wish_speed = 250.0f;
    const auto additional =
        minimum_number(30.0f, wish_speed) -
        ((velocity.x * direction.x + velocity.y * direction.y) + velocity.z * direction.z);
    if (additional > 0.0f) {
        const auto amount =
            minimum_number(additional, (convar(runtime.bindings.air_accelerate) * wish_speed) *
                                           runtime.bindings.interval);
        velocity.x = direction.x * amount + velocity.x;
        velocity.y = direction.y * amount + velocity.y;
    }
}
bool clear_landing_segment(const NoFallDamageBindings& b, void* player, MovementVector first,
                           MovementVector last) {
    auto trace = empty_trace();
    b.collision.trace_ray(
        make_movement_hull_ray(first, last, {-16.0f, -16.0f, 0.0f}, {16.0f, 16.0f, 54.0f}), 1u,
        {MovementCollisionFilterKind::skip_player,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(player)), 0, 0},
        trace);
    return !trace.start_solid && !trace.all_solid && trace.fraction >= 0.8999999761581421f;
}
// Both complete ballistic loops in433953A3, followed by their two clearance
// traces. The caller passes velocity after the first half-gravity update.
bool landing_forecast(const NoFallDamageRuntime& runtime, void* player, MovementVector position,
                      MovementVector velocity, float floor, float yaw) {
    const auto& b = runtime.bindings;
    const auto half_gravity = (convar(b.gravity) * b.interval) * 0.5f;
    position.z += 9.0f;
    auto apex = position;
    MovementVector steep_position{};
    bool has_steep_position = false, alternating = false;
    const auto threshold = floor + 11.0f;
    if (position.z > threshold) {
        do {
            velocity.z -= half_gravity;
            if (*runtime.air_strafe_enabled)
                accelerate_forecast(runtime, velocity, yaw, alternating);
            position.x = b.interval * velocity.x + position.x;
            position.y = b.interval * velocity.y + position.y;
            position.z = position.z + b.interval * velocity.z;
            if (position.z > apex.z)
                apex = position;
            if (!has_steep_position && velocity.z < 0.0f &&
                -velocity.z > std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y)) {
                steep_position = position;
                has_steep_position = true;
            }
            const auto again = position.z > threshold;
            velocity.z -= half_gravity;
            if (!again)
                break;
        } while (true);
    }
    if (velocity.z > -580.0f || 9.0f > position.z - floor)
        return false;
    MovementTraceFlagGuard guard(b.movement_environment.engine_trace_flag);
    if (has_steep_position) {
        if (!clear_landing_segment(b, player, apex, steep_position))
            return false;
        apex = steep_position;
    }
    return clear_landing_segment(b, player, apex, position);
}

struct Vertex {
    float x, y;
    std::uint32_t color;
};
void triangles(ui::RenderQueue& queue, std::span<const Vertex> vertices) {
    const auto count = static_cast<std::uint32_t>(vertices.size());
    auto* payload = queue.append(7, count * sizeof(Vertex) + 16);
    std::memcpy(payload, &count, 4);
    std::memcpy(payload + 4, vertices.data(), vertices.size_bytes());
}
bool disc_project(const esp::Projection& p, MovementVector v, Vertex& output) noexcept {
    const auto& m = p.matrix;
    const auto w = ((v.y * m[13] + v.x * m[12]) + v.z * m[14]) + m[15];
    if (0.001f > w)
        return false;
    const auto inverse = 1.0f / w;
    const auto x = ((v.y * m[1] + v.x * m[0]) + v.z * m[2]) + m[3];
    const auto y = ((v.y * m[5] + v.x * m[4]) + v.z * m[6]) + m[7];
    output.x = (x * inverse) * p.horizontal_scale + p.horizontal_scale;
    output.y = p.vertical_scale - (y * inverse) * p.vertical_scale;
    return true;
}
//4337DCA0/43397246, fixed pitch-90/radius24 used by this owner.
void draw_disc(ui::RenderQueue& queue, const esp::Projection& projection, MovementVector position,
               std::uint32_t color) {
    MovementVector first, second;
    movement_angle_vectors({-90.0f, 0.0f, 0.0f}, nullptr, &second, &first);
    second = {-second.x, -second.y, -second.z};
    auto previous = MovementVector{first.x * 24.0f + position.x, first.y * 24.0f + position.y,
                                   first.z * 24.0f + position.z};
    Vertex center{0, 0, color};
    if (!disc_project(projection, position, center))
        return;
    std::array<Vertex, 48> vertices;
    for (int i = 1; i <= 16; ++i) {
        const auto angle = float(i) * 0.3926990926265716552734375f;
        const auto sine = std::sin(angle), cosine = std::cos(angle);
        const MovementVector next{
            ((first.x * cosine) * 24.0f + position.x) + (second.x * sine) * 24.0f,
            ((first.y * cosine) * 24.0f + position.y) + (second.y * sine) * 24.0f,
            ((first.z * cosine) * 24.0f + position.z) + (second.z * sine) * 24.0f};
        auto* triangle = vertices.data() + 3 * (i - 1);
        triangle[0] = center;
        triangle[1].color = color & 0x00ffffffu;
        triangle[2].color = color & 0x00ffffffu;
        if (!disc_project(projection, previous, triangle[1]) ||
            !disc_project(projection, next, triangle[2]))
            return;
        previous = next;
    }
    triangles(queue, vertices);
}
//Complete43394A38. Seven world points, six triangles, black/alpha40.
void draw_arrow(ui::RenderQueue& queue, const esp::Projection& projection, MovementVector start,
                MovementVector end) {
    const MovementVector delta{end.x - start.x, end.y - start.y, end.z - start.z};
    const auto scale =
        1.0f / (std::sqrt((delta.x * delta.x + delta.y * delta.y) + delta.z * delta.z) +
                std::numeric_limits<float>::epsilon());
    const MovementVector direction{delta.x * scale, delta.y * scale, delta.z * scale};
    const MovementVector across{direction.y - direction.z * 0.0f, direction.z * 0.0f - direction.x,
                                direction.x * 0.0f - direction.y * 0.0f};
    constexpr float width = 6.0f;
    const auto half = static_cast<float>(double(width) * 0.5);
    const MovementVector neck{end.x - direction.x * width, end.y - direction.y * width,
                              end.z - direction.z * width};
    const std::array<MovementVector, 7> points{
        {{start.x - across.x * half, start.y - across.y * half, start.z - across.z * half},
         {neck.x - across.x * half, neck.y - across.y * half, neck.z - across.z * half},
         {neck.x - across.x * width, neck.y - across.y * width, neck.z - across.z * width},
         end,
         {neck.x + across.x * width, neck.y + across.y * width, neck.z + across.z * width},
         {neck.x + across.x * half, neck.y + across.y * half, neck.z + across.z * half},
         {start.x + across.x * half, start.y + across.y * half, start.z + across.z * half}}};
    for (const auto& point : points) {
        Point screen;
        if (!esp::project_point(projection, {point.x, point.y, point.z}, screen))
            return;
    }
    constexpr std::array<unsigned, 18> order{4, 3, 2, 0, 6, 5, 5, 1, 0, 2, 3, 4, 5, 6, 0, 0, 1, 5};
    std::array<Vertex, 18> vertices;
    for (std::size_t i = 0; i < order.size(); ++i) {
        const auto point = points[order[i]];
        Point screen;
        if (!esp::project_point(projection, {point.x, point.y, point.z}, screen))
            return;
        vertices[i] = {float(screen.x), float(screen.y), 0x28000000u};
    }
    triangles(queue, vertices);
}
} //namespace

void initialize_no_fall_damage_config(ConfigRegistries& r, NoFallDamageConfig& config) noexcept {
    initialize_boolean_config(r.descriptors, config.enabled, 0x8E38F1F3u);
    initialize_color_config(r.descriptors, config.color, 0x6E4E5CC7u, 0x00FF8E00u);
}
void NoFallDamageRuntime::bind_configuration(ConfigLink& descriptors) noexcept {
    air_strafe_enabled = &static_cast<BooleanConfig*>(find_config(descriptors, 0xF2EB89E9u))->value;
}
void NoFallDamageRuntime::phase2(void* player, client::Command& command) {
    if (!config.enabled.value || field<std::uint8_t>(player, bindings.offsets.movement_type) != 2 ||
        (field<std::uint32_t>(player, bindings.offsets.flags) & 1u))
        return;
    auto position = origin(player);
    if (!(field<std::uint32_t>(player, bindings.offsets.flags) & 2u))
        position.z += 9.0f;
    const auto trace = trace_down(bindings, player, position);
    if (!acceptable_floor(trace))
        return;
    auto distance = trace.fraction * 1000.0f;
    auto vertical = field<MovementVector>(player, bindings.offsets.velocity).z;
    if (vertical < 0.0f && distance < 11.0f) {
        if (-580.0f > vertical && distance > 9.0f)
            command.movement.buttons = (command.movement.buttons & ~4u) | 2u;
        return;
    }
    const auto half_gravity = (bindings.interval * convar(bindings.gravity)) * 0.5f;
    static_cast<void>(engine::invoke<const MovementVector*>(player, 10));
    if (distance > 11.0f) {
        do {
            vertical -= half_gravity;
            distance += bindings.interval * vertical;
            vertical -= half_gravity;
        } while (distance > 11.0f);
    }
    if (vertical > -580.0f || !(distance >= 9.0f))
        return;
    command.movement.buttons = (command.movement.buttons | 4u) & ~2u;
}
void NoFallDamageRuntime::draw(ui::RenderQueue& queue, void* player) {
    if (!config.enabled.value || !(config.color.value & 0xff000000u) ||
        !(field<std::uint32_t>(player, bindings.offsets.flags) & 1u) ||
        field<std::uint8_t>(player, bindings.offsets.movement_type) != 2)
        return;
    MovementAngles view;
    engine::invoke<void>(bindings.engine, 18, &view);
    view.pitch = 0.0f;
    MovementVector forward, right;
    movement_angle_vectors(view, &forward, &right, nullptr);
    auto trace_origin = origin(player);
    trace_origin = {forward.x * 100.0f + trace_origin.x, forward.y * 100.0f + trace_origin.y,
                    forward.z * 100.0f + trace_origin.z + 9.0f};
    const auto trace = trace_down(bindings, player, trace_origin);
    if (!acceptable_floor(trace) || 250.0f > trace.fraction * 1000.0f)
        return;
    auto velocity = field<MovementVector>(player, bindings.offsets.velocity);
    velocity.z = convar(bindings.jump_impulse) + velocity.z -
                 (bindings.interval * convar(bindings.gravity)) * 0.5f;
    const auto first_origin = origin(player);
    std::uint32_t alternatives =
        landing_forecast(*this, player, first_origin, velocity, trace.end.z, view.yaw) ? 1u : 0u;
    client::Command command{};
    command.angles = {view.pitch, view.yaw, view.roll};
    command.movement.forward_move = 450.0f;
    command.movement.buttons = (command.movement.buttons & ~2u) | 0x400000u;
    const auto ticks = _mm_cvttss_si32(_mm_set_ss(0.75f / bindings.interval + 0.5f));
    const auto flags = field<std::uint32_t>(player, bindings.offsets.flags);
    const auto live_velocity = field<MovementVector>(player, bindings.offsets.velocity);
    const auto current_origin = origin(player);
    MovementSimulation simulation{};
    initialize_live_movement_simulation(simulation, player, current_origin, live_velocity, flags, 0,
                                        bindings.simulation_capture);
    for (std::int32_t i = 0; i < ticks; ++i) {
        simulate_live_movement_tick(simulation, bindings.collision, bindings.movement_environment,
                                    player, command);
        if (!(simulation.player_flags & 1u) ||
            16.0f > simulation.velocity.y * simulation.velocity.y +
                        simulation.velocity.x * simulation.velocity.x)
            break;
    }
    if (!(simulation.player_flags & 1u) &&
        landing_forecast(*this, player, simulation.position, simulation.velocity, trace.end.z,
                         view.yaw))
        alternatives |= 2u;
    if (!alternatives)
        return;
    draw_disc(queue, bindings.projection, origin(player), config.color.value);
    const MovementVector length{forward.x * 10.0f, forward.y * 10.0f, forward.z * 10.0f};
    const auto draw_at = [&](MovementVector center, bool reverse) {
        const MovementVector low{center.x - length.x, center.y - length.y, center.z - length.z};
        const MovementVector high{length.x + center.x, length.y + center.y, length.z + center.z};
        if (reverse)
            draw_arrow(queue, bindings.projection, high, low);
        else
            draw_arrow(queue, bindings.projection, low, high);
    };
    if (alternatives == 1u)
        draw_at(origin(player), false);
    else if (alternatives == 2u)
        draw_at(origin(player), true);
    else {
        auto first = origin(player);
        first = {first.x - right.x * 8.0f, first.y - right.y * 8.0f, first.z - right.z * 8.0f};
        draw_at(first, false);
        auto second = origin(player);
        second = {right.x * 8.0f + second.x, right.y * 8.0f + second.y, right.z * 8.0f + second.z};
        draw_at(second, true);
    }
}

namespace {
#if defined(_M_IX86) || defined(__i386__)
#define NOFALL_MEMBER __thiscall
#define NOFALL_FAST __fastcall
#else
#define NOFALL_MEMBER
#define NOFALL_FAST
#endif
void NOFALL_FAST draw_no_fall(void* endpoint, ui::RenderQueue* queue, void* player, std::int32_t*) {
    static_cast<NoFallDamageHudHandler*>(endpoint)->runtime.draw(*queue, player);
}
const hud::HudFrameEntry hud_table[]{draw_no_fall};
using OwnerEntry = void(NOFALL_MEMBER*)(void*);
struct NoFallDamageOwner;
void NOFALL_MEMBER no_fall_damage_destroy(void*);
void NOFALL_MEMBER no_fall_damage_bind(void*);
void NOFALL_MEMBER no_fall_damage_unchanged(void*) {} //433CCE8E
const OwnerEntry no_fall_damage_lifecycle_table[]{no_fall_damage_destroy,   no_fall_damage_bind,      no_fall_damage_unchanged, no_fall_damage_unchanged, no_fall_damage_unchanged,
                                   no_fall_damage_unchanged, no_fall_damage_unchanged, no_fall_damage_unchanged, no_fall_damage_unchanged};
struct NoFallDamageOwner {
    const OwnerEntry* vtable = nullptr;
    ConfigRegistries& registries;
    NoFallDamageRuntime runtime;
    client::NativeCommandPhaseReceiver command_receiver{};
    NoFallDamageHudHandler hud;
    engine::JobMemory memory;
    NoFallDamageOwner(engine::OwnerRegistryBinding owners, client::CommandPhaseRegistry& commands,
                      engine::OwnerRegistryBinding hud_owners, ConfigRegistries& r,
                      NoFallDamageConfig& config, NoFallDamageBindings bindings)
        : registries(r), runtime(config, bindings), hud(runtime), memory(owners.memory) {
        owners.append(this); //4348DCBC
        commands.append(command_receiver, &runtime); //4348DCC4
        hud::register_hud_frame(hud_owners, &hud); //4348DCCC
        vtable = no_fall_damage_lifecycle_table; //4348DCD4
        commands.publish(command_receiver, client::CommandOwnerId::owner_43472998); //4348DCDC
        hud.vtable = hud_table; //4348DCE4
        initialize_no_fall_damage_config(r, config);
        runtime.air_strafe_enabled = nullptr;
    }
};
void NOFALL_MEMBER no_fall_damage_bind(void* object) {
    auto& owner = *static_cast<NoFallDamageOwner*>(object);
    owner.runtime.bind_configuration(owner.registries.descriptors);
}
void NOFALL_MEMBER no_fall_damage_destroy(void* object) {
    auto* owner = static_cast<NoFallDamageOwner*>(object);
    const auto memory = owner->memory;
    owner->~NoFallDamageOwner();
    auto* bytes = static_cast<std::byte*>(object);
    const auto displacement = field<std::uint32_t>(bytes, -8);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
} //namespace
NoFallDamageHudHandler::NoFallDamageHudHandler(NoFallDamageRuntime& owner) : runtime(owner) {}
std::size_t no_fall_damage_owner_size() noexcept {
    return sizeof(NoFallDamageOwner);
}
NoFallDamageRuntime& construct_no_fall_damage(void* storage, engine::OwnerRegistryBinding owners,
                                              client::CommandPhaseRegistry& commands,
                                              engine::OwnerRegistryBinding hud_owners,
                                              ConfigRegistries& r, NoFallDamageConfig& config,
                                              NoFallDamageBindings bindings) {
    return (::new (storage) NoFallDamageOwner(owners, commands, hud_owners, r, config, bindings))
        ->runtime;
}
#undef NOFALL_MEMBER
#undef NOFALL_FAST
} //namespace skeet::misc

// ============================================================================
// Subcomponent: persistent_kill_feed.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
void __fastcall update_handler(void* object, std::uint8_t) {
#else
void update_handler(void* object, std::uint8_t) {
#endif
    static_cast<PersistentKillFeedFrameHandler*>(static_cast<engine::FrameUpdateEndpoint*>(object))
        ->runtime.update();
}
const std::array<engine::FrameUpdateEntry, 1> persistent_kill_feed_frame_table{update_handler};
// deduplicated field
template <class T> void persistent_kill_feed_assign(void* object, std::ptrdiff_t offset, T value) { std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(T)); }
}
void* PersistentKillFeedRuntime::death_notice() {
    if (!bindings_.find_hud)
        return nullptr;
    auto* element = bindings_.find_hud(bindings_.hud_context, hud_name_.begin);
    return element ? static_cast<std::byte*>(element) - 0x14 : nullptr;
}
void PersistentKillFeedRuntime::clear(void* owner) {
    const auto previous = field<std::uint8_t>(owner, 0x1c);
    persistent_kill_feed_assign(owner, 0x1c, std::uint8_t{0});
    engine::invoke<void>(static_cast<std::byte*>(owner) + 0x14, 7);
    pending_count = 0;
    persistent_kill_feed_assign(owner, 0x1c, previous);
}
void PersistentKillFeedRuntime::on_event(events::GameEventReader& event, std::uint32_t hash) {
    if (!settings_.enabled.value)
        return;
    if (engine::invoke<bool>(bindings_.engine, 82))
        return;
    if (engine::invoke<bool>(bindings_.engine, 93))
        return;
    if (hash == events::player_death_hash) {
        events::FieldValue attacker, assister, victim;
        if (event.read(0x455b4dbf, events::FieldType::integer, attacker))
            return;
        if (event.read(0x721dbb88, events::FieldType::integer, assister))
            return;
        if (event.read(0x8361150e, events::FieldType::integer, victim))
            return;
        const auto source = engine::invoke<std::int32_t>(bindings_.engine, 9, attacker.integer);
        const auto assist = engine::invoke<std::int32_t>(bindings_.engine, 9, assister.integer);
        const auto local = engine::invoke<std::int32_t>(bindings_.engine, 12);
        if ((source != local && assist != local) || victim.integer == attacker.integer)
            return;
        auto* hud = death_notice();
        if (!hud)
            return;
        float lifetime = 10000.0f;
        if (bindings_.game_rules && *bindings_.game_rules) {
            const auto* rules = *bindings_.game_rules;
            if (field<std::uint8_t>(rules, bindings_.offsets.rules_d000a932))
                lifetime = 30.0f;
            else {
                const auto duration = static_cast<float>(
                    field<std::int32_t>(rules, bindings_.offsets.rules_972bc124));
                const auto end = duration + field<float>(rules, bindings_.offsets.rules_9f8e7f71);
                const auto remaining = end - field<float>(bindings_.global_vars, 0x10);
                lifetime = maximum_number(1.5f, remaining);
            }
        }
        persistent_kill_feed_assign(hud, 0x64, lifetime);
        ++pending_count;
    } else if ((hash == 0x3d07d291 || pending_count > 64) && pending_count) {
        if (auto* hud = death_notice())
            clear(hud);
    }
}
void PersistentKillFeedRuntime::reset() {
    if (auto* hud = death_notice())
        persistent_kill_feed_assign(hud, 0x64, 1.5f);
}
void PersistentKillFeedRuntime::update() {
    if (!settings_.enabled.value && pending_count)
        if (auto* hud = death_notice())
            clear(hud);
}
void PersistentKillFeedRuntime::release() {
    if (engine::invoke<bool>(bindings_.engine, 26) && pending_count)
        if (auto* hud = death_notice())
            clear(hud);
}
void PersistentKillFeedRuntime::reset_state() noexcept {
    pending_count = 0;
}
PersistentKillFeedFrameHandler::PersistentKillFeedFrameHandler(engine::OwnerRegistryBinding owners,
                                                               PersistentKillFeedRuntime& concrete)
    : runtime(concrete) {
    engine::register_frame_update(owners, this);
    vtable = persistent_kill_feed_frame_table.data();
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: persistent_kill_feed_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define FEED_MEMBER __thiscall
#else
#define FEED_MEMBER
#endif
using OwnerEntry = void(FEED_MEMBER*)(void*);
struct KillFeedInstance;
void FEED_MEMBER persistent_kill_feed_owner_destroy(void*);
void FEED_MEMBER persistent_kill_feed_owner_unchanged(void*) {} //433CCE8E
void FEED_MEMBER persistent_kill_feed_owner_release(void*);
void FEED_MEMBER persistent_kill_feed_owner_reset_state(void*);
void FEED_MEMBER persistent_kill_feed_owner_forward_reset(void*);
const std::array<OwnerEntry, 9> persistent_kill_feed_owner_owner_table{persistent_kill_feed_owner_destroy,   persistent_kill_feed_owner_unchanged,     persistent_kill_feed_owner_unchanged,
                                            persistent_kill_feed_owner_unchanged, persistent_kill_feed_owner_release,       persistent_kill_feed_owner_reset_state,
                                            persistent_kill_feed_owner_unchanged, persistent_kill_feed_owner_forward_reset, persistent_kill_feed_owner_unchanged};
struct KillFeedInstance {
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    NativeText hud_name;
    PersistentKillFeedRuntime runtime;
    alignas(events::KillFeedEventObserver)
        std::array<std::byte, sizeof(events::KillFeedEventObserver)> event_storage;
    alignas(PersistentKillFeedFrameHandler)
        std::array<std::byte, sizeof(PersistentKillFeedFrameHandler)> frame_storage;
    KillFeedInstance(engine::OwnerRegistryBinding owners,
                     engine::OwnerRegistryBinding event_receivers,
                     engine::OwnerRegistryBinding frame_receivers, ConfigRegistries& registry,
                     PersistentKillFeedConfig& config, PersistentKillFeedBindings& bindings)
        : memory(owners.memory), runtime(config, bindings, hud_name) {
        owners.append(this);
        auto* event = ::new (event_storage.data()) events::KillFeedEventObserver(runtime);
        events::register_event_observer(event_receivers.registry, *event, event_receivers.memory);
        ::new (frame_storage.data()) PersistentKillFeedFrameHandler(frame_receivers, runtime);
        vtable = persistent_kill_feed_owner_owner_table.data();
        initialize_persistent_kill_feed_config(registry, config);
        runtime.pending_count = 0;
        construct_native_text(hud_name, "CCSGO_HudDeathNotice", memory);
    }
    ~KillFeedInstance() {
        destroy_native_text(hud_name, memory); //433D11D3 ->4336B1E9
        std::launder(reinterpret_cast<PersistentKillFeedFrameHandler*>(frame_storage.data()))
            ->~PersistentKillFeedFrameHandler();
        std::launder(reinterpret_cast<events::KillFeedEventObserver*>(event_storage.data()))
            ->~KillFeedEventObserver();
    }
};
void FEED_MEMBER persistent_kill_feed_owner_destroy(void* object) { //433D11D3
    auto* owner = static_cast<KillFeedInstance*>(object);
    const auto memory = owner->memory;
    owner->~KillFeedInstance();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void FEED_MEMBER persistent_kill_feed_owner_release(void* object) {
    static_cast<KillFeedInstance*>(object)->runtime.release();
}
void FEED_MEMBER persistent_kill_feed_owner_reset_state(void* object) {
    static_cast<KillFeedInstance*>(object)->runtime.reset_state();
}
void FEED_MEMBER persistent_kill_feed_owner_forward_reset(void* object) {
    static_cast<KillFeedInstance*>(object)->vtable[5](object);
}
#undef FEED_MEMBER
}
std::size_t persistent_kill_feed_owner_size() noexcept {
    return sizeof(KillFeedInstance);
}
PersistentKillFeedRuntime&
construct_persistent_kill_feed_owner(void* storage, engine::OwnerRegistryBinding owners,
                                     engine::OwnerRegistryBinding event_receivers,
                                     engine::OwnerRegistryBinding frame_receivers,
                                     ConfigRegistries& registry, PersistentKillFeedConfig& config,
                                     PersistentKillFeedBindings& bindings) {
    return (::new (storage) KillFeedInstance(owners, event_receivers, frame_receivers, registry,
                                             config, bindings))
        ->runtime;
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: ping_spike.cpp
// ============================================================================

namespace skeet::misc {
bool PingSpikeRuntime::active() const {
    return *enabled && hotkey_active(hotkey->binding, keys) && engine::invoke<bool>(engine, 26) &&
           !engine::invoke<bool>(engine, 82) && !engine::invoke<bool>(engine, 93);
}
void PingSpikeRuntime::rewind_sequence(void* channel) const {
    if (!active())
        return;
    const auto latencies = _mm_add_sd(_mm_set_sd(*latency_4347a508), _mm_set_sd(*latency_4347b120));
    const auto latency = _mm_cvtpd_ps(latencies);
    const auto scale = _mm_set_ss(0.001f);
    auto requested = _mm_mul_ss(_mm_cvtepi32_ps(_mm_cvtsi32_si128(*milliseconds)), scale);
    const auto maximum =
        _mm_mul_ss(_mm_cvtepi32_ps(_mm_cvtsi32_si128(*maximum_unlag_milliseconds)), scale);
    if (_mm_comigt_ss(_mm_add_ss(requested, latency), maximum))
        requested = _mm_sub_ss(maximum, latency);
    requested = _mm_max_ss(requested, _mm_setzero_ps());
    requested = _mm_min_ss(requested, maximum);
    const auto ticks = _mm_cvttss_si32(
        _mm_add_ss(_mm_div_ss(requested, _mm_set_ss(*interval_per_tick)), _mm_set_ss(0.5f)));
    auto* destination = static_cast<std::byte*>(channel) + 0x1C;
    std::uint32_t sequence;
    std::memcpy(&sequence, destination, sizeof(sequence));
    sequence -= std::uint32_t(ticks);
    std::memcpy(destination, &sequence, sizeof(sequence));
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: ping_spike_channel.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// deduplicated field
std::int32_t increment(std::int32_t value) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(value) + 1u);
}
std::int32_t subtract_128(std::int32_t value) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(value) - 128u);
}
PingSpikeChannelRuntime* channel_runtime = nullptr;
}

void record_ping_spike_sequences(PingSpikeNetworkHistory& history, std::uint32_t direction,
                                 std::int32_t sequence, std::int32_t acknowledged,
                                 PingSpikeNetworkClock clock) {
    auto& flow = history[direction];
    const auto now = clock();
    const auto previous = flow.sequence;
    if (sequence > previous) {
        auto next = increment(previous);
        PingSpikeLatencySample* last = nullptr;
        for (std::uint32_t count = 0; next <= sequence && count < 128;
             ++count, next = increment(next)) {
            auto& sample = flow.samples[std::uint32_t(next) & 127u];
            sample.sent_at = now;
            sample.present = 0;
            sample.round_trip = -1.0;
            last = &sample;
        }
        last->present = 1;
    }
    flow.sequence = sequence;
    auto& opposite = history[direction == 0 ? 1 : 0];
    if (acknowledged <= subtract_128(opposite.sequence))
        return;
    auto& sample = opposite.samples[std::uint32_t(acknowledged) & 127u];
    if (!sample.present || sample.round_trip != -1.0)
        return;
    const auto elapsed = now - sample.sent_at;
    sample.round_trip = elapsed < 0.0 ? 0.0 : elapsed;
}

void average_ping_spike_sequences(PingSpikeNetworkHistory& history,
                                  std::uint32_t direction) noexcept {
    auto& flow = history[direction];
    double total = 0.0;
    std::int32_t count = 0;
    for (const auto& sample : flow.samples) {
        if (sample.present && sample.round_trip >= 0.0) {
            total += sample.round_trip;
            ++count;
        }
    }
    if (count <= 0)
        return;
    const auto average = total / double(count);
    const auto previous = flow.smoothed * 0.75;
    flow.raw_average = average;
    flow.smoothed = previous + average * 0.25;
}

float query_channel_interpolation(void* engine_client, const PingSpikeNetworkHistory& history) {
    auto* channel = engine::invoke<void*>(engine_client, 78);
    if (!channel)
        return 0.0f;
    std::array<float, 2> original;
    for (std::int32_t direction = 0; direction < 2; ++direction)
        original[direction] = engine::invoke<float>(channel, 9, direction);

    // Complete43406FB8(channel,13,13,1), followed by the native range check.
    auto** table = field<void**>(channel, 0);
    const auto operand = field<std::uint32_t>(table[13], 13);
    const auto displacement = operand - 4u;
    if ((displacement & 0xFFFu) != displacement)
        return original[1] + original[0];

    // Native stores ST0 directly to a qword here, without the single-precision
    // rounding used for the two virtual9 results above.
    const auto channel_time = engine::invoke<double>(channel, 2);
    const auto measured = float(history[0].raw_average + history[1].raw_average);
    const auto compared_time = double(measured) + channel_time;
    auto adjusted = original;
    auto* flow = static_cast<const std::byte*>(channel) + displacement;
    for (std::uint32_t direction = 0; direction < 2; ++direction, flow += 0x1E2C) {
        const auto sample_time = double(field<float>(flow, 0));
        if (!(1.0 > std::fabs(sample_time - compared_time)) || !(compared_time >= sample_time))
            continue;
        float total = 0.0f;
        std::int32_t count = 0;
        for (std::uint32_t sample = 0; sample < 128; ++sample) {
            if (!field<std::uint8_t>(flow, 0x32 + sample * 0x10))
                continue;
            const auto latency = field<float>(flow, 0x34 + sample * 0x10);
            if (!(latency > -1.0f))
                continue;
            total += latency;
            ++count;
        }
        if (count > 0)
            adjusted[direction] = total / float(count);
    }
    const auto original_sum = original[1] + original[0];
    const auto adjusted_sum = adjusted[1] + adjusted[0];
    return 1.0f > std::fabs(adjusted_sum - original_sum) ? adjusted_sum : original_sum;
}

std::int32_t
acknowledge_datagram_sequence(std::span<const client::StartupAcknowledgement, 150> history,
                              std::int32_t sequence) noexcept {
    const auto* record = reinterpret_cast<const std::byte*>(history.data()) +
                         std::ptrdiff_t(sequence % 150) * sizeof(client::StartupAcknowledgement);
    return field<std::uint32_t>(record, 0) == std::bit_cast<std::uint32_t>(sequence)
               ? field<std::int32_t>(record, 4)
               : sequence;
}

PingSpikeChannelRuntime::PingSpikeChannelRuntime(PingSpikeRuntime& runtime,
                                                 PingSpikeChannelBindings shared)
    : feature(runtime), bindings(shared) {
    feature.latency_4347a508 = &bindings.history[0].smoothed;
    feature.latency_4347b120 = &bindings.history[1].smoothed;
}

std::int32_t PingSpikeChannelRuntime::send_datagram(void* channel, void* datagram) {
    const auto original_sequence = field<std::int32_t>(channel, 0x1C);
    const auto skips = bindings.pending_rewind_skips;
    if (skips & 0xFFu) {
        for (std::uint32_t bit = 0; bit < 8; ++bit) {
            if (skips & (1u << bit)) {
                bindings.pending_rewind_skips = skips & ~(1u << bit);
                break;
            }
        }
    } else
        feature.rewind_sequence(channel);

    auto** table = field<void**>(channel, 0);
    auto** original = field<void**>(table[-2], 8);
    using SendDatagram = engine::MemberFunction<std::int32_t, void*>;
    const auto result = reinterpret_cast<SendDatagram>(original[46])(channel, datagram);
    record_ping_spike_sequences(bindings.history, 0, result, original_sequence, bindings.clock);
    average_ping_spike_sequences(bindings.history, 0);
    bindings.interpolation = query_channel_interpolation(bindings.engine_client, bindings.history);
    const auto acknowledged = acknowledge_datagram_sequence(bindings.acknowledgements, result);
    std::memcpy(static_cast<std::byte*>(channel) + 0x1C, &original_sequence,
                sizeof original_sequence);
    return acknowledged;
}
void bind_ping_spike_channel_runtime(PingSpikeChannelRuntime& runtime) noexcept {
    channel_runtime = &runtime;
}
#if defined(_M_IX86) || defined(__i386__)
std::int32_t __fastcall ping_spike_send_datagram_hook(void* channel, void*, void* datagram) {
#else
std::int32_t ping_spike_send_datagram_hook(void* channel, void*, void* datagram) {
#endif
    return channel_runtime->send_datagram(channel, datagram);
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: purchase_log.cpp
// ============================================================================

namespace skeet::misc {
namespace {
constexpr std::uint32_t purchase_log_purchase_hash = 0xD6CAA6ED;
}

void PurchaseLogRuntime::item_purchase(events::GameEventReader& event,
                                       events::EngineEventPlayers& players) {
    if (!settings.purchases.value)
        return;
    events::FieldValue user, weapon;
    if (event.read(0x8361150E, events::FieldType::integer, user))
        return;
    if (event.read(0xFDF97DB0, events::FieldType::string, weapon))
        return;
    const auto index = players.player_for_user_id(user.integer);
    if (static_cast<std::uint32_t>(index) - 1 > 63)
        return;
    if (index == players.local_player_index())
        return;
    auto* target = engine::invoke<void*>(entity_list, 3, index);
    if (!target)
        return;
    const auto filter = targets.capture(players.local_entity());
    if (!rage::target_is_enemy(filter, target))
        return; // complete4339EF8C
    if (!weapon.string || events::event_hash(weapon.string) == 0x4DC70629)
        return;
    const auto name = events::player_log_name(index, players);
    std::array<char, 16> decoded_format;
    decode_native_text(format, decoded_format.data());
    output.format(decoded_format.data(), name.data(), weapon.string);
}

void PurchaseLogRuntime::purchase_listener(void* owner, events::GameEventReader& event,
                                           events::EventPlayerIndices& players) {
    static_cast<PurchaseLogRuntime*>(owner)->item_purchase(
        event, static_cast<events::EngineEventPlayers&>(players));
}
void PurchaseLogRuntime::damage_listener(void* owner, events::GameEventReader& event,
                                         events::EventPlayerIndices& players) {
    auto& runtime = *static_cast<PurchaseLogRuntime*>(owner);
    runtime.damage.player_hurt(event, static_cast<events::HitLogPlayerInfo&>(players),
                               runtime.output);
}
void PurchaseLogRuntime::bind_events(events::EventRegistry& registry) {
    registry.add({events::player_hurt_hash, this, damage_listener});
}
void PurchaseLogRuntime::configuration_changed(bool notify, events::EventRegistry& registry) {
    if (!notify)
        return;
    const auto enabled = settings.purchases.value;
    if (enabled == purchase_subscribed_)
        return;
    purchase_subscribed_ = enabled;
    const events::EventListener listener{purchase_log_purchase_hash, this, purchase_listener};
    if (enabled)
        registry.add(listener);
    else
        registry.remove(listener);
}
void PurchaseLogRuntime::disable_purchase_subscription(events::EventRegistry& registry) {
    if (purchase_subscribed_) {
        purchase_subscribed_ = 0;
        registry.remove({purchase_log_purchase_hash, this, purchase_listener});
    }
}
void PurchaseLogRuntime::unbind_events(events::EventRegistry& registry) {
    disable_purchase_subscription(registry);
    registry.remove({events::player_hurt_hash, this, damage_listener});
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: purchase_log_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
void __fastcall purchase_configuration_frame(void* object, std::uint8_t notify) {
#else
void purchase_configuration_frame(void* object, std::uint8_t notify) {
#endif
    auto& handler =
        *static_cast<PurchaseLogFrameHandler*>(static_cast<engine::FrameUpdateEndpoint*>(object));
    handler.runtime.configuration_changed(notify != 0, handler.events);
}
const engine::FrameUpdateEntry purchase_log_owner_frame_table[]{purchase_configuration_frame};
}
PurchaseLogFrameHandler::PurchaseLogFrameHandler(engine::OwnerRegistryBinding frames,
                                                 PurchaseLogRuntime& source,
                                                 events::EventRegistry& registry)
    : runtime(source), events(registry) {
    engine::register_frame_update(frames, this); //433FCB12, before table assignment
    vtable = purchase_log_owner_frame_table;
}

namespace {
#if defined(_M_IX86) || defined(__i386__)
#define PURCHASE_MEMBER __thiscall
#define PURCHASE_FAST __fastcall
#else
#define PURCHASE_MEMBER
#define PURCHASE_FAST
#endif
using OwnerEntry = void(PURCHASE_MEMBER*)(void*);
using ConsoleInputEntry = void(PURCHASE_MEMBER*)(void*, void*, const char*);
using NativePurchaseEvent = void(PURCHASE_FAST*)(void*, void*, void*);
void PURCHASE_MEMBER destroy_purchase_owner(void*);
void PURCHASE_MEMBER purchase_log_owner_capture_console_owner(void*);
void PURCHASE_MEMBER purchase_log_owner_restore_console_owner(void*);
void PURCHASE_MEMBER purchase_log_owner_console_input_owner_entry(void*, void*, const char*);
void PURCHASE_MEMBER purchase_log_owner_unchanged_owner(void*) {} //433CCE8E RET
void PURCHASE_FAST native_purchase_listener(void*, void*, void*);
void PURCHASE_FAST native_purchase_frame(void*, std::uint8_t);
struct OwnerTable {
    std::array<OwnerEntry, 9> lifecycle;
    ConsoleInputEntry console_input;
};
const OwnerTable purchase_log_owner_owner_table{{destroy_purchase_owner, purchase_log_owner_unchanged_owner, purchase_log_owner_unchanged_owner,
                              purchase_log_owner_capture_console_owner, purchase_log_owner_restore_console_owner, purchase_log_owner_unchanged_owner,
                              purchase_log_owner_unchanged_owner, purchase_log_owner_unchanged_owner, purchase_log_owner_unchanged_owner},
                             purchase_log_owner_console_input_owner_entry};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(OwnerTable, console_input) == 0x24);
static_assert(sizeof(OwnerTable) == 0x28);
#endif
const engine::FrameUpdateEntry native_frame_table[]{native_purchase_frame};
struct NativePurchaseSubscription {
    std::uint32_t hash;
    void* owner;
    NativePurchaseEvent callback;
    std::uint8_t subscribed;
    std::array<std::byte, 3> preserved;
};
struct PurchaseLogPrefix {
    const OwnerTable* vtable; //0,4346F750
    std::uint32_t preserved; //4
    engine::FrameUpdateEndpoint frame; //8,4346EF98
    EventLogConfig configuration; //C,values1C/30
    NativeEncryptedText hit_format; //34
    NativeEncryptedText purchase_format; //3C
    NativePurchaseSubscription purchase; //44,raw subscription50
    std::array<NativeText, 9> hitgroups; //54..D4
};
static_assert(std::is_trivially_copyable_v<PurchaseLogPrefix>);
// Clang's layout extension preserves the native inherited-config prefix.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
constexpr std::size_t purchase_frame_offset = offsetof(PurchaseLogPrefix, frame);
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(PurchaseLogPrefix) == 0xE4);
static_assert(offsetof(PurchaseLogPrefix, frame) == 8);
static_assert(offsetof(PurchaseLogPrefix, configuration) == 0xC);
static_assert(offsetof(PurchaseLogPrefix, hit_format) == 0x34);
static_assert(offsetof(PurchaseLogPrefix, purchase_format) == 0x3C);
static_assert(offsetof(PurchaseLogPrefix, purchase) == 0x44);
static_assert(offsetof(PurchaseLogPrefix, hitgroups) == 0x54);
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
}

class PurchaseLogLifecycleOwner {
public:
    PurchaseLogPrefix prefix;
    engine::JobMemory memory;
    events::EventRegistry& events;
    script::ConsoleInputHook& console;
    void* const& console_container;
    alignas(PurchaseLogRuntime) std::array<std::byte, sizeof(PurchaseLogRuntime)> state;

    PurchaseLogLifecycleOwner(engine::OwnerRegistryBinding lifecycle,
                              engine::OwnerRegistryBinding frames, ConfigRegistries& registries,
                              events::EventRegistry& registry, NormalLog& output,
                              rage::TargetRelationshipBindings targets, void* entity_list,
                              script::ConsoleInputHook& input, void* const& container,
                              const PurchaseLogPrefix& previous)
        : memory(lifecycle.memory), events(registry), console(input), console_container(container) {
        // The native constructor preserves reserved words/bytes and both
        // existing boolean values. Restore the caller's prefix before its
        // first observable registration, then perform only native writes.
        std::memcpy(&prefix, &previous, sizeof prefix);
        prefix.vtable = nullptr;
        lifecycle.append(this); //433676A0, before vtable assignment
        engine::register_frame_update(frames, &prefix.frame); //433FCB12
        prefix.vtable = &purchase_log_owner_owner_table;
        prefix.frame.vtable = native_frame_table;
        initialize_event_log_config(registries, prefix.configuration);

        constexpr char hit_format[] = "Hit %s in the %s for %d damage (%d health remaining)";
        static_assert(sizeof(hit_format) == 0x35);
        construct_native_encrypted_text(prefix.hit_format, hit_format, sizeof(hit_format), memory);
        // Native encrypts14 bytes: the12 visible characters and TWO zeros.
        constexpr std::array<char, 0xE> purchase_format{'%', 's', ' ', 'b', 'o', 'u', 'g',
                                                        'h', 't', ' ', '%', 's', 0,   0};
        construct_native_encrypted_text(prefix.purchase_format, purchase_format.data(),
                                        purchase_format.size(), memory);
        prefix.purchase.hash = 0xD6CAA6ED;
        prefix.purchase.owner = this;
        prefix.purchase.callback = native_purchase_listener;
        prefix.purchase.subscribed = 0;
        constexpr const char* groups[]{"body",      "head",     "chest",     "stomach", "left arm",
                                       "right arm", "left leg", "right leg", "neck"};
        for (std::size_t i = 0; i < prefix.hitgroups.size(); ++i)
            construct_native_text(prefix.hitgroups[i], groups[i], memory);

        ::new (state.data())
            PurchaseLogRuntime(prefix.configuration, output, targets, prefix.purchase_format,
                               prefix.hit_format, prefix.hitgroups, prefix.purchase.subscribed);
        runtime().entity_list = entity_list;
        runtime().bind_events(events); //actual constructor's permanent damage listener
    }
    PurchaseLogRuntime& runtime() noexcept {
        return *std::launder(reinterpret_cast<PurchaseLogRuntime*>(state.data()));
    }
};
#if UINTPTR_MAX == UINT32_MAX
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(PurchaseLogLifecycleOwner, prefix) == 0);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif

namespace {
void PURCHASE_FAST native_purchase_frame(void* endpoint, std::uint8_t notify) {
    auto* owner = reinterpret_cast<PurchaseLogLifecycleOwner*>(static_cast<std::byte*>(endpoint) -
                                                               purchase_frame_offset);
    owner->runtime().configuration_changed(notify != 0, owner->events);
}
void PURCHASE_FAST native_purchase_listener(void* object, void* raw_event, void* local) {
    auto& owner = *static_cast<PurchaseLogLifecycleOwner*>(object);
    events::EngineGameEventReader event(raw_event);
    events::EngineEventPlayers players(owner.events.engine_interface, local);
    owner.runtime().item_purchase(event, players); //same433A0D37 source and backing
}
void PURCHASE_MEMBER purchase_log_owner_capture_console_owner(void* object) { //43482378
    auto& owner = *static_cast<PurchaseLogLifecycleOwner*>(object);
    owner.console.capture(owner.console_container, &owner.prefix);
}
void PURCHASE_MEMBER purchase_log_owner_restore_console_owner(void* object) { //433F8C58
    static_cast<PurchaseLogLifecycleOwner*>(object)->console.restore();
}
void PURCHASE_MEMBER purchase_log_owner_console_input_owner_entry(void* object, void* engine_object,
                                               const char* text) { //433410B6
    process_purchase_console_input(*static_cast<PurchaseLogLifecycleOwner*>(object), engine_object,
                                   text);
}
void PURCHASE_MEMBER destroy_purchase_owner(void* object) { //433B6C4E ->433E0610
    auto* owner = static_cast<PurchaseLogLifecycleOwner*>(object);
    if (!owner)
        return;
    const auto memory = owner->memory;
    for (std::size_t i = owner->prefix.hitgroups.size(); i != 0; --i)
        destroy_native_text(owner->prefix.hitgroups[i - 1], memory);
    owner->runtime().disable_purchase_subscription(owner->events);
    destroy_native_encrypted_text(owner->prefix.purchase_format, memory);
    destroy_native_encrypted_text(owner->prefix.hit_format, memory);
    // The permanent damage triple remains until the common native registry
    // shutdown; that registry is cleared before final owner destruction.
    owner->runtime().~PurchaseLogRuntime();
    owner->~PurchaseLogLifecycleOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}
std::size_t purchase_log_owner_size() noexcept {
    return sizeof(PurchaseLogLifecycleOwner);
}
PurchaseLogLifecycleOwner&
construct_purchase_log(void* storage, engine::OwnerRegistryBinding lifecycle,
                       engine::OwnerRegistryBinding frames, ConfigRegistries& registries,
                       events::EventRegistry& events, NormalLog& output,
                       rage::TargetRelationshipBindings targets, void* entity_list,
                       script::ConsoleInputHook& console, void* const& container) {
    PurchaseLogPrefix previous;
    std::memcpy(&previous, storage, sizeof previous);
    return *::new (storage)
        PurchaseLogLifecycleOwner(lifecycle, frames, registries, events, output, targets,
                                  entity_list, console, container, previous);
}
PurchaseLogRuntime& purchase_log_runtime(PurchaseLogLifecycleOwner& owner) noexcept {
    return owner.runtime();
}
EventLogConfig& purchase_log_configuration(PurchaseLogLifecycleOwner& owner) noexcept {
    return owner.prefix.configuration;
}
void process_purchase_console_input(PurchaseLogLifecycleOwner& owner, void* engine_object,
                                    const char* text) {
    owner.console.process(engine_object, text);
}
#undef PURCHASE_MEMBER
#undef PURCHASE_FAST
} //namespace skeet::misc

// ============================================================================
// Subcomponent: quick_switch.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// deduplicated field
std::int32_t previous_number(std::int32_t number) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(number) - 1u);
}
std::uint8_t history_redraw(const prediction::CommandHistoryState& history) noexcept {
    // This byte is part of the shared history payload, immediately before
    // its +6D grenade pin state and +70 grenade throw timestamp.
    return field<std::uint8_t>(&history, 0x6C);
}
}

void* QuickSwitchRuntime::active_weapon(void* player) const {
    const auto handle = field<std::uint32_t>(player, bindings.offsets.active_weapon);
    if (handle == UINT32_MAX)
        return nullptr;
    return engine::invoke<void*>(bindings.entity_list, 4, handle);
}

void QuickSwitchRuntime::capture_before_prediction(void* player, const client::Command& command) {
    if (!*enabled || command.weapon_select != 0)
        return;
    auto* weapon = active_weapon(player);
    if (!weapon || engine::invoke<std::int32_t>(weapon, 455) != 9)
        return;
    state.captured_command = command.number;
    state.captured_pin_pulled = field<std::uint8_t>(weapon, bindings.offsets.grenade_pin_pulled);
    state.captured_throw_time = field<float>(weapon, bindings.offsets.grenade_throw_time);
}

void QuickSwitchRuntime::finish_prediction(void* player, const client::Command& command) {
    if (command.number != state.captured_command)
        return;
    auto* weapon = active_weapon(player);
    if (!weapon || engine::invoke<std::int32_t>(weapon, 455) != 9)
        return;
    if (state.captured_throw_time != 0.0f && state.captured_pin_pulled &&
        field<float>(weapon, bindings.offsets.grenade_throw_time) == 0.0f &&
        field<std::uint8_t>(weapon, bindings.offsets.grenade_pin_pulled) == 0 &&
        state.last_switch_command < command.number)
        state.pending_command = command.number;
}

void QuickSwitchRuntime::process_phase2(void* player, void* weapon, client::Command& command,
                                        std::uint8_t& send_packet) {
    if (!*enabled || !weapon) {
        state.pending_command = std::numeric_limits<std::int32_t>::max();
        return;
    }
    const auto& offsets = bindings.offsets;
    if (engine::invoke<std::int32_t>(weapon, 455) == 9) {
        const auto previous = previous_number(command.number);
        const auto* history =
            prediction::CommandHistoryView::from_owner(bindings.history_owner).find(previous);
        if (history) {
            // Native weapon slot2 returns the existing handle/identity word.
            // It is queried only after the exact-number history lookup passes.
            const auto identity = *engine::invoke<const std::uint32_t*>(weapon, 2);
            if (history->weapon_identity == identity) {
                const bool was_throwing = history_redraw(*history) == 0 && history->fired != 0 &&
                                          history->shot_time != 0.0f;
                const bool finished_throw =
                    field<std::uint8_t>(weapon, offsets.grenade_redraw) != 0 &&
                    field<std::uint8_t>(weapon, offsets.grenade_pin_pulled) == 0 &&
                    field<float>(weapon, offsets.grenade_throw_time) == 0.0f;
                if (was_throwing && finished_throw)
                    state.pending_command = previous_number(command.number);
            }
        }
    } else if (field<std::uint16_t>(weapon, offsets.weapon_definition) == 31 &&
               field<std::int32_t>(player, offsets.shots_fired) == 1 &&
               field<std::int32_t>(weapon, offsets.ammunition) == 0) {
        state.pending_command = previous_number(command.number);
    }
    if (*bindings.last_outgoing_command < state.pending_command)
        return;
    state.last_switch_command = command.number;
    state.pending_command = std::numeric_limits<std::int32_t>::max();
    if (bindings.game_rules && *bindings.game_rules) {
        auto* next = engine::invoke<void*>(*bindings.game_rules, 27, player, 0);
        if (next)
            command.weapon_select =
                engine::invoke<std::int32_t>(static_cast<std::byte*>(next) + 8, 10);
    }
    if (*bindings.choked_commands == 0)
        send_packet = 0;
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: quick_switch_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define QUICK_MEMBER __thiscall
#else
#define QUICK_MEMBER
#endif
using OwnerEntry = void(QUICK_MEMBER*)(void*);
void QUICK_MEMBER destroy_quick_switch(void*);
void QUICK_MEMBER quick_switch_owner_reset_quick_switch(void*);
void QUICK_MEMBER quick_switch_owner_forward_quick_switch_reset(void*);
void QUICK_MEMBER quick_switch_owner_unchanged_quick_switch(void*) {} //433CCE8E RET
const std::array<OwnerEntry, 9> quick_switch_owner_lifecycle_table{
    destroy_quick_switch,   quick_switch_owner_unchanged_quick_switch,     quick_switch_owner_unchanged_quick_switch,
    quick_switch_owner_unchanged_quick_switch, quick_switch_owner_unchanged_quick_switch,     quick_switch_owner_reset_quick_switch,
    quick_switch_owner_unchanged_quick_switch, quick_switch_owner_forward_quick_switch_reset, quick_switch_owner_unchanged_quick_switch};
}
class QuickSwitchLifecycleOwner {
public:
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    QuickSwitchRuntime& runtime;
    CommandOwner command;
    client::NativeCommandPhaseReceiver command_receiver{};
    network::NativeExploitRechargeReceiver recharge_receiver{};

    QuickSwitchLifecycleOwner(engine::OwnerRegistryBinding lifecycle,
                              client::CommandPhaseRegistry& commands,
                              network::ExploitRechargeObserverRegistry& recharge,
                              prediction::ObserverRegistry& prediction,
                              ConfigRegistries& registries, QuickSwitchConfig& config,
                              QuickSwitchRuntime& source)
        : memory(lifecycle.memory), runtime(source), command(&source) {
        lifecycle.append(this); //433676A0
        commands.append(command_receiver, &command); //4347DC1C ->43345EBC
        recharge.append_noop(recharge_receiver,
                             &runtime); //4347DC24; final table remains unpublished
        prediction.append(runtime); //4341CEE4, table4346FA2C
        vtable = quick_switch_owner_lifecycle_table.data(); //4347DC34, table4346DC9C
        commands.publish(command_receiver, client::CommandOwnerId::quick_switch); //4347DC3D
        recharge.publish_noop(recharge_receiver); //4347DC45, table434727FC
        prediction.publish(runtime); //4347DC4C, table4346FA2C
        initialize_quick_switch_config(registries, config);
        runtime.enabled = &config.enabled.value;
        quick_switch_owner_reset_quick_switch(this);
    }
};
namespace {
void QUICK_MEMBER quick_switch_owner_reset_quick_switch(void* object) { //4337E8DD
    auto& state = static_cast<QuickSwitchLifecycleOwner*>(object)->runtime.state;
    state.last_switch_command = -1;
    state.pending_command = std::numeric_limits<std::int32_t>::max();
}
void QUICK_MEMBER quick_switch_owner_forward_quick_switch_reset(void* object) { //433CF2F5
    const auto* owner = static_cast<QuickSwitchLifecycleOwner*>(object);
    owner->vtable[5](object);
}
void QUICK_MEMBER destroy_quick_switch(void* object) { //433B1A7A ->433837D0
    auto* owner = static_cast<QuickSwitchLifecycleOwner*>(object);
    const auto memory = owner->memory;
    owner->~QuickSwitchLifecycleOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}
std::size_t quick_switch_owner_size() noexcept {
    return sizeof(QuickSwitchLifecycleOwner);
}
QuickSwitchLifecycleOwner& construct_quick_switch(
    void* storage, engine::OwnerRegistryBinding lifecycle, client::CommandPhaseRegistry& commands,
    network::ExploitRechargeObserverRegistry& recharge, prediction::ObserverRegistry& prediction,
    ConfigRegistries& registries, QuickSwitchConfig& config, QuickSwitchRuntime& runtime) {
    const auto& image = prediction.image();
    if (&recharge.image() != &image ||
        &lifecycle.registry != &image.native_cell<engine::OwnerRegistry>(0x43475B78u))
        throw std::invalid_argument("Quick-switch observers require the retained lifecycle image");
    return *::new (storage) QuickSwitchLifecycleOwner(lifecycle, commands, recharge, prediction,
                                                      registries, config, runtime);
}
#undef QUICK_MEMBER
} //namespace skeet::misc

// ============================================================================
// Subcomponent: rebuy_fix.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::misc {
namespace {
// deduplicated field
std::uint32_t cvar_integer(void* variable) {
    for (;;) {
        auto* parent = field<void*>(variable, 0x1C);
        if (variable == parent)
            break;
        variable = parent;
    }
    return field<std::uint32_t>(variable, 0x30) ^
           static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(variable));
}
constexpr std::uint32_t spawn_hash = 0xC90B4171, rebuy_fix_purchase_hash = 0xD6CAA6ED;
}

bool RebuyFixRuntime::local_event(events::GameEventReader& event) {
    events::FieldValue user;
    if (event.read(0x8361150E, events::FieldType::integer, user) || user.integer == 0xFFFFFFFF)
        return false;
    const auto index = engine::invoke<std::int32_t>(engine, 9, user.integer);
    return index == engine::invoke<std::int32_t>(engine, 12);
}
void RebuyFixRuntime::player_spawn(events::GameEventReader& event) {
    if (!*enabled || !local_event(event))
        return;
    blocked = false;
}
void RebuyFixRuntime::item_purchase(events::GameEventReader& event) {
    item_purchase(event, local_player);
}
void RebuyFixRuntime::item_purchase(events::GameEventReader& event, void* local) {
    if (!*enabled || !local_event(event))
        return;
    if (!(std::fabs(last_buy_time - *current_time) > 0.25f))
        return;
    inventory.clear();
    for (std::uint32_t offset = 0; offset != 0xC0; offset += 4) {
        const auto handle = field<std::uint32_t>(local, offsets.inventory + offset);
        if (handle == 0xFFFFFFFF)
            continue;
        auto* weapon = engine::invoke<void*>(entity_list, 4, handle);
        if (!weapon)
            continue;
        auto* data = engine::invoke<void*>(weapon, 461);
        const auto identifier = field<std::int32_t>(data, 0xA8);
        if (identifier > 0)
            inventory.push_back(identifier);
    }
    if (field<std::int32_t>(local, offsets.armor) > 0)
        inventory.push_back(field<std::uint8_t>(local, offsets.helmet) ? 33 : 32);
}
void RebuyFixRuntime::update(const UserCommand& command) {
    const auto process = [&] {
        if (!*enabled)
            return;
        const auto anywhere = cvar_integer(buy_anywhere_cvar);
        if (anywhere != 1 && anywhere != field<std::uint32_t>(local_player, offsets.team) &&
            !field<std::uint8_t>(local_player, offsets.buy_zone))
            return;
        if (blocked || !field<std::uint8_t>(local_player, offsets.field_f7df3858 + 4))
            return;
        if (!(command.buttons & 0x2000) || (previous_buttons & 0x2000))
            return;
        last_buy_time = static_cast<float>(*time_4347a500 + *time_4347b118) + *current_time;
        // The native loop snapshots its end pointer. Engine calls do not extend
        // this inventory; item_purchase is time-gated during the command batch.
        const auto count = inventory.size();
        for (std::size_t index = 0; index != count; ++index) {
            const auto identifier = inventory[index];
            if (static_cast<std::uint32_t>(identifier) - 8 < 18) {
                if (engine::invoke<void*>(local_player, 264, 0))
                    continue;
            } else if (identifier == 2)
                continue;
            char text[32];
            std::snprintf(text, sizeof text, "buy %d %d", identifier, identifier);
            engine::invoke<void>(engine, 6, text, true);
        }
        engine::invoke<void>(engine, 114, "buymenu 0", true);
    };
    process();
    previous_buttons = command.buttons;
}
void RebuyFixRuntime::spawn_listener(void* owner, events::GameEventReader& event,
                                     events::EventPlayerIndices&) {
    static_cast<RebuyFixRuntime*>(owner)->player_spawn(event);
}
void RebuyFixRuntime::purchase_listener(void* owner, events::GameEventReader& event,
                                        events::EventPlayerIndices& players) {
    //The production EventMaster supplies EngineEventPlayers around the local
    //entity captured once after its pre-event observers, as in native EDX.
    const auto local = static_cast<events::EngineEventPlayers&>(players).local_entity();
    static_cast<RebuyFixRuntime*>(owner)->item_purchase(event, local);
}
void RebuyFixRuntime::bind_spawn(events::EventRegistry& registry, bool connected) {
    registry.add({spawn_hash, this, spawn_listener}, connected);
}
void RebuyFixRuntime::bind_spawn(events::EventRegistry& registry) {
    registry.add({spawn_hash, this, spawn_listener});
}
void RebuyFixRuntime::sync_purchase_subscription(events::EventRegistry& registry, bool connected) {
    const auto value = *enabled;
    if (value == purchase_subscription)
        return;
    purchase_subscription = value;
    const events::EventListener listener{rebuy_fix_purchase_hash, this, purchase_listener};
    if (value)
        registry.add(listener, connected);
    else
        registry.remove(listener, connected);
}
void RebuyFixRuntime::sync_purchase_subscription(events::EventRegistry& registry) {
    const auto value = *enabled;
    if (value == purchase_subscription)
        return;
    purchase_subscription = value;
    const events::EventListener listener{rebuy_fix_purchase_hash, this, purchase_listener};
    if (value)
        registry.add(listener);
    else
        registry.remove(listener);
}
void RebuyFixRuntime::disable_purchase_subscription(events::EventRegistry& registry) {
    if (!purchase_subscription)
        return;
    purchase_subscription = 0;
    registry.remove({rebuy_fix_purchase_hash, this, purchase_listener});
}
void RebuyFixRuntime::unbind_events(events::EventRegistry& registry, bool connected) {
    registry.remove({spawn_hash, this, spawn_listener}, connected);
    registry.remove({rebuy_fix_purchase_hash, this, purchase_listener}, connected);
    purchase_subscription = 0;
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: rebuy_fix_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_REBUY_ENTRY __fastcall
#else
#define SKEET_REBUY_ENTRY
#endif
void SKEET_REBUY_ENTRY cleanup(void* receiver, void*) {
    static_cast<RebuyPrimaryEndpoint*>(receiver)->owner->destroy();
}
void SKEET_REBUY_ENTRY empty(void*, void*) {}
void SKEET_REBUY_ENTRY frame_update(void* receiver, std::uint8_t notify) {
    static_cast<RebuyFrameEndpoint*>(receiver)->owner->frame_update(notify);
}
#undef SKEET_REBUY_ENTRY
const std::array<void*, 9> rebuy_fix_owner_primary_table{
    {reinterpret_cast<void*>(&cleanup), reinterpret_cast<void*>(&empty),
     reinterpret_cast<void*>(&empty), reinterpret_cast<void*>(&empty),
     reinterpret_cast<void*>(&empty), reinterpret_cast<void*>(&empty),
     reinterpret_cast<void*>(&empty), reinterpret_cast<void*>(&empty),
     reinterpret_cast<void*>(&empty)}};
const std::array<engine::FrameUpdateEntry, 1> rebuy_fix_owner_frame_table{{frame_update}};
}
RebuyFixOwner::RebuyFixOwner(RebuyFixOwnerBindings bindings)
    : bindings_(bindings), command_owner_(&runtime_), primary_{nullptr, this},
      frame_{{nullptr}, this} {
    // Preserve the constructor's three independent registration points.
    engine::register_owner(bindings_.owners.registry, &primary_,
                           bindings_.owners.memory); //43482185
    bindings_.command_owners.append(command_receiver_, &command_owner_); //43482192
    engine::register_frame_update(bindings_.frame_updates, &frame_); //4348219A
    primary_.vtable = rebuy_fix_owner_primary_table.data(); //434821A1
    bindings_.command_owners.publish(command_receiver_, client::CommandOwnerId::rebuy); //434821A8
    frame_.vtable = rebuy_fix_owner_frame_table.data(); //434821B1
    initialize_rebuy_config(bindings_.configuration, bindings_.settings);
    runtime_.enabled = &bindings_.settings.enabled.value;
    runtime_.engine = bindings_.miscellaneous.interfaces.engine;
    runtime_.entity_list = bindings_.miscellaneous.interfaces.entity_list;
    runtime_.buy_anywhere_cvar = bindings_.buy_anywhere_cvar;
    runtime_.local_player = bindings_.miscellaneous.interfaces.local_player;
    runtime_.current_time = bindings_.current_time;
    runtime_.time_4347a500 = bindings_.time_4347a500;
    runtime_.time_4347b118 = bindings_.time_4347b118;
    runtime_.offsets = bindings_.offsets;
    runtime_.bind_spawn(bindings_.events);
    bindings_.miscellaneous.rebuy_fix = &runtime_;
}
RebuyFixOwner::~RebuyFixOwner() {
    //4332B079 disables only the conditional item_purchase model. The native
    //player_spawn triple remains in the global listener vector until teardown.
    runtime_.disable_purchase_subscription(bindings_.events);
    if (bindings_.miscellaneous.rebuy_fix == &runtime_)
        bindings_.miscellaneous.rebuy_fix = nullptr;
    //The Runtime member's vector releases its inventory before owner storage.
}
void RebuyFixOwner::frame_update(std::uint8_t notify) {
    if (notify)
        runtime_.sync_purchase_subscription(bindings_.events);
}
void RebuyFixOwner::destroy() {
    const auto memory = bindings_.owners.memory;
    std::destroy_at(this);
    engine::release_owner_storage(memory, this);
}
RebuyFixOwner* create_rebuy_fix_owner(RebuyFixOwnerBindings bindings) {
    const auto size = std::max(std::size_t(0x54), sizeof(RebuyFixOwner));
    auto* const storage = engine::allocate_owner_storage(bindings.owners.memory, size);
    // The original allocation-failure edge reaches unresolved INT1; source allocation failure terminates this factory with bad_alloc.
    if (!storage)
        throw std::bad_alloc{};
    return ::new (storage) RebuyFixOwner(bindings);
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: reveal_overwatch.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define REVEAL_MEMBER __thiscall
#define REVEAL_FAST __fastcall
#else
#define REVEAL_MEMBER
#define REVEAL_FAST
#endif
using OwnerEntry = void(REVEAL_MEMBER*)(void*);
struct RevealOverwatchOwner;
struct RevealUpdateEndpoint : network::UpdateObserver {
    RevealOverwatchOwner* owner;
};

void REVEAL_MEMBER reveal_overwatch_destroy_owner(void*);
void REVEAL_MEMBER reveal_overwatch_reset_owner(void*);
void REVEAL_MEMBER reveal_overwatch_forward_reset_owner(void*);
void REVEAL_MEMBER reveal_overwatch_unchanged(void*) {} //433CCE8E, ret0
void REVEAL_MEMBER reveal_overwatch_begin_update(void*);
void REVEAL_MEMBER reveal_overwatch_end_update(void*);
void REVEAL_FAST unchanged_player_view(void*, void*, std::uintptr_t, std::uintptr_t,
                                       std::uintptr_t) {} //4337D653, ret12
void REVEAL_FAST unchanged_view(void*, float*, float*, float*) {} //4334A269, ret8
void REVEAL_FAST unchanged_post_view(void*, void*) {} //433CCE8E, ret0

const std::array<OwnerEntry, 9> reveal_overwatch_owner_table{reveal_overwatch_destroy_owner, reveal_overwatch_unchanged,           reveal_overwatch_unchanged,
                                            reveal_overwatch_unchanged,     reveal_overwatch_unchanged,           reveal_overwatch_reset_owner,
                                            reveal_overwatch_unchanged,     reveal_overwatch_forward_reset_owner, reveal_overwatch_unchanged};
const network::UpdateObserverVtable update_table{
    {reveal_overwatch_unchanged, reveal_overwatch_end_update, reveal_overwatch_unchanged, reveal_overwatch_unchanged, reveal_overwatch_begin_update},
    unchanged_player_view,
    unchanged_view,
    unchanged_post_view};

struct RevealOverwatchOwner {
    const OwnerEntry* vtable = nullptr;
    std::uintptr_t reserved = 0;
    RevealUpdateEndpoint update{{nullptr}, this};
    RevealOverwatchRuntime runtime;
    engine::JobMemory memory;

    RevealOverwatchOwner(engine::OwnerRegistryBinding owners, engine::OwnerRegistryBinding updates,
                         ConfigRegistries& registries, RevealOverwatchConfig& config,
                         void* const& engine_object)
        : runtime(engine_object), memory(owners.memory) {
        owners.append(this); //433676A0, before either vtable is installed
        network::register_update_observer(updates, update); //433B8C1C, owner+8
        vtable = reveal_overwatch_owner_table.data();
        update.vtable = &update_table;
        initialize_reveal_overwatch_config(registries, config);
        runtime.enabled = &config.enabled.value;
        runtime.pending = 0;
    }
};
void REVEAL_MEMBER reveal_overwatch_destroy_owner(void* object) { //433B1A7A ->433837D0
    auto* owner = static_cast<RevealOverwatchOwner*>(object);
    const auto memory = owner->memory;
    owner->~RevealOverwatchOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void REVEAL_MEMBER reveal_overwatch_reset_owner(void* object) {
    static_cast<RevealOverwatchOwner*>(object)->runtime.reset();
}
void REVEAL_MEMBER reveal_overwatch_forward_reset_owner(void* object) { //433CF2F5
    engine::invoke<void>(object, 5);
}
void REVEAL_MEMBER reveal_overwatch_begin_update(void* object) {
    static_cast<RevealUpdateEndpoint*>(object)->owner->runtime.begin_network_update();
}
void REVEAL_MEMBER reveal_overwatch_end_update(void* object) {
    static_cast<RevealUpdateEndpoint*>(object)->owner->runtime.end_network_update();
}

// deduplicated field
void set_anonymous(void* parameters, std::uint8_t value) {
    std::memcpy(static_cast<std::byte*>(parameters) + 0x10, &value, sizeof value);
}
}

void RevealOverwatchRuntime::begin_network_update() {
    if (!*enabled)
        return;
    auto* parameters = engine::invoke<void*>(engine, 218);
    if (!parameters)
        return;
    const auto anonymous = field<std::uint8_t>(parameters, 0x10);
    if (!anonymous || field<std::int32_t>(parameters, 0x0C) == 0)
        return;
    pending = anonymous;
    set_anonymous(parameters, 0);
}
void RevealOverwatchRuntime::end_network_update() {
    if (!pending)
        return;
    auto* parameters = engine::invoke<void*>(engine, 218);
    if (!parameters)
        return;
    // Native restores literal1, even if the captured nonzero byte was different.
    set_anonymous(parameters, 1);
    pending = 0;
}
void RevealOverwatchRuntime::reset() noexcept {
    pending = 0;
}

std::size_t reveal_overwatch_owner_size() noexcept {
    return sizeof(RevealOverwatchOwner);
}
RevealOverwatchRuntime&
construct_reveal_overwatch(void* storage, engine::OwnerRegistryBinding owners,
                           engine::OwnerRegistryBinding updates, ConfigRegistries& registries,
                           RevealOverwatchConfig& config, void* const& engine_object) {
    auto* owner =
        ::new (storage) RevealOverwatchOwner(owners, updates, registries, config, engine_object);
    return owner->runtime;
}

#undef REVEAL_MEMBER
#undef REVEAL_FAST
} // namespace skeet::misc

// ============================================================================
// Subcomponent: reveal_ranks.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::misc {
namespace {
// deduplicated field
std::uint32_t command_name_hash(const char* text) { //43359C70
    std::uint32_t hash = 0x811C9DC5;
    if (text)
        for (auto* byte = reinterpret_cast<const unsigned char*>(text); *byte; ++byte)
            hash = (hash ^ *byte) * 0x01000193;
    return hash ^ 0x92CA5DF1;
}
void exchange_command(RevealRanksCommand* target, RevealRanksCommand value) {
    // Recorded kernel32!InterlockedExchange75BFD980 exchanges a native x86
    // pointer. Use the pointer-sized Windows operation for the semantic view.
    InterlockedExchangePointer(reinterpret_cast<void* volatile*>(target),
                               reinterpret_cast<void*>(value));
}
RevealRanksRuntime* command_owner = nullptr;

#if defined(_M_IX86) || defined(__i386__)
#define RANKS_MEMBER __thiscall
using RankDispatch = void(__cdecl*)(std::uint32_t*);
#else
#define RANKS_MEMBER
using RankDispatch = void (*)(std::uint32_t*);
#endif
using OwnerEntry = void(RANKS_MEMBER*)(void*);
struct RevealRanksInstance;
void RANKS_MEMBER reveal_ranks_destroy_owner(void*);
void RANKS_MEMBER reveal_ranks_install_owner(void*);
void RANKS_MEMBER reveal_ranks_restore_owner(void*);
void RANKS_MEMBER reveal_ranks_unchanged_owner(void*) {} //433CCE8E is exactly RET.
const std::array<OwnerEntry, 9> reveal_ranks_owner_table{reveal_ranks_destroy_owner,   reveal_ranks_unchanged_owner, reveal_ranks_unchanged_owner,
                                            reveal_ranks_install_owner,   reveal_ranks_restore_owner,   reveal_ranks_unchanged_owner,
                                            reveal_ranks_unchanged_owner, reveal_ranks_unchanged_owner, reveal_ranks_unchanged_owner};

void* reveal_ranks_allocate_owner(engine::JobMemory memory, std::size_t size) { //43387448
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    const auto allocation_size = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void reveal_ranks_free_owner(engine::JobMemory memory, void* object) { //433837D0
    auto* bytes = static_cast<std::byte*>(object);
    const auto displacement = field<std::uint32_t>(bytes, -8);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
struct RevealRanksInstance {
    const OwnerEntry* vtable = nullptr;
    RevealRanksRuntime** active;
    engine::JobMemory memory;
    alignas(RevealRanksRuntime) std::array<std::byte, sizeof(RevealRanksRuntime)> state;

    RevealRanksInstance(engine::OwnerRegistryBinding owners, ConfigRegistries& registry,
                        RevealRanksConfig& configuration, RevealRanksRuntime*& current,
                        RevealRanksBindings bindings)
        : active(&current), memory(owners.memory) {
        owners.append(this); //433676A0 precedes vtable/configuration initialization.
        vtable = reveal_ranks_owner_table.data();
        initialize_reveal_ranks_config(registry, configuration);
        ::new (state.data()) RevealRanksRuntime(configuration, bindings);
    }
    RevealRanksRuntime& runtime() noexcept {
        return *std::launder(reinterpret_cast<RevealRanksRuntime*>(state.data()));
    }
    ~RevealRanksInstance() {
        runtime().~RevealRanksRuntime();
    }
};
void RANKS_MEMBER reveal_ranks_destroy_owner(void* object) { //433F1E4F
    auto* instance = static_cast<RevealRanksInstance*>(object);
    const auto memory = instance->memory;
    *instance->active = nullptr;
    command_owner = nullptr;
    instance->~RevealRanksInstance();
    reveal_ranks_free_owner(memory, instance);
}
void RANKS_MEMBER reveal_ranks_install_owner(void* object) {
    static_cast<RevealRanksInstance*>(object)->runtime().install_hooks();
}
void RANKS_MEMBER reveal_ranks_restore_owner(void* object) {
    static_cast<RevealRanksInstance*>(object)->runtime().restore_hooks();
}
#undef RANKS_MEMBER
}

void* find_reveal_ranks_command(void* cvar, std::uint32_t name_hash) {
    for (auto* command = field<void*>(cvar, 0x30); command; command = field<void*>(command, 4)) {
        if (command_name_hash(engine::invoke<const char*>(command, 6)) == name_hash)
            return command;
    }
    return nullptr;
}

RevealRanksRuntime::RevealRanksRuntime(RevealRanksConfig& configuration,
                                       RevealRanksBindings bindings)
    : enabled(&configuration.enabled.value), bindings(bindings) {
    if (!bindings.context)
        return;
    commands[0] = find_reveal_ranks_command(bindings.cvar, 0x48DAD9F1); //+showscores
    commands[1] = find_reveal_ranks_command(bindings.cvar, 0x667FF061); //+score
}

void RevealRanksRuntime::install_hooks() {
    if (!bindings.context || !bindings.rank_data)
        return;
    // The supported engine dispatches these modern ConCommand callbacks with
    // a CCommand reference. Validate both union tags before changing either.
    for (auto* command : commands)
        if (command && (field<std::uint8_t>(command, 0x20) & 6u) != 2u)
            throw std::runtime_error("Score command requires the CCommand callback ABI");
    for (std::size_t index = 0; index != commands.size(); ++index) {
        if (!commands[index])
            continue;
        auto& hook = hooks[index];
        hook.target =
            reinterpret_cast<RevealRanksCommand*>(static_cast<std::byte*>(commands[index]) + 0x18);
        hook.saved = *hook.target;
        hook.replacement = reveal_ranks_score_command;
        exchange_command(hook.target, hook.replacement);
    }
}

void RevealRanksRuntime::restore_hooks() {
    for (auto& hook : hooks) {
        if (hook.target && hook.saved)
            exchange_command(hook.target, hook.saved);
        hook = {};
    }
}

void RevealRanksRuntime::on_score_command(const void* command_arguments) {
    // Native uses4347BDCC for both installed commands. Its zero-argument
    // declaration loses the current engine's required CCommand argument.
    hooks[0].saved(command_arguments);
    if (!*enabled || !bindings.context || !bindings.rank_data || !bindings.game_rules)
        return;
    auto* rules = *bindings.game_rules;
    if (!rules || !field<std::uint8_t>(rules, bindings.valve_matchmaking))
        return;
    auto* callbacks = field<void*>(bindings.context, 0x74);
    if (!callbacks)
        return;
    const auto dispatch = field<RankDispatch>(callbacks, 4);
    if (!dispatch)
        return;
    std::array<std::uint32_t, 64> ranks;
    ranks.fill(10);
    auto* data = *bindings.rank_data;
    ranks[2] =
        data ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(data) + 0x140u) : 0;
    dispatch(ranks.data());
}

void reveal_ranks_score_command(const void* command_arguments) {
    command_owner->on_score_command(command_arguments);
}

void initialize_reveal_ranks(engine::OwnerRegistryBinding owners, ConfigRegistries& registry,
                             RevealRanksConfig& configuration, RevealRanksRuntime*& active,
                             RevealRanksBindings bindings) {
    auto* storage = reveal_ranks_allocate_owner(owners.memory, sizeof(RevealRanksInstance));
    if (!storage) {
        active = nullptr;
        command_owner = nullptr;
        return;
    }
    auto* instance =
        ::new (storage) RevealRanksInstance(owners, registry, configuration, active, bindings);
    active = &instance->runtime();
    command_owner = active;
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: runtime.cpp
// ============================================================================

namespace skeet::misc {
using engine::invoke;
namespace {
// runtime_read
template <class T> T runtime_read(const void* object, std::uint32_t offset) { T val; std::memcpy(&val, static_cast<const std::byte*>(object) + offset, sizeof val); return val; }
template <class T> void runtime_write(void* object, std::ptrdiff_t offset, T value) { std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(T)); }
const char* command_line_import() {
    return GetCommandLineA();
}
int format_name_import(char* destination, std::size_t capacity, const char* name) {
    return sprintf_s(destination, capacity, "name %s", name);
}
int suffix_import(char* destination, int capacity) {
    return WideCharToMultiByte(CP_UTF8, 0, L"~\u3164~\u3164", -1, destination, capacity, nullptr,
                               nullptr);
}
void print_wins(void* cvar, const char* name, std::int32_t wins) {
    // ICvar::ConsolePrintf is a cdecl variadic entry, including its receiver on
    // the stack. It is not a thiscall despite being stored in a vtable.
    const auto table = *static_cast<void***>(cvar);
    using Function = void (*)(void*, const char*, ...);
    reinterpret_cast<Function>(table[26])(cvar, "    %s: %d\n", name, wins);
}
}

RuntimeImports::RuntimeImports()
    : command_line(command_line_import), format_name_command(format_name_import),
      append_name_suffix(suffix_import) {}
Runtime::Runtime(RuntimeInterfaces& interfaces, AutoAcceptPatchState& patch,
                 client::ChannelHookState& channel_state, RuntimeOffsets offsets)
    : interfaces(interfaces), offsets(offsets), auto_accept_patch(patch), channel(channel_state) {
    error_log.cvar = interfaces.cvar;
    error_log.disabled = interfaces.drawing_suspended;
    error_log.hud_slot = interfaces.log_hud_slot;
    clan_tag.engine = interfaces.engine;
    clan_tag.game_rules = interfaces.game_rules;
    clan_tag.game_phase_offset = offsets.game_phase;
    ping_spike.engine = interfaces.engine;
}

void Runtime::auto_accept_changed() {
    auto& patch = auto_accept_patch.metadata;
    if (!patch.replacement_table || !patch.target)
        return;
    InterlockedExchangePointer(static_cast<void* volatile*>(patch.target),
                               *auto_accept_enabled ? patch.replacement_table
                                                    : patch.original_table);
}

void Runtime::ping_spike_changed(bool enabled) {
    if (!enabled)
        return;
    client::restore_channel_hook(interfaces.engine, channel); //complete shared433CBDFD
}
void Runtime::restore_ping_channel() {
    if (!channel.replacement_table)
        return;
    auto* active = invoke<void*>(interfaces.engine, 78);
    if (!active || *static_cast<void***>(active) != channel.replacement_table)
        return;
    InterlockedExchangePointer(static_cast<void* volatile*>(active), channel.expected_table);
}

void Runtime::steal_player_name() {
    if (!invoke<bool>(interfaces.engine, 26))
        return;
    const auto maximum = *interfaces.max_clients;
    auto* local = interfaces.local_player;
    const auto local_index = invoke<std::int32_t>(static_cast<std::byte*>(local) + 8, 10);
    runtime_write<std::uint32_t>(interfaces.name_cvar, 0x60, 0);
    for (std::int32_t attempt = 0; attempt < maximum; ++attempt) {
        // x86 ADD wraps before signed IDIV. This matters for a stale saved index.
        const auto sum = std::uint32_t(last_name_index) + std::uint32_t(attempt);
        const auto signed_sum = static_cast<std::int32_t>(sum);
        const auto index = signed_sum % maximum + 1;
        if (index == local_index)
            continue;
        auto* player = invoke<void*>(interfaces.entity_list, 3, index);
        if (!player || !invoke<bool>(player, 158))
            continue;
        if (runtime_read<std::int32_t>(player, offsets.team) != runtime_read<std::int32_t>(local, offsets.team))
            continue;
        PlayerInfo info{};
        if (!invoke<bool>(interfaces.engine, 8, index, &info) || info.is_hltv())
            continue;
        std::array<char, 256> command{};
        const auto written =
            imports.format_name_command(command.data(), command.size(), info.name());
        if (written > 0) {
            imports.append_name_suffix(command.data() + written,
                                       static_cast<int>(command.size()) - written);
            command.back() = '\0';
            invoke<void>(interfaces.engine, 108, command.data());
        }
        last_name_index = index;
        return;
    }
}

void Runtime::dump_matchmaking_wins() {
    if (!interfaces.game_rules || !*interfaces.game_rules)
        return;
    if (!runtime_read<std::uint8_t>(*interfaces.game_rules, offsets.valve_matchmaking)) {
        error_log.cvar = interfaces.cvar;
        error_log.write("You must be in a Valve matchmaking server.");
        return;
    }
    if (!interfaces.player_resource || !*interfaces.player_resource)
        return;
    auto* resource = *interfaces.player_resource;
    const auto maximum = *interfaces.max_clients;
    for (std::int32_t index = 1; index <= maximum; ++index) {
        auto* player = invoke<void*>(interfaces.entity_list, 3, index);
        if (!player || !invoke<bool>(player, 158))
            continue;
        PlayerInfo info;
        if (!invoke<bool>(interfaces.engine, 8, index, &info) || info.is_hltv())
            continue;
        const auto wins =
            runtime_read<std::int32_t>(resource, offsets.competitive_wins + std::uint32_t(index) * 4);
        print_wins(interfaces.cvar, info.name(), wins);
    }
}

void Runtime::upgrade_tablet_changed() {
    const auto* command_line = imports.command_line();
    if (!command_line || !std::strstr(command_line, "-insecure"))
        return;
    if (!*upgrade_tablet_enabled) {
        for (const auto& entry : tablet_flags) {
            auto* parent = runtime_read<void*>(entry.variable, 0x1C);
            runtime_write(parent, 0x14, entry.flags);
            parent = runtime_read<void*>(entry.variable, 0x1C);
            runtime_write(parent, 8, entry.flags);
        }
        tablet_flags.clear();
        return;
    }
    if (!tablet_flags.empty())
        return;
    auto* iterator = invoke<void*>(interfaces.cvar, 45);
    invoke<void>(iterator, 0);
    while (invoke<bool>(iterator, 2)) {
        auto* variable = invoke<void*>(iterator, 3);
        if (!invoke<bool>(variable, 1) && (invoke<std::uint32_t>(variable, 5) & 0x4012)) {
            const auto original = invoke<std::uint32_t>(variable, 5);
            tablet_flags.push_back({variable, original});
            auto* parent = runtime_read<void*>(variable, 0x1C);
            const auto mask = ~std::uint32_t(0x4012);
            runtime_write(parent, 0x14, runtime_read<std::uint32_t>(parent, 0x14) & mask);
            parent = runtime_read<void*>(variable, 0x1C);
            runtime_write(parent, 8, runtime_read<std::uint32_t>(parent, 8) & mask);
        }
        invoke<void>(iterator, 1);
    }
    invoke<void>(interfaces.allocator, 5, iterator);
}

} // namespace skeet::misc

// ============================================================================
// Subcomponent: settings_runtime.cpp
// ============================================================================

namespace skeet::misc {
namespace {
void restore_grid(ui::Category& category, std::uint32_t packed) {
    category.column = std::bit_cast<std::int8_t>(std::uint8_t(packed));
    category.column_span = std::bit_cast<std::int8_t>(std::uint8_t(packed >> 8));
    category.row = std::bit_cast<std::int8_t>(std::uint8_t(packed >> 16));
    category.row_span = std::bit_cast<std::int8_t>(std::uint8_t(packed >> 24));
}
}

SettingsRuntime::SettingsRuntime() noexcept
    : previous_dpi_scale(standalone_previous_dpi_), previous_lock_layout(standalone_previous_lock_),
      previous_custom_events(standalone_previous_custom_events_), engine(standalone_engine_),
      custom_event_state(standalone_event_manager_), default_layout(standalone_layout_) {}
SettingsRuntime::SettingsRuntime(std::int32_t& previous_dpi, std::uint8_t& previous_lock,
                                 std::uint8_t& previous_custom, void*& engine_client,
                                 void*& event_manager, std::array<std::int32_t, 21>& layout,
                                 std::array<std::uint32_t, 2>& additional) noexcept
    : reset_targets(additional), previous_dpi_scale(previous_dpi),
      previous_lock_layout(previous_lock), previous_custom_events(previous_custom),
      engine(engine_client), custom_event_state(event_manager), default_layout(layout) {}

void initialize_source_settings_runtime_data(engine::NativeDataImage& data) {
    data.native_cell<std::int32_t>(0x4347CA04u) = 0;
    data.native_cell<std::uint8_t>(0x4346D550u) = 1;
    data.native_cell<std::uint8_t>(0x4347CA38u) = 0;
    data.native_cell<std::array<std::int32_t, 21>>(0x43468CB0u) = {9};
    data.native_cell<std::array<std::uint32_t, 2>>(0x43476B34u) = {};
}

void SettingsRuntime::dpi_scale_changed() { // 0x433379D4
    gui_dpi->refresh(false);
    lock_layout_changed();
}

void SettingsRuntime::lock_layout_changed() { // 0x4332A6C5
    const auto current_dpi = *dpi_scale;
    const auto current_lock = *lock_layout;
    if (previous_dpi_scale == current_dpi && previous_lock_layout == current_lock)
        return;
    previous_dpi_scale = current_dpi;
    previous_lock_layout = current_lock;
    persistence->persist(0, 0); // 0x4340B9C9, module gate and exact registry snapshot
}

void SettingsRuntime::allow_custom_events_changed() { // 0x4333D25B
    // Native4333D25B treats an absent widget value pointer as zero.
    const auto value = allow_custom_events ? *allow_custom_events : std::uint8_t{};
    if (value != previous_custom_events && engine::invoke<bool>(engine, 26))
        *(static_cast<std::uint8_t*>(custom_event_state) + 0xA0) = 1;
    previous_custom_events = value;
}

void SettingsRuntime::reset_menu_layout() { // 0x43342FF1 with native built-in block
    if (default_layout[0] == 9) {
        // The original intentionally ignores record slots 1, 2 and 16..18.
        constexpr std::size_t indices[] = {3, 5, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 19, 20};
        for (std::size_t i = 0; i < std::size(indices); ++i)
            restore_grid(*reset_targets.categories[i], std::uint32_t(default_layout[indices[i]]));
        for (std::size_t i = 0; i < reset_targets.additional_categories.size(); ++i)
            restore_grid(*reset_targets.additional_categories[i],
                         reset_targets.additional_defaults[i]);
    }
    for (auto* tab : main_menu->tabs) {
        tab->layout_categories();
        tab->reflow_categories();
        tab->resize(main_menu->input.slider_metrics);
    }
}

void SettingsRuntime::max_usercmd_ticks_changed(std::int32_t value) { // 0x433EE188
    const auto decremented = std::bit_cast<std::int32_t>(std::uint32_t(value) - 1u);
    const auto maximum = std::clamp(decremented, 2, 60);
    fakelag_limit->maximum = maximum;
    auto* setting = fakelag_limit->value;
    if (setting && maximum < *setting)
        __atomic_exchange_n(setting, maximum, __ATOMIC_SEQ_CST);
    fakelag_limit->value_changed();
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: settings_runtime_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
client::ClientRuntimeStorage& require_owners(client::ClientRuntimeStorage& storage,
                                             client::ConfigurationRuntimeOwner& settings,
                                             ui::MainMenuWindowOwner& window,
                                             ConfigStorageOwner& configuration,
                                             UnloadRuntime& unload) {
    auto& data = storage.data();
    auto& menu = window.window();
    if (&settings.storage() != &storage || &window.menu_data().data() != &data ||
        &configuration.native_storage() != &storage.config_storage() ||
        &configuration.configuration() != &storage.configuration() ||
        &storage.menu_startup_data().dpi != &window.menu_data().dpi || &unload.main_menu != &menu ||
        unload.input != &storage.input_runtime() || storage.input_runtime().main_menu != &menu ||
        unload.config_registry != &storage.configuration().descriptors || !unload.hooks ||
        &unload.hooks->state.owners != &storage.owner_registries().at(0x43475B78u) ||
        &unload.hooks->state.engine != &data.native_cell<void*>(0x4346A514u))
        throw std::invalid_argument(
            "Settings callbacks require the same source window, CONFIG, input and unload owners");
    if (!menu.event_state || !data.native_cell<void*>(0x4346A514u) ||
        !data.native_cell<void*>(0x43468364u) ||
        &settings.settings_menu().dpi_scale != &window.menu_data().dpi.selection ||
        &settings.settings_menu().lock_layout != &menu.event_state->window_size_locked ||
        &settings.custom_events().enabled.value != &data.native_cell<std::uint8_t>(0x434691B8u) ||
        data.native_cell<std::array<std::int32_t, 21>>(0x43468CB0u)[0] != 9)
        throw std::invalid_argument(
            "Settings callbacks require initialized canonical engine, setting and default-layout cells");
    return storage;
}
}
SettingsRuntimeOwner::SettingsRuntimeOwner(client::ClientRuntimeStorage& storage,
                                           client::ConfigurationRuntimeOwner& settings,
                                           ui::MainMenuWindowOwner& window,
                                           ConfigStorageOwner& configuration, UnloadRuntime& unload)
    : storage_(require_owners(storage, settings, window, configuration, unload)), window_(window),
      configuration_(configuration),
      runtime_(storage.data().native_cell<std::int32_t>(0x4347CA04u),
               storage.data().native_cell<std::uint8_t>(0x4346D550u),
               storage.data().native_cell<std::uint8_t>(0x4347CA38u),
               storage.data().native_cell<void*>(0x4346A514u),
               storage.data().native_cell<void*>(0x43468364u),
               storage.data().native_cell<std::array<std::int32_t, 21>>(0x43468CB0u),
               storage.data().native_cell<std::array<std::uint32_t, 2>>(0x43476B34u)) {
    runtime_.gui_dpi = &window.menu_data().dpi;
    runtime_.main_menu = &window.window();
    runtime_.persistence = &configuration.persistence();
    runtime_.unload = &unload;
    runtime_.dpi_scale = &settings.settings_menu().dpi_scale;
    runtime_.lock_layout = &settings.settings_menu().lock_layout;
    runtime_.allow_custom_events = &settings.custom_events().enabled.value;
}
void SettingsRuntimeOwner::capture_default_layout(ui::RootMenu& menu) {
    if (layout_captured_)
        throw std::logic_error("The original menu default layout has already been captured");
    if (&menu.window() != &window_.window() || runtime_.main_menu != &menu.window() ||
        !runtime_.fakelag_limit)
        throw std::invalid_argument(
            "Default layout capture requires the actual constructed RootMenu");
    const auto targets = menu.layout_targets();
    constexpr std::array<std::size_t, 15> indices{2,  3,  4,  5,  6,  7,  8, 9,
                                                  10, 11, 12, 13, 14, 15, 16};
    for (std::size_t index = 0; index < indices.size(); ++index)
        if (!runtime_.reset_targets.categories[index] ||
            runtime_.reset_targets.categories[index] != targets.categories[indices[index]])
            throw std::invalid_argument(
                "Default layout capture requires RootMenu's own category bindings");
    for (std::size_t index = 0; index < targets.players.size(); ++index)
        if (!runtime_.reset_targets.additional_categories[index] ||
            runtime_.reset_targets.additional_categories[index] != targets.players[index])
            throw std::invalid_argument(
                "Default layout capture requires RootMenu's own player categories");
    menu.capture_layout(runtime_.default_layout);
    layout_captured_ = true;
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: unload.cpp
// ============================================================================
namespace skeet::misc {
void UnloadRuntime::request() { // 0x433A0637
    if (!timer_callback)
        throw std::logic_error("Client unload profile has not been selected");
    if (!main_menu.native_window)
        throw std::invalid_argument("Client unload main window is unresolved");
    if (input->main_menu != &main_menu)
        throw std::invalid_argument("Client unload and input must share the main menu window");
    if (request_gate.allow && !request_gate.allow(request_gate.context))
        return;
    main_menu.transition_state = 0;
    main_menu.input.mouse_flags &= ~std::uint32_t(1);
    if (__atomic_exchange_n(&input->disabled, 1, __ATOMIC_SEQ_CST) == 1)
        return;
    (void)deserialize_config_registry(*config_registry, baseline_config.bytes(),
                                      *preserve_hotkey_toggle != 0);
    refresh_bound_settings();
    input->release_cursor();
    input->destroy_cursor();
    hooks->cleanup();
    const auto window = main_menu.native_window;
    if (!window)
        throw std::invalid_argument("Client unload main window was cleared during cleanup");
    (void)imports.set_timer(window, 0x332, 250, timer_callback);
}

void SettingsRuntime::request_unload() {
    unload->request();
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: unload_state.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::misc {
namespace {
std::uintptr_t set_timer_import(std::uintptr_t window, std::uintptr_t identifier,
                                std::uint32_t interval, UnloadTimerCallback callback) {
    return SetTimer(reinterpret_cast<HWND>(window), identifier, interval,
                    reinterpret_cast<TIMERPROC>(callback));
}
}
UnloadImports::UnloadImports() : set_timer(set_timer_import) {}

void UnloadRuntime::refresh_bound_settings() { // 0x433AB1C2 / 0x4337EABC
    const auto stream = current_config.bytes(); // capture original ECX/EDX range before callbacks
    if (stream.empty())
        return;
    if (source_ui_) {
        script::restore_script_ui_values(*source_ui_, stream, source_preserve_toggle_);
        return;
    }
    for (const auto& binding : config_bindings) {
        if (binding.disabled || !binding.descriptor)
            continue;
        const bool changed =
            deserialize_config_value(*binding.descriptor, stream, *preserve_hotkey_toggle != 0);
        if (binding.widget) {
            binding.widget->config_loaded();
            if (changed)
                binding.widget->value_changed();
        }
    }
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: viewmodel_hooks.cpp
// ============================================================================

namespace skeet::misc {
namespace {
ViewmodelHooksRuntime* active_runtime = nullptr;
// deduplicated field
template <class T> void viewmodel_hooks_assign(void* object, std::ptrdiff_t offset, T value) { std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(T)); }
const engine::VtableHookMetadata& viewmodel_hooks_metadata(void* receiver) {
    const auto* table = *static_cast<void***>(receiver);
    return *static_cast<const engine::VtableHookMetadata*>(table[-2]);
}
}
std::span<const ViewmodelHookReplacement, 6> viewmodel_hook_replacements() {
    static const std::array<ViewmodelHookReplacement, 6> replacements{{
        {0, 0x43402028, reinterpret_cast<void*>(&viewmodel_destroy_hook)},
        {206, 0x43359D27, reinterpret_cast<void*>(&viewmodel_206_hook)},
        {113, 0x43324F45, reinterpret_cast<void*>(&viewmodel_113_hook)},
        {247, 0x4336BA27, reinterpret_cast<void*>(&viewmodel_247_hook)},
        {248, 0x4340821A, reinterpret_cast<void*>(&viewmodel_248_hook)},
        {250, 0x433B0541, reinterpret_cast<void*>(&viewmodel_250_hook)},
    }};
    return replacements;
}
ViewmodelHooksRuntime::ViewmodelHooksRuntime(ViewmodelLagState& owner_state,
                                             ViewmodelLagOwner*& owner,
                                             const ui::SettingsMenuValues& settings,
                                             ViewmodelHookInputs inputs, effects::ViewRuntime& view,
                                             engine::VtableHookRegistry& registry,
                                             engine::HookDescriptorStorage& descriptors)
    : owner_state_(owner_state), owner_(owner), enabled_(settings.fix_viewmodel_lag),
      inputs_(inputs), view_(view), registry_(registry), image_(descriptors.image()),
      descriptor_(descriptors.at(viewmodel_hook_descriptor)), feature_{enabled_,
                                                                       inputs_.engine,
                                                                       inputs_.entity_list,
                                                                       &inputs_.prediction_time,
                                                                       &inputs_.interval,
                                                                       inputs_.interpolation_cvar,
                                                                       inputs_.offsets} {
    if (&owner_ != &image_.native_cell<ViewmodelLagOwner*>(viewmodel_lag_owner_slot))
        throw std::invalid_argument(
            "Viewmodel hooks require canonical retained-image owner slot43475860");
}
const ViewmodelLagBindings& ViewmodelHooksRuntime::feature_bindings() const noexcept {
    return feature_;
}
ViewmodelLagHookBindings
ViewmodelHooksRuntime::hook_bindings(const ViewmodelLagBindings& feature) const {
    return {feature,
            inputs_.input,
            inputs_.local_player,
            inputs_.player_tick_base,
            &inputs_.prediction_time,
            &inputs_.interpolation_fraction};
}
void ViewmodelHooksRuntime::prepare(void** original_table, engine::JobMemory memory,
                                    engine::HookDescriptorSystem system) {
    engine::prepare_vtable_hook(descriptor_, original_table, memory, system);
    bind_replacements();
}
void ViewmodelHooksRuntime::prepare_from_module(std::byte* module,
                                                engine::NativeImageEnvironment images,
                                                engine::JobMemory memory,
                                                engine::HookDescriptorSystem system) {
    // This is the whole source recipe14 operation. It is not inserted before
    // the preserved VM's separate raw replacement-operand loop.
    engine::prepare_rtti_vtable_hook(descriptor_, module, viewmodel_hook_class_hash, 0, images,
                                     memory, system);
    bind_replacements();
}
void ViewmodelHooksRuntime::bind_replacements() noexcept {
    // Publish before the atomic swap; native may install the vtable even when
    // allocating its cleanup-list node subsequently fails.
    active_runtime = this;
    for (const auto& replacement : viewmodel_hook_replacements())
        descriptor_.replacement_table[replacement.slot] = replacement.source_entry;
}
engine::VtableHookNode* ViewmodelHooksRuntime::install(void* receiver) {
    return engine::install_vtable_hook(registry_, receiver, descriptor_);
}
std::uintptr_t ViewmodelHooksRuntime::destroy(void* receiver, std::uint32_t argument) { //43402028
    engine::uninstall_vtable_hook(registry_, receiver);
    // Native rereads the receiver after restoring its table and invokes slot0
    // through433E533F. The engine deleting-destructor argument is unchanged.
    return engine::invoke<std::uintptr_t>(receiver, 0, argument);
}
bool ViewmodelHooksRuntime::method113(void* receiver, float argument) {
    const auto feature = feature_bindings();
    return viewmodel_method_113(viewmodel_lag_owner_state(owner_), hook_bindings(feature), receiver,
                                argument);
}
std::uint32_t ViewmodelHooksRuntime::method206(void* receiver, std::uint32_t first,
                                               std::uint32_t second, std::uint32_t third,
                                               std::uint32_t fourth, float fifth) {
    const auto feature = feature_bindings();
    return viewmodel_method_206(viewmodel_lag_owner_state(owner_), hook_bindings(feature), receiver,
                                {first, second, third, fourth}, fifth);
}
void ViewmodelHooksRuntime::method247(void* receiver, std::int32_t argument) {
    const auto feature = feature_bindings();
    viewmodel_method_247(viewmodel_lag_owner_state(owner_), hook_bindings(feature), receiver,
                         argument);
}
std::uint32_t ViewmodelHooksRuntime::method248(void* receiver, std::uint32_t first,
                                               std::uint32_t second) { //4340821A
    const auto token = field<std::uint32_t>(receiver, inputs_.offsets.animation_token_0c);
    const auto previous = field<std::uint32_t>(receiver, inputs_.offsets.field_18);
    const auto original =
        reinterpret_cast<engine::MemberFunction<std::uint32_t, std::uint32_t, std::uint32_t>>(
            viewmodel_hooks_metadata(receiver).original_table[248]);
    const auto result = original(receiver, first, second);
    viewmodel_hooks_assign(receiver, inputs_.offsets.field_18, previous);
    viewmodel_hooks_assign(receiver, inputs_.offsets.animation_token_0c, token);
    return result;
}
void ViewmodelHooksRuntime::method250(void* receiver, std::uintptr_t first, std::uintptr_t second,
                                      std::uintptr_t third) {
    view_.recoil_scale_hooked(receiver, inputs_.encoded_recoil_scale, first, second, third);
}
void ViewmodelHooksRuntime::update_entities(void* player, std::uintptr_t) {
    const auto feature = feature_bindings();
    repair_viewmodel_lag(owner_state_, feature, player);
}
void ViewmodelHooksRuntime::frame_update() {
    consume_viewmodel_prediction(owner_state_);
}

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_VIEWMODEL_HOOK_ABI __fastcall
#else
#define SKEET_VIEWMODEL_HOOK_ABI
#endif
std::uintptr_t SKEET_VIEWMODEL_HOOK_ABI viewmodel_destroy_hook(void* receiver, void*,
                                                               std::uint32_t argument) {
    return active_runtime->destroy(receiver, argument);
}
bool SKEET_VIEWMODEL_HOOK_ABI viewmodel_113_hook(void* receiver, void*, float argument) {
    return active_runtime->method113(receiver, argument);
}
std::uint32_t SKEET_VIEWMODEL_HOOK_ABI viewmodel_206_hook(void* receiver, void*, std::uint32_t a,
                                                          std::uint32_t b, std::uint32_t c,
                                                          std::uint32_t d, float e) {
    return active_runtime->method206(receiver, a, b, c, d, e);
}
void SKEET_VIEWMODEL_HOOK_ABI viewmodel_247_hook(void* receiver, void*, std::int32_t argument) {
    active_runtime->method247(receiver, argument);
}
std::uint32_t SKEET_VIEWMODEL_HOOK_ABI viewmodel_248_hook(void* receiver, void*,
                                                          std::uint32_t first,
                                                          std::uint32_t second) {
    return active_runtime->method248(receiver, first, second);
}
void SKEET_VIEWMODEL_HOOK_ABI viewmodel_250_hook(void* receiver, void*, std::uintptr_t first,
                                                 std::uintptr_t second, std::uintptr_t third) {
    active_runtime->method250(receiver, first, second, third);
}
#undef SKEET_VIEWMODEL_HOOK_ABI
} //namespace skeet::misc

// ============================================================================
// Subcomponent: viewmodel_lag.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::misc {
namespace {
// deduplicated field
template <class T> void viewmodel_lag_assign(void* object, std::ptrdiff_t offset, T value) { std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(T)); }
std::int32_t signed_word(std::uint32_t value) {
    return std::bit_cast<std::int32_t>(value);
}
bool available(const ViewmodelLagBindings& b) {
    return !engine::invoke<bool>(b.engine, 82) && !engine::invoke<bool>(b.engine, 93);
}
void* viewmodel(const ViewmodelLagBindings& b, void* player) {
    const auto handles = b.offsets.handles;
    for (unsigned index = 0; index != 2; ++index) {
        const auto handle = field<std::uint32_t>(player, handles + index * 4);
        if (handle == 0xFFFFFFFF)
            continue;
        auto* entity = engine::invoke<void*>(b.entity_list, 4, handle);
        if (entity && !(field<std::uint32_t>(entity, b.offsets.flags) & 0x20))
            return entity;
    }
    return nullptr;
}
bool interpolation_enabled(const void* variable) { // complete43407DE2
    for (;;) {
        const auto* parent = field<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    const auto encoded = field<std::uint32_t>(variable, 0x2C);
    return std::bit_cast<float>(encoded ^ static_cast<std::uint32_t>(
                                              reinterpret_cast<std::uintptr_t>(variable))) != 0;
}
void** original_table(void* receiver) {
    auto** installed = *static_cast<void***>(receiver);
    return static_cast<const engine::VtableHookMetadata*>(installed[-2])->original_table;
}
}
void initialize_viewmodel_lag(ViewmodelLagState& state) noexcept {
    state.previous_animation_time = 0;
    for (auto& record : state.snapshots)
        record.weapon_handle = 0xFFFFFFFF;
}
void reset_viewmodel_lag(ViewmodelLagState& state) noexcept {
    state.tick_base = state.last_command = state.prediction_generation = 0;
    for (auto& record : state.ticks)
        record.command = -1;
    for (auto& record : state.snapshots) {
        record.command = 0xFFFFFFFF;
        record.weapon_handle = 0xFFFFFFFF;
    }
}
void record_viewmodel_command(ViewmodelLagState& state, const client::Command& command) noexcept {
    state.last_command = command.number;
    state.ticks[command.number % 150] = {command.number, state.tick_base};
}
void record_viewmodel_command_range(ViewmodelLagState& state, const ViewmodelLagBindings& b,
                                    std::int32_t final_command, std::int32_t count) {
    if (!b.enabled || !available(b))
        return;
    state.last_command = final_command;
    auto command = signed_word(std::uint32_t(final_command) - std::uint32_t(count));
    while (count > 0) {
        command = signed_word(std::uint32_t(command) + 1);
        state.ticks[command % 150] = {command, state.tick_base};
        state.tick_base = signed_word(std::uint32_t(state.tick_base) + 1);
        --count;
    }
}
void advance_viewmodel_prediction(ViewmodelLagState& state, const ViewmodelLagBindings& b) {
    if (b.enabled && available(b))
        state.prediction_generation = signed_word(std::uint32_t(state.prediction_generation) + 1);
}
void consume_viewmodel_prediction(ViewmodelLagState& state) noexcept {
    if (state.prediction_generation > 0)
        --state.prediction_generation;
    else
        state.tick_base = signed_word(std::uint32_t(state.tick_base) + 1);
}
void prepare_viewmodel_prediction(ViewmodelLagState& state, const ViewmodelLagBindings& b,
                                  void* player, const client::Command& command) {
    state.command_number = command.number;
    state.previous_query = state.previous_generation = -1;
    if (!b.enabled || !available(b))
        return;
    if (auto* entity = viewmodel(b, player))
        state.previous_animation_time = field<float>(entity, b.offsets.animation_base - 8);
}
void capture_viewmodel_prediction(ViewmodelLagState& state, const ViewmodelLagBindings& b,
                                  void* player, const client::Command& command) {
    state.command_number = state.previous_query = state.previous_generation = -1;
    if (!b.enabled || !available(b))
        return;
    auto* entity = viewmodel(b, player);
    if (!entity)
        return;
    const auto animation_time = field<float>(entity, b.offsets.animation_base - 8);
    if (animation_time != state.previous_animation_time && animation_time == *b.current_time) {
        const auto& mapped = state.ticks[command.number % 150];
        if (mapped.command == command.number)
            viewmodel_lag_assign(entity, b.offsets.animation_base - 8,
                   static_cast<float>(mapped.tick_base) * *b.interval);
    }
    auto& record = state.snapshots[std::uint32_t(command.number) % 150];
    record.command = std::uint32_t(command.number);
    record.random_seed = std::uint32_t(command.random_seed);
    record.weapon_handle = field<std::uint32_t>(entity, b.offsets.weapon_handle);
    record.sequence = field<std::int32_t>(entity, b.offsets.sequence);
    record.animation_token_08 = field<std::int32_t>(entity, b.offsets.animation_token_08);
    record.animation_token_0c = field<std::int32_t>(entity, b.offsets.animation_token_0c);
    record.field_18 = field<std::uint32_t>(entity, b.offsets.field_18);
    record.player_byte_1c = field<std::uint8_t>(player, b.offsets.player_byte);
    record.selected_model = field<std::int16_t>(entity, b.offsets.selected_model);
}
void copy_previous_viewmodel_snapshot(ViewmodelLagState& state, const ViewmodelLagBindings& b,
                                      std::int32_t command_number) {
    if (!b.enabled || !available(b))
        return;
    // Slot3 receives CCommandContext's base. Its +8 is the embedded
    // Command.number, because the Command itself begins at context+4.
    const auto number = std::uint32_t(command_number);
    auto& destination = state.snapshots[number % 150];
    destination = state.snapshots[(number - 1) % 150];
    destination.command = number;
}
void repair_viewmodel_lag(ViewmodelLagState& state, const ViewmodelLagBindings& b, void* player) {
    if (!b.enabled || !available(b) || !available(b) ||
        !interpolation_enabled(b.interpolation_cvar))
        return;
    auto* entity = viewmodel(b, player);
    if (!entity)
        return;
    const auto animation_time = field<float>(entity, b.offsets.animation_base - 8);
    if (animation_time != field<float>(entity, b.offsets.animation_base - 4) &&
        animation_time == *b.current_time) {
        const auto value = field<float>(entity, b.offsets.animation_value);
        if (value != field<float>(entity, b.offsets.sequence + 0x30) && value == 0) {
            const auto acknowledged = engine::invoke<std::uint32_t>(b.engine, 205);
            const auto& record = state.snapshots[acknowledged % 150];
            if (record.command == acknowledged &&
                record.weapon_handle == field<std::uint32_t>(entity, b.offsets.weapon_handle) &&
                record.sequence == field<std::int32_t>(entity, b.offsets.sequence) &&
                record.animation_token_08 ==
                    field<std::int32_t>(entity, b.offsets.animation_token_08) &&
                record.animation_token_0c ==
                    field<std::int32_t>(entity, b.offsets.animation_token_0c) &&
                record.selected_model == field<std::int16_t>(entity, b.offsets.selected_model)) {
                viewmodel_lag_assign(entity, b.offsets.animation_base - 8,
                       field<std::uint32_t>(entity, b.offsets.animation_base - 4));
                viewmodel_lag_assign(entity, b.offsets.animation_value,
                       field<std::uint32_t>(entity, b.offsets.sequence + 0x30));
            }
        }
    }
    if (field<float>(entity, b.offsets.animation_base - 8) != *b.current_time)
        return;
    const auto acknowledged = engine::invoke<std::int32_t>(b.engine, 205);
    const auto& mapped = state.ticks[acknowledged % 150];
    if (mapped.command == acknowledged)
        viewmodel_lag_assign(entity, b.offsets.animation_base - 8,
               static_cast<float>(mapped.tick_base) * *b.interval);
}
bool viewmodel_method_113(ViewmodelLagState* state, const ViewmodelLagHookBindings& b,
                          void* receiver, float argument) { // complete43324F45
    void* saved_player = nullptr;
    std::int32_t saved_tick = 0;
    if (state && b.runtime.enabled && available(b.runtime)) {
        const auto* commands = field<const client::Command*>(b.input, 0xF0);
        const auto number = std::uint32_t(state->last_command);
        const auto& command = commands[number % 150];
        if (std::uint32_t(command.number) == number && std::uint32_t(command.tick) == 0x7F7FFFFF)
            return true;
        if (b.local_player) {
            saved_player = b.local_player;
            saved_tick = field<std::int32_t>(saved_player, b.player_tick_base);
            viewmodel_lag_assign(saved_player, b.player_tick_base, state->tick_base);
        }
    }
    const auto original =
        reinterpret_cast<engine::MemberFunction<bool, float>>(original_table(receiver)[113]);
    const auto result = original(receiver, argument);
    if (saved_player)
        viewmodel_lag_assign(saved_player, b.player_tick_base, saved_tick);
    return result;
}
std::uint32_t viewmodel_method_206(ViewmodelLagState* state, const ViewmodelLagHookBindings& b,
                                   void* receiver, std::array<std::uint32_t, 4> arguments,
                                   float argument) { // complete43359D27
    float* saved_target = nullptr;
    float saved_time = 0;
    if (state && b.runtime.enabled && available(b.runtime)) {
        saved_target = b.prediction_time;
        saved_time = *saved_target;
        const auto prior_tick = signed_word(std::uint32_t(state->tick_base) - 1);
        const auto tick_time = static_cast<float>(prior_tick) * *b.runtime.interval;
        const auto fraction_time = *b.interpolation_fraction * *b.runtime.interval;
        *saved_target = tick_time + fraction_time;
    }
    const auto original =
        reinterpret_cast<engine::MemberFunction<std::uint32_t, std::uint32_t, std::uint32_t,
                                                std::uint32_t, std::uint32_t, float>>(
            original_table(receiver)[206]);
    const auto result =
        original(receiver, arguments[0], arguments[1], arguments[2], arguments[3], argument);
    if (saved_target)
        *saved_target = saved_time;
    return result;
}
void viewmodel_method_247(ViewmodelLagState* state, const ViewmodelLagHookBindings& b,
                          void* receiver, std::int32_t argument) { // complete4336BA27
    if (state && b.runtime.enabled && available(b.runtime) && state->command_number != -1) {
        if (state->command_number == state->previous_generation &&
            state->previous_query == argument)
            return;
        state->previous_generation = state->command_number;
        state->previous_query = argument;
    }
    const auto original =
        reinterpret_cast<engine::MemberFunction<void, std::int32_t>>(original_table(receiver)[247]);
    original(receiver, argument);
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: viewmodel_lag_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define VIEWMODEL_MEMBER __thiscall
#define VIEWMODEL_FAST __fastcall
#else
#define VIEWMODEL_MEMBER
#define VIEWMODEL_FAST
#endif
using PrimaryEntry = void(VIEWMODEL_MEMBER*)(void*);
struct FrameEndpoint : engine::FrameUpdateEndpoint {
    ViewmodelLagOwner* owner;
};
void VIEWMODEL_MEMBER viewmodel_lag_owner_destruction_entry(void*);
void VIEWMODEL_MEMBER viewmodel_lag_owner_reset_entry(void*);
void VIEWMODEL_MEMBER viewmodel_lag_owner_refresh_entry(void*);
void VIEWMODEL_MEMBER viewmodel_lag_owner_unchanged_entry(void*) {} //433CCE8E
void VIEWMODEL_FAST frame_entry(void*, std::uint8_t);
const std::array<PrimaryEntry, 9> viewmodel_lag_owner_primary_table{
    viewmodel_lag_owner_destruction_entry, viewmodel_lag_owner_unchanged_entry, viewmodel_lag_owner_unchanged_entry, viewmodel_lag_owner_unchanged_entry, viewmodel_lag_owner_unchanged_entry,
    viewmodel_lag_owner_reset_entry,       viewmodel_lag_owner_unchanged_entry, viewmodel_lag_owner_refresh_entry,   viewmodel_lag_owner_unchanged_entry};
const std::array<engine::FrameUpdateEntry, 1> viewmodel_lag_owner_frame_table{frame_entry};
void require_owner_bindings(const ViewmodelLagOwnerRegistries& registries,
                            ViewmodelHooksRuntime& hooks) {
    const auto& image = hooks.image();
    if (&registries.prediction.image() != &image || &registries.recharge.image() != &image)
        throw std::invalid_argument(
            "Viewmodel prediction and recharge registries require the hook initialization image");
    if (&registries.primary.registry != &image.native_cell<engine::OwnerRegistry>(0x43475B78u) ||
        &registries.frames.registry != &image.native_cell<engine::OwnerRegistry>(0x43475AD8u) ||
        &hooks.shared_owner() != &image.native_cell<ViewmodelLagOwner*>(viewmodel_lag_owner_slot))
        throw std::invalid_argument(
            "Viewmodel owner requires canonical lifecycle/frame registries and owner slot");
}
} //namespace

class ViewmodelLagOwner {
public:
    const PrimaryEntry* vtable = nullptr;
    ViewmodelHooksRuntime& hooks;
    engine::JobMemory memory;
    prediction::ViewmodelPredictionObserver prediction;
    client::NativeEntityObserverReceiver entity_receiver{};
    client::NativeCommandPhaseReceiver command_receiver{};
    network::ExploitViewmodelRecharge recharge;
    FrameEndpoint frame{{nullptr}, this};

    ViewmodelLagOwner(const ViewmodelLagOwnerRegistries& registries, ViewmodelHooksRuntime& runtime)
        : hooks(runtime), memory(registries.primary.memory),
          prediction{hooks.owner_state(), hooks.feature_bindings()},
          recharge{hooks.owner_state(), hooks.feature_bindings()} {
        //43486C44..43486C73: these are six separate registrar calls. The two
        //semantic views below borrow the same owner; neither owns another ring.
        registries.primary.append(this);
        registries.prediction.append(prediction);
        registries.entities.append_viewmodel_lag(entity_receiver, prediction); //43486C57
        registries.commands.append(command_receiver, &hooks.owner_state()); //43486C5F
        registries.recharge.append(recharge); //43486C67, table still unpublished
        engine::register_frame_update(registries.frames, &frame);

        registries.entities.publish_viewmodel_lag(entity_receiver); //43486C76, table43472644
        registries.commands.publish(command_receiver,
                                    client::CommandOwnerId::owner_4346f048); //43486C7E
        registries.recharge.publish(recharge); //43486C8A, table4346F528
        //43486C91 clears+44 before publishing frame/primary/prediction tables.
        //All six registrations precede the first final endpoint table write.
        auto& state = hooks.owner_state();
        state.previous_animation_time = 0;
        frame.vtable = viewmodel_lag_owner_frame_table.data();
        vtable = viewmodel_lag_owner_primary_table.data();
        registries.prediction.publish(prediction); //43486CAA, table4346F0D4
        //43486CB2..BC writes only the weapon-handle word at+500+index*20.
        for (auto& snapshot : state.snapshots)
            snapshot.weapon_handle = 0xFFFFFFFFu;
    }
};
static_assert(sizeof(ViewmodelLagOwner) <= viewmodel_lag_owner_allocation);

namespace {
void VIEWMODEL_MEMBER viewmodel_lag_owner_destruction_entry(void* storage) {
    auto* const owner = static_cast<ViewmodelLagOwner*>(storage);
    destroy_viewmodel_lag_owner(owner, owner->hooks.shared_owner());
}
void VIEWMODEL_MEMBER viewmodel_lag_owner_reset_entry(void* storage) {
    reset_viewmodel_lag_owner(static_cast<ViewmodelLagOwner*>(storage));
}
void VIEWMODEL_MEMBER viewmodel_lag_owner_refresh_entry(void* storage) {
    refresh_viewmodel_lag_owner(static_cast<ViewmodelLagOwner*>(storage));
}
void VIEWMODEL_FAST frame_entry(void* receiver, std::uint8_t) { //4337D213, DL unused
    auto& frame = *static_cast<FrameEndpoint*>(static_cast<engine::FrameUpdateEndpoint*>(receiver));
    consume_viewmodel_prediction(frame.owner->hooks.owner_state());
}
void factory_entry(void* opaque) {
    auto& context = *static_cast<ViewmodelLagFactoryContext*>(opaque);
    create_viewmodel_lag_owner(context.registries, context.hooks);
}

void* viewmodel_lag_owner_allocate(engine::JobMemory memory, std::size_t size) { //43387448
    auto* const allocation = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!allocation)
        return nullptr;
    const auto base = reinterpret_cast<std::uintptr_t>(allocation);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - base), total = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}
#undef VIEWMODEL_MEMBER
#undef VIEWMODEL_FAST
} //namespace

std::size_t viewmodel_lag_owner_size() noexcept {
    return viewmodel_lag_owner_allocation;
}
ViewmodelLagOwner* construct_viewmodel_lag_owner(void* storage,
                                                 const ViewmodelLagOwnerRegistries& registries,
                                                 ViewmodelHooksRuntime& hooks) {
    require_owner_bindings(registries, hooks);
    return ::new (storage) ViewmodelLagOwner(registries, hooks);
}
ViewmodelLagOwner* create_viewmodel_lag_owner(const ViewmodelLagOwnerRegistries& registries,
                                              ViewmodelHooksRuntime& hooks) {
    require_owner_bindings(registries, hooks);
    auto* const storage = viewmodel_lag_owner_allocate(registries.primary.memory, viewmodel_lag_owner_size());
    ViewmodelLagOwner* owner = nullptr;
    if (storage)
        owner = construct_viewmodel_lag_owner(storage, registries, hooks);
    // Supplied mem_int3.h instruction record restores this actual store. Its
    // source value is this invocation's constructor result, including null.
    hooks.shared_owner() = owner;
    return owner;
}
ViewmodelLagState* viewmodel_lag_owner_state(ViewmodelLagOwner* owner) noexcept {
    return owner ? &owner->hooks.owner_state() : nullptr;
}
void destroy_viewmodel_lag_owner(ViewmodelLagOwner* storage, ViewmodelLagOwner*& active) {
    active = nullptr; //433A102F, before433837D0's null check
    if (!storage)
        return;
    auto* const owner = storage;
    const auto memory = owner->memory;
    owner->~ViewmodelLagOwner();
    auto* const bytes = reinterpret_cast<std::byte*>(storage);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void reset_viewmodel_lag_owner(ViewmodelLagOwner* owner) {
    reset_viewmodel_lag(owner->hooks.owner_state());
}
void refresh_viewmodel_lag_owner(ViewmodelLagOwner* owner) {
    owner->vtable[5](owner);
}
void bind_viewmodel_lag_factory(client::NativeStartupSources& sources,
                                ViewmodelLagFactoryContext& context) {
    if (&sources.data() != &context.hooks.image())
        throw std::invalid_argument(
            "Viewmodel factory requires the scheduler's canonical data owner");
    require_owner_bindings(context.registries, context.hooks);
    sources.bind_initializer({0x4347E13Eu, &context, factory_entry});
}
} //namespace skeet::misc

// ============================================================================
// Subcomponent: viewmodel_runtime_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
std::ptrdiff_t field(client::ClientRuntimeStorage& storage, std::uint32_t salt) {
    const auto& globals = storage.offsets().globals;
    for (const auto& recipe : engine::build_receive_offset_recipes())
        if (recipe.initial_xor_key == salt)
            return engine::DecodedOffset(globals, recipe.pointer_identity, recipe.xor_key_identity,
                                         salt, 0);
    for (const auto& recipe : engine::build_data_map_offset_recipes())
        if (recipe.initial_xor_key == salt)
            return engine::DecodedOffset(globals, recipe.pointer_identity, recipe.xor_key_identity,
                                         salt, 0);
    if (salt == 0x3EDECE61u)
        return engine::DecodedOffset(globals, 0x434675C8u, 0x4346A5B8u, salt, 0);
    throw std::invalid_argument("Viewmodel field lacks its recovered offset recipe");
}
ViewmodelLagOffsets offsets(client::ClientRuntimeStorage& s) {
    const auto f = [&](std::uint32_t salt) { return field(s, salt); };
    return {f(0x71DCEE42), f(0x38CA5228), f(0xD5FD2E2C), f(0xA116369A),
            f(0xA25D8271), f(0x695FF325), f(0xAEF72A04), f(0x43D8CA19),
            f(0x53E7C7D2), f(0x47877DEB), f(0xBEFF3172)};
}
template <class T> T& pointed(engine::NativeDataImage& data, std::uint32_t cell) {
    auto* pointer = data.native_cell<T*>(cell);
    if (!pointer)
        throw std::invalid_argument(
            "Viewmodel contexts require their acquired global clock aliases");
    return *pointer;
}
engine::OwnerRegistryBinding registry(client::ClientRuntimeStorage& s, std::uint32_t cell) {
    return {s.owner_registries().at(cell), s.memory()};
}
client::ClientRuntimeStorage&
checked(client::ClientRuntimeStorage& storage, const client::EngineRuntimeBindings& engine,
        client::ConfigurationRuntimeOwner& config, skins::SkinsRuntimeOwner& skins,
        effects::EffectsRuntimeOwner& effects, engine::HookRuntimeOwner& hooks) {
    auto& data = storage.data();
    const auto& sequence = skins.sequence_bindings();
    if (&engine.variables.data() != &data || &config.storage() != &storage ||
        &effects.storage() != &storage || &hooks.storage() != &storage ||
        &hooks.descriptor_storage().image() != &data ||
        &sequence.settings != &config.skins_settings() ||
        &sequence.state != &skins.sequence_state() ||
        &sequence.entity_list != &data.native_cell<void*>(0x4346A890u) ||
        &sequence.events != &storage.event_registry())
        throw std::invalid_argument(
            "Viewmodel contexts require canonical settings, skins sequence history, effects and hooks");
    if (!storage.offsets().words || storage.offsets().globals.pointers.empty() ||
        !data.native_cell<void*>(0x4346A894u) || !data.native_cell<void*>(0x43467B04u) ||
        !engine.variables.at(0x4346A850u).get())
        throw std::invalid_argument(
            "Viewmodel contexts require acquired input, clock, offsets and interpolation ConVar");
    for (const auto cell : {0x4346A514u, 0x4346A890u}) {
        bool found = false;
        const auto manifest = engine::build_interface_manifest();
        for (std::size_t i = 0; i < manifest.size(); ++i)
            if (manifest[i].destination_cell == cell) {
                if (!data.native_cell<void*>(cell) ||
                    data.native_cell<void*>(cell) != engine.interfaces.objects[i])
                    throw std::invalid_argument(
                        "Viewmodel interface differs from its acquired canonical object");
                found = true;
                break;
            }
        if (!found)
            throw std::logic_error("Viewmodel interface is absent from the acquisition manifest");
    }
    return storage;
}
struct StateStorage {
    engine::JobMemory memory;
    ViewmodelLagState* value;
    StateStorage(client::ClientRuntimeStorage& storage,
                 std::array<skins::SequenceHistory, 150>& history)
        : memory(storage.memory()),
          value(static_cast<ViewmodelLagState*>(
              engine::allocate_owner_storage(memory, sizeof(ViewmodelLagState)))) {
        if (!value)
            throw std::bad_alloc();
        // The external canonical state retains the native allocation's ZERO
        // policy; its one history reference names the existing skins ring.
        ::new (static_cast<void*>(value)) ViewmodelLagState(history);
    }
    ~StateStorage() {
        value->~ViewmodelLagState();
        engine::release_owner_storage(memory, value);
    }
};
}
struct ViewmodelRuntimeOwner::Data {
    client::ClientRuntimeStorage& storage;
    StateStorage state;
    ViewmodelHookInputs inputs;
    ViewmodelHooksRuntime hooks;
    ViewmodelLagOwnerRegistries registries;
    ViewmodelLagFactoryContext factory;
    Data(client::ClientRuntimeStorage& source, const client::EngineRuntimeBindings& engine,
         client::ConfigurationRuntimeOwner& config, skins::SkinsRuntimeOwner& skins,
         effects::EffectsRuntimeOwner& effects, engine::HookRuntimeOwner& hook_owner)
        : storage(checked(source, engine, config, skins, effects, hook_owner)),
          state(storage, skins.sequence_state().history),
          inputs{source.data().native_cell<void*>(0x4346A514u),
                 source.data().native_cell<void*>(0x4346A890u),
                 source.data().native_cell<void*>(0x4346A894u),
                 source.data().native_cell<void*>(0x43475808u),
                 engine.variables.at(0x4346A850u),
                 pointed<float>(source.data(), 0x43467EB8u),
                 pointed<float>(source.data(), 0x4346A338u),
                 reinterpret_cast<float*>(source.data().native_cell<void*>(0x43467B04u))[9],
                 offsets(source),
                 field(source, 0x3EDECE61u),
                 source.data().native_cell<std::uint32_t>(0x4346A534u)},
          hooks(*state.value,
                source.data().native_cell<ViewmodelLagOwner*>(viewmodel_lag_owner_slot),
                config.settings_menu().values(), inputs, effects.view(), hook_owner.registry(),
                hook_owner.descriptor_storage()),
          registries{registry(source, 0x43475B78u), source.prediction_observers(),
                     source.entity_observers(),     source.command_phases(),
                     source.recharge_observers(),   registry(source, 0x43475AD8u)},
          factory{registries, hooks} {}
};
ViewmodelRuntimeOwner::ViewmodelRuntimeOwner(client::ClientRuntimeStorage& s,
                                             const client::EngineRuntimeBindings& e,
                                             client::ConfigurationRuntimeOwner& c,
                                             skins::SkinsRuntimeOwner& skins,
                                             effects::EffectsRuntimeOwner& effects,
                                             engine::HookRuntimeOwner& hooks)
    : data_(std::make_unique<Data>(s, e, c, skins, effects, hooks)) {}
ViewmodelRuntimeOwner::~ViewmodelRuntimeOwner() = default;
client::ClientRuntimeStorage& ViewmodelRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
ViewmodelLagState& ViewmodelRuntimeOwner::state() noexcept {
    return *data_->state.value;
}
ViewmodelHooksRuntime& ViewmodelRuntimeOwner::hooks() noexcept {
    return data_->hooks;
}
ViewmodelLagFactoryContext& ViewmodelRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void ViewmodelRuntimeOwner::bind_initializers(client::NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument("Viewmodel factory requires its canonical source image");
    bind_viewmodel_lag_factory(sources, data_->factory);
}
} //namespace skeet::misc
