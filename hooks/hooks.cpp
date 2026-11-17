// Master Hooks Manager
#include "skeet/client/command_orchestrator.hpp"
#include "skeet/client/hook_descriptor_cleanup.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/client/shutdown.hpp"
#include <array>
#include <stdexcept>
#include <utility>
#include "skeet/client/local_player_animation_hooks.hpp"
#include <intrin.h>
#include "skeet/client/player_command_hooks.hpp"
#include "skeet/animation/local_helpers.hpp"
#include "skeet/engine/interface_call.hpp"
#include <cstring>
#include "skeet/client/player_physics_hooks.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "player_physics_hooks_detail.hpp"


// ============================================================================
// Subcomponent: create_move_hook.cpp
// ============================================================================

//A C-linkage bridge gives both supported x86 assemblers one stable concrete
//source symbol. The outer saved-EBX address remains unchanged while these four
//arguments are marshalled; the bridge calls the complete433D2E40 source body.
extern "C" std::uint8_t __cdecl skeet_create_move_dispatch(void* player, std::uint8_t* send,
                                                           float sample_time,
                                                           skeet::client::Command* command) {
    return skeet::client::create_move_command_hook(player, send, sample_time, command);
}

namespace skeet::client {
#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((naked)) std::uint8_t __fastcall player_create_move_hook(void*, void*, float,
                                                                       Command*) {
    __asm__("pushl %ebp\n\t"
            "movl %esp, %ebp\n\t"
            "pushl %ebx\n\t"
            "movl %esp, %edx\n\t"
            "pushl 12(%ebp)\n\t"
            "pushl 8(%ebp)\n\t"
            "pushl %edx\n\t"
            "pushl %ecx\n\t"
            "call _skeet_create_move_dispatch\n\t"
            "addl $16, %esp\n\t"
            "popl %ebx\n\t"
            "movl %ebp, %esp\n\t"
            "popl %ebp\n\t"
            "retl $8");
}
#elif defined(_MSC_VER) && defined(_M_IX86)
__declspec(naked) std::uint8_t __fastcall player_create_move_hook(void*, void*, float, Command*) {
    __asm {
        push ebp
        mov ebp,esp
        push ebx
        mov edx,esp
        push dword ptr [ebp+12]
        push dword ptr [ebp+8]
        push edx
        push ecx
        call skeet_create_move_dispatch
        add esp,16
        pop ebx
        mov esp,ebp
        pop ebp
        ret 8
    }
}
#else
#error The native4334F53D saved-EBX handoff requires a32-bit x86 target.
#endif
} //namespace skeet::client

// ============================================================================
// Subcomponent: hook_descriptor_cleanup.cpp
// ============================================================================

namespace skeet::client {
namespace {
struct DescriptorCleanup {
    std::uint32_t initializer, finalizer, descriptor;
};
// Each initializer is MOV ECX,finalizer; JMP433837F5. Each finalizer is
// MOV ECX,descriptor; JMP433D2DC1. These are ordinary native bodies, not
// captured operand returns or a claimed game-owner factory order.
constexpr std::array<DescriptorCleanup, 26> recipes{{
    {0x4341f48c, 0x43406146, 0x43475954}, //0C4
    {0x43400941, 0x43341b52, 0x43475b48}, //0C8
    {0x4335939c, 0x434178d5, 0x43475878}, //0CC
    {0x433e87aa, 0x4341a06a, 0x43475998}, //0D0
    {0x433acf51, 0x43381d7a, 0x43475830}, //0D4
    {0x4340c133, 0x43402a1a, 0x4347589c}, //0D8
    {0x433f97b6, 0x433cdc42, 0x43475a28}, //0DC
    {0x433cd6c5, 0x433b196d, 0x43475b60}, //0E0
    {0x4341f401, 0x43328ad1, 0x43475864}, //0E4
    {0x4336a58b, 0x433b32e6, 0x43475aec}, //0E8
    {0x43388e1c, 0x433ebc6a, 0x43475b18}, //0EC
    {0x433b6e1b, 0x433f8fed, 0x43475a08}, //0F0
    {0x4336a322, 0x4338231e, 0x43475b34}, //0F4
    {0x4340c482, 0x43366333, 0x43475ac4}, //0F8
    {0x43336f65, 0x433662cd, 0x43475908}, //0FC
    {0x4339d11f, 0x4332b66e, 0x434758f4}, //100
    {0x4341cbcf, 0x434178eb, 0x434757f4}, //104
    {0x4337cab9, 0x433f52c9, 0x434757c8}, //108
    {0x433a17ae, 0x43348276, 0x43475a3c}, //10C
    {0x4334c748, 0x4331ff41, 0x43475b04}, //110
    {0x4333bf21, 0x433c9fa8, 0x43475a68}, //114
    {0x433e90fd, 0x433c1247, 0x434757b0}, //118
    {0x4335ba76, 0x433c8f1b, 0x434757e0}, //11C
    {0x4341875b, 0x433e3cab, 0x43475814}, //120
    {0x4338a373, 0x43368994, 0x434758b8}, //124
    {0x433d0e9d, 0x4339d52d, 0x4347591c}, //128
}};
HookDescriptorCleanupContext* active_cleanup = nullptr;

template <std::size_t Index> void finalize_descriptor() {
    auto& context = *active_cleanup;
    context.descriptors.clear(context.storage.at(recipes[Index].descriptor));
}
template <std::size_t... Index> constexpr auto finalizer_entries(std::index_sequence<Index...>) {
    return std::array<void (*)(), sizeof...(Index)>{finalize_descriptor<Index>...};
}
constexpr auto finalizers = finalizer_entries(std::make_index_sequence<recipes.size()>{});

template <std::size_t Index> void register_descriptor_finalizer(void* owner) {
    auto& context = *static_cast<HookDescriptorCleanupContext*>(owner);
    (void)context.callbacks.append(context.source_finalizer(recipes[Index].finalizer));
}
template <std::size_t... Index>
void bind_initializers(NativeStartupSources& sources, HookDescriptorCleanupContext& context,
                       std::index_sequence<Index...>) {
    (sources.bind_initializer(
         {recipes[Index].initializer, &context, register_descriptor_finalizer<Index>}),
     ...);
}
}

void (*HookDescriptorCleanupContext::source_finalizer(std::uint32_t address))() {
    for (std::size_t index = 0; index < recipes.size(); ++index) {
        if (recipes[index].finalizer == address) {
            if (active_cleanup && active_cleanup != this)
                throw std::logic_error(
                    "Descriptor exit callbacks require one stable source binding");
            // Source binding publication for the same native global storage.
            // This does not add a native initialization or cleanup operation.
            active_cleanup = this;
            return finalizers[index];
        }
    }
    throw std::out_of_range("Finalizer is not a recovered hook descriptor cleanup entry");
}

void bind_hook_descriptor_initializers(NativeStartupSources& sources,
                                       HookDescriptorCleanupContext& context) {
    bind_initializers(sources, context, std::make_index_sequence<recipes.size()>{});
}
}

// ============================================================================
// Subcomponent: local_player_animation_hooks.cpp
// ============================================================================

namespace skeet::client {
namespace {
LocalPlayerAnimationHooks* active;
void** original(void* player) noexcept {
    const auto* metadata =
        static_cast<const engine::VtableHookMetadata*>((*static_cast<void***>(player))[-2]);
    return metadata->original_table;
}
#if defined(_M_IX86) || defined(__i386__)
#define LOCAL_PLAYER_FASTCALL __fastcall
#else
#define LOCAL_PLAYER_FASTCALL
#endif
std::uint32_t LOCAL_PLAYER_FASTCALL strafe_entry(void* player, void*, std::uint32_t first,
                                                 std::uint32_t second) {
    return active->update_strafe(player, first, second,
                                 reinterpret_cast<std::uintptr_t>(_ReturnAddress()));
}
void LOCAL_PLAYER_FASTCALL animation_entry(void* player, void*) {
    active->update_client_animation(player);
}
#undef LOCAL_PLAYER_FASTCALL
}
std::uintptr_t LocalPlayerAnimationHooks::network_fix(void* player, void* supplied_object,
                                                      std::uintptr_t caller) {
    return misc::process_network_fix(player, supplied_object, caller, bindings.network_fix);
}
void LocalPlayerAnimationHooks::crosshair_view(void* player, void* view_setup,
                                               std::uintptr_t caller) {
    hud::prepare_crosshair_view(bindings.crosshair_config, bindings.crosshair, player, view_setup,
                                bindings.suspended != 0, reinterpret_cast<const std::byte*>(caller),
                                original(player));
}
// The original veneers copy the caller's nonvolatile register before their
// tail jump. Normal C++ prologues cannot reconstruct those hidden inputs.
extern "C" std::uintptr_t __cdecl
skeet_local_network_fix_bridge(void* player, void* supplied_object, std::uintptr_t caller) {
    return active->network_fix(player, supplied_object, caller);
}
extern "C" void __cdecl skeet_local_crosshair_view_bridge(void* player, void* view_setup,
                                                          std::uintptr_t caller) {
    active->crosshair_view(player, view_setup, caller);
}
#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
extern "C" __attribute__((naked)) void skeet_local_network_fix_entry() {
    __asm__("pushl (%esp)\n\t"
            "pushl %ebp\n\t"
            "pushl %ecx\n\t"
            "call _skeet_local_network_fix_bridge\n\t"
            "addl $12, %esp\n\t"
            "ret");
}
extern "C" __attribute__((naked)) void skeet_local_crosshair_view_entry() {
    __asm__("pushl (%esp)\n\t"
            "pushl %edi\n\t"
            "pushl %ecx\n\t"
            "call _skeet_local_crosshair_view_bridge\n\t"
            "addl $12, %esp\n\t"
            "ret");
}
#elif defined(_MSC_VER) && defined(_M_IX86)
extern "C" __declspec(naked) void skeet_local_network_fix_entry() {
    __asm {
        push dword ptr [esp]
        push ebp
        push ecx
        call skeet_local_network_fix_bridge
        add esp,12
        ret
    }
}
extern "C" __declspec(naked) void skeet_local_crosshair_view_entry() {
    __asm {
        push dword ptr [esp]
        push edi
        push ecx
        call skeet_local_crosshair_view_bridge
        add esp,12
        ret
    }
}
#endif
void* LocalPlayerAnimationHooks::native_replacement_entry(std::size_t slot, std::uint32_t entry) {
    void* replacement;
    if (slot == 222 && entry == 0x43341FA7U)
        replacement = reinterpret_cast<void*>(strafe_entry);
    else if (slot == 224 && entry == 0x43329436U)
        replacement = reinterpret_cast<void*>(animation_entry);
#if defined(_M_IX86) || defined(__i386__)
    else if (slot == 268 && entry == 0x433C6689U)
        replacement = reinterpret_cast<void*>(skeet_local_network_fix_entry);
    else if (slot == 294 && entry == 0x433E009AU)
        replacement = reinterpret_cast<void*>(skeet_local_crosshair_view_entry);
#else
    else if ((slot == 268 && entry == 0x433C6689U) || (slot == 294 && entry == 0x433E009AU))
        throw std::logic_error("Local player EBP/EDI veneers require the native x86 ABI");
#endif
    else
        throw std::invalid_argument("Unknown local player animation hook slot/entry");
    return replacement;
}
void LocalPlayerAnimationHooks::bind_native_replacement(engine::VtableHookMetadata& metadata,
                                                        std::size_t slot, std::uint32_t entry) {
    auto* replacement = native_replacement_entry(slot, entry);
    active = this;
    metadata.replacement_table[slot] = replacement;
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: player_command_hooks.cpp
// ============================================================================

namespace skeet::client {
namespace {
PlayerCommandHooks* active_command_runtime;
template <class T> T field(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class Result, class... Args>
Result original(void* player, std::size_t slot, Args... args) {
    const auto* metadata =
        static_cast<const engine::VtableHookMetadata*>((*static_cast<void***>(player))[-2]);
    return reinterpret_cast<engine::MemberFunction<Result, Args...>>(
        metadata->original_table[slot])(player, args...);
}
}
void PlayerCommandHooks::prediction_gated_callback(void* player) {
    if (field<std::uint8_t>(bindings.prediction, 0x18))
        original<void>(player, 230);
}
std::uint32_t PlayerCommandHooks::ragdoll_result(void* player) {
    if (bindings.effects.disable_ragdolls.value)
        return 0;
    return original<std::uint32_t>(player, 298);
}
void PlayerCommandHooks::publish_animation_command(void* player, std::uintptr_t first,
                                                   std::uintptr_t second) {
    const auto* command = field<const Command*>(player, bindings.fields.current_command_f58e6118);
    if (command) {
        animation::mark_animation_modifiers(
            field<void*>(player, bindings.fields.animation_state_3d52a1a7));
        const auto number = std::uint32_t(command->number);
        prediction_history_storage(*bindings.history).command_tags[number % 150U] = number;
    }
    original<void>(player, 350, first, second);
}

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_PLAYER_COMMAND_ENTRY __fastcall
#else
#define SKEET_PLAYER_COMMAND_ENTRY
#endif
namespace {
void SKEET_PLAYER_COMMAND_ENTRY prediction_entry(void* player, void*) {
    active_command_runtime->prediction_gated_callback(player);
}
std::uint32_t SKEET_PLAYER_COMMAND_ENTRY ragdoll_entry(void* player, void*) {
    return active_command_runtime->ragdoll_result(player);
}
void SKEET_PLAYER_COMMAND_ENTRY animation_command_entry(void* player, void*, std::uintptr_t first,
                                                        std::uintptr_t second) {
    active_command_runtime->publish_animation_command(player, first, second);
}
}
void* PlayerCommandHooks::native_replacement_entry(std::size_t slot, std::uint32_t entry) {
    void* replacement;
    if (entry == 0x4331E412U && slot == 230)
        replacement = reinterpret_cast<void*>(&prediction_entry);
    else if (entry == 0x433AB4B6U && slot == 298)
        replacement = reinterpret_cast<void*>(&ragdoll_entry);
    else if (entry == 0x433D18B0U && slot == 350)
        replacement = reinterpret_cast<void*>(&animation_command_entry);
    else
        throw std::invalid_argument("Unknown captured player-command hook entry/slot");
    return replacement;
}
void PlayerCommandHooks::bind_native_replacement(engine::VtableHookMetadata& metadata,
                                                 std::size_t slot, std::uint32_t entry) {
    auto* replacement = native_replacement_entry(slot, entry);
    active_command_runtime = this;
    metadata.replacement_table[slot] = replacement;
}
#undef SKEET_PLAYER_COMMAND_ENTRY
} // namespace skeet::client

// ============================================================================
// Subcomponent: player_physics_hooks.cpp
// ============================================================================

namespace skeet::client {
namespace {
PlayerPhysicsHooks* active_physics_runtime;
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_PLAYER_PHYSICS_ABI __fastcall
#else
#define SKEET_PLAYER_PHYSICS_ABI
#endif
void* SKEET_PLAYER_PHYSICS_ABI studio_header_entry(void* player, void*) {
    return active_physics_runtime->studio_header(player);
}
__declspec(noinline) void SKEET_PLAYER_PHYSICS_ABI
absolute_velocity_entry(void* player, void*, std::array<float, 3>* output) {
    auto* return_slot = static_cast<std::uintptr_t*>(_AddressOfReturnAddress());
    active_physics_runtime->estimate_absolute_velocity(player, *output, *return_slot);
}
bool SKEET_PLAYER_PHYSICS_ABI collision_entry(void* player, void*, std::int32_t group,
                                              std::uint32_t mask) {
    return active_physics_runtime->should_collide(player, group, mask);
}
void SKEET_PLAYER_PHYSICS_ABI interpolation_update_entry(void* player, void*, std::uint32_t a,
                                                         std::uint32_t b, std::uint32_t c,
                                                         std::uint32_t d) {
    active_physics_runtime->update_interpolated_state(player, {a, b, c, d});
}
std::uint8_t SKEET_PLAYER_PHYSICS_ABI interpolate_entry(void* player, void*,
                                                        std::uint32_t time_bits) {
    return active_physics_runtime->interpolate(player, time_bits);
}
std::array<float, 3>* SKEET_PLAYER_PHYSICS_ABI eye_position_entry(void* player, void*,
                                                                  std::array<float, 3>* output) {
    active_physics_runtime->eye_position(player, *output);
    return output;
}
#undef SKEET_PLAYER_PHYSICS_ABI
}
PlayerPhysicsHookFields
native_player_physics_hook_fields(const engine::NativeOffsetGlobals& globals) {
    const auto decode = [&](std::uint32_t pointer, std::uint32_t mask, std::uint32_t key) {
        return std::ptrdiff_t(*globals.pointers.at(pointer) ^ globals.keys.at(mask) ^ key);
    };
    return {decode(0x4346A8E4, 0x434688CC, 0x36375C23), decode(0x434679F8, 0x434675F8, 0x3309C0CF)};
}
void* PlayerPhysicsHooks::native_replacement_entry(std::size_t slot, std::uint32_t native_entry) {
    void* entry = nullptr;
    if (slot == 109 && native_entry == 0x43402307U)
        entry = reinterpret_cast<void*>(&studio_header_entry);
    else if (slot == 145 && native_entry == 0x4333181EU)
        entry = reinterpret_cast<void*>(&absolute_velocity_entry);
    else if (slot == 173 && native_entry == 0x4333C562U)
        entry = reinterpret_cast<void*>(&collision_entry);
    else if (slot == 78 && native_entry == 0x433ABBF0U)
        entry = reinterpret_cast<void*>(&interpolation_update_entry);
    else if (slot == 113 && native_entry == 0x433875BEU)
        entry = reinterpret_cast<void*>(&interpolate_entry);
    else if (slot == 169 && native_entry == 0x43382222U)
        entry = reinterpret_cast<void*>(&eye_position_entry);
    else
        throw std::invalid_argument("Unknown native player physics hook replacement");
    return entry;
}
void PlayerPhysicsHooks::bind_native_replacement(engine::VtableHookMetadata& metadata,
                                                 std::size_t slot, std::uint32_t native_entry) {
    auto* entry = native_replacement_entry(slot, native_entry);
    active_physics_runtime = this;
    player_physics_detail::bind_velocity_continuation(bindings.velocity_continuation_434757c4);
    metadata.replacement_table[slot] = entry;
}
} //namespace skeet::client
