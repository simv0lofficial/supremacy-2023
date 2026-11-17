// CS:GO Interfaces & Netvars Implementation
#include "skeet/engine/animation_startup_patterns.hpp"
#include "skeet/engine/netvars.hpp"
#include <array>
#include <cstring>
#include "skeet/engine/bindings.hpp"
#include <algorithm>
#include <stdexcept>
#include <windows.h>
#include "skeet/engine/hooks.hpp"
#include <new>
#include "skeet/engine/loaded_modules.hpp"
#include <intrin.h>
#include "skeet/engine/offset_storage.hpp"
#include "skeet/engine/netvar_initialization.hpp"
#include "skeet/engine/pe_exports.hpp"
#include "skeet/engine/build_animation_bindings.hpp"
#include "skeet/animation/remote_setup_bones.hpp"
#include <initializer_list>
#include "skeet/engine/build_client_state_aliases.hpp"
#include <limits>
#include "skeet/engine/build_command_bindings.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/engine/build_entity_animation_bindings.hpp"
#include "skeet/engine/build_esp_bindings.hpp"
#include <span>
#include "skeet/engine/build_eye_adjustment_bindings.hpp"
#include "skeet/engine/build_final_startup_bindings.hpp"
#include "skeet/engine/overlay_build.hpp"
#include <cstdio>
#include "skeet/engine/build_game_state_bindings.hpp"
#include "skeet/client/startup.hpp"
#include "skeet/engine/build_history_bindings.hpp"
#include "skeet/engine/build_hitbox_clip_bindings.hpp"
#include "skeet/engine/trace.hpp"
#include "skeet/engine/build_hook_initialization.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/focus_overlay_hooks.hpp"
#include "skeet/animation/fix_animation_backup.hpp"
#include "skeet/misc/ping_spike_channel.hpp"
#include "skeet/events/game_events.hpp"
#include "skeet/render/frame_runtime.hpp"
#include <atomic>
#include <bit>
#include <xmmintrin.h>
#include "skeet/engine/player_hook_family.hpp"
#include "skeet/animation/fix_animations.hpp"
#include "skeet/effects/entity_visibility.hpp"
#include "skeet/effects/harmless_molotov.hpp"
#include "skeet/effects/model_hooks.hpp"
#include "skeet/misc/network_console_handlers.hpp"
#include "skeet/misc/viewmodel_hooks.hpp"
#include "skeet/misc/auto_accept_owner.hpp"
#include "skeet/network/client_state_hook_family.hpp"
#include "skeet/network/packet_dispatch.hpp"
#include "skeet/script/client_api.hpp"
#include "skeet/engine/build_legit_bindings.hpp"
#include "skeet/engine/build_loader_inspection_bindings.hpp"
#include "skeet/engine/build_local_animation_bindings.hpp"
#include "skeet/engine/build_material_bindings.hpp"
#include "skeet/engine/build_misc_engine_bindings.hpp"
#include "skeet/engine/build_module_bindings.hpp"
#include "skeet/engine/source_initialization_imports.hpp"
#include <string>
#include "skeet/engine/build_packet_bindings.hpp"
#include "skeet/engine/build_prediction_scratch_bindings.hpp"
#include "skeet/engine/build_runtime_type_bindings.hpp"
#include <sstream>
#include "skeet/engine/build_scene_bindings.hpp"
#include "skeet/rage/engine_ballistics.hpp"
#include "skeet/engine/build_script_bindings.hpp"
#include "skeet/engine/build_shared_esp_bindings.hpp"
#include "skeet/engine/rtti.hpp"
#include "skeet/engine/build_skin_bindings.hpp"
#include "skeet/skins/native_entity_cosmetics.hpp"
#include "skeet/skins/native_schema.hpp"
#include "skeet/skins/native_agents.hpp"
#include "skeet/skins/refresh.hpp"
#include "skeet/engine/build_skin_receive_patches.hpp"
#include "skeet/skins/agent_receive.hpp"
#include "skeet/skins/sequence_receive.hpp"
#include "skeet/engine/build_startup_convars.hpp"
#include "skeet/engine/build_visual_interface_bindings.hpp"
#include "skeet/engine/build_window_bindings.hpp"
#include "skeet/engine/module_range_initialization.hpp"
#include "skeet/engine/build_world_bindings.hpp"
#include "skeet/engine/client_class_factory_hooks.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/client/entity_command_runtime_owner.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/engine/hook_runtime_owner.hpp"
#include "skeet/engine/hook_descriptor_storage.hpp"
#include "skeet/engine/common_controls_initialization.hpp"
#include "skeet/engine/forwarded_exports.hpp"
#include "skeet/engine/executable_pattern.hpp"
#include "skeet/engine/first_startup_services.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/client/startup_bindings.hpp"
#include "skeet/client/visual_runtime_owner.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/menu/root_menu.hpp"
#include "skeet/menu/startup_data.hpp"
#include "skeet/render/startup_renderer.hpp"
#include "skeet/script/modules.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/engine/game_system.hpp"
#include "skeet/client/shutdown.hpp"
#include "skeet/effects/view_dispatch.hpp"
#include "skeet/network/update_observers.hpp"
#include "skeet/misc/config.hpp"
#include "game_system_detail.hpp"
#include <emmintrin.h>
#include "skeet/engine/game_system_patch.hpp"
#include "skeet/engine/game_window_proxy.hpp"
#include "skeet/engine/owner_allocation.hpp"
#include "skeet/esp/map_entity_parser.hpp"
#include "skeet/engine/hook_descriptors.hpp"
#include "skeet/client/misc_runtime_owner.hpp"
#include "skeet/animation/fix_runtime_owner.hpp"
#include "skeet/misc/runtime.hpp"
#include "skeet/effects/player_auxiliary_hooks.hpp"
#include "skeet/network/client_state_runtime_owner.hpp"
#include "skeet/script/owner.hpp"
#include "skeet/script/source_runtime_owner.hpp"
#include "skeet/engine/initialization_failure.hpp"
#include "skeet/engine/interface_discovery.hpp"
#include "skeet/engine/jobs.hpp"
#include <immintrin.h>
#include "skeet/engine/map_game_system_owner.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/engine/native_data_image.hpp"
#include <utility>
#include "skeet/engine/native_owner_registry_storage.hpp"
#include <cstddef>
#include "skeet/engine/native_service_table.hpp"
#include <cstdlib>
#include "skeet/engine/owner_registry.hpp"
#include "skeet/client/command_orchestrator.hpp"
#include "skeet/client/local_player_animation_hooks.hpp"
#include "skeet/client/player_command_hooks.hpp"
#include "skeet/client/player_command_queries.hpp"
#include "skeet/client/player_physics_hooks.hpp"
#include "skeet/effects/free_look_capture.hpp"
#include "skeet/prediction/player_simulation.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include "skeet/engine/scene_queries.hpp"
#include "skeet/engine/scene_runtime_owner.hpp"
#include "skeet/engine/source_data_initialization.hpp"
#include "skeet/misc/settings_runtime.hpp"
#include "skeet/engine/startup_container_cleanup.hpp"
#include "skeet/menu/settings_menu_storage.hpp"
#include "skeet/client/target_mask_owner.hpp"
#include "skeet/engine/unload.hpp"
#include "skeet/render/device_listener.hpp"
#include <type_traits>
#include "skeet/engine/startup_convars.hpp"
#include "skeet/engine/startup_operations.hpp"
#include "skeet/engine/startup_vector_cleanup.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/engine/tier0_startup_exports.hpp"


#include "util/address.h"

namespace skeet::engine {
using skeet::read;
using skeet::write;
using skeet::store;
using skeet::field;
using skeet::add;
using skeet::subtract;
using skeet::multiply;
using skeet::divide;

inline void require(bool condition, const char* message) {
    if (!condition)
        throw std::runtime_error(message);
}
}

// ============================================================================
// Subcomponent: animation_startup_patterns.cpp
// ============================================================================

namespace skeet::engine {
void initialize_animation_update_flag(NativeDataImage& data) {
    const auto* code = data.native_cell<const std::byte*>(0x434693F0u);
    if (!code)
        return;
    constexpr std::array<std::uint8_t, 11> pattern{0xC6, 0x05, 0xCC, 0xCC, 0xCC, 0xCC,
                                                   0xCC, 0xF3, 0x0F, 0x5F, 0x05};
    const auto* match = find_byte_pattern(const_cast<std::byte*>(code), 512, pattern.data(),
                                          static_cast<std::uint32_t>(pattern.size()), 0xCC);
    if (!match)
        return;
    std::uint32_t address;
    std::memcpy(&address, match + 2, sizeof address);
    data.native_cell<std::uint8_t*>(0x43467B24u) =
        reinterpret_cast<std::uint8_t*>(std::uintptr_t(address));
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: bindings.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
namespace {
void check_name(std::string_view value, const char* description) {
    if (value.empty() || value.find('\0') != std::string_view::npos)
        throw std::invalid_argument(description);
}

constexpr std::array consumers{
    InterfaceRequest{"entity_list", "client.dll", "VClientEntityList003"}, // blob +0x3B1F2
    InterfaceRequest{"surface", "vguimatsurface.dll", "VGUI_Surface031"}, // +0x4A69B
    InterfaceRequest{"base_client", "client.dll",
                     "VClient018"}, //working dump; blob+0x4B02F names client_panorama.dll
    InterfaceRequest{"vgui_system", "vgui2.dll", "VGUI_System010"}, // +0x4BA28
    InterfaceRequest{"trace", "engine.dll", "EngineTraceClient004"}, // +0x5D3DA
    InterfaceRequest{"effects", "engine.dll", "VEngineEffects001"}, // +0x5D6D6
    InterfaceRequest{"string_tables", "engine.dll", "VEngineClientStringTable001"}, // +0x5E2AA
    InterfaceRequest{"engine", "engine.dll", "VEngineClient014"} // +0x6AB87
};
}

InterfaceFactory LoadedModuleFactories::factory(std::string_view module) {
    check_name(module, "Invalid engine module name");
#if defined(_WIN32)
    const std::string name(module);
    const auto loaded = GetModuleHandleA(name.c_str());
    if (!loaded)
        throw std::runtime_error("Engine module is not loaded: " + name);
    const auto address = GetProcAddress(loaded, "CreateInterface");
    if (!address)
        throw std::runtime_error("Engine module has no CreateInterface export: " + name);
    return reinterpret_cast<InterfaceFactory>(address);
#else
    throw std::runtime_error("Loaded engine interface factories require Windows");
#endif
}

void InterfaceBindings::bind(std::span<const InterfaceRequest> requests,
                             FactoryProvider& provider) {
    std::vector<BoundInterface> discovered;
    discovered.reserve(requests.size());
    // Validate the complete manifest before any factory can have side effects.
    for (std::size_t i = 0; i != requests.size(); ++i) {
        const auto& request = requests[i];
        check_name(request.name, "Invalid interface binding name");
        check_name(request.module, "Invalid interface module name");
        check_name(request.version, "Invalid interface version");
        for (std::size_t j = 0; j != i; ++j)
            if (requests[j].name == request.name)
                throw std::invalid_argument("Duplicate interface binding name");
    }
    for (const auto& request : requests) {
        const auto factory = provider.factory(request.module);
        if (!factory)
            throw std::runtime_error("Missing engine interface factory: " +
                                     std::string(request.module));
        const std::string version(request.version);
        int status = -1;
        void* object = factory(version.c_str(), &status);
        if (!object)
            throw std::runtime_error("Missing engine interface: " + std::string(request.module) +
                                     "!" + version);
        discovered.push_back(
            {std::string(request.name), std::string(request.module), version, object});
    }
    bindings_.swap(discovered);
}

void InterfaceBindings::clear() noexcept {
    bindings_.clear();
}

void* InterfaceBindings::at(std::string_view name) const {
    const auto found = std::find_if(bindings_.begin(), bindings_.end(),
                                    [name](const auto& binding) { return binding.name == name; });
    if (found == bindings_.end())
        throw std::out_of_range("Engine interface is not bound: " + std::string(name));
    return found->object;
}

std::span<const InterfaceRequest> embedded_interface_consumers() noexcept {
    return consumers;
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: bindings_hooks.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
namespace {
VtableHookNode* allocate_node(JobMemory memory) {
    const auto size = sizeof(VtableHookNode);
    auto* allocation = memory.allocate(memory.process_heap, 8, size + 32);
    if (!allocation)
        return nullptr;
    const auto raw = reinterpret_cast<std::uintptr_t>(allocation);
    const auto header = (raw + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - raw);
    const auto allocation_size = static_cast<std::uint32_t>(size + 32);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return new (reinterpret_cast<void*>(header + 16)) VtableHookNode{};
}
void free_node(VtableHookNode* node, JobMemory memory) {
    auto* object = reinterpret_cast<std::byte*>(node);
    std::uint32_t displacement;
    std::memcpy(&displacement, object - 8, 4);
    memory.free(memory.process_heap, 0, object - 16 - displacement);
}
void replace_if_current(void*** object, void** expected, void** replacement) {
#if defined(_WIN32)
    InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(object), replacement,
                                      expected);
#else
    __atomic_compare_exchange_n(object, &expected, replacement, false, __ATOMIC_SEQ_CST,
                                __ATOMIC_SEQ_CST);
#endif
}
void replace(void*** object, void** replacement) {
#if defined(_WIN32)
    InterlockedExchangePointer(reinterpret_cast<void* volatile*>(object), replacement);
#else
    __atomic_exchange_n(object, replacement, __ATOMIC_SEQ_CST);
#endif
}
}

VtableHookNode* install_vtable_hook(VtableHookRegistry& registry, void* object,
                                    const VtableHookMetadata& metadata) {
    auto*** location = static_cast<void***>(object);
    replace_if_current(location, metadata.original_table, metadata.replacement_table);
    auto* node = allocate_node(registry.memory);
    if (node) {
        node->object = object;
        node->previous = &registry.sentinel;
        node->next = registry.sentinel.next;
        registry.sentinel.next->previous = node;
        registry.sentinel.next = node;
    }
    return node;
}

void uninstall_vtable_hook(VtableHookRegistry& registry, void* object) {
    for (auto* node = registry.sentinel.previous; node != &registry.sentinel;
         node = node->previous) {
        if (node->object != object)
            continue;
        node->next->previous = node->previous;
        node->previous->next = node->next;
        free_node(node, registry.memory);
        auto*** location = static_cast<void***>(object);
        const auto* metadata = static_cast<const VtableHookMetadata*>((*location)[-2]);
        replace(location, metadata->original_table);
        return;
    }
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: bindings_loaded_modules.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
std::uint32_t loaded_module_name_hash(const char16_t* name, std::int32_t length) noexcept {
    std::uint32_t hash = 0x811c9dc5;
    auto remaining = static_cast<std::uint32_t>(length);
    if (name) {
        while (*name && remaining != 0) {
            auto value = static_cast<std::uint16_t>(*name++);
            if (static_cast<std::uint16_t>(value - 0x41) <= 0x19)
                value = static_cast<std::uint16_t>(value + 0x20);
            hash = (hash ^ value) * 0x1000193;
            --remaining;
        }
    }
    return hash ^ 0x92ca5df1;
}

void* find_loaded_module(const LoadedModuleEnvironment& environment, std::uint32_t hash,
                         bool salted_hash, bool lock_loader) {
    if (!environment.loader)
        return nullptr;
    auto* const sentinel = &environment.loader->modules;
    // Native433F29C5 captures this first link before acquiring the PEB lock.
    auto* current = sentinel->next;
    const auto wanted = salted_hash ? hash ^ 0x92ca5df1 : hash;
    LoaderModule* found = nullptr;
    if (lock_loader)
        environment.acquire_peb_lock();
    while (current != sentinel) {
        auto* module = reinterpret_cast<LoaderModule*>(current);
        const auto actual =
            loaded_module_name_hash(module->base_name.buffer, module->base_name.length >> 1);
        if ((actual ^ 0x92ca5df1) == wanted) {
            found = module;
            break;
        }
        current = current->next;
    }
    if (lock_loader)
        environment.release_peb_lock();
    return found ? found->base : nullptr;
}

LoadedModuleEnvironment native_loaded_module_environment() {
#if defined(_WIN32)
#if defined(_M_IX86) || defined(__i386__)
    const auto peb = static_cast<std::uintptr_t>(__readfsdword(0x30));
    constexpr auto loader_offset = 0xc;
#else
    const auto peb = static_cast<std::uintptr_t>(__readgsqword(0x60));
    constexpr auto loader_offset = 0x18;
#endif
    LoaderData* loader;
    std::memcpy(&loader, reinterpret_cast<const std::byte*>(peb) + loader_offset, sizeof(loader));
    const auto native = GetModuleHandleA("ntdll.dll");
    if (!native)
        throw std::runtime_error("ntdll.dll is not loaded");
    const auto acquire = GetProcAddress(native, "RtlAcquirePebLock");
    const auto release = GetProcAddress(native, "RtlReleasePebLock");
    if (!acquire || !release)
        throw std::runtime_error("Native PEB lock export is missing");
    return {loader, reinterpret_cast<PebLockFunction>(acquire),
            reinterpret_cast<PebLockFunction>(release)};
#else
    throw std::runtime_error("Native loaded-module enumeration requires Windows");
#endif
}

void* find_loaded_module(std::uint32_t hash, bool salted_hash, bool lock_loader) {
    return find_loaded_module(native_loaded_module_environment(), hash, salted_hash, lock_loader);
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: bindings_native_init.cpp
// ============================================================================

namespace skeet::engine {
namespace {
void initialize_receive_binding(void* base_client, InitializationMetadata& metadata,
                                std::uint32_t address, std::uint32_t*& cursor, bool with_addend) {
    OffsetBinding binding;
    binding.class_hash = static_cast<std::uint32_t>(metadata.operand(address));
    binding.property_hash = static_cast<std::uint32_t>(metadata.operand(address + 4));
    if (with_addend)
        binding.addend = static_cast<std::uint32_t>(metadata.operand(address + 16));
    binding.xor_key = reinterpret_cast<std::uint32_t*>(metadata.operand(address + 8));
    binding.pointer_output = reinterpret_cast<std::uint32_t**>(metadata.operand(address + 12));
    bind_receive_offset(base_client, binding, cursor);
}
}

void initialize_native_netvars(void* base_client, std::byte* client_module,
                               InitializationMetadata& metadata, std::uint32_t*& cursor,
                               DiscoveredDataMaps& maps) {
    for (std::uint32_t descriptor = 0x1678; descriptor != 0x24e8; descriptor += 0x10)
        initialize_receive_binding(base_client, metadata, descriptor, cursor, false);
    for (std::uint32_t descriptor = 0x1390; descriptor != 0x1674; descriptor += 0x14)
        initialize_receive_binding(base_client, metadata, descriptor, cursor, true);
    maps.c69c5c29 = find_data_map(client_module, 0xc69c5c29);
    maps.a9cd7264 = find_data_map(client_module, 0xa9cd7264);
    for (std::uint32_t descriptor = 0x1278; descriptor != 0x1390; descriptor += 0x14) {
        DataMapOffsetBinding binding;
        binding.map = *reinterpret_cast<DataMap**>(metadata.operand(descriptor));
        binding.table_hash = static_cast<std::uint32_t>(metadata.operand(descriptor + 4));
        binding.property_hash = static_cast<std::uint32_t>(metadata.operand(descriptor + 8));
        binding.xor_key = reinterpret_cast<std::uint32_t*>(metadata.operand(descriptor + 12));
        binding.pointer_output =
            reinterpret_cast<std::uint32_t**>(metadata.operand(descriptor + 16));
        bind_data_map_offset(binding, cursor);
    }
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: bindings_offset_storage.cpp
// ============================================================================

namespace skeet::engine {
namespace {
struct Recipe {
    std::uint32_t source_pointer, source_key, xor_value;
    std::int32_t displacement;
    std::uint32_t destination_key, destination_pointer;
};
// Exact native4348D65D..4348D9E6 operands, in execution order.
constexpr std::array bindings_offset_recipes{
    Recipe{0x43469198, 0x4346a538, 0xd5fd2e2c, 4, 0x4346a6b4, 0x43467004},
    Recipe{0x434670e0, 0x43467d28, 0x4e66bb34, 12, 0x43468ed4, 0x43468d30},
    Recipe{0x434688d4, 0x43469030, 0xf8205eba, 12, 0x43467fac, 0x43468368},
    Recipe{0x4346a53c, 0x43468eb4, 0x13965961, 4, 0x4346920c, 0x4346a588},
    Recipe{0x43467e8c, 0x43467900, 0x32e4daf1, 4, 0x43468524, 0x4346a7d4},
    Recipe{0x4346a808, 0x43468be8, 0x2b554ad0, 20, 0x434674a8, 0x43468e7c},
    Recipe{0x434675cc, 0x43468538, 0xc7e1ecd2, -20, 0x4346a484, 0x43469064},
    Recipe{0x434682c0, 0x434683cc, 0x1952b857, 44, 0x43467b80, 0x43467d78},
    Recipe{0x4346934c, 0x434671f8, 0x334b460c, 1, 0x4346707c, 0x4346757c},
    Recipe{0x434693d8, 0x43468bc4, 0x3d10b94c, 4, 0x4346a5b8, 0x434675c8},
    Recipe{0x43468a8c, 0x43467d88, 0x7fd04456, 4, 0x43467c70, 0x43468fd0},
    Recipe{0x4346934c, 0x434671f8, 0x334b460c, 76, 0x43467b8c, 0x43468bf8},
    Recipe{0x434688a0, 0x43467b78, 0x5461ee58, -8, 0x43467fa8, 0x43467434},
    Recipe{0x434688a0, 0x43467b78, 0x5461ee58, -4, 0x43468c8c, 0x43468ebc},
    Recipe{0x434688a0, 0x43467b78, 0x5461ee58, 28, 0x43467b94, 0x434670f4},
    Recipe{0x43468da4, 0x434684ec, 0xcacaeecc, -20, 0x4346a8b0, 0x43467f4c},
    Recipe{0x4346796c, 0x43469084, 0x572aa59f, -28, 0x43467d24, 0x4346a62c},
    Recipe{0x43469230, 0x434679a8, 0xa1af8ffc, 20, 0x43467500, 0x4346a354},
    Recipe{0x4346a530, 0x43467564, 0x58ae2423, 8, 0x4346a3a8, 0x43468970},
    Recipe{0x4346a6a4, 0x43467120, 0x9d63bb7f, -4, 0x4346a540, 0x43469210}};
std::uint32_t* allocate_words(JobMemory memory) {
    auto* allocation = memory.allocate(memory.process_heap, 8, 0x1020);
    if (!allocation)
        return nullptr;
    const auto raw = reinterpret_cast<std::uintptr_t>(allocation);
    const auto header = (raw + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - raw);
    constexpr std::uint32_t offset_storage_size = 0x1020;
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &offset_storage_size, 4);
    return reinterpret_cast<std::uint32_t*>(header + 16);
}
void free_words(std::uint32_t* words, JobMemory memory) {
    if (!words)
        return;
    auto* object = reinterpret_cast<std::byte*>(words);
    std::uint32_t displacement;
    std::memcpy(&displacement, object - 8, 4);
    memory.free(memory.process_heap, 0, object - 16 - displacement);
}
void publish(NativeOffsetStorage& storage, std::uint32_t key_id, std::uint32_t pointer_id,
             std::uint32_t offset) {
    auto& key = storage.globals.keys.at(key_id);
    key ^= offset;
    auto& output = storage.globals.pointers[pointer_id];
    publish_native_offset(key, output, storage.cursor);
}
RecvVarProxy* proxy_slot(RecvProp* property) {
    if (!property)
        return reinterpret_cast<RecvVarProxy*>(std::uintptr_t(0x20));
    return &property->proxy;
}
}

void seed_native_offset_words(std::span<std::uint32_t, 0x400> words,
                              std::uint32_t timestamp_low) noexcept {
    for (std::uint32_t index = 1; index != 0x400; ++index) {
        timestamp_low = timestamp_low * 0x343fd + 0x269ec3;
        words[index] = timestamp_low >> 16;
    }
}
void publish_native_offset(std::uint32_t& key, std::uint32_t*& pointer,
                           std::uint32_t*& cursor) noexcept {
    pointer = cursor;
    key ^= *cursor;
    ++cursor;
}

namespace {
template <class InitializeNetvars>
void initialize_offset_storage(NativeOffsetStorage& storage, JobMemory memory, void* base_client,
                               std::uint32_t timestamp_low, InitializeNetvars initialize_netvars) {
    auto* replacement = allocate_words(memory);
    if (replacement != storage.words) {
        free_words(storage.words, memory);
        storage.words = replacement;
    }
    if (!storage.words)
        throw std::bad_alloc();
    storage.words[0] = 0;
    seed_native_offset_words(std::span<std::uint32_t, 0x400>(storage.words, 0x400), timestamp_low);
    storage.globals.pointers[0x43468a90] = storage.words;
    storage.globals.pointers[0x43467a88] = storage.words;
    storage.globals.pointers[0x43467ebc] = storage.words;
    storage.words[0] = 0;
    storage.cursor = storage.words + 1;
    storage.property_proxy_slots[0] =
        proxy_slot(find_receive_property(base_client, 0x5f5bab75, 0x2de6daf5, 0x95daf62c));
    storage.property_proxy_slots[1] =
        proxy_slot(find_receive_property(base_client, 0x5f5bab75, 0x2de6daf5, 0x3006e9eb));
    (void)find_receive_property(base_client, 0x5f5bab75, 0x2de6daf5, 0xa5f4b9b7);
    storage.property_proxy_slots[2] =
        proxy_slot(find_receive_property(base_client, 0x0f0ede49, 0x8c24da3d, 0xff2eac3b));
    storage.property_proxy_slots[3] =
        proxy_slot(find_receive_property(base_client, 0x0f0ede49, 0x8c24da3d, 0xd5c777c9));
    initialize_netvars();
    const auto base =
        *storage.globals.pointers.at(0x4346a5f8) ^ storage.globals.keys.at(0x43467b88) ^ 0x950f7b20;
    constexpr std::array<std::uint32_t, 5> keys{0x43468dd8, 0x43468f2c, 0x4346a394, 0x43468e1c,
                                                0x43467578};
    constexpr std::array<std::uint32_t, 5> pointers{0x43467cd8, 0x43467634, 0x43468ed0, 0x43467ba4,
                                                    0x4346a51c};
    for (std::uint32_t index = 0; index != 5; ++index)
        publish(storage, keys[index], pointers[index], base + 4 * (index + 1));
    publish(storage, 0x43468eb4, 0x4346a53c,
            find_data_map_offset(*storage.maps.c69c5c29, 0x1e4c8218, 0xb1fb5791) - 12);
    publish(storage, 0x43467558, 0x43468d68,
            find_data_map_offset(*storage.maps.a9cd7264, 0xa9cd7264, 0x3c3e400b));
    publish(storage, 0x4346a718, 0x43467430,
            find_data_map_offset(*storage.maps.a9cd7264, 0x43fefb46, 0x8a8558a2));
    publish(storage, 0x43468d2c, 0x4346828c,
            find_data_map_offset(*storage.maps.a9cd7264, 0x43fefb46, 0x99c8229d));
    for (const auto& recipe : bindings_offset_recipes) {
        const auto source = *storage.globals.pointers.at(recipe.source_pointer) ^
                            storage.globals.keys.at(recipe.source_key) ^ recipe.xor_value;
        publish(storage, recipe.destination_key, recipe.destination_pointer,
                source + static_cast<std::uint32_t>(recipe.displacement));
    }
}
}
void initialize_native_offset_storage(NativeOffsetStorage& storage, JobMemory memory,
                                      void* base_client, std::byte* client_module,
                                      InitializationMetadata& metadata,
                                      std::uint32_t timestamp_low) {
    initialize_offset_storage(storage, memory, base_client, timestamp_low, [&] {
        initialize_native_netvars(base_client, client_module, metadata, storage.cursor,
                                  storage.maps);
    });
}
void initialize_build_offset_storage(NativeOffsetStorage& storage, JobMemory memory,
                                     void* base_client, std::byte* client_module,
                                     std::uint32_t timestamp_low) {
    struct StaticKey {
        std::uint32_t identity, value;
    };
    // These views share word0. Entity-animation engine binding resolves their
    // collision-height/time keys before any feature consumes the offsets.
    storage.globals.keys.try_emplace(0x43467160u, 0x0D560FACu);
    storage.globals.keys.try_emplace(0x43467038u, 0x432B1F5Bu);
    // Initial static key words from the pinned input build. These29 keys
    // belong to the ordinary post-netvar publications, not to saved offsets.
    constexpr std::array<StaticKey, 29> derived_keys{
        {{0x43468DD8u, 0x01D0BACCu}, {0x43468F2Cu, 0xBB78DC43u}, {0x4346A394u, 0xABD16CE2u},
         {0x43468E1Cu, 0x74019D87u}, {0x43467578u, 0x852D149Au}, {0x43468EB4u, 0x13965961u},
         {0x43467558u, 0x836CB5DBu}, {0x4346A718u, 0xB8969256u}, {0x43468D2Cu, 0x9E2078A1u},
         {0x4346A6B4u, 0x1599541Fu}, {0x43468ED4u, 0x2A671B42u}, {0x43467FACu, 0x3B1F6ECCu},
         {0x4346920Cu, 0x1B04305Au}, {0x43468524u, 0x465B42F8u}, {0x434674A8u, 0x54783141u},
         {0x4346A484u, 0x17978090u}, {0x43467B80u, 0x1CAAF0A8u}, {0x4346707Cu, 0x043FA575u},
         {0x4346A5B8u, 0x3EDECE61u}, {0x43467C70u, 0xF58E6118u}, {0x43467B8Cu, 0xD070684Cu},
         {0x43467FA8u, 0x1BC6D160u}, {0x43468C8Cu, 0x8DE575A5u}, {0x43467B94u, 0x442A38AEu},
         {0x4346A8B0u, 0x3D52A1A7u}, {0x43467D24u, 0x442A38AEu}, {0x43467500u, 0x3353C75Fu},
         {0x4346A3A8u, 0x666EC5F2u}, {0x4346A540u, 0x442A38AEu}}};
    for (const auto& key : derived_keys)
        storage.globals.keys.try_emplace(key.identity, key.value);
    initialize_offset_storage(storage, memory, base_client, timestamp_low, [&] {
        initialize_build_netvars(base_client, client_module, storage);
    });
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: bindings_pe_exports.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
namespace {
/* [dedup read/write] */
#if defined(_WIN32)
const std::byte* WINAPI native_nt_header(const std::byte* module) {
    using Function = PIMAGE_NT_HEADERS(WINAPI*)(PVOID);
    const auto native = GetModuleHandleA("ntdll.dll");
    if (!native)
        throw std::runtime_error("ntdll.dll is not loaded");
    const auto address = GetProcAddress(native, "RtlImageNtHeader");
    if (!address)
        throw std::runtime_error("ntdll!RtlImageNtHeader export is missing");
    const auto function = reinterpret_cast<Function>(address);
    return reinterpret_cast<const std::byte*>(function(const_cast<std::byte*>(module)));
}
#endif
}

NativeImageEnvironment native_image_environment() {
#if defined(_WIN32)
    return {&native_nt_header};
#else
    throw std::runtime_error("Native image exports require Windows");
#endif
}

std::byte* resolve_pe_export(std::byte* module, std::uint32_t hash, std::uint32_t ordinal,
                             bool salted_hash, NativeImageEnvironment environment) {
    if (!module)
        return nullptr;
    const auto* headers = environment.image_nt_header(module);
    if (!headers || read<std::uint16_t>(headers + 0x18) != 0x10b)
        return nullptr;
    const auto directory_rva = read<std::uint32_t>(headers + 0x78);
    const auto directory_size = read<std::uint32_t>(headers + 0x7c);
    if (!directory_rva || !directory_size)
        return nullptr;
    const auto* directory = module + directory_rva;
    const auto functions_rva = read<std::uint32_t>(directory + 0x1c);
    const auto ordinals_rva = read<std::uint32_t>(directory + 0x24);
    const auto names_rva = read<std::uint32_t>(directory + 0x20);
    const auto* functions = functions_rva ? module + functions_rva : nullptr;
    const auto* ordinals = ordinals_rva ? module + ordinals_rva : nullptr;
    const auto* names = names_rva ? module + names_rva : nullptr;
    const auto wanted = hash && salted_hash ? hash ^ 0x92ca5df1 : hash;
    const auto count = read<std::uint32_t>(directory + (hash ? 0x18 : 0x14));
    for (std::uint32_t index = 0; index != count; ++index) {
        std::uint32_t function_rva;
        if (!wanted) {
            if (ordinal != read<std::uint32_t>(directory + 0x10) + index)
                continue;
            function_rva = read<std::uint32_t>(functions + index * 4);
        } else {
            const auto name_rva = read<std::uint32_t>(names + index * 4);
            const auto actual = netvar_name_hash(reinterpret_cast<const char*>(module + name_rva));
            if (wanted != (actual ^ 0x92ca5df1))
                continue;
            const auto number = read<std::uint16_t>(ordinals + index * 2);
            function_rva = read<std::uint32_t>(functions + number * 4);
        }
        const auto directory_end = directory_rva + directory_size;
        if (function_rva >= directory_rva && function_rva < directory_end)
            return nullptr;
        return module + function_rva;
    }
    return nullptr;
}

std::byte* resolve_pe_export(std::byte* module, std::uint32_t hash, std::uint32_t ordinal,
                             bool salted_hash) {
    return resolve_pe_export(module, hash, ordinal, salted_hash, native_image_environment());
}

InterfaceRegistration* find_interface_registry(std::byte* module,
                                               NativeImageEnvironment environment) {
    auto* factory = resolve_pe_export(module, 0x2fd78929, 0, true, environment);
    if (!factory || factory[0] != std::byte{0x55} || factory[1] != std::byte{0x8b} ||
        factory[2] != std::byte{0xec})
        return nullptr;
    std::uint32_t pointer_address;
    if (factory[3] == std::byte{0x5d}) {
        const auto relative = read<std::int32_t>(factory + 5);
        pointer_address = read<std::uint32_t>(factory + relative + 15);
    } else if (factory[3] == std::byte{0x56}) {
        pointer_address = read<std::uint32_t>(factory + 6);
    } else {
        return nullptr;
    }
    return *reinterpret_cast<InterfaceRegistration**>(static_cast<std::uintptr_t>(pointer_address));
}
InterfaceRegistration* find_interface_registry(std::byte* module) {
    return find_interface_registry(module, native_image_environment());
}

InterfaceRegistration* find_interface_registration(InterfaceRegistration* first,
                                                   std::uint32_t hash) noexcept {
    for (auto* node = first; node; node = node->next)
        if (netvar_name_hash(node->name) == hash)
            return node;
    return nullptr;
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_animation_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct animation_bindings_Section {
    char name[8];
    std::uint32_t rva, size, flags;
};
struct animation_bindings_Profile {
    std::uint32_t nt, timestamp, size, checksum;
    std::array<animation_bindings_Section, 3> sections;
};
// These identities and instruction anchors describe this supported build pair;
// they are source integration evidence, not the original protected resolver.
constexpr animation_bindings_Profile animation_client_build{0x150,
                               0x65272025,
                               0x5509000,
                               0xFA635F,
                               {{{".text", 0x1000, 0xB38409, 0x60000020},
                                 {".rdata", 0xB3A000, 0x26629C, 0x40000040},
                                 {".data", 0xDA1000, 0x460C3E0, 0xC0000040}}}};
constexpr animation_bindings_Profile animation_tier0_build{0x110,
                              0x63DC10BA,
                              0x307000,
                              0x5E167,
                              {{{".text", 0x1000, 0x3BD56, 0x60000020},
                                {".rdata", 0x3D000, 0x1157E, 0x40000040},
                                {".data", 0x4F000, 0x2ADF90, 0xC0000040}}}};
struct animation_bindings_Image {
    std::byte* base;
    const animation_bindings_Profile& profile;
    bool animation_bindings_within(const void* pointer, std::size_t size, std::size_t section) const noexcept {
        const auto& s = profile.sections[section];
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin = reinterpret_cast<std::uintptr_t>(base) + s.rva;
        return address >= begin && size <= s.size && address - begin <= s.size - size;
    }
    const std::byte* animation_bindings_rdata(std::uint32_t rva, std::size_t size) const {
        require(rva <= profile.size && size <= profile.size - rva,
                "Animation PE table exceeds its image");
        const auto* value = base + rva;
        require(animation_bindings_within(value, size, 1), "Animation PE table is outside read-only data");
        return value;
    }
};
animation_bindings_Image animation_bindings_checked(void* module, const animation_bindings_Profile& profile) {
    require(module != nullptr, "Required animation client/tier0 module is not loaded");
    auto* base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == profile.nt,
            "Unsupported animation DOS/NT-header layout");
    const auto* nt = base + profile.nt;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B,
            "Unsupported animation PE32 layout");
    require(read<std::uint32_t>(nt + 8) == profile.timestamp &&
                read<std::uint32_t>(nt + 0x50) == profile.size &&
                read<std::uint32_t>(nt + 0x58) == profile.checksum,
            "Unsupported animation client/tier0 build identity");
    std::array<bool, 3> found{};
    for (std::size_t i = 0; i < 7; ++i) {
        const auto* section = nt + 0xF8 + i * 40;
        const auto rva = read<std::uint32_t>(section + 12), size = read<std::uint32_t>(section + 8);
        require(rva <= profile.size && size <= profile.size - rva,
                "Animation section exceeds its image");
        for (std::size_t j = 0; j < found.size(); ++j) {
            const auto& wanted = profile.sections[j];
            if (std::memcmp(section, wanted.name, 8))
                continue;
            require(!found[j] && rva == wanted.rva && size == wanted.size &&
                        read<std::uint32_t>(section + 36) == wanted.flags,
                    "Unsupported animation section fingerprint");
            found[j] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Required animation image section is missing");
    return {base, profile};
}
bool match(const std::byte* code, std::initializer_list<int> pattern) noexcept {
    for (const auto byte : pattern) {
        if (byte >= 0 && static_cast<unsigned char>(*code) != byte)
            return false;
        ++code;
    }
    return true;
}
void animation_bindings_expect(const animation_bindings_Image& image, const std::byte* code, std::initializer_list<int> pattern) {
    require(image.animation_bindings_within(code, pattern.size(), 0) && match(code, pattern),
            "Animation instruction fingerprint differs");
}
const std::byte* animation_bindings_unique(const animation_bindings_Image& image, std::initializer_list<int> pattern, std::uint32_t rva) {
    const auto& text = image.profile.sections[0];
    const std::byte* found = nullptr;
    for (std::uint32_t i = 0; i <= text.size - pattern.size(); ++i) {
        const auto* at = image.base + text.rva + i;
        if (!match(at, pattern))
            continue;
        require(!found, "Animation instruction anchor is ambiguous");
        found = at;
    }
    require(found == image.base + rva, "Animation instruction anchor is missing or unsupported");
    return found;
}
const std::byte* animation_bindings_absolute(const animation_bindings_Image& image, const std::byte* operand, std::size_t size,
                          std::size_t section) {
    const auto* value =
        reinterpret_cast<const std::byte*>(std::uintptr_t(read<std::uint32_t>(operand)));
    require(image.animation_bindings_within(value, size, section),
            "Decoded animation pointer has wrong section provenance");
    return value;
}
const std::byte* call(const animation_bindings_Image& image, const std::byte* code, std::uint32_t rva) {
    animation_bindings_expect(image, code, {0xE8, -1, -1, -1, -1});
    const auto offset = std::int64_t(code - image.base) + 5 + read<std::int32_t>(code + 1);
    require(offset >= 0 && offset < image.profile.size, "Animation call leaves its loaded image");
    const auto* value = image.base + static_cast<std::size_t>(offset);
    require(image.animation_bindings_within(value, 1, 0) && value == image.base + rva,
            "Animation call target is unsupported");
    return value;
}
bool equal_name(const animation_bindings_Image& image, const std::byte* at, const char* name, bool fold = false) {
    for (;; ++at, ++name) {
        require(image.animation_bindings_within(at, 1, 1), "Animation PE name leaves read-only data");
        auto actual = static_cast<unsigned char>(*at), wanted = static_cast<unsigned char>(*name);
        if (fold) {
            if (actual >= 'A' && actual <= 'Z')
                actual += 32;
            if (wanted >= 'A' && wanted <= 'Z')
                wanted += 32;
        }
        if (actual != wanted)
            return false;
        if (!actual)
            return true;
    }
}
const std::byte* named_export(const animation_bindings_Image& image, const char* name) {
    const auto* nt = image.base + image.profile.nt;
    const auto rva = read<std::uint32_t>(nt + 0x78), size = read<std::uint32_t>(nt + 0x7C);
    const auto* directory = image.animation_bindings_rdata(rva, 40);
    const auto count = read<std::uint32_t>(directory + 24),
               functions_count = read<std::uint32_t>(directory + 20);
    require(count <= image.profile.size / 4 && functions_count <= image.profile.size / 4,
            "Animation export count exceeds its image");
    const auto* names = image.animation_bindings_rdata(read<std::uint32_t>(directory + 32), count * 4);
    const auto* ordinals = image.animation_bindings_rdata(read<std::uint32_t>(directory + 36), count * 2);
    const auto* functions = image.animation_bindings_rdata(read<std::uint32_t>(directory + 28), functions_count * 4);
    for (std::uint32_t i = 0; i < count; ++i) {
        if (!equal_name(image, image.animation_bindings_rdata(read<std::uint32_t>(names + i * 4), 1), name))
            continue;
        const auto ordinal = read<std::uint16_t>(ordinals + i * 2);
        require(ordinal < functions_count, "Animation export ordinal exceeds its function table");
        const auto target = read<std::uint32_t>(functions + ordinal * 4);
        require(!(target >= rva && target - rva < size) && target < image.profile.size,
                "Animation predicate export is forwarded or invalid");
        const auto* result = image.base + target;
        require(image.animation_bindings_within(result, 1, 0),
                "Animation predicate export is outside executable text");
        return result;
    }
    throw std::runtime_error("Required tier0 ThreadInMainThread export is missing");
}
const std::byte* import_slot(const animation_bindings_Image& image, const char* module, const char* name) {
    const auto* nt = image.base + image.profile.nt;
    const auto size = read<std::uint32_t>(nt + 0x84);
    const auto* directories = image.animation_bindings_rdata(read<std::uint32_t>(nt + 0x80), size);
    for (std::uint32_t offset = 0; offset + 20 <= size; offset += 20) {
        const auto* directory = directories + offset;
        const auto original = read<std::uint32_t>(directory),
                   module_name = read<std::uint32_t>(directory + 12),
                   iat = read<std::uint32_t>(directory + 16);
        if (!original && !module_name && !iat)
            break;
        if (!equal_name(image, image.animation_bindings_rdata(module_name, 1), module, true))
            continue;
        require(original && iat, "Animation import requires original named thunks");
        for (std::uint32_t index = 0; index < image.profile.size / 4; ++index) {
            const auto value = read<std::uint32_t>(image.animation_bindings_rdata(original + index * 4, 4));
            if (!value)
                break;
            if (value & 0x80000000u)
                continue;
            if (equal_name(image, image.animation_bindings_rdata(value + 2, 1), name))
                return image.animation_bindings_rdata(iat + index * 4, 4);
        }
    }
    throw std::runtime_error("Required animation named import is missing");
}
struct Targets {
    const std::byte *prepare, *attachments, *bone_update, *predicate;
    const std::int32_t* counter;
};
Targets animation_bindings_acquire(const animation_bindings_Image& client, const animation_bindings_Image& tier0) {
    // The native SetupBones path calls prepare and immediately reads the live
    // global counter; a second counter read independently confirms its operand.
    const auto* prepare_site =
        animation_bindings_unique(client, {0x8D, 0x4F, 0xFC, 0xE8, -1, -1, -1,   -1,   0xA1, -1,   -1, -1, -1,
                        0x09, 0xB7, 0x9C, 0x26, 0,  0,  0x89, 0x87, 0x8C, 0x26, 0,  0},
               0x1D3490);
    const auto* prepare = call(client, prepare_site + 3, 0x1D2CA0);
    const auto* counter = animation_bindings_absolute(client, prepare_site + 9, 4, 2);
    require(counter == client.base + 0x5332374, "Animation bone counter operand is unsupported");
    const auto* counter_site =
        animation_bindings_unique(client, {0x8B, 0x87, 0x8C, 0x26, 0,  0,  0xF3, 0x0F, 0x11, 0x44, 0x24, 0x18,
                        0x3B, 0x05, -1,   -1,   -1, -1, 0x0F, 0x84, 0xC0, 0,    0,    0},
               0x1D33B8);
    require(animation_bindings_absolute(client, counter_site + 0xE, 4, 2) == counter,
            "Native animation counter operands disagree");
    animation_bindings_expect(client, prepare, {0x55, 0x8B, 0xEC, 0x51, 0x80, 0x3D, -1,   -1,   -1, -1,   0,
                             0x56, 0x8B, 0xF1, 0x0F, 0x84, 0xC4, 0,    0,    0,  0x80, 0x3D,
                             -1,   -1,   -1,   -1,   0,    0x0F, 0x85, 0xB7, 0,  0,    0});
    animation_bindings_absolute(client, prepare + 6, 1, 2);
    animation_bindings_absolute(client, prepare + 0x16, 1, 2);
    animation_bindings_expect(client, prepare + 0xD8, {0x5E, 0x8B, 0xE5, 0x5D, 0xC3});
    const auto* attachments_site =
        animation_bindings_unique(client, {0xF7, 0x44, 0x24, 0x4C, 0,    0x02, 0,  0,  0x75, 0x13,
                        0xF7, 0x44, 0x24, 0x14, 0,    0x02, 0,  0,  0x74, 0x09,
                        0x56, 0x8D, 0x4F, 0xFC, 0xE8, -1,   -1, -1, -1},
               0x1D3DD7);
    const auto* attachments = call(client, attachments_site + 0x18, 0x1D1BA0);
    animation_bindings_expect(client, attachments,
           {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x48, 0x53, 0x8B, 0x5D, 0x08, 0x89, 0x4D,
            0xF4, 0x56, 0x57, 0x85, 0xDB, 0x0F, 0x84, 0xF9, 0x01, 0,    0});
    animation_bindings_expect(client, attachments + 0x210, {0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0});
    const auto* update_site =
        animation_bindings_unique(client, {0xF7, 0xC7, 0, 0xFF, 0x0F, 0,    0x74, 0x0E, 0x57, 0xFF, 0xB6,
                        0xA8, 0x26, 0, 0,    0x8B, 0xCE, 0xE8, -1,   -1,   -1,   -1},
               0x3E9748);
    const auto* bone_update = call(client, update_site + 0x11, 0x432E20);
    animation_bindings_expect(client, bone_update,
           {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x70, 0x56, 0x57, 0x8B, 0xF9,
            0x89, 0x7C, 0x24, 0x38, 0x83, 0xBF, 0x50, 0x29, 0,    0,    0,    0x75, 0x16});
    animation_bindings_expect(client, bone_update + 0x687, {0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC2, 0x08, 0});
    const auto* predicate = named_export(tier0, "ThreadInMainThread");
    require(predicate == tier0.base + 0x12FE0, "tier0 thread predicate export is unsupported");
    animation_bindings_expect(tier0, predicate,
           {0xFF, 0x15, -1, -1, -1, -1, 0x33, 0xC9, 0x3B, 0x05, -1, -1, -1, -1, 0x0F, 0x94, 0xC0,
            0xC3});
    const auto* current_thread_slot = animation_bindings_absolute(tier0, predicate + 2, 4, 1);
    require(current_thread_slot == import_slot(tier0, "KERNEL32.dll", "GetCurrentThreadId"),
            "Tier0 predicate does not call its genuine thread-ID import");
    animation_bindings_absolute(tier0, predicate + 0xA, 4, 2);
    const auto* client_thread_slot = import_slot(client, "tier0.dll", "ThreadInMainThread");
    require(read<std::uint32_t>(client_thread_slot) == reinterpret_cast<std::uintptr_t>(predicate),
            "Client thread predicate import differs from the actual tier0 export");
    return {prepare, attachments, bone_update, predicate,
            reinterpret_cast<const std::int32_t*>(counter)};
}
const std::byte* acquire_animation_update(const animation_bindings_Image& client) {
    const auto* caller =
        animation_bindings_unique(client, {0xF3, 0x0F, 0x10, 0x55, 0xFC, 0x51, 0x8B, 0x8E, 0x60, 0x99, 0,
                        0,    0xF3, 0x0F, 0x10, 0x48, 0x04, 0xE8, -1,   -1,   -1,   -1},
               0x3E7AAB);
    const auto* update = call(client, caller + 0x11, 0x43E9E0);
    const auto* second =
        animation_bindings_unique(client, {0xF3, 0x0F, 0x10, 0x54, 0x24, 0x04, 0x51, 0x8B, 0x8E, 0x60, 0x99, 0,
                        0,    0xF3, 0x0F, 0x10, 0x48, 0x04, 0xE8, -1,   -1,   -1,   -1},
               0x3DE5A7);
    require(call(client, second + 0x12, 0x43E9E0) == update,
            "Native animation Update callers disagree");
    animation_bindings_expect(client, update,
           {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x18, 0x56, 0x57, 0x8B, 0xF9, 0xF3,
            0x0F, 0x11, 0x54, 0x24, 0x0C, 0xF3, 0x0F, 0x11, 0x4C, 0x24, 0x10, 0x8B, 0x4F, 0x60,
            0x85, 0xC9, 0x0F, 0x84, 0x93, 0x04, 0,    0,    0x8B, 0x01, 0x8B, 0x80, 0x70, 0x02,
            0,    0,    0xFF, 0xD0, 0x84, 0xC0, 0x75, 0x10, 0x8B, 0x47, 0x60, 0x80, 0xB8, 0xE1,
            0x9A, 0,    0,    0,    0x0F, 0x84, 0x75, 0x04, 0,    0});
    animation_bindings_expect(client, update + 0x4B7, {0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0});
    require(client.animation_bindings_within(update, 512, 0), "Animation Update startup scan leaves executable text");
    // The next native startup search uses this exact11-byte wildcard pattern.
    // Validate its actual instruction operand, but leave its publication to
    // that recovered search operation rather than inventing a later VM write.
    animation_bindings_expect(client, update + 0xE0, {0xC6, 0x05, -1, -1, -1, -1, -1, 0xF3, 0x0F, 0x5F, 0x05});
    const auto* flag = animation_bindings_absolute(client, update + 0xE2, 1, 2);
    animation_bindings_expect(client, update + 0x4AD, {0xC6, 0x05, -1, -1, -1, -1, 0x01, 0x89, 0x47, 0x70});
    require(animation_bindings_absolute(client, update + 0x4AF, 1, 2) == flag,
            "Animation Update flag stores disagree");
    return update;
}
const std::byte* acquire_animation_velocity(const animation_bindings_Image& client) {
    const auto* update = acquire_animation_update(client);
    animation_bindings_expect(client, update + 0x364, {0x8B, 0xCF, 0xE8, -1, -1, -1, -1});
    const auto* helper = call(client, update + 0x366, 0x443410);
    animation_bindings_expect(client, helper,
           {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x30, 0x56, 0x57, 0x8B, 0x3D,
            -1,   -1,   -1,   -1,   0x8B, 0xF1, 0x8B, 0xCF, 0x89, 0x74, 0x24, 0x14, 0x8B,
            0x07, 0xFF, 0x90, 0x84, 0,    0,    0,    0x8B, 0x0D, -1,   -1,   -1,   -1,
            0xF3, 0x0F, 0x7E, 0x86, 0xC8, 0,    0,    0,    0x8B, 0x86, 0xD0, 0,    0,
            0,    0x89, 0x44, 0x24, 0x28, 0x66, 0x0F, 0xD6, 0x44, 0x24, 0x20, 0x8B, 0x01,
            0x8B, 0x80, 0x74, 0x01, 0,    0,    0xFF, 0xD0, 0x84, 0xC0, 0x75, 0x38, 0x8B,
            0x0D, -1,   -1,   -1,   -1,   0x8B, 0x01, 0x8B, 0x80, 0x48, 0x01, 0,    0,
            0xFF, 0xD0, 0x84, 0xC0, 0x75, 0x24, 0x8B, 0x4E, 0x60, 0x8D, 0x54, 0x24, 0x20,
            0x52, 0x8B, 0x01, 0xFF, 0x90, 0x44, 0x02, 0,    0});
    animation_bindings_absolute(client, helper + 0xD, 4, 2);
    const auto* interface_global = animation_bindings_absolute(client, helper + 0x23, 4, 2);
    require(animation_bindings_absolute(client, helper + 0x4F, 4, 2) == interface_global,
            "Animation vector helper globals disagree");
    const auto* block = helper + 0x71;
    animation_bindings_expect(client, block, {0xF3, 0x0F, 0x10, 0x4C, 0x24, 0x28, 0xF3, 0x0F, 0x10, 0x5C,
                           0x24, 0x24, 0xF3, 0x0F, 0x10, 0x64, 0x24, 0x20, 0xEB, 0x38});
    const auto* hash_window = block + 0x14 + read<std::int8_t>(block + 0x13);
    require(hash_window == block + 0x4C,
            "Animation vector branch does not reach the startup hash input");
    // Native4333D6A2 receives exactly these32 bytes through cell+4C. The final
    // three bytes are the COMISS opcode; its relocated animation_bindings_absolute operand is
    // outside that hash window. This is code validation, not a supplied hash.
    animation_bindings_expect(client, hash_window, {0x0F, 0x28, 0xC4, 0x0F, 0x28, 0xD3, 0xF3, 0x0F, 0x59, 0xC4, 0xF3,
                                 0x0F, 0x59, 0xD3, 0xF3, 0x0F, 0x58, 0xD0, 0x0F, 0x28, 0xC1, 0xF3,
                                 0x0F, 0x59, 0xC1, 0xF3, 0x0F, 0x58, 0xD0, 0x0F, 0x2F, 0x15});
    animation_bindings_absolute(client, hash_window + 0x20, 4, 1);
    // The next startup read copies this JBE displacement (8B), not an address.
    animation_bindings_expect(client, hash_window + 0x24, {0x0F, 0x86, 0x8B, 0, 0, 0});
    return block;
}
}

const std::byte* initialize_build_animation_update_code(NativeDataImage& data,
                                                        const LoadedModuleEnvironment& modules) {
    require(sizeof(void*) == 4 && data.size() >= NativeDataImage::image_size,
            "Animation Update code binding requires the complete canonical x86 data image");
    require(
        modules.loader && modules.acquire_peb_lock && modules.release_peb_lock,
        "Animation Update code binding requires the actual loaded-module list and lock imports");
    const auto client =
        animation_bindings_checked(find_loaded_module(modules, loaded_module_name_hash(u"client.dll", -1), true, true),
                animation_client_build);
    const auto* update = acquire_animation_update(client);
    data.native_cell<const std::byte*>(0x434693F0u) = update;
    return update;
}

const std::byte* initialize_build_animation_velocity_code(NativeDataImage& data,
                                                          const LoadedModuleEnvironment& modules) {
    require(sizeof(void*) == 4 && data.size() >= NativeDataImage::image_size,
            "Animation velocity code binding requires the complete canonical x86 data image");
    require(
        modules.loader && modules.acquire_peb_lock && modules.release_peb_lock,
        "Animation velocity code binding requires the actual loaded-module list and lock imports");
    const auto client =
        animation_bindings_checked(find_loaded_module(modules, loaded_module_name_hash(u"client.dll", -1), true, true),
                animation_client_build);
    const auto* block = acquire_animation_velocity(client);
    data.native_cell<const std::byte*>(0x43467F0Cu) = block;
    return block;
}

animation::BoneAttachmentUpdate
initialize_build_animation_bindings(NativeDataImage& data, const LoadedModuleEnvironment& modules) {
    require(sizeof(void*) == 4 && data.size() >= NativeDataImage::image_size,
            "Animation bindings require the complete canonical x86 data image");
    require(modules.loader && modules.acquire_peb_lock && modules.release_peb_lock,
            "Animation bindings require the actual loaded-module list and lock imports");
    const auto client =
        animation_bindings_checked(find_loaded_module(modules, loaded_module_name_hash(u"client.dll", -1), true, true),
                animation_client_build);
    const auto tier0 =
        animation_bindings_checked(find_loaded_module(modules, loaded_module_name_hash(u"tier0.dll", -1), true, true),
                animation_tier0_build);
    const auto targets = animation_bindings_acquire(client, tier0);
    const auto update = reinterpret_cast<animation::BoneAttachmentUpdate>(
        const_cast<std::byte*>(targets.bone_update));
    data.native_cell<animation::BoneEnginePrepare>(0x4346A38Cu) =
        reinterpret_cast<animation::BoneEnginePrepare>(const_cast<std::byte*>(targets.prepare));
    data.native_cell<animation::BoneEngineAttachments>(0x4346A6B0u) =
        reinterpret_cast<animation::BoneEngineAttachments>(
            const_cast<std::byte*>(targets.attachments));
    data.native_cell<const std::int32_t*>(0x43469060u) = targets.counter;
    data.native_cell<animation::BoneEngineThreadPredicate>(0x43476920u) =
        reinterpret_cast<animation::BoneEngineThreadPredicate>(
            const_cast<std::byte*>(targets.predicate));
    data.native_cell<animation::BoneAttachmentUpdate>(0x43468F80u) = update;
    return update;
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_client_state_aliases.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct client_state_aliases_Image {
    std::byte* base;
    void code(std::uint32_t rva, std::initializer_list<int> pattern) const {
        require(rva >= 0x1000u && pattern.size() <= 0x451C32u &&
                    rva - 0x1000u <= 0x451C32u - pattern.size(),
                "Client-state alias code exceeds engine text");
        std::size_t index = 0;
        for (const int value : pattern) {
            require(value < 0 || static_cast<unsigned char>(base[rva + index]) == value,
                    "Client-state alias instruction fingerprint differs");
            ++index;
        }
    }
    std::byte* operand(std::uint32_t rva, std::uint32_t target) const {
        auto* const pointer = base + target;
        require(read<std::uint32_t>(base + rva) == reinterpret_cast<std::uintptr_t>(pointer),
                "Client-state alias instruction operand has different provenance");
        return pointer;
    }
};
client_state_aliases_Image checked_engine(void* module) {
    require(module != nullptr, "Client-state aliases require the initialized engine module");
    auto* const base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Unsupported client-state alias DOS/NT-header layout");
    const auto* const nt = base + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x64EE7E86 &&
                read<std::uint32_t>(nt + 0x50) == 0x39C0000 &&
                read<std::uint32_t>(nt + 0x58) == 0x64174E,
            "Unsupported client-state alias engine build");
    struct client_state_aliases_Section {
        char name[8];
        std::uint32_t rva, size, flags;
    };
    constexpr std::array<client_state_aliases_Section, 2> client_state_aliases_sections{
        {{".text", 0x1000, 0x451C32, 0x60000020}, {".data", 0x528000, 0x3427638, 0xC0000040}}};
    for (const auto& expected : client_state_aliases_sections) {
        unsigned matches = 0;
        for (unsigned index = 0; index < 7; ++index) {
            const auto* const section = nt + 0xF8 + index * 40;
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(read<std::uint32_t>(section + 8) == expected.size &&
                        read<std::uint32_t>(section + 12) == expected.rva &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Unsupported client-state alias engine section");
            ++matches;
        }
        require(matches == 1, "Client-state alias engine section is missing or duplicated");
    }
    return {base};
}
}

BuildClientStateAliases initialize_build_client_state_aliases(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Client-state aliases require the original x86 ABI");
    require(data.size() >= NativeDataImage::image_size,
            "Client-state aliases require complete canonical storage");
    const auto image = checked_engine(data.native_cell<void*>(0x43468D20u));
    // The actual getter adds eight to the engine's raw split-screen record.
    // All following member offsets are relative to that raw record in code.
    image.code(0xE4490,
               {0x83, 0xF9, 0xFF, 0x75, 0x18, 0xA1, -1,   -1, -1,   -1,   0xB9, -1,   -1,   -1,
                -1,   0xFF, 0x50, 0x14, 0x8B, 0x04, 0x85, -1, -1,   -1,   -1,   0x83, 0xC0, 8,
                0xC3, 0x8B, 0x04, 0x8D, -1,   -1,   -1,   -1, 0x83, 0xC0, 8,    0xC3});
    image.operand(0xE4496, 0x59F198);
    image.operand(0xE449B, 0x59F198);
    const auto* const records = image.operand(0xE44A5, 0x59F19C);
    image.operand(0xE44B0, 0x59F19C);
    const auto record = read<std::uint32_t>(records);
    require(record && (record & 3u) == 0 &&
                record <= std::numeric_limits<std::uint32_t>::max() - 0x4E70u,
            "Client-state aliases require an initialized current engine record");
    auto* const state = reinterpret_cast<std::byte*>(std::uintptr_t(record + 8u));
    require(data.native_cell<void*>(0x43468E44u) == state,
            "Client-state aliases do not share the canonical current client state");

    image.code(0xC1609,
               {0xA1, -1, -1, -1, -1, 0xC7, 0x80, 0x28, 0x4D, 0, 0, 0,    0,    0,    0,
                0xA1, -1, -1, -1, -1, 0xC7, 0x80, 0x30, 0x4D, 0, 0, 0,    0,    0,    0,
                0xA1, -1, -1, -1, -1, 0xC7, 0x80, 0x2C, 0x4D, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF});
    for (const auto operand : {0xC160Au, 0xC1619u, 0xC1628u})
        image.operand(operand, 0x59F19C);
    image.code(0xB6840, {0xA1, -1, -1, -1, -1, 0x8B, 0x80, 0x6C, 1, 0, 0, 0xC3});
    image.operand(0xB6841, 0x59F19C);
    image.code(0x25E75, {0x8B, 0x35, -1,   -1,   -1,   -1,   0x80, 0xBE, 0x1C, 0x4D, 0,    0,
                         0,    0x66, 0x0F, 0x6E, 0x86, 0x70, 1,    0,    0,    0x0F, 0x5B, 0xC0});
    image.operand(0x25E77, 0x59F19C);

    // The network handshake's sequence is the actual raw client-state delta
    // tick. Independent engine prediction and full-update reset sites load
    // the same current record and consume/reset its+174 member. Native's
    // canonical public client-state pointer names raw+8, hence public+16C.
    image.code(0xE20F0, {0x57, 0x8B, 0x3D, -1,   -1,   -1,   -1, 0x83, 0xBF, 0x08, 1,    0,   0,
                         6,    0x75, 0x4A, 0x83, 0xBF, 0x74, 1,  0,    0,    0,    0x7C, 0x41});
    image.operand(0xE20F3, 0x59F19C);
    image.code(0xE211A, {0x8B, 0x87, 0x30, 0x4D, 0,    0, 0x03, 0x87, 0x2C, 0x4D,
                         0,    0,    0x8B, 0x97, 0x74, 1, 0,    0,    0x85, 0xD2});
    image.code(0xBD15E, {0x8B, 0x35, -1,   -1,   -1,   -1, 0x83, 0xBE, 0x74, 1,  0, 0,
                         0xFF, 0x74, 0x25, 0x8D, 0x4E, 8,  0xE8, -1,   -1,   -1, -1});
    image.operand(0xBD160, 0x59F19C);
    image.code(0xBD17F, {0xC7, 0x86, 0x74, 1, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF});

    // FireEvents and its reset method share the current record's event queue.
    // The queue starts at raw+4E60; its head is at+0C and each event's next
    // link is at+38. Native4337D273 reads this queue through43475A50.
    image.code(0xB7550,
               {0x8B, 0x0D, -1, -1, -1, -1, 0x8D, 0x89, 0x60, 0x4E, 0, 0, 0xE9, 0x3F, 0x1E, 0, 0});
    image.operand(0xB7552, 0x59F19C);
    image.code(0xE0087, {0x8B, 0x1D, -1, -1, -1, -1});
    image.operand(0xE0089, 0x59F19C);
    image.code(0xE0098, {0x8D, 0x8B, 0x60, 0x4E, 0, 0, 0xE8, 0xFD, 0x92, 0xFD, 0xFF});
    image.code(0xE00BA, {0x8B, 0xBB, 0x6C, 0x4E, 0, 0, 0x85, 0xFF});
    image.code(0xE00D0, {0x66, 0x83, 0x3F, 0, 0x8B, 0x47, 0x38});
    image.code(0xE0152, {0x8B, 0x47, 0x0C, 0x8B, 0x40, 4, 0x85, 0xC0});

    auto* const globals = image.base + 0x59EE60;
    require(data.native_cell<void*>(0x43467B04u) == globals,
            "Client-state aliases do not share the canonical engine globals");
    // Frame/time publication proves the common CGlobalVars object. Independent
    // tick-count and max-client stores prove those two member identities.
    image.code(0x2296FE,
               {0xF3, 0x0F, 0x11, 0x1D, -1,   -1,   -1,   -1,   0x89, 0x15, -1,   -1,   -1,
                -1,   0xF3, 0x0F, 0x11, 5,    -1,   -1,   -1,   -1,   0xF3, 0x0F, 0x11, 0x0D,
                -1,   -1,   -1,   -1,   0xF3, 0x0F, 0x11, 0x15, -1,   -1,   -1,   -1,   0xC3});
    image.operand(0x229702, 0x59EE60);
    image.operand(0x229708, 0x59EE64);
    image.operand(0x229710, 0x59EE68);
    image.operand(0x229718, 0x59EE6C);
    image.operand(0x229720, 0x59EE80);
    image.code(0xD9AB9, {0x8B, 0x86, 0x68, 1,  0,  0,    0x8B, 0xCE, 0xA3, -1, -1, -1, -1,
                         0xE8, -1,   -1,   -1, -1, 0xF3, 0x0F, 0x11, 5,    -1, -1, -1, -1});
    image.operand(0xD9AC2, 0x59EE7C);
    image.operand(0xD9ACF, 0x59EE70);
    image.code(0x2864E2, {0x8B, 0x86, 0x80, 3, 0, 0, 0xA3, -1, -1, -1, -1, 0x8B, 0x47, 8, 0xA3, -1,
                          -1, -1, -1});
    image.operand(0x2864E9, 0x59EE78);
    image.operand(0x2864F1, 0x59EE8C);

    // The correction subtracts acknowledged and adds outgoing. The separate
    // publication/reset site must name the same two live engine counters.
    image.code(0x1A3656, {0x2B, 5,    -1,   -1,   -1,   -1,   3,    5,    -1,   -1,  -1,
                          -1,   0x66, 0x0F, 0x6E, 0xD8, 0x33, 0xC0, 0x0F, 0x5B, 0xDB});
    auto* const acknowledged = image.operand(0x1A3658, 0x391CCC8);
    auto* const outgoing = image.operand(0x1A365E, 0x391CCDC);
    image.code(0x22B32E, {0x89, 0x35, -1, -1, -1, -1, 0xC7, 5, -1, -1, -1, -1, 0, 0, 0, 0});
    image.operand(0x22B330, 0x391CCDC);
    image.operand(0x22B336, 0x391CCC8);

    const BuildClientStateAliases result{reinterpret_cast<std::int32_t*>(state + 0x4D28),
                                         reinterpret_cast<std::int32_t*>(state + 0x4D24),
                                         reinterpret_cast<std::int32_t*>(state + 0x164),
                                         reinterpret_cast<std::int32_t*>(state + 0x168),
                                         reinterpret_cast<float*>(globals),
                                         reinterpret_cast<std::int32_t*>(globals + 0x1C),
                                         reinterpret_cast<std::int32_t*>(globals + 0x18),
                                         reinterpret_cast<std::int32_t*>(acknowledged),
                                         reinterpret_cast<std::int32_t*>(outgoing)};
    data.native_cell<std::int32_t*>(0x434757DCu) = result.choked_commands;
    data.native_cell<std::int32_t*>(0x43475B2Cu) = result.last_outgoing_command;
    data.native_cell<std::int32_t*>(0x43475A60u) = result.server_tick;
    data.native_cell<std::int32_t*>(0x434758D8u) = reinterpret_cast<std::int32_t*>(state + 0x16C);
    data.native_cell<void*>(0x43475A50u) = state + 0x4E58;
    data.native_cell<float*>(0x4346A334u) = result.real_time;
    data.native_cell<std::int32_t*>(0x43468D4Cu) = result.tick_count;
    data.native_cell<std::int32_t*>(0x43469300u) = result.maximum_clients;
    data.native_cell<std::int32_t*>(0x43468F9Cu) = result.acknowledged_tick;
    data.native_cell<std::int32_t*>(0x434670A8u) = result.outgoing_tick;
    return result;
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_command_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct command_bindings_Section {
    const char* name;
    std::uint32_t rva, size, flags;
};
constexpr std::array<command_bindings_Section, 3> command_bindings_sections{{{".text\0\0", 0x1000, 0xB38409, 0x60000020},
                                           {".rdata\0", 0xB3A000, 0x26629C, 0x40000040},
                                           {".data\0\0", 0xDA1000, 0x460C3E0, 0xC0000040}}};
struct command_bindings_Image {
    std::byte* base;
    bool contains(std::uint32_t rva, std::size_t size, unsigned kind) const noexcept {
        const auto& s = command_bindings_sections[kind];
        return rva >= s.rva && size <= s.size && rva - s.rva <= s.size - size;
    }
    std::byte* code(std::uint32_t rva, std::initializer_list<int> pattern) const {
        require(contains(rva, pattern.size(), 0), "Command binding instruction leaves client text");
        auto* const output = base + rva;
        auto* cursor = output;
        for (const int value : pattern) {
            require(value < 0 || std::uint8_t(*cursor) == value,
                    "Unsupported command engine instruction");
            ++cursor;
        }
        return output;
    }
    std::byte* operand(std::uint32_t at, std::uint32_t target, std::size_t size,
                       unsigned kind) const {
        require(contains(at, 4, 0) && contains(target, size, kind),
                "Command operand has wrong section provenance");
        require(read<std::uint32_t>(base + at) == reinterpret_cast<std::uintptr_t>(base + target),
                "Command operand has a different module target");
        return base + target;
    }
    void call(std::uint32_t at, std::uint32_t target) const {
        code(at, {0xE8, -1, -1, -1, -1});
        require(std::int64_t(at) + 5 + read<std::int32_t>(base + at + 1) == target,
                "Command engine call reaches a different function");
    }
    void pointer(std::uint32_t at, std::uint32_t target, std::size_t size, unsigned kind) const {
        require(contains(at, 4, 1) && contains(target, size, kind),
                "Command vtable/RTTI target has wrong provenance");
        require(read<std::uint32_t>(base + at) == reinterpret_cast<std::uintptr_t>(base + target),
                "Command vtable/RTTI identity differs");
    }
};
command_bindings_Image command_bindings_checked(void* module) {
    require(module != nullptr, "Command bindings require the initialized client.dll module");
    auto* const base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Unsupported command client DOS header");
    const auto* nt = base + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x65272025 &&
                read<std::uint32_t>(nt + 0x50) == 0x5509000 &&
                read<std::uint32_t>(nt + 0x58) == 0xFA635F,
            "Unsupported command client build");
    std::array<bool, 3> found{};
    for (unsigned index = 0; index < 7; ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        const auto size = read<std::uint32_t>(section + 8), rva = read<std::uint32_t>(section + 12);
        require(rva <= 0x5509000 && size <= 0x5509000 - rva, "Command PE section exceeds image");
        for (unsigned kind = 0; kind < command_bindings_sections.size(); ++kind) {
            const auto& expected = command_bindings_sections[kind];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[kind] && size == expected.size && rva == expected.rva &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Command PE section profile differs");
            found[kind] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Command client section is missing");
    return {base};
}
BuildCommandBindings command_bindings_acquire(const command_bindings_Image& client) {
    auto* const seed =
        client.code(0x9A6740, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x70, 0x6A, 0x58,
                               0x8D, 0x44, 0x24, 0x1C, 0x89, 0x4C, 0x24, 8,    0x6A, 0,    0x50});
    client.call(0x9A6756, 0xACB6E0);
    client.code(0x9A675B,
                {0x6A, 4, 0x8D, 0x54, 0x24, 0x14, 0xC7, 0x44, 0x24, 0x28, 1, 0x23, 0x45, 0x67});
    client.call(0x9A6795, 0x9A65E0);
    client.call(0x9A67A5, 0x9A66B0);
    client.code(0x9A679A, {0x83, 0xC4, 0x10, 0x8D, 0x54, 0x24, 0x18, 0x8D, 0x4C, 0x24, 8});
    client.code(0x9A67AA, {0x8B, 0x44, 0x24, 0x0E, 0x8B, 0xE5, 0x5D, 0xC3});
    // This real engine caller agrees with the source master's ECX command
    // number and its31-bit mask/store. No cdecl seed thunk is assumed.
    client.code(0x3774CE, {0x8B, 0x4F, 4});
    client.call(0x3774D1, 0x9A6740);
    client.code(0x3774D6, {0x25, 0xFF, 0xFF, 0xFF, 0x7F, 0x89, 0x47, 0x40});

    client.code(0x2EFA0, {0x68, -1, -1, -1, -1, 0xC7, 5, -1, -1, -1, -1, -1, -1, -1, -1});
    client.operand(0x2EFA1, 0xB19470, 1, 0);
    client.operand(0x2EFA7, 0x5334CC0, 4, 2);
    auto* const helper = client.operand(0x2EFAB, 0xDF5B30, 0x1C, 2);
    client.code(0xB19470, {0xB9, -1, -1,   -1, -1, 0xC7, 5,  -1, -1, -1, -1, -1, -1,
                           -1,   -1, 0xC7, 5,  -1, -1,   -1, -1, 0,  0,  0,  0});
    client.operand(0xB19471, 0xDF5B34, 0x18, 2);
    client.operand(0xB19477, 0xDF5B30, 0x1C, 2);
    client.operand(0xB1947B, 0xBDC908, 12, 1);
    client.operand(0xB19481, 0x5334CC0, 4, 2);
    require(read<std::uint32_t>(helper) == reinterpret_cast<std::uintptr_t>(client.base + 0xBDC908),
            "Actual CMoveHelperClient object is not initialized with its proven vtable");
    client.pointer(0xBDC904, 0xD593A4, 20, 1);
    client.pointer(0xD593B0, 0xE217B8, 32, 2);
    constexpr char name[] = ".?AVCMoveHelperClient@@";
    require(read<std::uint32_t>(client.base + 0xD593A8) == 0 &&
                std::memcmp(client.base + 0xE217C0, name, sizeof name) == 0,
            "Command move-helper RTTI differs");
    client.pointer(0xBDC90C, 0x31F9C0, 0x12, 0);
    client.pointer(0xBDC910, 0x31F9F0, 8, 0);
    client.code(0x31F9C0, {0x55, 0x8B, 0xEC, 0x8B, 0x45, 8, 0x89, 0x41, 0x18, 0x8B, 1, 0xFF, 0x50,
                           8, 0x5D, 0xC2, 4, 0});
    client.code(0x31F9F0, {0xC7, 0x41, 0x10, 0, 0, 0, 0, 0xC3});

    // Real prediction RunCommand sets and resets the two shared static words.
    client.code(0x343C1A, {0x85, 0xFF, 0x75, 0x0C, 0xC7, 5,    -1,   -1,   -1, -1, 0xFF, 0xFF,
                           0xFF, 0xFF, 0xEB, 8,    0x8B, 0x47, 0x40, 0xA3, -1, -1, -1,   -1});
    auto* const random_seed = client.operand(0x343C20, 0xDAC254, 4, 2);
    client.operand(0x343C2E, 0xDAC254, 4, 2);
    client.code(0x343C42, {0x89, 0x35, -1, -1, -1, -1, 0xF3, 0x0F, 0x10, 0x48, 0x20});
    auto* const player = client.operand(0x343C44, 0x53323C0, 4, 2);
    client.code(0x343E87, {0xC7, 5, -1, -1, -1, -1, 0xFF, 0xFF, 0xFF, 0xFF,
                           0xC7, 5, -1, -1, -1, -1, 0,    0,    0,    0});
    client.operand(0x343E89, 0xDAC254, 4, 2);
    client.operand(0x343E93, 0x53323C0, 4, 2);

    client.code(0x25D00, {0xA1, -1, -1,   -1, -1, 0x68, -1, -1, -1, -1, 0xA3, -1, -1,
                          -1,   -1, 0xC7, 5,  -1, -1,   -1, -1, -1, -1, -1,   -1});
    auto* const flags = client.operand(0x25D01, 0x5334DA4, 4, 2);
    client.operand(0x25D11, 0x5334DA4, 4, 2);
    client.operand(0x25D0B, 0xDEF48C, 4, 2);
    client.operand(0x25D15, 0xDEF488, 12, 2);
    client.code(0x680C0, {0xA1, -1, -1,   -1, -1, 0x68, -1, -1, -1, -1, 0xA3, -1, -1,
                          -1,   -1, 0xC7, 5,  -1, -1,   -1, -1, -1, -1, -1,   -1});
    client.operand(0x680C1, 0x5334DA4, 4, 2);
    client.operand(0x680D1, 0x5334DA4, 4, 2);
    // Both real prediction branches push slot/frame and call the same
    // RestoreEntityToPredictedFrame body. It restores each eligible predicted
    // entity through the genuine paired engine operations and returns RET8.
    client.code(0x344767, {0x80, 0x7C, 0x1E, 0x48, 0, 0x75, 0x26, 0x48, 0x50, 0x57});
    client.call(0x344771, 0x344610);
    client.code(0x344A28, {0x8D, 0x41, 0xFF, 0x50, 0x57});
    client.call(0x344A2D, 0x344610);
    client.code(0x344610, {0x55, 0x8B, 0xEC, 0x8B, 0x4D, 4,  0x56, 0xE8, -1,   -1,   -1,   -1,
                           0x8B, 0x75, 8,    0xA1, -1,   -1, -1,   -1,   0x83, 0xFE, 0xFF, 0x74,
                           7,    0x8B, 4,    0xB5, -1,   -1, -1,   -1,   0x85, 0xC0, 0x74, 0x5F});
    client.call(0x344617, 0x7CD410);
    client.operand(0x344620, 0x52BF6D8, 8, 2);
    client.operand(0x34462C, 0x52BF6D8, 8, 2);
    client.code(0x344660, {0x8B, 4,    0x9D, -1,   -1,   -1,   -1,   0x8B, 0x3C, 0xB0, 0x85,
                           0xFF, 0x74, 0x1D, 0x80, 0xBF, 0xEE, 2,    0,    0,    0,    0x74,
                           0x14, 0x6A, 2,    0xFF, 0x75, 0x0C, 0x51, 0x8B, 0xCF});
    client.operand(0x344663, 0x4DF45FC, 4, 2);
    client.call(0x34467F, 0x1EC950);
    client.code(0x344684, {0x8B, 0xCF});
    client.call(0x344686, 0x1ECA30);
    client.code(0x34468B, {0x46, 0x3B, 0x75, 8, 0x7C, 0xCF, 0x5F, 0x5B, 0x5E, 0x5D, 0xC2, 8, 0});
    return {reinterpret_cast<prediction::SeedGenerator>(seed), helper,
            reinterpret_cast<std::int32_t*>(random_seed), reinterpret_cast<void**>(player),
            reinterpret_cast<prediction::PredictionFlagNode**>(flags)};
}
}
BuildCommandBindings initialize_build_command_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4 && data.size() >= NativeDataImage::image_size,
            "Command engine binding requires complete canonical x86 source data");
    const auto result = command_bindings_acquire(command_bindings_checked(data.native_cell<void*>(0x4346A55Cu)));
    data.native_cell<prediction::SeedGenerator>(0x434679ACu) = result.seed_generator;
    data.native_cell<void*>(0x43468EC8u) = result.move_helper;
    data.native_cell<std::int32_t*>(0x43468328u) = result.random_seed;
    data.native_cell<void**>(0x434693A4u) = result.prediction_player;
    data.native_cell<prediction::PredictionFlagNode**>(0x43467C58u) = result.flag_head;
    data.native_cell<MemberFunction<void, std::int32_t, std::int32_t>>(0x4346D660u) =
        reinterpret_cast<MemberFunction<void, std::int32_t, std::int32_t>>(
            data.native_cell<std::byte*>(0x4346A55Cu) + 0x344610);
    return result;
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_entity_animation_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct entity_animation_bindings_Section {
    char name[8];
    std::uint32_t rva, size, flags;
};
constexpr std::array<entity_animation_bindings_Section, 3> entity_animation_bindings_sections{{{".text", 0x1000, 0xB38409, 0x60000020},
                                           {".rdata", 0xB3A000, 0x26629C, 0x40000040},
                                           {".data", 0xDA1000, 0x460C3E0, 0xC0000040}}};
struct entity_animation_bindings_ClientImage {
    std::byte* base;
    bool contains(std::uint32_t rva, std::size_t size, unsigned section) const noexcept {
        const auto& s = entity_animation_bindings_sections[section];
        return rva >= s.rva && size <= s.size && rva - s.rva <= s.size - size;
    }
    std::byte* code(std::uint32_t rva, std::initializer_list<int> pattern) const {
        require(contains(rva, pattern.size(), 0),
                "Entity animation instruction leaves executable text");
        auto* const result = base + rva;
        auto* cursor = result;
        for (const auto value : pattern) {
            require(value < 0 || std::uint8_t(*cursor) == value,
                    "Entity animation instruction fingerprint differs");
            ++cursor;
        }
        return result;
    }
    std::byte* operand(std::uint32_t at, std::uint32_t expected, std::size_t size) const {
        require(contains(at, 4, 0) && contains(expected, size, 2),
                "Entity animation operand has wrong section provenance");
        require(read<std::uint32_t>(base + at) == reinterpret_cast<std::uintptr_t>(base + expected),
                "Entity animation data operand has unsupported target");
        return base + expected;
    }
    void call(std::uint32_t at, std::uint32_t target) const {
        code(at, {0xE8, -1, -1, -1, -1});
        require(std::int64_t(at) + 5 + read<std::int32_t>(base + at + 1) == target,
                "Entity animation setter calls a different engine entry");
    }
};
entity_animation_bindings_ClientImage entity_animation_bindings_checked(void* module) {
    require(module != nullptr, "Entity animation requires the loaded client.dll");
    auto* const base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Unsupported entity animation DOS header");
    const auto* nt = base + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x65272025 &&
                read<std::uint32_t>(nt + 0x50) == 0x5509000 &&
                read<std::uint32_t>(nt + 0x58) == 0xFA635F,
            "Unsupported entity animation client build");
    std::array<bool, 3> found{};
    for (unsigned i = 0; i < 7; ++i) {
        const auto* section = nt + 0xF8 + i * 40;
        const auto size = read<std::uint32_t>(section + 8), rva = read<std::uint32_t>(section + 12);
        require(rva <= 0x5509000 && size <= 0x5509000 - rva,
                "Entity animation PE section exceeds image");
        for (unsigned j = 0; j < entity_animation_bindings_sections.size(); ++j) {
            const auto& expected = entity_animation_bindings_sections[j];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[j] && size == expected.size && rva == expected.rva &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Entity animation section fingerprint differs");
            found[j] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Entity animation client section is missing");
    return {base};
}
BuildEntityAnimationBindings entity_animation_bindings_acquire(const entity_animation_bindings_ClientImage& client) {
    auto* const invalidate =
        client.code(0x1AEC40, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x0C, 0x53,
                               0x8B, 0x5D, 0x08, 0x8B, 0xC3, 0x56, 0x83, 0xE0, 0x04, 0xC6,
                               0x44, 0x24, 0x0B, 0x00, 0x57, 0xC1, 0xE0, 0x0A, 0x8B, 0xF9});
    client.code(0x1AEE67, {0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0});
    auto* const angles =
        client.code(0x1EA950, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x64, 0x53, 0x56,
                               0x57, 0x8B, 0xF1, 0xE8, 0x5D, 0x08, 0,    0,    0x8B, 0x7D, 0x08,
                               0xF3, 0x0F, 0x10, 0x07, 0x0F, 0x2E, 0x86, 0xC4, 0,    0,    0});
    client.code(0x1EA99F, {0x6A, 0x02, 0x8B, 0xCE});
    client.call(0x1EA9A3, 0x1AEC40);
    client.code(0x1EAA8F, {0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0});
    auto* const origin =
        client.code(0x1EA790, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x51, 0x53, 0x56, 0x57, 0x8B,
                               0xF1, 0xE8, 0x1F, 0x0A, 0,    0,    0x8B, 0x7D, 0x08, 0xF3, 0x0F,
                               0x10, 0x07, 0x0F, 0x2E, 0x86, 0xA0, 0,    0,    0});
    client.code(0x1EA7DD, {0x6A, 0x01, 0x8B, 0xCE});
    client.call(0x1EA7E1, 0x1AEC40);
    client.code(0x1EA923, {0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0});
    auto* const bounds = client.code(
        0x28CC90, {0x53, 0x8B, 0xDC, 0x83, 0xEC, 0x08, 0x83, 0xE4, 0xF8, 0x83, 0xC4, 0x04, 0x55,
                   0x8B, 0x6B, 0x04, 0x89, 0x6C, 0x24, 0x04, 0x8B, 0xEC, 0x83, 0xEC, 0x18, 0x56,
                   0x57, 0x8B, 0x7B, 0x08, 0x8B, 0xD1, 0x8B, 0x4B, 0x0C, 0x8D, 0x72, 0x08});
    client.code(0x28CE3B, {0x5E, 0x8B, 0xE5, 0x5D, 0x8B, 0xE3, 0x5B, 0xC2, 0x08, 0});

    // Paired read/write instructions identify the live byte and double cells.
    // The resource operand remains an outer pointer; its value is never copied.
    client.code(0x1E50D7, {0xA0, -1,   -1,   -1,   -1, 0x0F, 0xB7, 0xCA, 0x66, 0x42, 0x66,
                           0x89, 0x15, -1,   -1,   -1, -1,   0xC6, 0x05, -1,   -1,   -1,
                           -1,   0,    0x88, 0x81, -1, -1,   -1,   -1,   0xC3});
    auto* const transform = client.operand(0x1E50D8, 0xDACC0F, 1);
    client.operand(0x1E50EA, 0xDACC0F, 1);
    client.code(0x3E3809,
                {0xF2, 0x0F, 0x10, 0x0D, -1,   -1,   -1,   -1,   0xF2, 0x0F, 0x10, 0x15, -1,
                 -1,   -1,   -1,   0x0F, 0x28, 0xC1, 0xF2, 0x0F, 0x10, 0x5C, 0x24, 0x20});
    auto* const timestamp = client.operand(0x3E380D, 0x5344B38, 8);
    client.code(0x3E383F, {0xF2, 0x0F, 0x11, 0x1D, -1, -1, -1, -1, 0xE8, 0x04, 0xC6, 0, 0});
    client.operand(0x3E3843, 0x5344B38, 8);
    client.code(0x1F7B6C, {0x8B, 0x35, -1,   -1,   -1,   -1,   0x85, 0xF6, 0x74, 0x25, 0x8B,
                           0x4D, 0x0C, 0x8B, 0xB6, 0xD8, 0x09, 0,    0,    0x83, 0xC1, 0x08,
                           0x8B, 0x01, 0xFF, 0x50, 0x28, 0x8B, 0x0D, -1,   -1,   -1,   -1,
                           0x50, 0x8D, 0x89, 0xD8, 0x09, 0,    0,    0xFF, 0x56, 0x20});
    auto* const resource = client.operand(0x1F7B6E, 0x3231380, 4);
    client.operand(0x1F7B89, 0x3231380, 4);
    // The smoke-handle vector has data at+0 and count at+C. The add/search
    // operation reads both, then passes that same vector to the insert body.
    // Publish its address, preserving initially empty live engine contents.
    client.code(0x3D1700, {0x55, 0x8B, 0xEC, 0x53, 0x8B, 0x1D, -1,   -1,   -1,  -1, 0x56,
                           0x57, 0x33, 0xFF, 0x85, 0xDB, 0x7E, 0x57, 0xA1, -1,  -1, -1,
                           -1,   0x8B, 0x14, 0xB8, 0x83, 0xFA, 0xFF, 0x74, 0x45});
    auto* const smoke = client.operand(0x3D1713, 0x525CAA0, 20);
    client.operand(0x3D1706, 0x525CAAC, 4);
    client.code(0x3D1769, {0x8D, 0x45, 0x08, 0xB9, -1,   -1,   -1,   -1,   0x50, 0x53,
                           0xE8, 0x88, 0x5E, 0xDC, 0xFF, 0x5F, 0x5E, 0x5B, 0x5D, 0xC3});
    client.operand(0x3D176D, 0x525CAA0, 20);
    // Native ragdoll433996EA/4339974E and entity433F9EF1 call this genuine
    // client entry with only ECX. Its two exits both use RET, not RET4.
    // It rebuilds the engine render-eligibility state after source flag edits.
    auto* const visibility =
        client.code(0x1E69D0, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x24, 0x53, 0x56,
                               0x57, 0x8B, 0xF9, 0x8D, 0xB7, 0x74, 0x02, 0,    0,    0x85, 0xF6,
                               0x74, 0x06, 0xC7, 0x06, 0,    0,    0,    0,    0x83, 0xEC, 0x08,
                               0x8D, 0x4C, 0x24, 0x18, 0xE8, 0xD6, 0x82, 0x07, 0});
    client.code(0x1E6B01, {0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC3});
    client.code(0x1E6B4F, {0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC3});
    return {reinterpret_cast<MemberFunction<void, std::int32_t>>(invalidate),
            reinterpret_cast<lagcomp::SetAbsoluteVector>(angles),
            reinterpret_cast<lagcomp::SetAbsoluteVector>(origin),
            reinterpret_cast<lagcomp::SetCollisionBounds>(bounds),
            reinterpret_cast<std::uint8_t*>(transform),
            reinterpret_cast<double*>(timestamp),
            reinterpret_cast<void**>(resource),
            smoke,
            reinterpret_cast<MemberFunction<void>>(visibility)};
}
}
BuildEntityAnimationBindings initialize_build_entity_animation_bindings(
    NativeDataImage& data, const LoadedModuleEnvironment& modules, NativeOffsetStorage& offsets) {
    require(sizeof(void*) == 4 && data.size() >= NativeDataImage::image_size,
            "Entity animation binding requires the complete canonical x86 data image");
    require(modules.loader && modules.acquire_peb_lock && modules.release_peb_lock,
            "Entity animation binding requires the loaded-module list and lock imports");
    auto* const module =
        find_loaded_module(modules, loaded_module_name_hash(u"client.dll", -1), true, true);
    require(module && module == data.native_cell<void*>(0x4346A55Cu),
            "Entity animation client differs from the canonical module binding");
    const auto client = entity_animation_bindings_checked(module);
    const auto result = entity_animation_bindings_acquire(client);
    // The working client encodes these two fields in the keys while both
    // pointers retain word0. Treating the zero word as the decoded offset
    // overwrites the entity vtable during history/animation preparation.
    client.code(0x432580, {0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x10, 0xF3, 0x0F, 0x10, 0x81, 0xB4, 0,
                           0,    0,    0xF3, 0x0F, 0x58, 0x40, 8,    0xA1, -1,   -1,   -1,   -1,
                           0xF3, 0x0F, 0x11, 0x81, 0x20, 0x99, 0,    0,    0x8B, 0x40, 0x10, 0x89,
                           0x81, 0x24, 0x99, 0,    0,    0x5D, 0xC2, 0x10, 0});
    client.operand(0x432594, 0xDAF8E0, 4);
    client.code(0x432EDA, {0xF3, 0x0F, 0x10, 0x50, 0x10, 0xF3, 0x0F, 0x5C, 0x97, 0x24, 0x99, 0, 0});
    client.code(0x432EFD, {0xF3, 0x0F, 0x10, 0x9F, 0x20, 0x99, 0, 0, 0x0F, 0x2F, 0xC2});
    require(offsets.words && offsets.words[0] == 0 &&
                offsets.globals.pointers.at(0x43467EBCu) == offsets.words &&
                offsets.globals.pointers.at(0x43467A88u) == offsets.words,
            "Collision history fields require their initialized shared offset word");
    auto& collision_height_key = offsets.globals.keys.at(0x43467038u);
    auto& collision_time_key = offsets.globals.keys.at(0x43467160u);
    const auto collision_height = read<std::uint32_t>(client.base + 0x43259C);
    const auto collision_time = read<std::uint32_t>(client.base + 0x4325A5);
    // Interpolation temporarily enables this genuine client byte, then
    // restores the saved byte independently of its neighboring float/time
    // state. Both source query hooks share its live address, not a snapshot.
    client.code(0x1ECB33, {0xA0, -1, -1, -1,   -1,   0x8B, 0xCE, 0xF3, 0x0F, 0x10, 5,  -1,
                           -1,   -1, -1, 0x88, 0x44, 0x24, 8,    0xA1, -1,   -1,   -1, -1});
    auto* const interpolation = client.operand(0x1ECB34, 0x5334C84, 1);
    client.operand(0x1ECB3E, 0x5334C88, 4);
    client.operand(0x1ECB47, 0x5334C7C, 4);
    client.code(0x1ECB60,
                {0xC6, 5,    -1,   -1,   -1,   -1,   1,    0x8B, 0x80, 0x78, 2,    0,    0,
                 0xFF, 0xD0, 0x84, 0xC0, 0x8D, 0x8E, 0xC4, 3,    0,    0,    0x0F, 0x94, 0xC0});
    client.operand(0x1ECB62, 0x5334C84, 1);
    client.code(0x1ECB9A, {0xA2, -1, -1, -1, -1, 0x8B, 0x44, 0x24, 4,  0xF3, 0x0F,
                           0x11, 5,  -1, -1, -1, -1,   0xA3, -1,   -1, -1,   -1});
    client.operand(0x1ECB9B, 0x5334C84, 1);
    client.operand(0x1ECBA7, 0x5334C88, 4);
    client.operand(0x1ECBAC, 0x5334C7C, 4);
    require(
        data.native_cell<void**>(0x43468FF0u) == result.player_resource,
        "Entity animation requires the same published player-resource slot as misc engine bindings");
    collision_height_key = 0x432B1F5Bu ^ collision_height;
    collision_time_key = 0x0D560FACu ^ collision_time;
    data.native_cell<decltype(result.invalidate_physics)>(0x43468C10u) = result.invalidate_physics;
    data.native_cell<decltype(result.set_absolute_angles)>(0x434688C8u) =
        result.set_absolute_angles;
    data.native_cell<decltype(result.set_absolute_origin)>(0x43467E44u) =
        result.set_absolute_origin;
    data.native_cell<decltype(result.set_collision_bounds)>(0x4346A7F0u) =
        result.set_collision_bounds;
    data.native_cell<std::uint8_t*>(0x434683D0u) = result.transform_override;
    data.native_cell<double*>(0x4346A7E4u) = result.animation_timestamp;
    data.native_cell<void*>(0x434679C0u) = result.smoke_registry;
    data.native_cell<decltype(result.update_render_visibility)>(0x43468320u) =
        result.update_render_visibility;
    data.native_cell<std::uint8_t*>(0x43468FA4u) = reinterpret_cast<std::uint8_t*>(interpolation);
    return result;
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_esp_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct esp_bindings_Region {
    std::uint32_t rva, size, flags;
};
constexpr esp_bindings_Region esp_text{0x1000, 0xB38409, 0x60000020}, esp_rdata{0xB3A000, 0x26629C, 0x40000040},
    esp_objects{0xDA1000, 0x460C3E0, 0xC0000040};
struct esp_bindings_Client {
    std::byte* base;
    bool contains(const void* pointer, std::size_t length, esp_bindings_Region region) const noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer),
                   first = reinterpret_cast<std::uintptr_t>(base) + region.rva;
        return address >= first && length <= region.size && address - first <= region.size - length;
    }
    void esp_bindings_expect(const std::byte* code, std::initializer_list<int> pattern) const {
        require(contains(code, pattern.size(), esp_text), "ESP method is outside genuine client esp_text");
        std::size_t index = 0;
        for (const auto value : pattern) {
            require(value < 0 || static_cast<unsigned char>(code[index]) == value,
                    "ESP client instruction fingerprint differs");
            ++index;
        }
    }
    const std::byte* esp_bindings_unique(std::initializer_list<int> pattern) const {
        const std::byte* result = nullptr;
        for (std::uint32_t offset = 0; offset <= esp_text.size - pattern.size(); ++offset) {
            const auto* code = base + esp_text.rva + offset;
            std::size_t index = 0;
            bool match = true;
            for (const auto value : pattern) {
                if (value >= 0 && static_cast<unsigned char>(code[index]) != value) {
                    match = false;
                    break;
                }
                ++index;
            }
            if (!match)
                continue;
            require(!result, "ESP client function anchor is ambiguous");
            result = code;
        }
        require(result != nullptr, "ESP client function anchor is missing");
        return result;
    }
    const std::byte* call(const std::byte* instruction) const {
        esp_bindings_expect(instruction, {0xE8, -1, -1, -1, -1});
        const auto* result = instruction + 5 + read<std::int32_t>(instruction + 1);
        require(contains(result, 1, esp_text), "ESP helper call leaves genuine client esp_text");
        return result;
    }
};
esp_bindings_Client esp_bindings_checked(NativeDataImage& data, NativeImageEnvironment images) {
    auto* module = data.native_cell<std::byte*>(0x4346A55C);
    require(module && images.image_nt_header,
            "ESP entries require the loaded client and PE-header service");
    const auto* nt = images.image_nt_header(module);
    require(nt == module + 0x150 && read<std::uint16_t>(module) == 0x5A4D &&
                read<std::uint32_t>(module + 0x3C) == 0x150 && read<std::uint32_t>(nt) == 0x4550 &&
                read<std::uint16_t>(nt + 4) == 0x14C && read<std::uint16_t>(nt + 6) == 7 &&
                read<std::uint16_t>(nt + 20) == 0xE0 && read<std::uint16_t>(nt + 24) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x65272025 &&
                read<std::uint32_t>(nt + 80) == 0x5509000 &&
                read<std::uint32_t>(nt + 88) == 0xFA635F,
            "Unsupported ESP client PE32 build");
    constexpr std::array<esp_bindings_Region, 3> esp_regions{esp_text, esp_rdata, esp_objects};
    for (std::size_t index = 0; index < esp_regions.size(); ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        require(read<std::uint32_t>(section + 12) == esp_regions[index].rva &&
                    read<std::uint32_t>(section + 8) == esp_regions[index].size &&
                    read<std::uint32_t>(section + 36) == esp_regions[index].flags,
                "ESP client section geometry differs");
    }
    return {module};
}
void pattern_adapter(void* context, std::uint32_t weapon, std::int32_t mode, std::int32_t pellet,
                     float& angle, float& radius) {
    const auto entry = *static_cast<const ShotgunPatternEntry*>(context);
    entry(weapon, mode, pellet, &angle, &radius);
}
}
effects::GlowEngine BuildEspBindings::glow_engine() const {
    if (!data.native_cell<void*>(0x4346832C))
        throw std::logic_error("The genuine glow registration entry is not initialized");
    return {data.native_cell<effects::GlowRegistry*>(0x43467460),
            data.native_cell<void*>(0x4346832C)};
}
rage::ShotgunPattern BuildEspBindings::shotgun_pattern() const {
    auto& entry = data.native_cell<ShotgunPatternEntry>(0x43467DDC);
    if (!entry)
        throw std::logic_error("The genuine shotgun pattern entry is not initialized");
    return {&entry, pattern_adapter};
}
BuildEspBindings initialize_build_esp_bindings(NativeDataImage& data,
                                               NativeImageEnvironment images) {
    const auto client = esp_bindings_checked(data, images);
    const auto* glow =
        client.esp_bindings_unique({0x55, 0x8B, 0xEC, 0x51, 0x53, 0x56, 0x57, 0x8B, 0xF9, 0xF3, 0x0F, 0x11,
                       0x5D, 0xFC, 0x8B, 0x5F, 0x14, 0x83, 0xFB, 0xFF, 0x75, 0x0C, 0xFF, 0x77,
                       0x0C, 0xE8, -1,   -1,   -1,   -1,   0x8B, 0xD8, 0xEB, 0x11});
    client.esp_bindings_expect(client.call(glow + 0x19),
                  {0x55, 0x8B, 0xEC, 0x56, 0x8B, 0xF1, 0x57, 0x8B, 0x4E, 0x0C,
                   0x8B, 0x56, 4,    0x8D, 0x41, 1,    0x3B, 0xC2, 0x7E, 0x0B});
    client.esp_bindings_expect(glow + 0xD8, {0xE8, -1, -1, -1, -1, 0x5F, 0x5E, 0x8B, 0xC3, 0x5B, 0x8B, 0xE5,
                                0x5D, 0xC2, 0x14, 0});
    client.esp_bindings_expect(client.call(glow + 0xD8),
                  {0x55, 0x8B, 0xEC, 0x51, 0x89, 0x4D, 0xFC, 0x6A, 5, 0x6A, 0x34});
    const auto* pattern =
        client.esp_bindings_unique({0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x10, 0x56, 0x8B, 0x75, 8,    0x8D, 0x45,
                       0xF4, 0x50, 0xB9, -1,   -1,   -1,   -1,   0x66, 0x89, 0x75, 0xF4, 0xE8,
                       -1,   -1,   -1,   -1,   0x0F, 0xB7, 0xC0, 0xB9, 0xFF, 0xFF, 0,    0,
                       0x66, 0x3B, 0xC1, 0x75, 9,    0x56, 0xE8, -1,   -1,   -1,   -1});
    const auto* cache = read<const std::byte*>(pattern + 0xF);
    require(client.contains(cache, 0x18, esp_objects) &&
                read<const std::byte*>(pattern + 0x36) == cache + 4,
            "Shotgun pattern lookup uses inconsistent genuine cache operands");
    client.esp_bindings_expect(client.call(pattern + 0x17),
                  {0x55, 0x8B, 0xEC, 0x56, 0x57, 0x8B, 0xF9, 0xB8, 0xFF, 0xFF, 0,
                   0,    0x0F, 0xB7, 0x77, 0x10, 0x66, 0x3B, 0xF0, 0x74, 0x57});
    const auto* construct = client.call(pattern + 0x2A);
    client.esp_bindings_expect(construct,
                  {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x0C, 0x56, 0x66, 0x8B, 0x75, 8,    0x8D, 0x45,
                   0xF4, 0x57, 0x50, 0xB9, -1,   -1,   -1,   -1,   0x66, 0x89, 0x75, 0xF4});
    require(read<const std::byte*>(construct + 0x11) == cache,
            "Shotgun pattern cache and creator operands disagree");
    client.esp_bindings_expect(pattern + 0x49,
                  {0x8B, 0x55, 0x10, 0x83, 0xFA, 0x40, 0x73, 0x1F, 0x8B, 0x45, 0x14, 0x8B, 0x8C,
                   0xD6, 4,    4,    0,    0,    0x89, 8,    0x8B, 0x45, 0x18, 0x8B, 0x8C, 0xD6,
                   8,    4,    0,    0,    0x89, 8,    0x5E, 0x8B, 0xE5, 0x5D, 0xC2, 0x14, 0});
    client.esp_bindings_expect(pattern + 0xAE, {0xC2, 0x14, 0});
    client.esp_bindings_expect(pattern + 0x82, {0xFF, 0x15, -1, -1, -1, -1});
    client.esp_bindings_expect(pattern + 0x9C, {0xFF, 0x15, -1, -1, -1, -1});
    const auto* random_slot = read<const std::byte*>(pattern + 0x84);
    require(client.contains(random_slot, 4, esp_rdata) &&
                read<const std::byte*>(pattern + 0x9E) == random_slot,
            "Shotgun pattern RandomFloat imports disagree");
    auto* vstdlib = data.native_cell<std::byte*>(0x43468A7C);
    require(vstdlib != nullptr, "Shotgun pattern requires the loaded vstdlib module");
    auto* random = resolve_pe_export(vstdlib, netvar_name_hash("RandomFloat"), 0, true, images);
    require(random && read<const std::byte*>(random_slot) == random,
            "Shotgun pattern does not use genuine vstdlib RandomFloat");
    data.native_cell<void*>(0x4346832C) = const_cast<std::byte*>(glow);
    data.native_cell<ShotgunPatternEntry>(0x43467DDC) =
        reinterpret_cast<ShotgunPatternEntry>(const_cast<std::byte*>(pattern));
    return {data};
}
BuildEspBindings initialize_build_esp_bindings(NativeDataImage& data) {
    return initialize_build_esp_bindings(data, native_image_environment());
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_eye_adjustment_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct eye_adjustment_bindings_Section {
    char name[8];
    std::uint32_t eye_adjustment_bindings_rva, size, flags;
};
constexpr std::array<eye_adjustment_bindings_Section, 3> eye_adjustment_bindings_sections{{{".text", 0x1000, 0xB38409, 0x60000020},
                                           {".rdata", 0xB3A000, 0x26629C, 0x40000040},
                                           {".data", 0xDA1000, 0x460C3E0, 0xC0000040}}};
struct eye_adjustment_bindings_Image {
    std::byte* base;
    bool contains(const void* pointer, std::size_t length, std::size_t index) const noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin = reinterpret_cast<std::uintptr_t>(base) + eye_adjustment_bindings_sections[index].eye_adjustment_bindings_rva;
        return address >= begin && length <= eye_adjustment_bindings_sections[index].size &&
               address - begin <= eye_adjustment_bindings_sections[index].size - length;
    }
    std::uint32_t eye_adjustment_bindings_rva(const void* pointer) const noexcept {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer) -
                                          reinterpret_cast<std::uintptr_t>(base));
    }
};
eye_adjustment_bindings_Image eye_adjustment_bindings_checked_image(void* module) {
    require(module != nullptr, "Eye adjustment requires the initialized client module");
    auto* base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Unsupported eye-adjustment client DOS/NT layout");
    const auto* nt = base + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x65272025 &&
                read<std::uint32_t>(nt + 0x50) == 0x5509000 &&
                read<std::uint32_t>(nt + 0x58) == 0xFA635F,
            "Unsupported eye-adjustment client PE32 build");
    std::array<bool, 3> found{};
    for (std::size_t index = 0; index < 7; ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        const auto size = read<std::uint32_t>(section + 8), eye_adjustment_bindings_rva = read<std::uint32_t>(section + 12);
        require(eye_adjustment_bindings_rva <= 0x5509000 && size <= 0x5509000 - eye_adjustment_bindings_rva,
                "Eye-adjustment client section exceeds its image");
        for (std::size_t wanted = 0; wanted < eye_adjustment_bindings_sections.size(); ++wanted) {
            const auto& expected = eye_adjustment_bindings_sections[wanted];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[wanted] && eye_adjustment_bindings_rva == expected.eye_adjustment_bindings_rva && size == expected.size &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Unsupported eye-adjustment section fingerprint");
            found[wanted] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Eye-adjustment client section is missing");
    return {base};
}
bool matches(const std::byte* code, std::span<const int> pattern) noexcept {
    for (std::size_t i = 0; i < pattern.size(); ++i)
        if (pattern[i] >= 0 && static_cast<unsigned char>(code[i]) != pattern[i])
            return false;
    return true;
}
void eye_adjustment_bindings_expect(const eye_adjustment_bindings_Image& image, const std::byte* code, std::initializer_list<int> pattern) {
    require(image.contains(code, pattern.size(), 0) &&
                matches(code, {pattern.begin(), pattern.size()}),
            "Eye-adjustment instruction fingerprint differs");
}
const std::byte* eye_adjustment_bindings_acquire(const eye_adjustment_bindings_Image& image) {
    constexpr std::array pattern{0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x08, 0xF3, 0x0F,
                                 0x11, 0x54, 0x24, 0x04, 0x85, 0xD2, 0x0F, 0x88, 0x0F, 0x01, 0,
                                 0,    0x8B, 0x41, 0x04, 0x85, 0xC0, 0x75, 0x12, 0x8B, 0x01, 0x85,
                                 0xC0, 0x74, 0x08, 0x8B, 0x80, 0x2C, 0x01, 0,    0,    0xEB, 0x07,
                                 0x33, 0xC0, 0xEB, 0x03, 0x8B, 0x40, 0x50, 0x3B, 0xD0, 0x0F, 0x8D,
                                 0xEB, 0,    0,    0,    0x52, 0xE8, -1,   -1,   -1,   -1,   0xF3,
                                 0x0F, 0x10, 0x6C, 0x24, 0x04, 0x8B, 0xC8, 0x0F, 0x57, 0xF6};
    const std::byte* found = nullptr;
    for (std::uint32_t i = 0; i <= eye_adjustment_bindings_sections[0].size - pattern.size(); ++i) {
        const auto* code = image.base + eye_adjustment_bindings_sections[0].eye_adjustment_bindings_rva + i;
        if (!matches(code, pattern))
            continue;
        require(found == nullptr, "Eye-adjustment pose implementation is ambiguous");
        found = code;
    }
    require(found && image.eye_adjustment_bindings_rva(found) == 0x7DE760,
            "Eye-adjustment pose implementation is missing or unsupported");
    const auto call_rva = std::int64_t(image.eye_adjustment_bindings_rva(found)) + 0x41 + read<std::int32_t>(found + 0x3D);
    require(call_rva == 0x36B690, "Eye-adjustment descriptor helper call differs");
    const auto* descriptor = image.base + call_rva;
    eye_adjustment_bindings_expect(image, descriptor,
           {0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x08, 0x57, 0x8B, 0xF9, 0x8B, 0x4F, 0x04, 0x85,
            0xC9, 0x75, 0x15, 0x8B, 0x17, 0x8D, 0x0C, 0x80, 0x5F, 0x8B, 0x82, 0x30, 0x01,
            0,    0,    0x8D, 0x04, 0x88, 0x03, 0xC2, 0x5D, 0xC2, 0x04, 0});
    const auto constant = [&](std::size_t offset, std::uint32_t value) {
        const auto* address =
            reinterpret_cast<const std::byte*>(std::uintptr_t(read<std::uint32_t>(found + offset)));
        require(image.contains(address, 4, 1) && read<std::uint32_t>(address) == value,
                "Eye-adjustment arithmetic constant lacks verified module provenance");
    };
    constant(0x6B, 0x80000000u);
    constant(0x73, 0x4B000000u);
    constant(0x7F, 0x3F000000u);
    constant(0xB2, 0x3F800000u);
    constant(0xD7, 0x3F800000u);
    eye_adjustment_bindings_expect(image, found + 0xD3,
           {0xF3, 0x0F, 0x10, 0x15, -1,   -1,   -1,   -1,   0xF3, 0x0F, 0x10, 0x49, 0x08,
            0xF3, 0x0F, 0x10, 0x41, 0x0C, 0xF3, 0x0F, 0x5C, 0xE9, 0xF3, 0x0F, 0x5C, 0xC1,
            0x8B, 0x45, 0x08, 0xF3, 0x0F, 0x5E, 0xE8, 0x0F, 0x2F, 0xF5, 0xF3, 0x0F, 0x11,
            0x28, 0x76, 0x12, 0xC7, 0,    0,    0,    0,    0,    0xF3, 0x0F, 0x59, 0,
            0xF3, 0x0F, 0x58, 0xC1, 0x8B, 0xE5, 0x5D, 0xC3, 0x0F, 0x2F, 0xEA, 0x76, 0x06,
            0xC7, 0,    0,    0,    0x80, 0x3F, 0xF3, 0x0F, 0x59, 0,    0xF3, 0x0F, 0x58,
            0xC1, 0x8B, 0xE5, 0x5D, 0xC3, 0x8B, 0x45, 0x08, 0x0F, 0x57, 0xC0, 0xC7, 0,
            0,    0,    0,    0,    0x8B, 0xE5, 0x5D, 0xC3});
    return found;
}
}
void initialize_build_eye_adjustment_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Eye-adjustment engine publication requires x86");
    const auto image = eye_adjustment_bindings_checked_image(data.native_cell<void*>(0x4346A55Cu));
    const auto* entry = eye_adjustment_bindings_acquire(image);
    data.native_cell<std::uintptr_t>(0x43467CF0u) = reinterpret_cast<std::uintptr_t>(entry);
}
void initialize_build_pose_range_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Pose-range publication requires x86");
    const auto image = eye_adjustment_bindings_checked_image(data.native_cell<void*>(0x4346A55Cu));
    const auto* setter = eye_adjustment_bindings_acquire(image);
    const auto* entry = setter + 0x140;
    eye_adjustment_bindings_expect(image, entry,
           {0x55, 0x8B, 0xEC, 0x51, 0xF3, 0x0F, 0x11, 0x55, 0xFC, 0x85, 0xD2, 0x78, 0x3E, 0x8B,
            0x41, 0x04, 0x85, 0xC0, 0x75, 0x12, 0x8B, 0x01, 0x85, 0xC0, 0x74, 0x08, 0x8B, 0x80,
            0x2C, 0x01, 0,    0,    0xEB, 0x07, 0x33, 0xC0, 0xEB, 0x03, 0x8B, 0x40, 0x50, 0x3B,
            0xD0, 0x7D, 0x1E, 0x52, 0xE8, -1,   -1,   -1,   -1,   0xF3, 0x0F, 0x10, 0x40, 0x0C,
            0xF3, 0x0F, 0x5C, 0x40, 0x08, 0xF3, 0x0F, 0x59, 0x45, 0xFC, 0xF3, 0x0F, 0x58, 0x40,
            0x08, 0x8B, 0xE5, 0x5D, 0xC3, 0x0F, 0x57, 0xC0, 0x8B, 0xE5, 0x5D, 0xC3});
    const auto helper = std::int64_t(image.eye_adjustment_bindings_rva(entry)) + 0x33 + read<std::int32_t>(entry + 0x2F);
    require(helper == 0x36B690, "Pose-range descriptor helper differs from its verified setter");
    data.native_cell<std::uintptr_t>(0x43468CA4u) = reinterpret_cast<std::uintptr_t>(entry);
}
PoseRangeRuntime::PoseRangeRuntime(const NativeDataImage& data)
    : entry_(data.native_cell<std::uintptr_t>(0x43468CA4u)) {
    require(sizeof(void*) == 4 && entry_ != 0,
            "Pose-range conversion requires its acquired canonical x86 entry");
}
float PoseRangeRuntime::convert(const void* wrapper, std::uint32_t index, float normalized) const {
    require(entry_ != 0, "Pose-range conversion engine entry is no longer bound");
    return detail::invoke_pose_range(entry_, wrapper, index, normalized);
}
EyePoseParameterRuntime::EyePoseParameterRuntime(const NativeDataImage& data)
    : entry_(data.native_cell<std::uintptr_t>(0x43467CF0u)) {
    require(sizeof(void*) == 4 && entry_ != 0,
            "Eye pose conversion requires its acquired canonical x86 engine entry");
}
float EyePoseParameterRuntime::convert(const void* wrapper, std::uint32_t index, float angle,
                                       float& normalized) const {
    require(entry_ != 0, "Eye pose conversion engine entry is no longer bound");
    return detail::invoke_eye_pose_parameter(entry_, wrapper, index, angle, normalized);
}
#if defined(__i386__) && defined(__clang__)
__attribute__((naked, noinline)) float detail::invoke_pose_range(std::uintptr_t, const void*,
                                                                 std::uint32_t, float) {
    __asm__ __volatile__("pushl %ebp\n\t"
                         "movl %esp, %ebp\n\t"
                         "movl 8(%ebp), %eax\n\t"
                         "movl 12(%ebp), %ecx\n\t"
                         "movl 16(%ebp), %edx\n\t"
                         "movss 20(%ebp), %xmm2\n\t"
                         "call *%eax\n\t"
                         "subl $4, %esp\n\t"
                         "movss %xmm0, (%esp)\n\t"
                         "flds (%esp)\n\t"
                         "leave\n\t"
                         "ret");
}
__attribute__((naked, noinline)) float
detail::invoke_eye_pose_parameter(std::uintptr_t, const void*, std::uint32_t, float, float&) {
    __asm__ __volatile__("pushl %ebp\n\t"
                         "movl %esp, %ebp\n\t"
                         "movl 8(%ebp), %eax\n\t"
                         "movl 12(%ebp), %ecx\n\t"
                         "movl 16(%ebp), %edx\n\t"
                         "movss 20(%ebp), %xmm2\n\t"
                         "pushl 24(%ebp)\n\t"
                         "call *%eax\n\t"
                         "addl $4, %esp\n\t"
                         "subl $4, %esp\n\t"
                         "movss %xmm0, (%esp)\n\t"
                         "flds (%esp)\n\t"
                         "leave\n\t"
                         "ret");
}
#else
float detail::invoke_pose_range(std::uintptr_t, const void*, std::uint32_t, float) {
    throw std::logic_error(
        "Pose-range conversion requires the explicit Windows x86 SSE ABI bridge");
}
float detail::invoke_eye_pose_parameter(std::uintptr_t, const void*, std::uint32_t, float, float&) {
    throw std::logic_error("Eye pose conversion requires the explicit Windows x86 SSE ABI bridge");
}
#endif
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_final_startup_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct final_startup_bindings_Image {
    std::byte* base;
    std::uint32_t size, text_begin, text_size;
    void code(std::uint32_t offset, std::initializer_list<int> pattern) const {
        require(offset >= text_begin && pattern.size() <= text_size &&
                    offset - text_begin <= text_size - pattern.size(),
                "Final startup instruction exceeds its module text");
        std::size_t index = 0;
        for (int value : pattern) {
            if (value >= 0 && base[offset + index] != std::byte(value)) {
                char message[192];
                std::snprintf(
                    message, sizeof message,
                    "Final startup offset mismatch: module %p + 0x%08X, expected %02X, got %02X",
                    static_cast<void*>(base), offset + static_cast<std::uint32_t>(index),
                    static_cast<unsigned>(value), std::to_integer<unsigned>(base[offset + index]));
                throw std::runtime_error(message);
            }
            ++index;
        }
    }
    bool contains(const void* pointer, std::size_t length) const {
        const auto first = reinterpret_cast<std::uintptr_t>(base),
                   address = reinterpret_cast<std::uintptr_t>(pointer);
        return address >= first && length <= size && address - first <= size - length;
    }
    const std::byte* final_startup_bindings_unique(std::initializer_list<int> pattern) const {
        const std::byte* found = nullptr;
        for (std::uint32_t offset = text_begin; offset - text_begin <= text_size - pattern.size();
             ++offset) {
            std::size_t index = 0;
            bool matches = true;
            for (int value : pattern) {
                if (value >= 0 && base[offset + index] != std::byte(value)) {
                    matches = false;
                    break;
                }
                ++index;
            }
            if (matches) {
                require(!found, "Final startup instruction pattern is ambiguous");
                found = base + offset;
            }
        }
        require(found != nullptr, "A required final startup instruction pattern is absent");
        return found;
    }
};
final_startup_bindings_Image final_startup_bindings_checked(void* pointer, std::uint32_t nt_offset, std::uint32_t timestamp, std::uint32_t size,
              std::uint32_t checksum) {
    auto* const base = static_cast<std::byte*>(pointer);
    require(base && read<std::uint16_t>(base) == 0x5A4D &&
                read<std::uint32_t>(base, 0x3C) == nt_offset,
            "Unsupported final startup DOS/NT layout");
    const auto* const nt = base + nt_offset;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt, 4) == 0x14C &&
                read<std::uint16_t>(nt, 0x14) == 0xE0 && read<std::uint16_t>(nt, 0x18) == 0x10B &&
                read<std::uint32_t>(nt, 8) == timestamp && read<std::uint32_t>(nt, 0x50) == size &&
                read<std::uint32_t>(nt, 0x58) == checksum,
            "Unsupported final startup module build");
    final_startup_bindings_Image result{base, size, 0, 0};
    unsigned found = 0;
    const auto sections = read<std::uint16_t>(nt, 6);
    require(sections && sections <= 16, "Final startup PE section count is invalid");
    for (unsigned index = 0; index < sections; ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        if (!std::memcmp(section, ".text\0\0\0", 8)) {
            result.text_begin = read<std::uint32_t>(section, 12);
            result.text_size = read<std::uint32_t>(section, 8);
            require(result.text_size && result.text_begin < size &&
                        result.text_size <= size - result.text_begin &&
                        (read<std::uint32_t>(section, 36) & 0x60000000u) == 0x60000000u,
                    "Final startup text extent or protection differs");
            ++found;
        }
    }
    require(found == 1, "Final startup module has no final_startup_bindings_unique executable text section");
    return result;
}
final_startup_bindings_Image checked_overlay(void* pointer) {
    const auto identity = require_overlay_build_identity(static_cast<const std::byte*>(pointer));
    return final_startup_bindings_checked(pointer, 0x130, identity.timestamp, 0x190000, identity.checksum);
}
}
BuildFinalStartupBindings initialize_build_final_startup_bindings(NativeDataImage& data) {
    const auto client =
        final_startup_bindings_checked(data.native_cell<void*>(0x4346A55Cu), 0x150, 0x65272025, 0x5509000, 0xFA635F);
    const auto engine =
        final_startup_bindings_checked(data.native_cell<void*>(0x43468D20u), 0x150, 0x64EE7E86, 0x39C0000, 0x64174E);
    const auto overlay = checked_overlay(data.native_cell<void*>(0x43468D08u));
    client.code(0x3DC9B0, {0x55, 0x8B, 0xEC, 0x8B, 0x55, 8,    0x83, 0xEC, 0x1C, 0xF6,
                           0x42, 0x1C, 1,    0x0F, 0x84, 0x85, 0,    0,    0,    0x8B,
                           0x4A, 0x1C, 0x8B, 0xC1, 0xD1, 0xE8, 0xA8, 1,    0x74, 0x7A});
    client.code(0x3DCA1D, {0x8D, 0x88, -1, -1, -1, -1, 0xF3, 0x0F, 0x11, 0x45, 0xE4});
    const auto feedback = read<std::uint32_t>(client.base + 0x3DCA1F);
    require(feedback >= 0x14 && feedback < 0x20000,
            "Final player feedback field is outside the supported entity layout");
    // The real decoder uses a36-byte record selected from its type word. The
    // final float callback is its field+8 in the first record at52B268.
    engine.code(0x1FFDC4, {0x8B, 0x42, 8, 0x56, 0x52, 0x8D, 4, 0xC0, 0x8B, 4, 0x85, -1, -1, -1, -1,
                           0xFF, 0xD0});
    const auto table = read<std::byte*>(engine.base + 0x1FFDCF);
    require(table == engine.base + 0x52B268, "Final float decoder table operand differs");
    engine.code(0x1FD950,
                {0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 8, 0x8B, 0x56, 0x34, 0x8B, 0x4E, 0x30, 0xE8});
    auto** const float_slot = reinterpret_cast<void**>(table + 8);
    const auto* const returned =
        overlay.final_startup_bindings_unique({0x3D, -1, -1,   -1, -1,   0x73, 0x1A, 0x68, -1,   -1,   -1,   -1,
                        0xE8, -1, -1,   -1, -1,   0x8B, 0x0D, -1,   -1,   -1,   -1,   0x83,
                        0xC4, 4,  0x8B, 1,  0x6A, 0,    0xFF, 0x50, 0x14, 0x3B, 0x7B, 0x3C});
    auto** const stream = read<void**>(returned + 19);
    const auto* const stored = overlay.final_startup_bindings_unique({0x89, 0x1D, -1, -1, -1, -1, 0xF3, 0x0F, 0x10, 0x83});
    const auto* const matrix = read<const std::byte*>(stored + 2);
    require(overlay.contains(stream, sizeof(void*)) && overlay.contains(matrix, sizeof(void*)),
            "Final overlay operands do not refer to their actual module storage");
    data.native_cell<const std::byte*>(0x4346D540u) = client.base + 0x3DC9B0;
    data.native_cell<void**>(0x43476A40u) = float_slot;
    data.native_cell<const std::byte*>(0x43468D94u) = returned;
    data.native_cell<const std::byte*>(0x43468350u) = matrix;
    // These two static masks precede the actual final XOR publication. They
    // are native data, not retained instructions or captured entity pointers.
    data.native_cell<std::uint32_t>(0x434689D0u) = 0x74108329u ^ (feedback - 0x14u);
    data.native_cell<std::uint32_t>(0x434691E0u) = 0x0C5D8DD5u ^ feedback;
    return {client.base + 0x3DCA1D, feedback, float_slot, returned, stream};
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_game_state_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct game_state_bindings_SectionProfile {
    char name[8];
    std::uint32_t game_state_bindings_rva, size, flags;
};
struct game_state_bindings_BuildProfile {
    std::uint32_t nt_offset, timestamp, image_size, checksum;
    std::uint16_t section_count;
    std::array<game_state_bindings_SectionProfile, 3> sections;
};
constexpr game_state_bindings_BuildProfile game_state_client_build{0x150,
                                    0x65272025,
                                    0x5509000,
                                    0xFA635F,
                                    7,
                                    {{{".text", 0x1000, 0xB38409, 0x60000020},
                                      {".rdata", 0xB3A000, 0x26629C, 0x40000040},
                                      {".data", 0xDA1000, 0x460C3E0, 0xC0000040}}}};
constexpr game_state_bindings_BuildProfile game_state_engine_build{0x150,
                                    0x64EE7E86,
                                    0x39C0000,
                                    0x64174E,
                                    7,
                                    {{{".text", 0x1000, 0x451C32, 0x60000020},
                                      {".rdata", 0x453000, 0xD4766, 0x40000040},
                                      {".data", 0x528000, 0x3427638, 0xC0000040}}}};
struct game_state_bindings_Image {
    std::byte* base;
    const game_state_bindings_BuildProfile& build;
    bool contains(const void* pointer, std::size_t length, std::size_t section) const noexcept {
        const auto& region = build.sections[section];
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin = reinterpret_cast<std::uintptr_t>(base) + region.game_state_bindings_rva;
        return address >= begin && length <= region.size && address - begin <= region.size - length;
    }
    std::uint32_t game_state_bindings_rva(const void* pointer) const noexcept {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer) -
                                          reinterpret_cast<std::uintptr_t>(base));
    }
};
game_state_bindings_Image game_state_bindings_checked_image(void* module, const game_state_bindings_BuildProfile& profile) {
    require(module != nullptr, "Game-state bindings require initialized client/engine modules");
    auto* const base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D &&
                read<std::uint32_t>(base + 0x3C) == profile.nt_offset,
            "Unsupported game-state DOS/NT-header layout");
    const auto* nt = base + profile.nt_offset;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == profile.section_count &&
                read<std::uint16_t>(nt + 0x14) == 0xE0 && read<std::uint16_t>(nt + 0x18) == 0x10B,
            "Game-state bindings require the verified PE32 layout");
    require(read<std::uint32_t>(nt + 8) == profile.timestamp &&
                read<std::uint32_t>(nt + 0x50) == profile.image_size &&
                read<std::uint32_t>(nt + 0x58) == profile.checksum,
            "Unsupported client/engine game-state build identity");
    std::array<bool, 3> found{};
    for (std::uint16_t index = 0; index < profile.section_count; ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        const auto game_state_bindings_rva = read<std::uint32_t>(section + 12), size = read<std::uint32_t>(section + 8);
        require(game_state_bindings_rva <= profile.image_size && size <= profile.image_size - game_state_bindings_rva,
                "Game-state PE section exceeds its image");
        for (std::size_t wanted = 0; wanted < profile.sections.size(); ++wanted) {
            const auto& expected = profile.sections[wanted];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[wanted] && game_state_bindings_rva == expected.game_state_bindings_rva && size == expected.size &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Unsupported game-state section fingerprint");
            found[wanted] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Game-state PE section is missing");
    return {base, profile};
}
/* [dedup matches] */
void game_state_bindings_expect_code(const game_state_bindings_Image& image, const std::byte* code, std::initializer_list<int> pattern) {
    require(image.contains(code, pattern.size(), 0),
            "Game-state method is outside executable text");
    require(matches(code, {pattern.begin(), pattern.size()}),
            "Game-state method instruction fingerprint differs");
}
const std::byte* game_state_bindings_unique_code(const game_state_bindings_Image& image, std::initializer_list<int> pattern,
                             std::uint32_t expected_rva) {
    const auto& text = image.build.sections[0];
    const std::byte* found = nullptr;
    for (std::uint32_t offset = 0; offset <= text.size - pattern.size(); ++offset) {
        const auto* code = image.base + text.game_state_bindings_rva + offset;
        if (!matches(code, {pattern.begin(), pattern.size()}))
            continue;
        require(found == nullptr, "Game-state instruction anchor is ambiguous");
        found = code;
    }
    require(found && image.game_state_bindings_rva(found) == expected_rva,
            "Game-state instruction anchor is missing or unsupported");
    return found;
}
const std::byte* game_state_bindings_absolute(const game_state_bindings_Image& image, const std::byte* operand, std::size_t section,
                          std::size_t length) {
    const auto* pointer =
        reinterpret_cast<const std::byte*>(std::uintptr_t(read<std::uint32_t>(operand)));
    require(image.contains(pointer, length, section),
            "Game-state pointer has invalid module/section provenance");
    return pointer;
}
const std::byte* interface_table(const game_state_bindings_Image& image, void* object, std::size_t entries) {
    require(image.contains(object, 4, 2), "Game-state interface object is outside its module data");
    return game_state_bindings_absolute(image, static_cast<const std::byte*>(object), 1, entries * 4);
}
const std::byte* method(const game_state_bindings_Image& image, const std::byte* table, std::size_t slot,
                        std::uint32_t expected_rva) {
    const auto* function = game_state_bindings_absolute(image, table + slot * 4, 0, 1);
    require(image.game_state_bindings_rva(function) == expected_rva,
            "Game-state interface has an unsupported virtual method");
    return function;
}
void require_rtti(const game_state_bindings_Image& image, const std::byte* table, std::uint32_t object_offset,
                  const char* type_name) {
    require(image.contains(table - 4, 4, 1), "Game-state vtable has no RTTI prefix");
    const auto* locator = game_state_bindings_absolute(image, table - 4, 1, 20);
    require(read<std::uint32_t>(locator) == 0 &&
                read<std::uint32_t>(locator + 4) == object_offset &&
                read<std::uint32_t>(locator + 8) == 0,
            "Game-state RTTI locator has an unsupported object layout");
    const auto length = std::strlen(type_name) + 1;
    const auto* type = game_state_bindings_absolute(image, locator + 12, 2, 8 + length);
    game_state_bindings_absolute(image, locator + 16, 1, 16);
    require(std::memcmp(type + 8, type_name, length) == 0,
            "Game-state RTTI has an unexpected concrete type");
}
struct game_state_bindings_Acquired {
    std::byte* globals;
    void* input;
    void* client_state;
};
game_state_bindings_Acquired game_state_bindings_acquire(const game_state_bindings_Image& client, const game_state_bindings_Image& engine, void* base_client, void* engine_client) {
    const auto* client_table = interface_table(client, base_client, 17);
    const auto* initialize = method(client, client_table, 0, 0x25FC10);
    game_state_bindings_expect_code(client, initialize,
                {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x81, 0xEC, 0x0C, 0x01, 0,    0,
                 0x56, 0x83, 0xEC, 0x0C, 0xE8, -1,   -1,   -1,   -1,   0x8B, 0x45, 0x0C,
                 0x8D, 0x4D, 0x08, 0x83, 0xC4, 0x0C, 0xA3, -1,   -1,   -1,   -1});
    const auto* frame_stage = method(client, client_table, 11, 0x260740);
    game_state_bindings_expect_code(client, frame_stage, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xC0, 0x83, 0xEC, 0x3C, 0xA1,
                                      -1,   -1,   -1,   -1,   0x56, 0xF3, 0x0F, 0x10, 0x40, 0x14});
    const auto* globals_slot = game_state_bindings_absolute(client, frame_stage + 0xA, 2, 4);
    require(read<std::uint32_t>(initialize + 0x1F) ==
                reinterpret_cast<std::uintptr_t>(globals_slot),
            "Client initialization and frame-stage globals operands disagree");
    auto* const globals = const_cast<std::byte*>(game_state_bindings_absolute(engine, globals_slot, 2, 0x40));

    const auto* input_call = method(client, client_table, 16, 0x260A60);
    game_state_bindings_expect_code(client, input_call, {0xA1, -1, -1, -1, -1, 0xB9, -1, -1, -1, -1, 0xFF, 0x60, 0x5C});
    const auto* input = game_state_bindings_absolute(client, input_call + 6, 2, 0xF8);
    require(read<std::uint32_t>(input_call + 1) == reinterpret_cast<std::uintptr_t>(input),
            "CInput virtual call uses different object and vtable operands");
    const auto* input_table = game_state_bindings_absolute(client, input, 1, 24 * 4);
    require_rtti(client, input_table, 0, ".?AVCCSInput@@");

    const auto* engine_table = interface_table(engine, engine_client, 13);
    const auto* local_player = method(engine, engine_table, 12, 0xB6800);
    game_state_bindings_expect_code(engine, local_player,
                {0xA1, -1,   -1, -1, -1, 0xB9, -1,   -1,   -1,   -1,   0xFF, 0x50, 0x14, 0x8B,
                 0x04, 0x85, -1, -1, -1, -1,   0x8B, 0x80, 0x80, 0x01, 0,    0,    0x40, 0xC3});
    const auto* get_state = game_state_bindings_unique_code(
        engine, {0x83, 0xF9, 0xFF, 0x75, 0x18, 0xA1, -1,   -1, -1,   -1,   0xB9, -1,   -1,   -1,
                 -1,   0xFF, 0x50, 0x14, 0x8B, 0x04, 0x85, -1, -1,   -1,   -1,   0x83, 0xC0, 0x08,
                 0xC3, 0x8B, 0x04, 0x8D, -1,   -1,   -1,   -1, 0x83, 0xC0, 0x08, 0xC3},
        0xE4490);
    const auto* manager = game_state_bindings_absolute(engine, get_state + 0xB, 2, 12);
    require(read<std::uint32_t>(get_state + 6) == reinterpret_cast<std::uintptr_t>(manager) &&
                read<std::uint32_t>(local_player + 1) ==
                    reinterpret_cast<std::uintptr_t>(manager) &&
                read<std::uint32_t>(local_player + 6) == reinterpret_cast<std::uintptr_t>(manager),
            "Engine client-state current-slot manager operands disagree");
    const auto* records = game_state_bindings_absolute(engine, get_state + 0x15, 2, 8);
    require(
        records == manager + 4 &&
            read<std::uint32_t>(get_state + 0x20) == reinterpret_cast<std::uintptr_t>(records) &&
            read<std::uint32_t>(local_player + 0x10) == reinterpret_cast<std::uintptr_t>(records),
        "Engine client-state current/explicit record operands disagree");
    const auto* manager_table = game_state_bindings_absolute(engine, manager, 1, 6 * 4);
    const auto* current_slot = game_state_bindings_absolute(engine, manager_table + 5 * 4, 0, 3);
    // This exact build's current-slot implementation always returns zero.
    // Requiring the real method body avoids inventing a split-screen policy.
    game_state_bindings_expect_code(engine, current_slot, {0x33, 0xC0, 0xC3});
    const auto record = read<std::uint32_t>(records);
    require(record != 0 && (record & 3) == 0 &&
                record <= std::numeric_limits<std::uint32_t>::max() - 12,
            "Engine current client-state record is not initialized");
    auto* const client_state = reinterpret_cast<std::byte*>(std::uintptr_t(record + 8));
    const auto* state_table = game_state_bindings_absolute(engine, client_state + 4, 1, 4);
    require_rtti(engine, state_table, 4, ".?AVCClientState@@");
    return {globals, const_cast<std::byte*>(input), client_state};
}
}

void initialize_build_game_state_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Game-state bindings require the original x86 ABI");
    require(data.size() >= NativeDataImage::image_size,
            "Game-state bindings require the complete canonical data image");
    const auto client = game_state_bindings_checked_image(data.native_cell<void*>(0x4346A55Cu), game_state_client_build);
    const auto engine = game_state_bindings_checked_image(data.native_cell<void*>(0x43468D20u), game_state_engine_build);
    const auto bindings = game_state_bindings_acquire(client, engine, data.native_cell<void*>(0x434683E0u),
                                  data.native_cell<void*>(0x4346A514u));
    data.native_cell<void*>(0x43467B04u) = bindings.globals;
    data.native_cell<void*>(0x4346A894u) = bindings.input;
    data.native_cell<void*>(0x43468E44u) = bindings.client_state;
    data.native_cell<float*>(0x43467EB8u) = reinterpret_cast<float*>(bindings.globals + 0x10);
    data.native_cell<std::int32_t*>(0x434678F8u) =
        reinterpret_cast<std::int32_t*>(bindings.globals + 4);
    data.native_cell<float*>(0x4346A338u) = reinterpret_cast<float*>(bindings.globals + 0x20);
}

void initialize_build_client_factories(NativeDataImage& data) {
    const auto client = game_state_bindings_checked_image(data.native_cell<void*>(0x4346A55Cu), game_state_client_build);
    const auto* glow =
        game_state_bindings_unique_code(client, {0xA1, -1,   -1, -1, -1, 0xA8, 1,  0x75, 0x4B, 0x0F, 0x57,
                             0xC0, 0xC7, 5,  -1, -1, -1,   -1, 0,    0,    0,    0,
                             0x0F, 0x11, 5,  -1, -1, -1,   -1, 0x83, 0xC8, 1},
                    0x2C8F50);
    game_state_bindings_expect_code(client, glow + 0x20,
                {0xC7, 5,  -1,   -1, -1,   -1,   0,  0,    0,  0,  0x0F, 0x28, 5,    -1,   -1,
                 -1,   -1, 0x68, -1, -1,   -1,   -1, 0xA3, -1, -1, -1,   -1,   0x0F, 0x11, 5,
                 -1,   -1, -1,   -1, 0xC7, 5,    -1, -1,   -1, -1, 0,    0,    0,    0,    0xE8,
                 -1,   -1, -1,   -1, 0x83, 0xC4, 4,  0xB8, -1, -1, -1,   -1,   0xC3});
    const auto* glow_guard = game_state_bindings_absolute(client, glow + 1, 2, 4);
    const auto* glow_object = game_state_bindings_absolute(client, glow + 0x19, 2, 0x2C);
    require(glow_object == glow_guard + 4 &&
                game_state_bindings_absolute(client, glow + 0xE, 2, 4) == glow_object + 0x20 &&
                game_state_bindings_absolute(client, glow + 0x22, 2, 4) == glow_object + 0x24 &&
                game_state_bindings_absolute(client, glow + 0x37, 2, 4) == glow_guard &&
                game_state_bindings_absolute(client, glow + 0x3E, 2, 16) == glow_object + 0x10 &&
                game_state_bindings_absolute(client, glow + 0x44, 2, 4) == glow_object + 0x28 &&
                game_state_bindings_absolute(client, glow + 0x55, 2, 0x2C) == glow_object,
            "Glow factory singleton initialization and return operands disagree");
    (void)game_state_bindings_absolute(client, glow + 0x2D, 1, 16);
    (void)game_state_bindings_absolute(client, glow + 0x32, 0, 1);

    const auto* items =
        game_state_bindings_unique_code(client, {0xA1, -1, -1,   -1,   -1,   0x85, 0xC0, 0x75, 0x53, 0xA1, -1,
                             -1,   -1, -1,   0x56, 0x68, 0xE8, 7,    0,    0,    0x8B, 8,
                             0x8B, 1,  0xFF, 0x50, 4,    0x8B, 0xF0, 0x85, 0xF6, 0x74, 0x2E},
                    0x712740);
    game_state_bindings_expect_code(client, items + 0x21,
                {0x68, 0xE8, 7,    0,    0,    0x6A, 0,    0x56, 0xE8, -1,   -1,   -1,
                 -1,   0x83, 0xC4, 0x0C, 0xC7, 6,    -1,   -1,   -1,   -1,   0x8D, 0x4E,
                 4,    0xE8, -1,   -1,   -1,   -1,   0xC7, 6,    -1,   -1,   -1,   -1,
                 0x8B, 0xC6, 0x89, 0x35, -1,   -1,   -1,   -1,   0x5E, 0xC3, 0xC7, 5,
                 -1,   -1,   -1,   -1,   0,    0,    0,    0,    0x33, 0xC0, 0x5E, 0xC3});
    const auto* item_slot = game_state_bindings_absolute(client, items + 1, 2, 4);
    require(game_state_bindings_absolute(client, items + 0x49, 2, 4) == item_slot &&
                game_state_bindings_absolute(client, items + 0x51, 2, 4) == item_slot,
            "Item-system factory singleton allocation and lookup operands disagree");
    (void)game_state_bindings_absolute(client, items + 0xA, 1, 4); // genuine imported g_pMemAlloc variable
    (void)game_state_bindings_absolute(client, items + 0x33, 1, 4); // base-class vtable during construction
    (void)game_state_bindings_absolute(client, items + 0x41, 1, 4); // final item-system vtable
    // All relative calls must remain the actual compiled helper identities.
    // No game allocation or constructor executes during binding discovery.
    for (const auto pair : std::array<std::pair<const std::byte*, std::uint32_t>, 3>{
             {{glow + 0x4C, 0xAC74D7u}, {items + 0x29, 0xACB6E0u}, {items + 0x3A, 0x6D1160u}}}) {
        const auto target = reinterpret_cast<std::uintptr_t>(pair.first) + 5 +
                            static_cast<std::intptr_t>(read<std::int32_t>(pair.first + 1));
        const auto* pointer = reinterpret_cast<const std::byte*>(target);
        require(client.contains(pointer, 1, 0) && client.game_state_bindings_rva(pointer) == pair.second,
                "Client singleton factory has an unsupported helper call");
    }
    // The refresh notifications borrow the address of the engine's lazy
    // CUtlVector pointer. Its null contents remain valid until the real
    // client getter allocates it; discovery never invokes that allocator.
    game_state_bindings_expect_code(client, client.base + 0x448C4, {0xC7, 5, -1, -1, -1, -1, 0, 0, 0, 0});
    const auto* refresh = game_state_bindings_absolute(client, client.base + 0x448C6, 2, 4);
    require(client.game_state_bindings_rva(refresh) == 0x5344CE0,
            "Startup refresh pointer has unsupported client provenance");
    game_state_bindings_expect_code(client, client.base + 0x418DC0,
                {0x56, 0x33, 0xF6, 0xA1, -1, -1, -1, -1, 0x85, 0xC0, 0x75, 0x79});
    require(game_state_bindings_absolute(client, client.base + 0x418DC4, 2, 4) == refresh,
            "Startup refresh initializer and notifier name different pointer slots");
    game_state_bindings_expect_code(client, client.base + 0x418E40,
                {0xA3, -1,   -1, -1,   -1,   0x3B, 0x70, 0x0C, 0x7D, 0x0F, 0x8B, 0,    0x8B, 0x0C,
                 0xB0, 0x8B, 1,  0xFF, 0x10, 0x46, 0xE9, 0x6A, 0xFF, 0xFF, 0xFF, 0x5E, 0xC3});
    require(game_state_bindings_absolute(client, client.base + 0x418E41, 2, 4) == refresh,
            "Startup refresh notifier does not reload its own vector publication");
    game_state_bindings_expect_code(client, client.base + 0x418FA0, {0xA1, -1, -1, -1, -1, 0x85, 0xC0, 0x75, 0x49});
    game_state_bindings_expect_code(
        client, client.base + 0x418FDF,
        {0x89, 0x0D, -1, -1, -1, -1, 0xC3, 0xC7, 5, -1, -1, -1, -1, 0, 0, 0, 0, 0x33, 0xC0, 0xC3});
    require(game_state_bindings_absolute(client, client.base + 0x418FA1, 2, 4) == refresh &&
                game_state_bindings_absolute(client, client.base + 0x418FE1, 2, 4) == refresh &&
                game_state_bindings_absolute(client, client.base + 0x418FE8, 2, 4) == refresh,
            "Startup refresh getter success/failure publications disagree");
    data.native_cell<client::StartupFactory>(0x434678C4u) =
        reinterpret_cast<client::StartupFactory>(const_cast<std::byte*>(glow));
    data.native_cell<client::StartupFactory>(0x43468B9Cu) =
        reinterpret_cast<client::StartupFactory>(const_cast<std::byte*>(items));
    data.native_cell<client::StartupRefreshSource*>(0x43467C04u) =
        reinterpret_cast<client::StartupRefreshSource*>(const_cast<std::byte*>(refresh));
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_history_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct history_bindings_Section {
    const char* name;
    std::uint32_t rva, size, flags;
};
constexpr std::array<history_bindings_Section, 3> history_bindings_sections{{{".text\0\0", 0x1000, 0xB38409, 0x60000020},
                                           {".rdata\0", 0xB3A000, 0x26629C, 0x40000040},
                                           {".data\0\0", 0xDA1000, 0x460C3E0, 0xC0000040}}};
struct history_bindings_Image {
    std::byte* base;
    std::byte* code(std::uint32_t rva, std::initializer_list<int> pattern) const {
        require(rva >= 0x1000 && pattern.size() <= history_bindings_sections[0].size &&
                    rva - 0x1000 <= history_bindings_sections[0].size - pattern.size(),
                "History function leaves client executable text");
        auto* const result = base + rva;
        auto* cursor = result;
        for (const int expected : pattern) {
            require(expected < 0 || std::uint8_t(*cursor) == expected,
                    "Unsupported history engine instruction");
            ++cursor;
        }
        return result;
    }
    void call(std::uint32_t rva, std::uint32_t target) const {
        code(rva, {0xE8, -1, -1, -1, -1});
        require(std::int64_t(rva) + 5 + read<std::int32_t>(base + rva + 1) == target,
                "History engine function has a different direct callee");
    }
    void string_operand(std::uint32_t operand, std::uint32_t rva) const {
        require(rva >= history_bindings_sections[1].rva && rva - history_bindings_sections[1].rva < history_bindings_sections[1].size,
                "History string operand is outside client read-only data");
        require(read<std::uint32_t>(base + operand) == reinterpret_cast<std::uintptr_t>(base + rva),
                "History string operand has different client provenance");
    }
};
history_bindings_Image history_bindings_checked(void* module) {
    require(module, "History binding requires the initialized client.dll module");
    auto* const base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Unsupported history engine DOS header");
    const auto* nt = base + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x65272025 &&
                read<std::uint32_t>(nt + 0x50) == 0x5509000 &&
                read<std::uint32_t>(nt + 0x58) == 0xFA635F,
            "Unsupported history engine client build");
    std::array<bool, 3> found{};
    for (unsigned index = 0; index < 7; ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        const auto size = read<std::uint32_t>(section + 8), rva = read<std::uint32_t>(section + 12);
        require(rva <= 0x5509000 && size <= 0x5509000 - rva,
                "History client section exceeds image");
        for (unsigned entry = 0; entry < history_bindings_sections.size(); ++entry) {
            const auto& expected = history_bindings_sections[entry];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[entry] && size == expected.size && rva == expected.rva &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "History client section profile differs");
            found[entry] = true;
        }
    }
    require(found[0] && found[1] && found[2], "History client section is missing");
    return {base};
}
BuildHistoryBindings history_bindings_acquire(const history_bindings_Image& image) {
    auto* const name =
        image.code(0x1D9440, {0x55, 0x8B, 0xEC, 0x83, 0x7D, 8,  0xFF, 0x56, 0x8B, 0xF1, 0x75,
                              0x0A, 0xB8, -1,   -1,   -1,   -1, 0x5E, 0x5D, 0xC2, 4,    0,
                              0x83, 0xBE, 0x50, 0x29, 0,    0,  0,    0x75, 0x14});
    image.string_operand(0x1D944D, 0xB96700);
    image.call(0x1D946E, 0x1CF260);
    image.call(0x1D9485, 0x19D7D0);
    image.code(0x1D9473, {0x8B, 0x8E, 0x50, 0x29, 0, 0, 0x85, 0xC9, 0x74, 0x12, 0x83, 0x39, 0, 0x74,
                          0x0D, 0x8B, 0x55, 8});
    image.code(0x1D948A, {0x5E, 0x5D, 0xC2, 4, 0, 0xB8, -1, -1, -1, -1, 0x5E, 0x5D, 0xC2, 4, 0});
    image.string_operand(0x1D9490, 0xB966F4);
    auto* const activity =
        image.code(0x1D94A0, {0x55, 0x8B, 0xEC, 0x53, 0x8B, 0x5D, 8,    0x56, 0x8B, 0xF1,
                              0x83, 0xFB, 0xFF, 0x0F, 0x84, 0x8E, 0,    0,    0,    0x83,
                              0xBE, 0x50, 0x29, 0,    0,    0,    0x75, 0x14});
    image.call(0x1D94CB, 0x1CF260);
    image.call(0x1D94E1, 0x36B3D0);
    image.call(0x1D9516, 0x36B600);
    image.call(0x1D952A, 0x19CD60);
    image.code(0x1D952F,
               {0x8B, 0x47, 0x10, 0x5F, 0x5E, 0x5B, 0x5D, 0xC2, 4,    0,    0x5E, 0x33, 0xC0, 0x5B,
                0x5D, 0xC2, 4,    0,    0x5E, 0x83, 0xC8, 0xFF, 0x5B, 0x5D, 0xC2, 4,    0});
    auto* const displacement = image.code(
        0x19D600, {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x0C, 0x56, 0x8B, 0xF1, 0x57, 0x8B, 0xFA,
                   0x85, 0xF6, 0x75, 0x14, 0x68, -1,   -1,   -1,   -1,   0xFF, 0x15, -1,
                   -1,   -1,   -1,   0x83, 0xC4, 4,    0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC3});
    image.string_operand(0x19D611, 0xB8EA10);
    image.call(0x19D624, 0x36B3D0);
    image.call(0x19D65B, 0x7E0370);
    image.code(0x19D64D,
               {0x8D, 0x45, 0xF4, 0x8B, 0xD7, 0x50, 0xFF, 0x75, 0x0C, 0x8B, 0xCE, 0xFF, 0x75, 8});
    image.code(0x19D660, {0x83, 0xC4, 0x0C, 0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC3});
    image.code(0x19D6C4, {0x8B, 0x45, 0x0C, 0xC7, 0, 0, 0, 0, 0,    0xC7, 0x40, 4,    0,    0,   0,
                          0,    0xC7, 0x40, 8,    0, 0, 0, 0, 0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC3});
    return {reinterpret_cast<MemberFunction<std::int32_t, std::int32_t>>(activity),
            reinterpret_cast<MemberFunction<const char*, std::int32_t>>(name), displacement};
}
}
BuildHistoryBindings initialize_build_history_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4 && data.size() >= NativeDataImage::image_size,
            "History engine bindings require the complete canonical x86 data image");
    const auto result = history_bindings_acquire(history_bindings_checked(data.native_cell<void*>(0x4346A55Cu)));
    data.native_cell<decltype(result.activity)>(0x4346A364u) = result.activity;
    data.native_cell<decltype(result.sequence_name)>(0x434678C8u) = result.sequence_name;
    data.native_cell<void*>(0x43468DC0u) = result.sequence_displacement;
    return result;
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_hitbox_clip_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup read/write] */
/* [dedup require] */
void hitbox_clip_bindings_expect(const std::byte* code, std::initializer_list<int> bytes) {
    for (const auto value : bytes) {
        require(value < 0 || std::uint8_t(*code) == value,
                "Hitbox clip engine instruction fingerprint differs");
        ++code;
    }
}
int invoke_clip(void* entry, const TraceRay* ray, const void* hitbox, const Matrix3x4* matrix,
                void* trace) {
#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
    const void* arguments[]{matrix, trace, entry};
    int result;
    __asm__ __volatile__("pushl 4(%3)\n\tpushl (%3)\n\tcall *8(%3)\n\taddl $8,%%esp"
                         : "=a"(result), "+c"(ray), "+d"(hitbox)
                         : "S"(arguments)
                         : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "st",
                           "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory",
                           "cc");
    return result;
#elif defined(_MSC_VER) && defined(_M_IX86)
    int result;
    __asm {push trace push matrix mov ecx,ray mov edx,hitbox mov eax,entry call eax add esp,8 mov result,eax}
    return result;
#else
    (void)entry;
    (void)ray;
    (void)hitbox;
    (void)matrix;
    (void)trace;
    throw std::logic_error("Hitbox clip requires its native x86 register ABI");
#endif
}
int clip(void* context, const RageRay& input, const RageScanHitbox& box, RageTrace& output) {
    auto& data = *static_cast<NativeDataImage*>(context);
    auto* const entry = data.native_cell<void*>(0x43468C3Cu);
    require(entry != nullptr, "Hitbox clip engine entry is not published");
    TraceRay ray;
    for (unsigned axis = 0; axis != 3; ++axis) {
        ray.start[axis] = input.start[axis];
        ray.delta[axis] = input.delta[axis];
    }
    ray.is_ray = 1;
    ray.swept = (input.delta[0] * input.delta[0] + input.delta[1] * input.delta[1]) +
                    input.delta[2] * input.delta[2] !=
                0.0f;
    // Other engine trace fields have no consumed input in this boundary. Keep
    // their scratch bytes untouched while preserving the decoded live fields.
    std::array<std::byte, trace_result_size> trace;
    write(trace.data(), 0x0C, output.point);
    write(trace.data(), 0x18, output.normal);
    write(trace.data(), 0x2C, output.fraction);
    write(trace.data(), 0x37, std::uint8_t(output.start_solid));
    const auto result = invoke_clip(entry, &ray, box.engine_hitbox, &box.transform, trace.data());
    output.point = read<RageVector>(trace.data(), 0x0C);
    output.normal = read<RageVector>(trace.data(), 0x18);
    output.fraction = read<float>(trace.data(), 0x2C);
    output.start_solid = read<std::uint8_t>(trace.data(), 0x37) != 0;
    return result;
}
}
void* initialize_build_hitbox_clip_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Hitbox clip engine producer requires x86 pointers");
    auto* const base = data.native_cell<std::byte*>(0x4346A55Cu);
    require(base != nullptr, "Hitbox clip requires its actual loaded client module");
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base, 0x3C) == 0x150,
            "Unsupported hitbox clip client DOS header");
    const auto* const nt = base + 0x150;
    require(
        read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt, 4) == 0x14C &&
            read<std::uint16_t>(nt, 6) == 7 && read<std::uint16_t>(nt, 0x14) == 0xE0 &&
            read<std::uint16_t>(nt, 0x18) == 0x10B && read<std::uint32_t>(nt, 8) == 0x65272025 &&
            read<std::uint32_t>(nt, 0x50) == 0x5509000 && read<std::uint32_t>(nt, 0x58) == 0xFA635F,
        "Unsupported hitbox clip client build");
    const auto* const text = nt + 0xF8;
    require(!std::memcmp(text, ".text\0\0\0", 8) && read<std::uint32_t>(text, 8) == 0xB38409 &&
                read<std::uint32_t>(text, 12) == 0x1000 &&
                read<std::uint32_t>(text, 36) == 0x60000020,
            "Unsupported hitbox clip client text section");
    auto* const entry = base + 0x7DEB00;
    hitbox_clip_bindings_expect(entry, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0xF3, 0x0F, 0x10, 0x42, 0x30, 0x81, 0xEC,
                   0xB0, 0,    0,    0,    0x0F, 0x57, 0xC9, 0x0F, 0x2F, 0xC1, 0x56, 0x57, 0x8B,
                   0xF9, 0x76, 0x14, 0xFF, 0x75, 0x0C, 0xFF, 0x75, 8,    0xE8, -1,   -1,   -1,
                   -1,   0x83, 0xC4, 8,    0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC3});
    require(std::int64_t(0x7DEB28) + read<std::int32_t>(entry, 0x24) == 0x7DE900,
            "Hitbox clip capsule branch target differs");
    hitbox_clip_bindings_expect(entry + 0x31, {0x8B, 0x45, 0x0C, 0xF3, 0x0F, 0x10, 0x0D, -1,   -1,   -1,   -1,   0x8B,
                          0x75, 8,    0xF3, 0x0F, 0x10, 0x57, 0x10, 0xF3, 0x0F, 0x10, 0x40, 0x2C});
    data.native_cell<void*>(0x43468C3Cu) = entry;
    return entry;
}
RageBoxClipper native_hitbox_clipper(NativeDataImage& data) {
    require(data.native_cell<void*>(0x43468C3Cu) != nullptr,
            "Hitbox clip requires its verified published engine entry");
    return {&data, clip};
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_hook_callbacks.cpp
// ============================================================================
#if defined(_MSC_VER)
#define SKEET_BUILD_CALLER _ReturnAddress()
#define SKEET_BUILD_NOINLINE __declspec(noinline)
#else
#define SKEET_BUILD_CALLER __builtin_return_address(0)
#define SKEET_BUILD_NOINLINE [[gnu::noinline]]
#endif
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_BUILD_HOOK_ABI __fastcall
#else
#define SKEET_BUILD_HOOK_ABI
#endif

namespace skeet::engine {
namespace {
BuildHookCallbacks* active_hook_callbacks;
/* [dedup read/write] */
/* [dedup read/write] */
const VtableHookMetadata& descriptor(void* object) {
    return *static_cast<const VtableHookMetadata*>((*static_cast<void***>(object))[-2]);
}
template <class Result, class... Args>
Result original(void* object, std::size_t slot, Args... args) {
    return reinterpret_cast<MemberFunction<Result, Args...>>(
        descriptor(object).original_table[slot])(object, args...);
}
SKEET_BUILD_NOINLINE std::uintptr_t SKEET_BUILD_HOOK_ABI channel_reset_entry(void* receiver,
                                                                             void*) {
    return active_hook_callbacks->channel_reset(receiver, reinterpret_cast<std::uintptr_t>(SKEET_BUILD_CALLER));
}
void SKEET_BUILD_HOOK_ABI channel_packet_entry(void* receiver, void*, std::uintptr_t first,
                                               std::uintptr_t second) {
    active_hook_callbacks->channel_packet(receiver, first, second);
}
void SKEET_BUILD_HOOK_ABI thinkable_detach_entry(void* receiver, void*) {
    active_hook_callbacks->detach_thinkable(receiver);
}
void SKEET_BUILD_HOOK_ABI local_detach_entry(void* receiver, void*) {
    active_hook_callbacks->detach_networkable(receiver, true);
}
void SKEET_BUILD_HOOK_ABI remote_detach_entry(void* receiver, void*) {
    active_hook_callbacks->detach_networkable(receiver, false);
}
void SKEET_BUILD_HOOK_ABI local_frame_entry(void* receiver, void*) {
    active_hook_callbacks->record_local_frame(receiver);
}
void SKEET_BUILD_HOOK_ABI remote_change_entry(void* receiver, void*, std::int32_t update) {
    active_hook_callbacks->before_remote_data_change(receiver, update);
}
std::uintptr_t SKEET_BUILD_HOOK_ABI renderable_shadow_entry(void* receiver, void*) {
    return active_hook_callbacks->renderable_shadow(receiver);
}
std::uintptr_t SKEET_BUILD_HOOK_ABI event_entry(void* receiver, void*, void* event) {
    return active_hook_callbacks->game_event(receiver, event);
}
std::uintptr_t SKEET_BUILD_HOOK_ABI weapon_entry(void* receiver, void*) {
    return active_hook_callbacks->weapon_query(receiver);
}
SKEET_BUILD_NOINLINE std::uintptr_t SKEET_BUILD_HOOK_ABI overlay_entry(void* receiver, void*) {
    return active_hook_callbacks->overlay_stream(receiver, reinterpret_cast<std::uintptr_t>(SKEET_BUILD_CALLER));
}
}

BuildHookCallbacks::BuildHookCallbacks(HookDescriptorStorage& storage, VtableHookRegistry& hooks,
                                       const NativeOffsetStorage& offsets,
                                       animation::FixAnimationsRuntime& animations,
                                       animation::FixAnimationBackupRuntime& backups,
                                       misc::PingSpikeChannelRuntime& channel,
                                       ui::FrameRuntime& frame)
    : data_(storage.image()), offsets_(offsets), animations_(animations), backups_(backups),
      channel_(channel), frame_(frame) {
    if (&backups.detach.hooks != &hooks ||
        &animations.bindings.globals.local_player != &data_.native_cell<void*>(0x43475808) ||
        &channel.bindings.pending_rewind_skips != &data_.native_cell<std::uint32_t>(0x4347C9F4))
        throw std::invalid_argument(
            "Build hooks must share canonical player, channel and cleanup storage");
}

std::uintptr_t BuildHookCallbacks::channel_reset(void* channel, std::uintptr_t caller) { //43416B44
    if (caller == data_.native_cell<std::uintptr_t>(0x43467DE8))
        if (auto* reset = data_.native_cell<std::uint8_t*>(0x43467B24))
            *reset = 0;
    return original<std::uintptr_t>(channel, 22);
}
void BuildHookCallbacks::channel_packet(void* channel, std::uintptr_t first,
                                        std::uintptr_t second) { //4336C1B7
    const auto previous = read<std::uint32_t>(channel, 0x28);
    misc::average_ping_spike_sequences(channel_.bindings.history, 1);
    original<void>(channel, 39, first, second);
    channel_.bindings.pending_rewind_skips |= read<std::uint32_t>(channel, 0x28) ^ previous;
    misc::record_ping_spike_sequences(channel_.bindings.history, 1,
                                      read<std::int32_t>(channel, 0x1C),
                                      read<std::int32_t>(channel, 0x20), channel_.bindings.clock);
}
void BuildHookCallbacks::detach_thinkable(void* receiver) { //43367D1D
    auto* unknown = invoke<void*>(receiver, 0);
    backups_.detach_entity(invoke<void*>(unknown, 7));
    // detach_entity restores this receiver. Native rereads its current table.
    invoke<void>(receiver, 4);
}
void BuildHookCallbacks::detach_networkable(void* receiver, bool local) { //43403563/433FAA6D
    if (local &&
        data_.native_cell<std::int32_t>(0x4346D54C) ==
            *data_.native_cell<const std::int32_t*>(0x434678F8) &&
        data_.native_cell<std::int32_t>(0x4347C9EC) == 2)
        return;
    auto* unknown = invoke<void*>(receiver, 0);
    backups_.detach_entity(invoke<void*>(unknown, 7));
    if (local)
        data_.native_cell<void*>(0x43475808) = nullptr;
    invoke<void>(receiver, 1);
}
void BuildHookCallbacks::record_local_frame(void* receiver) { //433836C8
    if (data_.native_cell<std::int32_t>(0x4347C9EC) == 2)
        data_.native_cell<std::int32_t>(0x4346D54C) =
            *data_.native_cell<const std::int32_t*>(0x434678F8);
    original<void>(receiver, 13);
}
void BuildHookCallbacks::before_remote_data_change(void* receiver, std::int32_t update) { //4339CFA8
    const auto index = invoke<std::uint32_t>(receiver, 10);
    auto& state = animations_.bindings.players.get(index)->native;
    if (update == 1)
        write(&state, 0x1D4, std::uint32_t(0));
    else if (update == 0) {
        const auto time = invoke<float>(data_.native_cell<void*>(0x4346A514), 14);
        const auto interval = read<float>(data_.native_cell<void*>(0x43467B04), 0x20);
        const auto rounded = float(static_cast<long double>(time) / interval + 0.5L);
        const auto tick = _mm_cvttss_si32(_mm_set_ss(rounded));
        auto* player = static_cast<std::byte*>(receiver) - 8;
        write(player, animations_.bindings.fields.animation_frame_74019d87, std::uint32_t(0));
        const auto origin = read<std::array<std::uint32_t, 3>>(&state, 0x1C8);
        write(&state, 0x120, origin);
        write(&state, 0x1A4, tick);
        write(&state, 0x11C, read<std::uint32_t>(invoke<void*>(player, 77), 8));
    }
    if (update == 0 || update == 1)
        write(&state, 0x668, UINT32_MAX);
    original<void>(receiver, 3, update);
}
std::uintptr_t BuildHookCallbacks::renderable_shadow(void* receiver) { //4336F76D
    return data_.native_cell<std::uint8_t>(0x43467AEC) ? 0 : original<std::uintptr_t>(receiver, 37);
}
std::uintptr_t BuildHookCallbacks::game_event(void* receiver, void* event) { //433E9471
    auto* player = data_.native_cell<void*>(0x43475808);
    bool exchanged = false;
    if (event && player && *static_cast<void***>(player) == data_.native_cell<void**>(0x43475964)) {
        const auto* name = invoke<const char*>(event, 1);
        if (name && events::event_hash(name) == events::player_death_hash) {
            std::atomic_ref<void**>(*static_cast<void***>(player))
                .exchange(data_.native_cell<void**>(0x4347595C));
            exchanged = true;
        }
    }
    const auto result = original<std::uintptr_t>(receiver, 1, event);
    if (!exchanged)
        return result;
    // Native leaves the XCHG result in EAX on this path.
    return reinterpret_cast<std::uintptr_t>(std::atomic_ref<void**>(*static_cast<void***>(player))
                                                .exchange(data_.native_cell<void**>(0x43475964)));
}
std::uintptr_t BuildHookCallbacks::weapon_query(void* weapon) { //4334ADEE
    void* player = nullptr;
    void** previous = nullptr;
    const auto owner = read<std::uint32_t>(
        weapon, DecodedOffset(offsets_.globals, 0x434679D8, 0x43467FB4, 0x87907BC3, 0));
    if (owner != UINT32_MAX) {
        auto* entity = invoke<void*>(data_.native_cell<void*>(0x4346A890), 4, owner);
        if (entity) {
            player = invoke<void*>(entity, 161);
            if (player && invoke<bool>(player, 158)) {
                auto** table = *static_cast<void***>(player);
                if (table == data_.native_cell<void**>(0x43475840) ||
                    table == data_.native_cell<void**>(0x43475964)) {
                    previous = table;
                    std::atomic_ref<void**>(*static_cast<void***>(player))
                        .exchange(data_.native_cell<void**>(0x43475838));
                }
            }
        }
    }
    const auto result = original<std::uintptr_t>(weapon, 98);
    if (previous && player)
        std::atomic_ref<void**>(*static_cast<void***>(player)).exchange(previous);
    return result;
}
std::uintptr_t BuildHookCallbacks::overlay_stream(void* receiver,
                                                  std::uintptr_t caller) { //4341F338
    if (caller == data_.native_cell<std::uintptr_t>(0x43468D94))
        frame_.render();
    return original<std::uintptr_t>(receiver, 2);
}
void BuildHookCallbacks::bind_static(std::uint32_t identity, VtableHookMetadata& metadata,
                                     std::uint32_t slot) {
    void* entry;
    switch (identity) {
    case 0x43416B44:
        entry = reinterpret_cast<void*>(&channel_reset_entry);
        break;
    case 0x4336C1B7:
        entry = reinterpret_cast<void*>(&channel_packet_entry);
        break;
    case 0x43367D1D:
        entry = reinterpret_cast<void*>(&thinkable_detach_entry);
        break;
    case 0x43403563:
        entry = reinterpret_cast<void*>(&local_detach_entry);
        break;
    case 0x433FAA6D:
        entry = reinterpret_cast<void*>(&remote_detach_entry);
        break;
    case 0x433836C8:
        entry = reinterpret_cast<void*>(&local_frame_entry);
        break;
    case 0x4339CFA8:
        entry = reinterpret_cast<void*>(&remote_change_entry);
        break;
    case 0x4336F76D:
        entry = reinterpret_cast<void*>(&renderable_shadow_entry);
        break;
    case 0x433E9471:
        entry = reinterpret_cast<void*>(&event_entry);
        break;
    case 0x4334ADEE:
        entry = reinterpret_cast<void*>(&weapon_entry);
        break;
    case 0x43416A0F:
        bind_focus_overlay_first_entry(metadata);
        return;
    case 0x4341F338:
        entry = reinterpret_cast<void*>(&overlay_entry);
        break;
    default:
        throw std::invalid_argument("Unknown build hook callback identity");
    }
    metadata.replacement_table[slot] = entry;
}
void BuildHookCallbacks::bind(std::uint32_t identity, VtableHookMetadata& metadata,
                              std::uint32_t slot) {
    if (identity != 0x43416A0F)
        active_hook_callbacks = this;
    bind_static(identity, metadata, slot);
}
}
#undef SKEET_BUILD_HOOK_ABI
#undef SKEET_BUILD_CALLER
#undef SKEET_BUILD_NOINLINE

// ============================================================================
// Subcomponent: build_hook_initialization.cpp
// ============================================================================

namespace skeet::engine {
namespace {
// Build-specific recipe data independently identified in contexts1792..2135.
// Module cells refer to the canonical source data owner. RTTI is resolved from
// those actual loaded modules; no saved original-table pointer is used.
constexpr std::array<BuildHookDescriptorRecipe, 26> descriptors{{
    {BuildHookDescriptor::channel, 0x43475B04, 0x43468D20, 0x81967230, 0x00000000, 0x7FC, 0xB0C,
     4}, // CNetChan
    {BuildHookDescriptor::local_player, 0x43475954, 0x4346A55C, 0x1B1A0D29, 0x00000000, 0x1110,
     0x6D0, 22}, // C_CSPlayer
    {BuildHookDescriptor::local_thinkable, 0x43475878, 0x4346A55C, 0x1B1A0D29, 0x3C81EC5C, 0x37C,
     0x8B8, 2}, // C_CSPlayer / IClientThinkable
    {BuildHookDescriptor::local_renderable, 0x43475998, 0x4346A55C, 0x1B1A0D29, 0xC494EC7A, 0x894,
     0x2F8, 1}, // C_CSPlayer / IClientRenderable
    {BuildHookDescriptor::local_networkable, 0x43475B48, 0x4346A55C, 0x1B1A0D29, 0xA04AE66E, 0xF9C,
     0x348, 4}, // C_CSPlayer / IClientNetworkable
    {BuildHookDescriptor::remote_player, 0x43475830, 0x4346A55C, 0x1B1A0D29, 0x00000000, 0x1248,
     0xF00, 10}, // C_CSPlayer
    {BuildHookDescriptor::remote_thinkable, 0x43475B60, 0x4346A55C, 0x1B1A0D29, 0x3C81EC5C, 0x7CC,
     0xB2C, 2}, // C_CSPlayer / IClientThinkable
    {BuildHookDescriptor::remote_renderable, 0x4347589C, 0x4346A55C, 0x1B1A0D29, 0xC494EC7A, 0x1270,
     0x124C, 2}, // C_CSPlayer / IClientRenderable
    {BuildHookDescriptor::remote_networkable, 0x43475A28, 0x4346A55C, 0x1B1A0D29, 0xA04AE66E,
     0x1140, 0x318, 4}, // C_CSPlayer / IClientNetworkable
    {BuildHookDescriptor::game_event_listener, 0x43475864, 0x4346A55C, 0x1B1A0D29, 0xC7BCB2A1,
     0xEE4, 0xFB8, 1}, // C_CSPlayer / CGameEventListener
    {BuildHookDescriptor::ragdoll_networkable, 0x43475AEC, 0x4346A55C, 0x17843051, 0xA04AE66E,
     0x79C, 0x380, 2}, // C_CSRagdoll / IClientNetworkable
    {BuildHookDescriptor::fire_bullets_networkable, 0x43475A68, 0x4346A55C, 0xB2595DD8, 0xA04AE66E,
     0x10F0, 0x340, 1}, // C_TEFireBullets / IClientNetworkable
    {BuildHookDescriptor::molotov, 0x43475A08, 0x4346A55C, 0x0C2ECE3E, 0x00000000, 0x1128, 0x8D8,
     2}, // C_MolotovGrenade
    {BuildHookDescriptor::incendiary, 0x43475B18, 0x4346A55C, 0x4316B52C, 0x00000000, 0x444, 0x108C,
     2}, // C_IncendiaryGrenade
    {BuildHookDescriptor::viewmodel, 0x43475B34, 0x4346A55C, 0x17CEECD0, 0x00000000, 0x6C8, 0x448,
     6}, // C_PredictedViewModel
    {BuildHookDescriptor::inferno, 0x43475AC4, 0x4346A55C, 0x40D7A20F, 0x00000000, 0xFB4, 0x10B4,
     2}, // C_Inferno
    {BuildHookDescriptor::particle_effect, 0x43475908, 0x4346A55C, 0xE6C657EA, 0x043BBC72, 0x10B0,
     0x122C, 1}, // CNewParticleEffect / IParticleEffect
    {BuildHookDescriptor::particle_unknown, 0x434758F4, 0x4346A55C, 0xE6C657EA, 0x4A0A8D9A, 0x8FC,
     0xB64, 1}, // CNewParticleEffect / IClientUnknown
    {BuildHookDescriptor::particle_renderable, 0x434757F4, 0x4346A55C, 0xE6C657EA, 0xC494EC7A,
     0xEFC, 0x338, 1}, // CNewParticleEffect / IClientRenderable
    {BuildHookDescriptor::focus_overlay, 0x434757C8, 0x43468D20, 0xBA6192DD, 0x00000000, 0x10D8,
     0x10F4, 1}, // CFocusOverlayPanel
    {BuildHookDescriptor::client_state, 0x43475A3C, 0x43468D20, 0x747CA973, 0x00000000, 0x8B4,
     0x818, 13}, // CClientState
    {BuildHookDescriptor::studio, 0x434757B0, 0x4346A650, 0x9223010A, 0x00000000, 0x7B4, 0xB3C,
     5}, // CStudioRenderContext
    {BuildHookDescriptor::leaf_system, 0x434757E0, 0x4346A55C, 0x9D79E359, 0x00000000, 0x814, 0xF50,
     1}, // CClientLeafSystem
    {BuildHookDescriptor::auto_accept, 0x43475814, 0x43469234, 0xF8B3A903, 0x00000000, 0x914, 0x8C8,
     2}, // panorama::CUISoundSystemSource2
    {BuildHookDescriptor::console, 0x434758B8, 0x4346A55C, 0x46E07364, 0x00000000, 0x1088, 0x478,
     1}, // CGameConsoleDialog
    {BuildHookDescriptor::overlay_stream, 0x4347591C, 0x43468D08, 0xF0EC2C9D, 0x00000000, 0x314,
     0x898, 1}, // CSharedMemStream
}};
constexpr std::array<BuildHookNativeReplacement, 4> channel_entries{{
    {22, 0x43416B44, 0xB10},
    {39, 0x4336C1B7, 0xB18},
    {46, 0x433204DC, 0xB20},
    {40, 0x433877FF, 0xB28},
}};
constexpr std::array<BuildHookNativeReplacement, 2> local_thinkable_entries{{
    {1, 0x433FD3BF, 0x8BC},
    {4, 0x43367D1D, 0x8C4},
}};
constexpr std::array<BuildHookNativeReplacement, 1> local_renderable_entries{{
    {13, 0x433ABC77, 0x2FC},
}};
constexpr std::array<BuildHookNativeReplacement, 4> local_networkable_entries{{
    {1, 0x43403563, 0x34C},
    {6, 0x433E129A, 0x354},
    {7, 0x433B7093, 0x35C},
    {13, 0x433836C8, 0x364},
}};
constexpr std::array<BuildHookNativeReplacement, 2> remote_thinkable_entries{{
    {1, 0x43383CA2, 0xB30},
    {4, 0x43367D1D, 0xB38},
}};
constexpr std::array<BuildHookNativeReplacement, 2> remote_renderable_entries{{
    {13, 0x4334AD2A, 0x1250},
    {37, 0x4336F76D, 0x1258},
}};
constexpr std::array<BuildHookNativeReplacement, 4> remote_networkable_entries{{
    {1, 0x433FAA6D, 0x31C},
    {3, 0x4339CFA8, 0x324},
    {6, 0x433E129A, 0x32C},
    {7, 0x4337DE0B, 0x334},
}};
constexpr std::array<BuildHookNativeReplacement, 1> game_event_listener_entries{{
    {1, 0x433E9471, 0xFBC},
}};
constexpr std::array<BuildHookNativeReplacement, 2> ragdoll_networkable_entries{{
    {1, 0x4340C3A8, 0x384},
    {5, 0x433990E9, 0x38C},
}};
constexpr std::array<BuildHookNativeReplacement, 1> fire_bullets_networkable_entries{{
    {7, 0x433B06F0, 0x344},
}};
constexpr std::array<BuildHookNativeReplacement, 2> molotov_entries{{
    {0, 0x43402028, 0x8DC},
    {98, 0x4334ADEE, 0x8E4},
}};
constexpr std::array<BuildHookNativeReplacement, 2> incendiary_entries{{
    {0, 0x43402028, 0x1090},
    {98, 0x4334ADEE, 0x1098},
}};
constexpr std::array<BuildHookNativeReplacement, 2> inferno_entries{{
    {0, 0x43402028, 0x10B8},
    {110, 0x433CEAAB, 0x10C0},
}};
constexpr std::array<BuildHookNativeReplacement, 1> particle_effect_entries{{
    {0, 0x433E7C16, 0x1230},
}};
constexpr std::array<BuildHookNativeReplacement, 1> particle_unknown_entries{{
    {0, 0x4340B2C6, 0xB68},
}};
constexpr std::array<BuildHookNativeReplacement, 1> particle_renderable_entries{{
    {9, 0x433C8A72, 0x33C},
}};
constexpr std::array<BuildHookNativeReplacement, 1> focus_overlay_entries{{
    {131, 0x43416A0F, 0x10F8},
}};
constexpr std::array<BuildHookNativeReplacement, 1> overlay_stream_entries{{
    {2, 0x4341F338, 0x89C},
}};
std::span<const BuildHookNativeReplacement> secondary_entries(BuildHookDescriptor kind) {
    switch (kind) {
    case BuildHookDescriptor::channel:
        return channel_entries;
    case BuildHookDescriptor::local_thinkable:
        return local_thinkable_entries;
    case BuildHookDescriptor::local_renderable:
        return local_renderable_entries;
    case BuildHookDescriptor::local_networkable:
        return local_networkable_entries;
    case BuildHookDescriptor::remote_thinkable:
        return remote_thinkable_entries;
    case BuildHookDescriptor::remote_renderable:
        return remote_renderable_entries;
    case BuildHookDescriptor::remote_networkable:
        return remote_networkable_entries;
    case BuildHookDescriptor::game_event_listener:
        return game_event_listener_entries;
    case BuildHookDescriptor::ragdoll_networkable:
        return ragdoll_networkable_entries;
    case BuildHookDescriptor::fire_bullets_networkable:
        return fire_bullets_networkable_entries;
    case BuildHookDescriptor::molotov:
        return molotov_entries;
    case BuildHookDescriptor::incendiary:
        return incendiary_entries;
    case BuildHookDescriptor::inferno:
        return inferno_entries;
    case BuildHookDescriptor::particle_effect:
        return particle_effect_entries;
    case BuildHookDescriptor::particle_unknown:
        return particle_unknown_entries;
    case BuildHookDescriptor::particle_renderable:
        return particle_renderable_entries;
    case BuildHookDescriptor::focus_overlay:
        return focus_overlay_entries;
    case BuildHookDescriptor::overlay_stream:
        return overlay_stream_entries;
    default:
        throw std::invalid_argument("Descriptor has an existing whole-family manifest");
    }
}
void bind_secondary(const BuildHookDescriptorRecipe& recipe, VtableHookMetadata& metadata,
                    const BuildHookFamilies* f) {
    for (const auto& entry : secondary_entries(recipe.kind)) {
        switch (entry.native_entry) {
        case 0x433FD3BF:
        case 0x43383CA2:
        case 0x433E129A:
        case 0x433B7093:
        case 0x4337DE0B:
            if (f)
                f->animations.bind_native_replacement(metadata, entry.slot, entry.native_entry);
            else
                metadata.replacement_table[entry.slot] =
                    animation::FixAnimationsRuntime::native_replacement_entry(entry.slot,
                                                                              entry.native_entry);
            break;
        case 0x433ABC77: // MOV EDX,EBP; JMP433E80F7. The body ignores incoming EDX.
            if (f)
                f->animations.bind_native_replacement(metadata, entry.slot, 0x433E80F7);
            else
                metadata.replacement_table[entry.slot] =
                    animation::FixAnimationsRuntime::native_replacement_entry(entry.slot,
                                                                              0x433E80F7);
            break;
        case 0x4334AD2A:
            if (f)
                f->remote_bones.bind(metadata);
            else
                metadata.replacement_table[entry.slot] =
                    animation::RemoteSetupBonesRuntime::source_entry();
            break;
        case 0x433204DC:
            if (f)
                misc::bind_ping_spike_channel_runtime(f->channel);
            metadata.replacement_table[entry.slot] =
                reinterpret_cast<void*>(&misc::ping_spike_send_datagram_hook);
            break;
        case 0x433877FF:
            if (f)
                misc::bind_network_message_runtime(f->messages);
            metadata.replacement_table[entry.slot] =
                reinterpret_cast<void*>(&misc::send_network_message_hook);
            break;
        case 0x43402028:
            metadata.replacement_table[entry.slot] =
                reinterpret_cast<void*>(&misc::viewmodel_destroy_hook);
            break;
        default:
            if (f)
                f->callbacks.bind(entry.native_entry, metadata, entry.slot);
            else
                BuildHookCallbacks::bind_static(entry.native_entry, metadata, entry.slot);
            break;
        }
    }
}
template <class Entries>
void copy_static_entries(VtableHookMetadata& metadata, const Entries& entries) {
    for (const auto& entry : entries) {
        if constexpr (requires { entry.native.slot; })
            metadata.replacement_table[entry.native.slot] = entry.source_entry;
        else
            metadata.replacement_table[entry.slot] = entry.source_entry;
    }
}
void bind_descriptor_entries(const BuildHookDescriptorRecipe& recipe, VtableHookMetadata& metadata,
                             const BuildHookFamilies* f) {
    switch (recipe.kind) {
    case BuildHookDescriptor::local_player:
        if (f)
            f->players.bind_replacements(PlayerPrimaryKind::local);
        else
            bind_player_primary_static_entries(metadata, PlayerPrimaryKind::local);
        break;
    case BuildHookDescriptor::remote_player:
        if (f)
            f->players.bind_replacements(PlayerPrimaryKind::remote);
        else
            bind_player_primary_static_entries(metadata, PlayerPrimaryKind::remote);
        break;
    case BuildHookDescriptor::viewmodel:
        if (f)
            f->viewmodels.bind_replacements();
        else
            copy_static_entries(metadata, misc::viewmodel_hook_replacements());
        break;
    case BuildHookDescriptor::client_state:
        if (f)
            f->client_state.bind_replacements();
        else
            copy_static_entries(metadata, network::client_state_source_replacements());
        break;
    case BuildHookDescriptor::studio:
        if (f)
            f->models.bind_studio_replacements();
        else
            copy_static_entries(metadata, effects::studio_model_hook_replacements());
        break;
    case BuildHookDescriptor::leaf_system:
        if (f)
            f->models.bind_occlusion_replacements();
        else
            copy_static_entries(metadata, effects::model_occlusion_hook_replacements());
        break;
    case BuildHookDescriptor::auto_accept:
        if (f)
            f->auto_accept.bind_replacements();
        else
            copy_static_entries(metadata, misc::auto_accept_hook_replacements());
        break;
    case BuildHookDescriptor::console:
        metadata.replacement_table[279] = reinterpret_cast<void*>(&script::console_input_hook);
        break;
    case BuildHookDescriptor::ragdoll_networkable:
        if (f)
            f->ragdolls.bind_networkable_table(metadata);
        else
            effects::EntityNetworkableRuntime::bind_static_networkable_table(metadata);
        break;
    case BuildHookDescriptor::fire_bullets_networkable:
        if (f)
            network::bind_packet_dispatch_runtime(f->packets);
        metadata.replacement_table[7] = reinterpret_cast<void*>(&network::packet_update_hook);
        break;
    case BuildHookDescriptor::inferno:
        metadata.replacement_table[0] = reinterpret_cast<void*>(&misc::viewmodel_destroy_hook);
        if (f)
            f->particles.bind_table(effects::HarmlessMolotovHook::creation, metadata);
        else
            effects::HarmlessMolotovRuntime::bind_static_table(
                effects::HarmlessMolotovHook::creation, metadata);
        break;
    case BuildHookDescriptor::particle_effect:
        if (f)
            f->particles.bind_table(effects::HarmlessMolotovHook::primary, metadata);
        else
            effects::HarmlessMolotovRuntime::bind_static_table(
                effects::HarmlessMolotovHook::primary, metadata);
        break;
    case BuildHookDescriptor::particle_unknown:
        if (f)
            f->particles.bind_table(effects::HarmlessMolotovHook::secondary, metadata);
        else
            effects::HarmlessMolotovRuntime::bind_static_table(
                effects::HarmlessMolotovHook::secondary, metadata);
        break;
    case BuildHookDescriptor::particle_renderable:
        if (f)
            f->particles.bind_table(effects::HarmlessMolotovHook::renderable, metadata);
        else
            effects::HarmlessMolotovRuntime::bind_static_table(
                effects::HarmlessMolotovHook::renderable, metadata);
        break;
    default:
        bind_secondary(recipe, metadata, f);
        break;
    }
}
}
std::span<const BuildHookDescriptorRecipe, 26> build_hook_descriptors() noexcept {
    return descriptors;
}
std::vector<BuildHookNativeReplacement> build_hook_native_replacements(BuildHookDescriptor kind) {
    std::vector<BuildHookNativeReplacement> result;
    const auto append = [&](const auto& entries, std::uint32_t operand) {
        for (const auto& e : entries) {
            result.push_back({e.slot, e.native_entry, operand + 4});
            operand += 8;
        }
    };
    switch (kind) {
    case BuildHookDescriptor::local_player:
        append(local_player_native_replacements, local_player_replacement_stream);
        break;
    case BuildHookDescriptor::remote_player:
        append(remote_player_native_replacements, remote_player_replacement_stream);
        break;
    case BuildHookDescriptor::viewmodel:
        append(misc::viewmodel_hook_replacements(), misc::viewmodel_hook_replacements_operand);
        break;
    case BuildHookDescriptor::client_state:
        append(network::client_state_native_replacements, network::client_state_replacement_stream);
        break;
    case BuildHookDescriptor::studio:
        append(effects::studio_model_hook_replacements(), 0xB3C);
        break;
    case BuildHookDescriptor::leaf_system:
        append(effects::model_occlusion_hook_replacements(), 0xF50);
        break;
    case BuildHookDescriptor::auto_accept:
        append(misc::auto_accept_hook_replacements(), misc::auto_accept_hook_replacements_operand);
        break;
    case BuildHookDescriptor::console:
        result.push_back({279, 0x4334E722, 0x47C});
        break;
    default: {
        const auto entries = secondary_entries(kind);
        result.assign(entries.begin(), entries.end());
        break;
    }
    }
    return result;
}

namespace {
void validate_families(HookDescriptorStorage& storage, VtableHookRegistry& hooks,
                       const BuildHookFamilies& f) {
    if (&f.players.metadata(PlayerPrimaryKind::local) !=
            &storage.at(local_player_descriptor_identity) ||
        &f.players.metadata(PlayerPrimaryKind::remote) !=
            &storage.at(remote_player_descriptor_identity) ||
        &f.client_state.metadata() != &storage.at(network::client_state_descriptor_identity) ||
        &f.auto_accept.metadata() != &storage.at(misc::auto_accept_hook_descriptor) ||
        &f.console.metadata != &storage.at(0x434758B8) ||
        &f.viewmodels.image() != &storage.image() ||
        static_cast<void*>(&f.models.studio_descriptor()) !=
            &storage.at(effects::studio_model_hook_descriptor) ||
        static_cast<void*>(&f.models.occlusion_descriptor()) !=
            &storage.at(effects::model_occlusion_hook_descriptor) ||
        &f.particles.bindings.hook_registry != &hooks ||
        f.particles.bindings.entity_hooks[0] != &storage.at(0x43475908) ||
        f.particles.bindings.entity_hooks[1] != &storage.at(0x434758F4) ||
        f.particles.bindings.entity_hooks[2] != &storage.at(0x434757F4))
        throw std::invalid_argument(
            "Build hook families must share canonical descriptors and cleanup storage");
}
}
void prepare_build_hook_descriptors(HookDescriptorStorage& storage, JobMemory memory,
                                    NativeImageEnvironment images, HookDescriptorSystem system) {
    for (const auto& recipe : descriptors) {
        auto& metadata = storage.at(recipe.descriptor);
        prepare_rtti_vtable_hook(metadata,
                                 storage.image().native_cell<std::byte*>(recipe.module_cell),
                                 recipe.type_hash, recipe.base_hash, images, memory, system);
        // The complete original26-group phase terminates12 when the Steam
        // module is absent: it finishes25groups/93entries, reads the overlay
        // descriptor words, then fails before its replacement operands.
        // Helper-local null handling does not make that final group optional.
        if (!metadata.replacement_table)
            throw std::runtime_error("Required build hook RTTI table or allocation is unavailable");
        bind_descriptor_entries(recipe, metadata, nullptr);
    }
}
void bind_build_hook_runtime_contexts(HookDescriptorStorage& storage, VtableHookRegistry& hooks,
                                      const BuildHookFamilies& families) {
    validate_families(storage, hooks, families);
    for (const auto& recipe : descriptors) {
        auto& metadata = storage.at(recipe.descriptor);
        if (!metadata.replacement_table)
            throw std::logic_error("Early build hook descriptor phase has not completed");
        bind_descriptor_entries(recipe, metadata, &families);
    }
}
void initialize_build_hooks(HookDescriptorStorage& storage, VtableHookRegistry& hooks,
                            NativeImageEnvironment images, HookDescriptorSystem system,
                            const BuildHookFamilies& families) {
    validate_families(storage, hooks, families);
    prepare_build_hook_descriptors(storage, hooks.memory, images, system);
    bind_build_hook_runtime_contexts(storage, hooks, families);
}
}

// ============================================================================
// Subcomponent: build_legit_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct legit_bindings_Section {
    const char* name;
    std::uint32_t rva, size, flags;
};
constexpr std::array<legit_bindings_Section, 3> legit_bindings_sections{{{".text\0\0", 0x1000, 0xB38409, 0x60000020},
                                           {".rdata\0", 0xB3A000, 0x26629C, 0x40000040},
                                           {".data\0\0", 0xDA1000, 0x460C3E0, 0xC0000040}}};
struct legit_bindings_Image {
    std::byte* base;
    bool contains(std::uint32_t rva, std::size_t length, unsigned kind) const noexcept {
        const auto& s = legit_bindings_sections[kind];
        return rva >= s.rva && length <= s.size && rva - s.rva <= s.size - length;
    }
    void code(std::uint32_t rva, std::initializer_list<int> expected) const {
        require(contains(rva, expected.size(), 0), "LEGIT smoke code leaves client text");
        for (const auto value : expected) {
            require(value < 0 || std::uint8_t(base[rva]) == value,
                    "LEGIT smoke instruction fingerprint differs");
            ++rva;
        }
    }
    void operand(std::uint32_t rva, std::uint32_t target, unsigned kind, std::size_t length) const {
        require(contains(rva, 4, 0) && contains(target, length, kind) &&
                    read<std::uint32_t>(base + rva) ==
                        reinterpret_cast<std::uintptr_t>(base + target),
                "LEGIT smoke operand has different module/section provenance");
    }
};
legit_bindings_Image legit_bindings_checked(NativeDataImage& data) {
    auto* base = data.native_cell<std::byte*>(0x4346A55Cu);
    require(base != nullptr, "LEGIT smoke requires the actual acquired client module");
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Unsupported LEGIT smoke DOS/NT layout");
    const auto* nt = base + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x65272025 &&
                read<std::uint32_t>(nt + 0x50) == 0x5509000 &&
                read<std::uint32_t>(nt + 0x58) == 0xFA635F,
            "Unsupported LEGIT smoke client PE32 build");
    std::array<bool, 3> found{};
    for (unsigned row = 0; row < 7; ++row) {
        const auto* s = nt + 0xF8 + 40 * row;
        const auto rva = read<std::uint32_t>(s + 12), size = read<std::uint32_t>(s + 8);
        require(rva <= 0x5509000 && size <= 0x5509000 - rva,
                "LEGIT smoke client section exceeds image");
        for (unsigned i = 0; i < legit_bindings_sections.size(); ++i)
            if (std::memcmp(s, legit_bindings_sections[i].name, 8) == 0) {
                require(!found[i] && rva == legit_bindings_sections[i].rva && size == legit_bindings_sections[i].size &&
                            read<std::uint32_t>(s + 36) == legit_bindings_sections[i].flags,
                        "LEGIT smoke section fingerprint differs");
                found[i] = true;
            }
    }
    require(found[0] && found[1] && found[2], "LEGIT smoke requires its actual client legit_bindings_sections");
    return {base};
}
std::int32_t entity_index(void*, BallisticEntity entity) {
    return invoke<std::int32_t>(reinterpret_cast<std::byte*>(std::uintptr_t(entity)) + 8, 10);
}
bool through_smoke(void* context, const legit::Vector& first, const legit::Vector& last) {
    return line_passes_through_smoke(*static_cast<NativeDataImage*>(context), first, last);
}
}
BuildLegitBindings initialize_build_legit_bindings(NativeDataImage& data) {
    const auto image = legit_bindings_checked(data);
    image.code(0x3D1820, {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08, 0x8B, 0x15, -1,   -1,   -1,
                          -1,   0x0F, 0x57, 0xC0, 0x56, 0x57, 0x33, 0xFF, 0xF3, 0x0F, 0x11,
                          0x45, 0xFC, 0x85, 0xD2, 0x0F, 0x8E, 0xAF, 0,    0,    0});
    image.operand(0x3D1828, 0x525CAAC, 2, 4);
    image.code(0x3D1840,
               {0x83, 0x3D, -1,   -1,   -1,   -1,   0,    0x0F, 0x84, 0x99, 0,    0,    0,    0xA1,
                -1,   -1,   -1,   -1,   0x8B, 0x0C, 0xB8, 0x83, 0xF9, 0xFF, 0x0F, 0x84, 0x88, 0,
                0,    0,    0x0F, 0xB7, 0xC1, 0xC1, 0xE0, 0x04, 0x05, -1,   -1,   -1,   -1});
    image.operand(0x3D1842, 0x5334764, 2, 4);
    image.operand(0x3D184E, 0x525CAA0, 2, 16);
    image.operand(0x3D1865, 0x4E051CC, 2, 16);
    image.code(0x3D1887, {0xF3, 0x0F, 0x7E, 0x45, 0x14, 0x83, 0xEC, 0x0C, 0x8B, 0x4D, 0x1C,
                          0x8B, 0xD4, 0x83, 0xEC, 0x0C, 0x8B, 0xF0, 0x8B, 0x45, 0x10});
    image.code(0x3D18C4, {0xE8, -1, -1, -1, -1, 0x0F, 0x2E, 0x05, -1, -1, -1, -1, 0x9F, 0xF6, 0xC4,
                          0x44, 0x7B, 0x2B});
    require(std::int64_t(0x3D18C9) + read<std::int32_t>(image.base + 0x3D18C5) == 0x418AB0,
            "LEGIT smoke geometric helper call differs");
    image.code(0x418AB0, {0x55, 0x8B, 0xEC});
    image.operand(0x3D18CC, 0xD2ED04, 1, 4);
    image.code(0x3D18EF,
               {0x33, 0xC0, 0x0F, 0x2F, 0x05, -1,   -1,   -1,   -1,   0x0F, 0x97, 0xC0, 0x5F,
                0x5E, 0x8B, 0xE5, 0x5D, 0xC3, 0x5F, 0xB0, 0x01, 0x5E, 0x8B, 0xE5, 0x5D, 0xC3});
    image.operand(0x3D18F4, 0xD2E9D8, 1, 4);
    data.native_cell<LineSmokeEntry>(0x4346A82Cu) =
        reinterpret_cast<LineSmokeEntry>(image.base + 0x3D1820);
    return {data};
}
legit::TriggerTraceBindings BuildLegitBindings::trigger_trace() const noexcept {
    return {&data, entity_index, through_smoke};
}
bool line_passes_through_smoke(const NativeDataImage& data, const legit::Vector& first,
                               const legit::Vector& last) {
    const auto entry = data.native_cell<LineSmokeEntry>(0x4346A82Cu);
    if (!entry)
        return false; //433E7C94..9B, exact native optional-entry gate
    return entry(first[0], first[1], first[2], last[0], last[1], last[2], 1);
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_loader_inspection_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct loader_inspection_bindings_Section {
    char name[8];
    std::uint32_t rva, size, flags;
};
struct loader_inspection_bindings_Profile {
    std::uint32_t nt, timestamp, size, checksum;
    std::uint16_t section_count;
    std::array<loader_inspection_bindings_Section, 3> sections;
};
constexpr loader_inspection_bindings_Profile loader_inspection_engine_build{0x150,
                               0x64EE7E86,
                               0x39C0000,
                               0x64174E,
                               7,
                               {{{".text", 0x1000, 0x451C32, 0x60000020},
                                 {".rdata", 0x453000, 0xD4766, 0x40000040},
                                 {".data", 0x528000, 0x3427638, 0xC0000040}}}};
constexpr loader_inspection_bindings_Profile loader_inspection_material_build{0x118,
                                 0x63DC3DE4,
                                 0x114000,
                                 0xDE784,
                                 6,
                                 {{{".text", 0x1000, 0x91D46, 0x60000020},
                                   {".rdata", 0x93000, 0x28074, 0x40000040},
                                   {".data", 0xBC000, 0x42480, 0xC0000040}}}};
struct loader_inspection_bindings_Image {
    std::byte* base;
    const loader_inspection_bindings_Profile& profile;
    bool loader_inspection_bindings_within(const void* pointer, std::size_t size, std::size_t section) const noexcept {
        const auto& region = profile.sections[section];
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto first = reinterpret_cast<std::uintptr_t>(base) + region.rva;
        return address >= first && size <= region.size && address - first <= region.size - size;
    }
};
loader_inspection_bindings_Image loader_inspection_bindings_checked(void* module, const loader_inspection_bindings_Profile& profile) {
    require(module != nullptr, "Required loader-inspection module is not published");
    auto* base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == profile.nt,
            "Loader-inspection DOS/NT layout differs");
    const auto* nt = base + profile.nt;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == profile.section_count &&
                read<std::uint16_t>(nt + 0x14) == 0xE0 && read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == profile.timestamp &&
                read<std::uint32_t>(nt + 0x50) == profile.size &&
                read<std::uint32_t>(nt + 0x58) == profile.checksum,
            "Unsupported loader-inspection PE32 build");
    std::array<bool, 3> found{};
    for (std::size_t i = 0; i < profile.section_count; ++i) {
        const auto* section = nt + 0xF8 + i * 40;
        const auto rva = read<std::uint32_t>(section + 12), size = read<std::uint32_t>(section + 8);
        require(rva <= profile.size && size <= profile.size - rva,
                "Loader-inspection section exceeds image");
        for (std::size_t j = 0; j < found.size(); ++j) {
            const auto& expected = profile.sections[j];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[j] && rva == expected.rva && size == expected.size &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Loader-inspection section fingerprint differs");
            found[j] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Required loader-inspection section is missing");
    return {base, profile};
}
/* [dedup matches] */
void loader_inspection_bindings_expect(const loader_inspection_bindings_Image& image, const std::byte* code, std::initializer_list<int> pattern) {
    require(image.loader_inspection_bindings_within(code, pattern.size(), 0) && matches(code, pattern),
            "Loader-inspection instruction fingerprint differs");
}
const std::byte* loader_inspection_bindings_unique(const loader_inspection_bindings_Image& image, std::initializer_list<int> pattern, std::uint32_t rva) {
    const auto& text = image.profile.sections[0];
    const std::byte* found = nullptr;
    for (std::size_t i = 0; i <= text.size - pattern.size(); ++i) {
        const auto* code = image.base + text.rva + i;
        if (!matches(code, pattern))
            continue;
        require(!found, "Loader-inspection code anchor is ambiguous");
        found = code;
    }
    require(found == image.base + rva, "Loader-inspection code anchor is missing or unsupported");
    return found;
}
const std::byte* loader_inspection_bindings_absolute(const loader_inspection_bindings_Image& image, const std::byte* operand, std::size_t size,
                          std::size_t section) {
    const auto* pointer =
        reinterpret_cast<const std::byte*>(std::uintptr_t(read<std::uint32_t>(operand)));
    require(image.loader_inspection_bindings_within(pointer, size, section),
            "Loader-inspection operand has invalid section provenance");
    return pointer;
}
const std::byte* relative(const loader_inspection_bindings_Image& image, const std::byte* instruction, std::uint8_t opcode,
                          std::uint32_t rva) {
    require(image.loader_inspection_bindings_within(instruction, 5, 0) && static_cast<std::uint8_t>(*instruction) == opcode,
            "Loader-inspection relative transfer differs");
    const auto offset =
        std::int64_t(instruction - image.base) + 5 + read<std::int32_t>(instruction + 1);
    require(offset == rva && offset >= 0 && offset < image.profile.size,
            "Loader-inspection transfer target differs");
    const auto* target = image.base + static_cast<std::size_t>(offset);
    require(image.loader_inspection_bindings_within(target, 1, 0), "Loader-inspection transfer leaves executable text");
    return target;
}
}

void initialize_build_loader_inspection_inputs(NativeDataImage& data) {
    require(sizeof(void*) == 4 && data.size() >= NativeDataImage::image_size,
            "Loader inspection requires the complete canonical x86 data image");
    const auto engine = loader_inspection_bindings_checked(data.native_cell<void*>(0x43468D20u), loader_inspection_engine_build);
    const auto material = loader_inspection_bindings_checked(data.native_cell<void*>(0x4346A600u), loader_inspection_material_build);
    auto* const receiver = data.native_cell<void*>(0x4346A514u);
    require(engine.loader_inspection_bindings_within(receiver, sizeof(void*), 2),
            "Loader inspection requires the actual VEngineClient014 receiver");
    const auto* table = read<const std::byte*>(receiver);
    const auto* state_method = read<const std::byte*>(table + 222 * sizeof(void*));
    require(state_method == engine.base + 0xB7550, "Engine inspection virtual222 differs");
    loader_inspection_bindings_expect(engine, state_method,
           {0x8B, 0x0D, -1, -1, -1, -1, 0x8D, 0x89, 0x60, 0x4E, 0, 0, 0xE9, -1, -1, -1, -1});
    loader_inspection_bindings_absolute(engine, state_method + 2, 4, 2);
    const auto* state_target = relative(engine, state_method + 0xC, 0xE9, 0xB93A0);
    loader_inspection_bindings_expect(engine, state_target,
           {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x1C, 0x8B, 0xD1, 0x83, 0x7A, 0x20, 0,
            0x8D, 0x4A, 0x20});

    const auto* clock_block =
        loader_inspection_bindings_unique(engine, {0x2B, 0x05, -1,   -1,   -1,   -1,   0x03, 0x05, -1,   -1,   -1,
                        -1,   0x66, 0x0F, 0x6E, 0xD8, 0x33, 0xC0, 0x0F, 0x5B, 0xDB, 0x85,
                        0xDB, 0x0F, 0x44, 0xC8, 0x51, 0x50, 0xF3, 0x0F, 0x59, 0x1D, -1,
                        -1,   -1,   -1,   0x8B, 0xCE, 0xE8, -1,   -1,   -1,   -1},
               0x1A3656);
    const auto* first_counter = loader_inspection_bindings_absolute(engine, clock_block + 2, 4, 2);
    require(loader_inspection_bindings_absolute(engine, clock_block + 8, 4, 2) == first_counter + 0x14,
            "Engine inspection counter operands disagree");
    loader_inspection_bindings_absolute(engine, clock_block + 0x20, 4, 2);
    const auto* clock_target = relative(engine, clock_block + 0x26, 0xE8, 0x1A6090);
    loader_inspection_bindings_expect(engine, clock_target,
           {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x60, 0x56, 0x8B, 0xF1, 0xF3, 0x0F, 0x11, 0x5D, 0xFC});

    const auto* material_loop =
        loader_inspection_bindings_unique(material, {0x8B, 0x7D, 0xE8, 0x33, 0xF6, 0x85, 0xFF, 0x7E, 0x24, 0x0F, 0x1F, 0,
                          0x83, 0x3C, 0xB3, 0,    0x75, 0x16, 0x8B, 0x4D, 0x08, 0x56, 0x8B, 0x01,
                          0xFF, 0x50, 0x0C, 0x8B, 0x4D, 0xF0, 0x8B, 0x10, 0xE8, -1,   -1,   -1,
                          -1,   0x89, 0x04, 0xB3, 0x46, 0x3B, 0xF7, 0x7C, 0xDF},
               0xBBD4);
    const auto* material_call = material_loop + 0x20;
    const auto* material_target = relative(material, material_call, 0xE8, 0x24B10);
    loader_inspection_bindings_expect(material, material_target,
           {0x55, 0x8B, 0xEC, 0x6A, 0xFF, 0x68, -1,   -1,   -1,   -1,  0x64,
            0xA1, 0,    0,    0,    0,    0x50, 0x64, 0x89, 0x25, 0,   0,
            0,    0,    0x51, 0x53, 0x56, 0x57, 0x8B, 0xFA, 0x8B, 0xD9});
    loader_inspection_bindings_absolute(material, material_target + 6, 1, 0);

    const auto* tree_walk =
        loader_inspection_bindings_unique(engine, {0x56, 0x8B, 0x35, -1,   -1,   -1,   -1,   0x57, 0x8B, 0x3D, -1,   -1,
                        -1,   -1,   0x66, 0x90, 0x83, 0xFE, 0xFF, 0x74, 0x3D, 0x6B, 0xC6, 0x6C,
                        0x8B, 0x04, 0x38, 0x83, 0xF8, 0xFF, 0x74, 0x04, 0x8B, 0xF0, 0xEB, 0xEC},
               0xDA360);
    const auto* tree_header = loader_inspection_bindings_absolute(engine, tree_walk + 0xA, 0x1C, 2);
    require(tree_header == engine.base + 0x59E9CC &&
                loader_inspection_bindings_absolute(engine, tree_walk + 3, 4, 2) == tree_header + 0xC,
            "Engine inspection tree operands disagree");

    data.native_cell<const std::byte*>(0x4346D4D0u) = state_method;
    data.native_cell<const std::byte*>(0x4346D4C8u) = material_call;
    data.native_cell<const std::byte*>(0x4346D4C4u) = clock_block;
    data.native_cell<const std::byte*>(0x4346D4C0u) = tree_header;
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_local_animation_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct Module {
    std::byte* base;
    std::uint32_t text_end;
    std::byte* code(std::uint32_t rva, std::initializer_list<int> pattern) const {
        require(rva >= 0x1000 && rva < text_end && pattern.size() <= text_end - rva,
                "Local animation code leaves its verified module text");
        auto* const address = base + rva;
        auto* cursor = address;
        for (auto value : pattern) {
            require(value < 0 || std::uint8_t(*cursor) == value,
                    "Local animation engine instructions differ");
            ++cursor;
        }
        return address;
    }
    void call(std::uint32_t rva, std::uint32_t target) const {
        code(rva, {0xE8, -1, -1, -1, -1});
        require(std::int64_t(rva) + 5 + read<std::int32_t>(base + rva + 1) == target,
                "Local animation engine direct call target differs");
    }
};
Module local_animation_bindings_checked(void* object, std::uint32_t timestamp, std::uint32_t size, std::uint32_t checksum,
               std::uint32_t text_size) {
    require(object != nullptr, "Local animation requires its actual loaded module");
    auto* const base = static_cast<std::byte*>(object);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Unsupported local animation module DOS header");
    const auto* const nt = base + 0x150;
    require(
        read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
            read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
            read<std::uint16_t>(nt + 0x18) == 0x10B && read<std::uint32_t>(nt + 8) == timestamp &&
            read<std::uint32_t>(nt + 0x50) == size && read<std::uint32_t>(nt + 0x58) == checksum,
        "Unsupported local animation engine build");
    const auto* const text = nt + 0xF8;
    require(std::memcmp(text, ".text\0\0\0", 8) == 0 &&
                read<std::uint32_t>(text + 8) == text_size &&
                read<std::uint32_t>(text + 12) == 0x1000 &&
                read<std::uint32_t>(text + 36) == 0x60000020,
            "Unsupported local animation text section");
    return {base, 0x1000 + text_size};
}
float layer_weight(void* entry, void* state, std::int32_t layer) {
#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
    float result;
    __asm__ volatile("pushl %[index]\n\txorl %%edx,%%edx\n\tcall *%%eax\n\tmovss %%xmm0,%[result]"
                     : [result] "=m"(result), "+a"(entry), "+c"(state)
                     : [index] "m"(layer)
                     : "edx", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                       "memory", "cc");
    return result;
#elif defined(_MSC_VER) && defined(_M_IX86)
    float result;
    __asm {push layer mov ecx,state xor edx,edx mov eax,entry call eax movss result,xmm0}
    return result;
#else
    (void)entry;
    (void)state;
    (void)layer;
    throw std::logic_error("Local layer weight requires the native x86 SIMD ABI");
#endif
}
}
BuildLocalAnimationBindings initialize_build_local_animation_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Local animation binding requires x86 pointers");
    const auto client =
        local_animation_bindings_checked(data.native_cell<void*>(0x4346A55Cu), 0x65272025, 0x5509000, 0xFA635F, 0xB38409);
    const auto engine =
        local_animation_bindings_checked(data.native_cell<void*>(0x43468D20u), 0x64EE7E86, 0x39C0000, 0x64174E, 0x451C32);
    auto* const mapping = client.code(
        0x36CAC0, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x81, 0xEC, 0xAC, 0, 0, 0, 0x53, 0x56,
                   0x57, 0x8B, 0xF9, 0x8B, 0x17, 0x83, 0xBA, 0x50, 1,    0, 0, 0, 0x74, 0x34});
    client.code(0x36CD15, {0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC3});
    client.code(0x36CD20, {0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC3});
    auto* const select =
        client.code(0x19D140, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x34, 0x53, 0x56,
                               0x8B, 0x75, 8,    0x8B, 0xD9, 0x57, 0x89, 0x5C, 0x24, 0x20});
    for (const auto rva : {0x19D199u, 0x19D215u, 0x19D227u, 0x19D242u, 0x19D468u, 0x19D47Fu})
        client.code(rva, {0xC2, 0x10, 0});
    auto* const find = client.code(0x9A2C40, {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x0C, 0x56, 0x8B, 0x75,
                                              0x0C, 0x57, 0x8B, 0xF9, 0x85, 0xF6, 0x75, 0x13, 0x8B,
                                              0x45, 8,    0xB9, 0xFF, 0xFF, 0,    0});
    auto* const add =
        client.code(0x9A2CF0, {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x0C, 0x53, 0x8B, 0x5D, 0x0C,
                               0x57, 0x8B, 0xF9, 0x89, 0x7D, 0xF4, 0x85, 0xDB, 0x75, 0x13});
    for (const auto rva : {0x9A2C61u, 0x9A2C9Eu, 0x9A2D14u, 0x9A2E03u})
        client.code(rva, {0xC2, 8, 0});
    client.code(0x36C98B, {0xB9, -1, -1, -1, -1, 0x50});
    client.call(0x36C991, 0x9A2CF0);
    auto* const symbols =
        reinterpret_cast<void*>(std::uintptr_t(read<std::uint32_t>(client.base + 0x36C98C)));
    require(symbols == client.base + 0xDF7FC0, "Local animation symbol-table operand differs");
    auto* const activity = data.native_cell<std::byte*>(0x4346A460u);
    require(activity == client.base + 0xB8F4FC,
            "Local animation requires the actual resolved BasePlayerAnimState vtable");
    require(read<void*>(activity + 0x5C) == client.base + 0x1A3620 &&
                read<void*>(activity + 0x64) == client.base + 0x1A17D0,
            "Local activity interface methods differ");
    // Both supported methods read only receiver+1C. Words1..6 of the native
    // temporary receiver are uninitialized and have no semantic consumer.
    client.code(0x1A3620, {0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x71, 0x1C});
    client.code(0x1A3663, {0xC2, 4, 0});
    client.code(0x1A18A6, {0xC3});
    auto* const weight = client.code(
        0x442740, {0x55, 0x8B, 0xEC, 0x83, 0xEC, 8,    0x53, 0x56, 0x8B, 0x35, -1, -1, -1, -1,
                   0x57, 0x8B, 0xF9, 0x8B, 0xCE, 0x8B, 6,    0xFF, 0x90, 0x84, 0,  0,  0});
    client.code(0x44289B,
                {0xF3, 0x0F, 0x10, 0x45, 0xFC, 0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 4, 0});
    client.code(0x44138C, {0x33, 0xD2, 0x8B, 0x4E, 0x60, 0xFF, 0x74, 0x24, 0x1C, 0x52,
                           0x8B, 1,    0x8B, 0x80, 0x78, 3,    0,    0,    0xFF, 0xD0});
    client.code(0x2F10CB, {0x8B, 1, 0xFF, 0x90, 0x30, 4, 0, 0, 0x85, 0xC0, 0x74, 0x0A});
    client.code(0x430D06, {0x8B, 6, 0xFF, 0x90, 0x30, 4, 0, 0, 0x85, 0xC0, 0x74, 0x16});
    auto* const process =
        engine.code(0xD99C0, {0x53, 0x8A, 0xD9, 0x8B, 0x0D, -1, -1,   -1,   -1,   0x56, 0x57,
                              0x8B, 0xB9, 0x0C, 0x10, 0,    0,  0x85, 0xFF, 0x74, 0x16});
    engine.code(0xD9C66, {0x5F, 0x5E, 0x5B, 0xC3});
    engine.call(0xD9A13, 0x229070);
    engine.code(0xD9A18, {0x84, 0xC0, 0x0F, 0x84, 0x33, 2, 0, 0});
    const auto* const velocity = data.native_cell<const std::byte*>(0x43467F0Cu);
    require(velocity == client.base + 0x443481,
            "Local physics requires the earlier verified velocity-code producer");
    const auto* const branch = client.code(0x4434F1, {0x0F, 0x86, 0x8B, 0, 0, 0});
    auto* const continuation = const_cast<std::byte*>(branch + 6 + read<std::int32_t>(branch + 2));
    require(continuation == client.base + 0x443582, "Local velocity branch target differs");
    client.code(0x443582, {0xF3, 0x0F, 0x7E, 0x6C, 0x24, 0x20});
    // Exact slot88 query scopes: one native hook inspects its caller's outer
    // stack word, the other checks the direct team-query return address.
    // Both helpers independently reach virtual88 at byte offset160.
    client.code(0x4123A5, {0x8B, 0xCE});
    client.call(0x4123A7, 0x1B72A0);
    client.code(0x4123AC, {0x84, 0xC0, 0x75, 0x37, 0x38, 0x86, 0x34, 0x36, 0, 0});
    client.code(0x1B72A0, {0x56, 0x8B, 0xF1, 0x8B, 6, 0xFF, 0x90, 0x60, 1, 0, 0, 0x83, 0xF8, 1});
    client.code(0x5B1E25, {0x8B, 0xCB, 0x8B, 0xF0});
    client.call(0x5B1E29, 0x1B7230);
    client.code(0x5B1E2E, {0x3B, 0xC6, 0x74, 0x27, 0x8B, 0xCB});
    client.code(0x1B7244, {0x8B, 6, 0x8B, 0xCE, 0xFF, 0x90, 0x60, 1, 0, 0, 0x83, 0xF8, 1});
    // The separate sent-angle consumer calls virtual170 and copies all three
    // returned float words. This is an instruction continuation, not a getter.
    client.code(0x52EFA0, {0x8B, 6, 0x8B, 0xCE, 0x8B, 0x80, 0xA8, 2, 0, 0, 0xFF, 0xD0});
    client.code(0x52EFAC, {0x8B, 8,    0x89, 0x8F, 0x94, 0, 0, 0,    0x8B, 0x48,
                           4,    0x89, 0x8F, 0x98, 0,    0, 0, 0x8B, 0x40, 8});
    const BuildLocalAnimationBindings result{
        reinterpret_cast<animation::InitializeSequenceMapping>(mapping),
        reinterpret_cast<animation::SelectSymbolSequence>(select),
        reinterpret_cast<animation::LookupAnimationSymbol>(find),
        reinterpret_cast<animation::LookupAnimationSymbol>(add),
        symbols,
        activity,
        weight,
        reinterpret_cast<misc::NetworkFixEngineEntry>(process),
        reinterpret_cast<std::uintptr_t>(client.base + 0x4413A0),
        reinterpret_cast<std::uintptr_t>(client.base + 0x2F10D3),
        reinterpret_cast<std::uintptr_t>(client.base + 0x430D0E),
        reinterpret_cast<std::uintptr_t>(engine.base + 0xD9A18),
        reinterpret_cast<std::uintptr_t>(client.base + 0x443582)};
    data.native_cell<decltype(result.initialize_mapping)>(0x43469348u) = result.initialize_mapping;
    data.native_cell<decltype(result.select_symbol_sequence)>(0x4346A4F8u) =
        result.select_symbol_sequence;
    data.native_cell<decltype(result.find_symbol)>(0x434691DCu) = result.find_symbol;
    data.native_cell<decltype(result.add_symbol)>(0x4346728Cu) = result.add_symbol;
    data.native_cell<void*>(0x4346830Cu) = symbols;
    data.native_cell<void*>(0x4346A598u) = weight;
    data.native_cell<decltype(result.network_process)>(0x4346A454u) = result.network_process;
    data.native_cell<std::uintptr_t>(0x434688C4u) = result.strafe_call_site;
    data.native_cell<std::uintptr_t>(0x434689A0u) = result.proxy_call_site;
    data.native_cell<std::uintptr_t>(0x434692F4u) = result.sequence_call_site;
    data.native_cell<std::uintptr_t>(0x4346A64Cu) = result.proxy_context;
    data.native_cell<std::uintptr_t>(0x434757C4u) = result.velocity_continuation;
    data.native_cell<std::uintptr_t>(0x434674E0u) =
        reinterpret_cast<std::uintptr_t>(client.base + 0x4123AC);
    data.native_cell<std::uintptr_t>(0x4346756Cu) =
        reinterpret_cast<std::uintptr_t>(client.base + 0x5B1E2E);
    data.native_cell<std::uintptr_t>(0x43468518u) =
        reinterpret_cast<std::uintptr_t>(client.base + 0x52EFAC);
    return result;
}
animation::LocalLayerWeightEngine native_local_layer_weight_engine(void* entry) {
    return {entry, layer_weight};
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_material_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct material_bindings_Region {
    std::uint32_t rva, size, flags;
};
struct Build {
    std::uint32_t nt, sections, timestamp, size, checksum;
    std::array<material_bindings_Region, 3> regions;
};
constexpr Build material_bindings_build{0x118u,
                               6u,
                               0x63DC3DE4u,
                               0x114000u,
                               0xDE784u,
                               {{{0x1000u, 0x91D46u, 0x60000020u},
                                 {0x93000u, 0x28074u, 0x40000040u},
                                 {0xBC000u, 0x42480u, 0xC0000040u}}}};
constexpr Build localize_build{0x110u,
                               5u,
                               0x63DC12AFu,
                               0x3E000u,
                               0x38859u,
                               {{{0x1000u, 0x1FE99u, 0x60000020u},
                                 {0x21000u, 0xC3CCu, 0x40000040u},
                                 {0x2E000u, 0xC144u, 0xC0000040u}}}};
struct material_bindings_Image {
    std::byte* base;
    const Build& build;
    bool material_bindings_within(const void* pointer, std::size_t length, std::size_t region) const noexcept {
        const auto& bounds = build.regions[region];
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto first = reinterpret_cast<std::uintptr_t>(base) + bounds.rva;
        return address >= first && length <= bounds.size && address - first <= bounds.size - length;
    }
};
material_bindings_Image material_bindings_checked(std::byte* module, const Build& build, NativeImageEnvironment environment) {
    require(module && read<std::uint16_t>(module) == 0x5A4Du &&
                read<std::uint32_t>(module + 0x3C) == build.nt,
            "Material bindings require the supported loaded module headers");
    require(environment.image_nt_header != nullptr,
            "Material bindings require the actual NT-header boundary");
    const auto* nt = environment.image_nt_header(module);
    require(nt == module + build.nt && read<std::uint32_t>(nt) == 0x4550u &&
                read<std::uint16_t>(nt + 4) == 0x14Cu &&
                read<std::uint16_t>(nt + 6) == build.sections &&
                read<std::uint16_t>(nt + 20) == 0xE0u && read<std::uint16_t>(nt + 24) == 0x10Bu &&
                read<std::uint32_t>(nt + 8) == build.timestamp &&
                read<std::uint32_t>(nt + 0x50) == build.size &&
                read<std::uint32_t>(nt + 0x58) == build.checksum,
            "Material module PE32 build identity differs");
    constexpr std::array<const char*, 3> names{{".text", ".rdata", ".data"}};
    std::array<bool, 3> found{};
    for (std::uint32_t index = 0; index < build.sections; ++index) {
        const auto* section = nt + 0xF8u + index * 40u;
        const auto rva = read<std::uint32_t>(section + 12), size = read<std::uint32_t>(section + 8);
        require(rva <= build.size && size <= build.size - rva,
                "Material module section exceeds its image");
        for (std::size_t wanted = 0; wanted < names.size(); ++wanted) {
            if (std::strncmp(reinterpret_cast<const char*>(section), names[wanted], 8))
                continue;
            const auto& expected = build.regions[wanted];
            require(!found[wanted] && rva == expected.rva && size == expected.size &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Material module section fingerprint differs");
            found[wanted] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Material module sections are unavailable");
    return {module, build};
}
constexpr std::array<std::int16_t, 64> variable_pattern{{
    0x55, 0x8B, 0xEC, 0x6A, 0xFF, 0x68, -1,   -1,   -1,   -1,   0x64, 0xA1, 0x00, 0x00, 0x00, 0x00,
    0x50, 0x64, 0x89, 0x25, 0x00, 0x00, 0x00, 0x00, 0x51, 0x53, 0x56, 0x57, 0x8B, 0xFA, 0x8B, 0xD9,
    0xE8, 0xDB, 0xFD, 0xFF, 0xFF, 0x8B, 0xF0, 0x89, 0x75, 0xF0, 0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00,
    0x00, 0x85, 0xF6, 0x0F, 0x84, 0x80, 0x00, 0x00, 0x00, 0xB8, 0xFF, 0xFF, 0x00, 0x00, 0x8B, 0xCF,
}};
effects::CreateMaterialVariable material_variable(const material_bindings_Image& image) {
    const std::byte* found = nullptr;
    const auto& text = image.build.regions[0];
    for (std::uint32_t offset = 0; offset <= text.size - variable_pattern.size(); ++offset) {
        const auto* candidate = image.base + text.rva + offset;
        bool matches = true;
        for (std::size_t i = 0; i < variable_pattern.size(); ++i)
            if (variable_pattern[i] >= 0 &&
                static_cast<unsigned char>(candidate[i]) != variable_pattern[i]) {
                matches = false;
                break;
            }
        if (!matches)
            continue;
        require(!found, "Material-variable instruction signature is ambiguous");
        found = candidate;
    }
    require(found == image.base + 0x24B10u,
            "Genuine supported material-variable function is unavailable");
    // Complete fastcall body: ECX is the material stored at object+20, EDX
    // supplies the interned variable name, and both exits use plain RET.
    require(image.material_bindings_within(read<const void*>(found + 6), 1, 0),
            "Material-variable exception handler has invalid provenance");
    return reinterpret_cast<effects::CreateMaterialVariable>(const_cast<std::byte*>(found));
}
void* localization(const material_bindings_Image& image, NativeImageEnvironment environment) {
    auto* registry = find_interface_registry(image.base, environment);
    require(registry && image.material_bindings_within(registry, sizeof(InterfaceRegistration), 2),
            "Localize_001 registry is unavailable in the loaded module");
    auto* registration = find_interface_registration(registry, netvar_name_hash("Localize_001"));
    require(registration && image.material_bindings_within(registration, sizeof(InterfaceRegistration), 2) &&
                image.material_bindings_within(registration->name, sizeof("Localize_001"), 1) &&
                std::strcmp(registration->name, "Localize_001") == 0,
            "Required exact Localize_001 registration is unavailable");
    const auto* creator = reinterpret_cast<const std::byte*>(registration->create);
    require(creator == image.base + 0x1AF0u && image.material_bindings_within(creator, 6, 0) &&
                creator[0] == std::byte{0xB8} && creator[5] == std::byte{0xC3},
            "Localize_001 creator differs from its proven singleton getter");
    auto* object = read<void*>(creator + 1);
    require(object == image.base + 0x38A70u && image.material_bindings_within(object, 4, 2),
            "Localization singleton has invalid module provenance");
    auto* table = read<void**>(object);
    require(image.material_bindings_within(table, 12 * 4, 1),
            "Localization virtual table is outside its real module");
    for (std::size_t slot = 0; slot < 12; ++slot)
        require(image.material_bindings_within(table[slot], 1, 0),
                "Localization method is outside its real executable section");
    return object;
}
}
BuildMaterialBindings initialize_build_material_bindings(NativeDataImage& data,
                                                         const LoadedModuleEnvironment& modules,
                                                         NativeImageEnvironment images) {
    require(modules.loader && modules.acquire_peb_lock && modules.release_peb_lock,
            "Material acquisition requires the real loaded-module environment");
    auto* module = static_cast<std::byte*>(find_loaded_module(
        modules, loaded_module_name_hash(u"materialsystem.dll", 18), true, true));
    require(module && module == data.native_cell<std::byte*>(0x4346A600u),
            "Material acquisition must use the canonically published loaded materialsystem module");
    auto* localize_module = static_cast<std::byte*>(
        find_loaded_module(modules, loaded_module_name_hash(u"localize.dll", 12), true, true));
    const auto material = material_bindings_checked(module, material_bindings_build, images);
    const auto localize = material_bindings_checked(localize_module, localize_build, images);
    const auto create = material_variable(material);
    auto* strings = localization(localize, images);
    data.native_cell<effects::CreateMaterialVariable>(0x43467A68u) = create;
    data.native_cell<void*>(0x4346A31Cu) = strings;
    return {data, create, strings};
}
BuildMaterialBindings initialize_build_material_bindings(NativeDataImage& data) {
    return initialize_build_material_bindings(data, native_loaded_module_environment(),
                                              native_image_environment());
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_misc_engine_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct misc_engine_bindings_Image {
    std::byte* base;
    std::uint32_t text_size, data_rva, data_size;
    void code(std::uint32_t rva, std::initializer_list<int> pattern) const {
        require(rva >= 0x1000u && pattern.size() <= text_size &&
                    rva - 0x1000u <= text_size - pattern.size(),
                "Misc binding instruction exceeds module text");
        std::size_t index = 0;
        for (const int value : pattern) {
            require(value < 0 || static_cast<unsigned char>(base[rva + index]) == value,
                    "Misc binding instruction fingerprint differs");
            ++index;
        }
    }
    std::byte* operand(std::uint32_t rva, std::uint32_t target) const {
        auto* const pointer = base + target;
        require(read<std::uint32_t>(base + rva) == reinterpret_cast<std::uintptr_t>(pointer),
                "Misc binding operand has different module provenance");
        return pointer;
    }
    void call(std::uint32_t rva, std::uint32_t target) const {
        require(base[rva] == std::byte{0xE8} &&
                    rva + 5u + static_cast<std::uint32_t>(read<std::int32_t>(base + rva + 1)) ==
                        target,
                "Misc binding relative helper identity differs");
    }
};
misc_engine_bindings_Image misc_engine_bindings_checked_image(void* module, bool client) {
    require(module != nullptr, "Misc bindings require initialized client and engine modules");
    auto* const base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Unsupported misc binding DOS/NT-header layout");
    const auto* const nt = base + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == (client ? 0x65272025u : 0x64EE7E86u) &&
                read<std::uint32_t>(nt + 0x50) == (client ? 0x5509000u : 0x39C0000u) &&
                read<std::uint32_t>(nt + 0x58) == (client ? 0xFA635Fu : 0x64174Eu),
            "Unsupported misc binding client/engine build");
    const misc_engine_bindings_Image image{base, client ? 0xB38409u : 0x451C32u, client ? 0xDA1000u : 0x528000u,
                      client ? 0x460C3E0u : 0x3427638u};
    unsigned text = 0, data = 0;
    for (unsigned index = 0; index < 7; ++index) {
        const auto* const section = nt + 0xF8 + index * 40;
        if (std::memcmp(section, ".text\0\0\0", 8) == 0) {
            require(read<std::uint32_t>(section + 8) == image.text_size &&
                        read<std::uint32_t>(section + 12) == 0x1000 &&
                        read<std::uint32_t>(section + 36) == 0x60000020,
                    "Unsupported misc binding text section");
            ++text;
        }
        if (std::memcmp(section, ".data\0\0\0", 8) == 0) {
            require(read<std::uint32_t>(section + 8) == image.data_size &&
                        read<std::uint32_t>(section + 12) == image.data_rva &&
                        read<std::uint32_t>(section + 36) == 0xC0000040,
                    "Unsupported misc binding data section");
            ++data;
        }
    }
    require(text == 1 && data == 1, "Misc binding module section is missing or duplicated");
    return image;
}
}

BuildMiscEngineBindings initialize_build_misc_engine_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Misc engine bindings require the original x86 ABI");
    require(data.size() >= NativeDataImage::image_size,
            "Misc bindings require complete canonical storage");
    const auto client = misc_engine_bindings_checked_image(data.native_cell<void*>(0x4346A55Cu), true);
    const auto engine = misc_engine_bindings_checked_image(data.native_cell<void*>(0x43468D20u), false);
    // ECX and EDX are saved as the first/second strings before either helper
    // call. The body forwards both into the real ClanTagChanged KeyValues.
    engine.code(0x8DAB0,
                {0x53, 0x56, 0x57, 0x8B, 0xDA, 0x8B, 0xF9, 0xFF, 0x15, -1,   -1,   -1,   -1,   0x6A,
                 0x2C, 0x8B, 0xC8, 0x8B, 0x30, 0xFF, 0x56, 8,    0x85, 0xC0, 0x74, 0x13, 0x51, 0x6A,
                 0,    0x68, -1,   -1,   -1,   -1,   0x8B, 0xC8, 0xE8, -1,   -1,   -1,   -1,   0x8B,
                 0xF0, 0xEB, 2,    0x33, 0xF6, 0x6A, 1,    0x68, -1,   -1,   -1,   -1,   0x8B, 0xCE,
                 0xE8, -1,   -1,   -1,   -1,   0x85, 0xC0, 0x74, 8,    0x57, 0x8B, 0xC8, 0xE8, -1,
                 -1,   -1,   -1,   0x6A, 1,    0x68, -1,   -1,   -1,   -1,   0x8B, 0xCE, 0xE8, -1,
                 -1,   -1,   -1,   0x85, 0xC0, 0x74, 8,    0x53, 0x8B, 0xC8, 0xE8, -1,   -1,   -1,
                 -1,   0xA1, -1,   -1,   -1,   -1,   0xB9, -1,   -1,   -1,   -1,   0x56, 0xFF, 0x90,
                 0xEC, 2,    0,    0,    0x5F, 0x5E, 0x5B, 0xC3});
    engine.operand(0x8DAB9, 0x4537E4);
    const auto* const event_name = engine.operand(0x8DACE, 0x478410);
    const auto* const tag_name = engine.operand(0x8DAE2, 0x478420);
    const auto* const name_name = engine.operand(0x8DAFC, 0x47BD5C);
    require(std::memcmp(event_name, "ClanTagChanged", 15) == 0 &&
                std::memcmp(tag_name, "tag", 4) == 0 && std::memcmp(name_name, "name", 5) == 0,
            "Clan-tag engine message strings differ");
    engine.operand(0x8DB14, 0x59D3E8);
    engine.operand(0x8DB19, 0x59D3E8);
    engine.call(0x8DAD4, 0x2FE940);
    engine.call(0x8DAE8, 0x2FF6B0);
    engine.call(0x8DAF4, 0x300080);
    engine.call(0x8DB02, 0x2FF6B0);
    engine.call(0x8DB0E, 0x300080);

    // The hit-marker sound suppression list is the engine's real64-player
    // array. Its writer bounds the copy to64 entries, and an independent
    // sound-channel consumer reads the same array and count. A valid empty
    // list is retained through this address, not replaced by a null view.
    engine.code(0x242D0,
                {0x55, 0x8B, 0xEC, 0x56, 0xBE, 0x40, 0,    0,    0,    0x39, 0x75, 0x0C, 0x0F,
                 0x46, 0x75, 0x0C, 0x8D, 0x04, 0xB5, 0,    0,    0,    0,    0x50, 0xFF, 0x75,
                 8,    0x68, -1,   -1,   -1,   -1,   0xE8, -1,   -1,   -1,   -1,   0x83, 0xC4,
                 0x0C, 0x89, 0x35, -1,   -1,   -1,   -1,   0x5E, 0x5D, 0xC2, 8,    0});
    auto* const sound_players = engine.operand(0x242EC, 0x38FDCC8);
    engine.operand(0x242FA, 0x391AFA8);
    engine.call(0x242F0, 0x3FB9E0);
    engine.code(0x3A087,
                {0x8B, 0x0D, -1,   -1, -1, -1, 0x33, 0xC0, 0x85, 0xC9, 0x7E, 0x11, 0x8B, 0x56, 4,
                 0x3B, 0x14, 0x85, -1, -1, -1, -1,   0x74, 0x51, 0x40, 0x3B, 0xC1, 0x7C, 0xF2});
    engine.operand(0x3A089, 0x391AFA8);
    engine.operand(0x3A099, 0x38FDCC8);
    // Performance-owner slot1 retains the operand word at+4 of these actual
    // engine clock instructions. Keep the instruction records themselves;
    // the native constructor does not receive a snapshot of either clock.
    engine.code(0x8E550, {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x81, 0xEC, 0x9C, 0,    0,
                          0,    0xF2, 0x0F, 0x10, 5,    -1,   -1,   -1,   -1,   0x8D, 0x44,
                          0x24, 0x1C, 0x56, 0x6A, 0x20, 0xF2, 0x0F, 0x11, 0x41, 0x60});
    engine.operand(0x8E560, 0x391DA98);
    engine.code(0x22BE2E, {0xF2, 0x0F, 0x10, 0x45, 0xA0, 0x0F, 0x28, 0xC8, 0xF2, 0x0F, 0x5C,
                           0x0D, -1,   -1,   -1,   -1,   0xF2, 0x0F, 0x11, 5,    -1,   -1,
                           -1,   -1,   0xF2, 0x0F, 0x10, 0x45, 0xA8, 0x66, 0x0F, 0x2F, 0xC8});
    engine.operand(0x22BE3A, 0x391DA68);
    engine.operand(0x22BE42, 0x391DA68);

    client.code(0x195E80, {0x55, 0x8B, 0xEC, 0x83, 0x3D, -1, -1, -1, -1, 0, 0x74, 0x2A});
    auto* const game_rules = client.operand(0x195E85, 0x5334764);
    client.code(0x196BA3, {0x8B, 0x0D, -1,   -1, -1,   -1,   0x85, 0xC9, 0x0F, 0x84, 3,    1,
                           0,    0,    0x8B, 1,  0x8B, 0x80, 0x84, 0,    0,    0,    0xFF, 0xD0});
    client.operand(0x196BA5, 0x5334764);
    client.code(0x1B430E, {0xA1, -1,   -1, -1,   -1, 0x89, 0x44, 0x24, 0x60, 0x85, 0xC0,
                           0x74, 0x3B, 5,  0xD8, 9,  0,    0,    0x74, 0x34, 0x8B, 0x30,
                           0x83, 0xC1, 8,  0x8B, 1,  0x8B, 0x40, 0x28, 0xFF, 0xD0});
    auto* const player_resource = client.operand(0x1B430F, 0x3231380);
    // Both the element pointer and signed element count are loaded from the
    // same CUtlVector header; no current planted entity is captured here.
    client.code(0x3F6510, {0x55, 0x8B, 0xEC, 0x8B, 0x15, -1,   -1,   -1,   -1,   0x33, 0xC9,
                           0x56, 0x85, 0xD2, 0x7E, 0x1A, 0x8B, 0x45, 8,    0x8B, 0x35, -1,
                           -1,   -1,   -1,   0x8B, 0,    0x0F, 0x1F, 0x44, 0,    0,    0x39,
                           4,    0x8E, 0x74, 0x0C, 0x41, 0x3B, 0xCA, 0x7C, 0xF6});
    client.operand(0x3F6515, 0x525D44C);
    auto* const planted_bombs = client.operand(0x3F6525, 0x525D440);
    client.code(0x3F54CA, {0x8B, 0x0D, -1, -1, -1, -1,   0x33, 0xC0, 0x85, 0xC9, 0x7E, 0x2C, 0x8B,
                           0x15, -1,   -1, -1, -1, 0x0F, 0x1F, 0x40, 0,    0x39, 0x34, 0x82});
    client.operand(0x3F54CC, 0x525D44C);
    client.operand(0x3F54D8, 0x525D440);

    // Independently follow the real SetLocalPlayerReady registration, its
    // JavaScript wrapper and the one-string bridge to the actual stdcall
    // engine operation. The old static loader VA is not a live callback.
    client.code(0x6041DC,
                {0x68, -1, -1, -1, -1, 0x50, 0x8D, 0x45, 0xF0, 0x50, 0xFF, 0x15, -1, -1, -1, -1});
    client.operand(0x6041DD, 0x605E10);
    client.code(0x604208, {0x68, -1, -1, -1, -1, 0x50, 0x8D, 0x45, 0xEC, 0x50});
    auto* const ready_name = client.operand(0x604209, 0xC71A1C);
    require(std::memcmp(ready_name, "SetLocalPlayerReady", 20) == 0,
            "Auto-accept JavaScript registration name differs");
    client.code(0x604239, {0x68, -1, -1, -1, -1, 0xFF, 0x90, 0x94, 2, 0, 0});
    client.operand(0x60423A, 0xC71A1C);
    client.code(0x605F41, {0x8D, 0x44, 0x24, 0x0C, 0x8B, 0xD7, 0x50, 0x8B, 0xCE});
    client.call(0x605F4A, 0x606D30);
    client.code(0x606D30, {0x55, 0x8B, 0xEC, 0x8B, 0x45, 8, 0x53, 0x56, 0x8B, 0xF2, 0xFF, 0x30});
    client.call(0x606D3C, 0x601760);
    client.code(0x606D41,
                {0x8B, 0x0D, -1, -1, -1, -1, 0x8A, 0xD8, 0x8B, 0x11, 0xFF, 0x92, 4, 2, 0, 0});
    client.code(0x601760,
                {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x8B, 0x4D, 8,    0xBA, -1,   -1,   -1,   -1,
                 0xE8, -1,   -1,   -1,   -1,   0x85, 0xC0, 0x75, 0x12, 0x8D, 0x50, 2,    0x33, 0xC9,
                 0xE8, -1,   -1,   -1,   -1,   0xB0, 1,    0x8B, 0xE5, 0x5D, 0xC2, 4,    0,    0x83,
                 0x3D, -1,   -1,   -1,   -1,   0,    0x74, 0x0C, 0x51, 0xE8, -1,   -1,   -1,   -1,
                 0x8B, 0xE5, 0x5D, 0xC2, 4,    0,    0x32, 0xC0, 0x8B, 0xE5, 0x5D, 0xC2, 4,    0});
    auto* const deferred = client.operand(0x60176A, 0xC72164);
    require(std::memcmp(deferred, "deferred", 9) == 0,
            "Auto-accept deferred operation name differs");
    client.operand(0x60178B, 0x5344D68);
    client.call(0x60176E, 0x9962C0);
    client.call(0x60177C, 0x425580);
    client.call(0x601793, 0x425E70);

    const BuildMiscEngineBindings result{
        reinterpret_cast<misc::SetClanTag>(engine.base + 0x8DAB0),
        reinterpret_cast<void**>(game_rules), reinterpret_cast<void**>(player_resource),
        reinterpret_cast<const misc::AutomaticWeaponsBombList*>(planted_bombs)};
    data.native_cell<misc::SetClanTag>(0x43467550u) = result.set_clan_tag;
    data.native_cell<void**>(0x4346A3A4u) = result.game_rules;
    data.native_cell<void**>(0x43468FF0u) = result.player_resource;
    data.native_cell<const misc::AutomaticWeaponsBombList*>(0x434688C0u) = result.planted_bombs;
    data.native_cell<std::int32_t*>(0x4346D558u) = reinterpret_cast<std::int32_t*>(sound_players);
    data.native_cell<void*>(0x4346D554u) = client.base + 0x601760;
    data.native_cell<void*>(0x4346D560u) = engine.base + 0x8E55C;
    data.native_cell<void*>(0x4346D55Cu) = engine.base + 0x22BE36;
    return result;
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_module_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
constexpr std::array<BuildModuleRecipe, 17> build_module_recipes{{
    {0x434670ACu, 0xFAE7A604u, u"matchmaking.dll"},
    {0x434670E4u, 0x70A76D19u, u"server.dll"},
    {0x4346762Cu, 0xD45F4768u, u"panoramauiclient.dll"},
    {0x43467AFCu, 0x424EC73Eu, u"vguimatsurface.dll"},
    {0x43467B74u, 0xDF6E939Bu, u"filesystem_stdio.dll"},
    {0x43467DACu, 0xDB89A589u, u"inputsystem.dll"},
    {0x4346889Cu, 0xAD47D7B1u, u"vgui2.dll"},
    {0x43468A7Cu, 0x0BCD5926u, u"vstdlib.dll"},
    {0x43468B74u, 0x9BB09935u, u"vphysics.dll"},
    {0x43468D20u, 0xAA8F5C60u, u"engine.dll"},
    {0x43468D40u, 0x2D366476u, u"soundsystem.dll"},
    {0x43468D98u, 0x9BD3DF1Eu, u"datacache.dll"},
    {0x43469234u, 0xA7DF62D9u, u"panorama.dll"},
    {0x4346A55Cu, 0xFE901E85u, u"client.dll"},
    {0x4346A600u, 0x1533A6AAu, u"materialsystem.dll"},
    {0x4346A604u, 0xBBF5D94Cu, u"shaderapidx9.dll"},
    {0x4346A650u, 0x76C44BACu, u"studiorender.dll"},
}};
constexpr BuildModuleRecipe overlay_recipe{0x43468D08u, 0x8B08B50Du, u"gameoverlayrenderer.dll"};

void validate_recipe(NativeDataImage& data, const BuildModuleRecipe& recipe) {
    (void)data.native_cell<void*>(recipe.destination_cell);
    if (loaded_module_name_hash(recipe.module_name.data(),
                                static_cast<std::int32_t>(recipe.module_name.size())) !=
        recipe.salted_name_hash)
        throw std::logic_error("Build module name differs from its loader hash");
}
[[noreturn]] void missing_module(const BuildModuleRecipe& recipe) {
    throw std::runtime_error("Required build module is not loaded: " +
                             std::string(recipe.module_name.begin(), recipe.module_name.end()));
}
[[noreturn]] void invalid_module(const BuildModuleRecipe& recipe) {
    throw std::runtime_error("Loaded build module has an invalid basename or PE32 image: " +
                             std::string(recipe.module_name.begin(), recipe.module_name.end()));
}
struct PebLock {
    const LoadedModuleEnvironment& environment;
    explicit PebLock(const LoadedModuleEnvironment& source) : environment(source) {
        environment.acquire_peb_lock();
    }
    ~PebLock() {
        environment.release_peb_lock();
    }
};
/* [dedup read/write] */
bool exact_name(const LoaderUnicodeName& actual, std::u16string_view expected) noexcept {
    if (!actual.buffer || (actual.length & 1u) || actual.length / 2u != expected.size())
        return false;
    for (std::size_t index = 0; index < expected.size(); ++index) {
        auto value = actual.buffer[index];
        if (value >= u'A' && value <= u'Z')
            value += u'a' - u'A';
        if (value != expected[index])
            return false;
    }
    return true;
}
bool pe32_image(const LoaderModule& module) {
    if (module.image_size < 0x40)
        return false;
    const auto* const base = static_cast<const std::byte*>(module.base);
    if (read<std::uint16_t>(base) != 0x5A4Du)
        return false;
    const auto offset = read<std::uint32_t>(base + 0x3C);
    if (offset < 0x40 || offset > module.image_size - 0x1Au)
        return false;
    const auto* const nt = base + offset;
    const auto optional_size = read<std::uint16_t>(nt + 0x14);
    if (optional_size < 2 || optional_size > module.image_size - offset - 0x18u)
        return false;
    return read<std::uint32_t>(nt) == 0x00004550u && read<std::uint16_t>(nt + 4) == 0x014Cu &&
           read<std::uint16_t>(nt + 0x18) == 0x010Bu;
}
void* resolve_module(const LoadedModuleEnvironment& environment, const BuildModuleRecipe& recipe) {
    const PebLock locked(environment);
    // Reuse433F29A5's hash resolver under this same existing PEB lock. The
    //additional checks accept only the named x86 image, not another hash match.
    auto* const base = find_loaded_module(environment, recipe.salted_name_hash, true, false);
    if (!base)
        return nullptr;
    auto* const sentinel = &environment.loader->modules;
    for (auto* link = sentinel->next; link != sentinel; link = link->next) {
        const auto& module = *reinterpret_cast<const LoaderModule*>(link);
        if (module.base != base)
            continue;
        if (!exact_name(module.base_name, recipe.module_name) || !pe32_image(module))
            invalid_module(recipe);
        return base;
    }
    invalid_module(recipe);
}
}

std::span<const BuildModuleRecipe, 17> build_module_manifest() noexcept {
    return build_module_recipes;
}
const BuildModuleRecipe& build_overlay_module_recipe() noexcept {
    return overlay_recipe;
}
void* BuildModuleBases::at(std::u16string_view name) const {
    for (std::size_t index = 0; index < build_module_recipes.size(); ++index)
        if (build_module_recipes[index].module_name == name)
            return modules[index];
    if (overlay_recipe.module_name == name)
        return overlay;
    throw std::out_of_range("Name is not a build module binding");
}
BuildModuleBases publish_build_module_bases(NativeDataImage& data,
                                            const LoadedModuleEnvironment& environment) {
    if (!environment.loader || !environment.acquire_peb_lock || !environment.release_peb_lock)
        throw std::invalid_argument(
            "Build module publication requires the real loaded-module list and locks");
    for (const auto& recipe : build_module_recipes)
        validate_recipe(data, recipe);
    validate_recipe(data, overlay_recipe);
    BuildModuleBases result;
    for (std::size_t index = 0; index < build_module_recipes.size(); ++index) {
        result.modules[index] = resolve_module(environment, build_module_recipes[index]);
        if (!result.modules[index])
            missing_module(build_module_recipes[index]);
    }
    result.overlay = resolve_module(environment, overlay_recipe);
    for (std::size_t index = 0; index < build_module_recipes.size(); ++index)
        data.native_cell<void*>(build_module_recipes[index].destination_cell) = result.modules[index];
    data.native_cell<void*>(overlay_recipe.destination_cell) = result.overlay;
    return result;
}
BuildModuleBases publish_build_module_bases(NativeDataImage& data,
                                            const SourceInitializationImports& imports) {
    return publish_build_module_bases(data, imports.modules);
}
BuildModuleBases publish_build_module_bases(NativeDataImage& data) {
    return publish_build_module_bases(data, native_loaded_module_environment());
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_packet_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct packet_bindings_Section {
    char name[8];
    std::uint32_t packet_bindings_rva, size, flags;
};
constexpr std::array<packet_bindings_Section, 3> packet_bindings_sections{{{".text", 0x1000, 0x451C32, 0x60000020},
                                           {".rdata", 0x453000, 0xD4766, 0x40000040},
                                           {".data", 0x528000, 0x3427638, 0xC0000040}}};
struct packet_bindings_Image {
    std::byte* base;
    bool contains(const void* pointer, std::size_t length, std::size_t index) const noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin = reinterpret_cast<std::uintptr_t>(base) + packet_bindings_sections[index].packet_bindings_rva;
        return address >= begin && length <= packet_bindings_sections[index].size &&
               address - begin <= packet_bindings_sections[index].size - length;
    }
    std::uint32_t packet_bindings_rva(const void* pointer) const noexcept {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer) -
                                          reinterpret_cast<std::uintptr_t>(base));
    }
};
packet_bindings_Image packet_bindings_checked_image(void* module) {
    require(module != nullptr, "Packet query requires the initialized engine module");
    auto* base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Unsupported packet query engine DOS/NT layout");
    const auto* nt = base + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x64EE7E86 &&
                read<std::uint32_t>(nt + 0x50) == 0x39C0000 &&
                read<std::uint32_t>(nt + 0x58) == 0x64174E,
            "Unsupported packet query engine PE32 build");
    std::array<bool, 3> found{};
    for (std::size_t index = 0; index < 7; ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        const auto size = read<std::uint32_t>(section + 8), packet_bindings_rva = read<std::uint32_t>(section + 12);
        require(packet_bindings_rva <= 0x39C0000 && size <= 0x39C0000 - packet_bindings_rva,
                "Packet engine section exceeds its image");
        for (std::size_t wanted = 0; wanted < packet_bindings_sections.size(); ++wanted) {
            const auto& expected = packet_bindings_sections[wanted];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[wanted] && packet_bindings_rva == expected.packet_bindings_rva && size == expected.size &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Unsupported packet query section fingerprint");
            found[wanted] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Packet engine section is missing");
    return {base};
}
/* [dedup matches] */
void packet_bindings_expect(const packet_bindings_Image& image, const std::byte* code, std::initializer_list<int> pattern) {
    require(image.contains(code, pattern.size(), 0) &&
                matches(code, {pattern.begin(), pattern.size()}),
            "Packet instruction fingerprint differs");
}
const std::byte* packet_bindings_acquire(const packet_bindings_Image& image) {
    constexpr std::array pattern{
        0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF0, 0x81, 0xEC, 0xB8, 0,    0,    0,    0xF3, 0x0F, 0x10,
        0x35, -1,   -1,   -1,   -1,   0x8B, 0xC2, 0x56, 0x57, 0x8B, 0xF9, 0xF3, 0x0F, 0x11, 0x5C,
        0x24, 0x18, 0x8B, 0x0D, -1,   -1,   -1,   -1,   0x89, 0x44, 0x24, 0x5C, 0xF3, 0x0F, 0x10,
        0x57, 0x0C, 0xF3, 0x0F, 0x10, 0x4F, 0x10, 0xF3, 0x0F, 0x10, 0x47, 0x14};
    const std::byte* found = nullptr;
    for (std::uint32_t i = 0; i <= packet_bindings_sections[0].size - pattern.size(); ++i) {
        const auto* code = image.base + packet_bindings_sections[0].packet_bindings_rva + i;
        if (!matches(code, pattern))
            continue;
        require(!found, "Packet occlusion instruction anchor is ambiguous");
        found = code;
    }
    require(found && image.packet_bindings_rva(found) == 0x20CE50, "Unsupported packet occlusion implementation");
    require(image.contains(found, 0x613, 0), "Packet occlusion function exceeds executable text");
    const auto absolute = [&](std::size_t at, std::size_t section, std::uint32_t packet_bindings_rva) {
        const auto* value =
            reinterpret_cast<const std::byte*>(std::uintptr_t(read<std::uint32_t>(found + at)));
        require(image.contains(value, 4, section) && image.packet_bindings_rva(value) == packet_bindings_rva,
                "Packet occlusion operand lacks verified module provenance");
        return value;
    };
    require(read<std::uint32_t>(absolute(0x10, 1, 0x4F57A0)) == 0x3F000000,
            "Packet occlusion half constant differs");
    (void)absolute(0x22, 2, 0x5A74C4);
    packet_bindings_expect(image, found + 0x251, {0x8B, 0x75, 0x08, 0xF3, 0x0F, 0x10, 0x15, -1, -1, -1, -1});
    const auto call = [&](std::size_t at, std::uint32_t destination) {
        packet_bindings_expect(image, found + at, {0xE8, -1, -1, -1, -1});
        require(std::int64_t(image.packet_bindings_rva(found)) + at + 5 + read<std::int32_t>(found + at + 1) ==
                    destination,
                "Packet occlusion helper target differs");
    };
    call(0x40B, 0x20CCF0);
    call(0x464, 0x1E0F40);
    call(0x569, 0x1E1320);
    call(0x605, 0x1E0F40);
    packet_bindings_expect(image, found + 0x56E,
           {0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC3, 0x32, 0xC0, 0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC3});
    packet_bindings_expect(image, found + 0x60A, {0x83, 0xC4, 0x0C, 0x5F, 0x5E, 0x8B, 0xE5, 0x5D, 0xC3});
    return found;
}
}
void initialize_build_packet_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Packet engine publication requires x86");
    const auto image = packet_bindings_checked_image(data.native_cell<void*>(0x43468D20u));
    const auto* entry = packet_bindings_acquire(image);
    data.native_cell<std::uintptr_t>(0x4346A7CCu) = reinterpret_cast<std::uintptr_t>(entry);
}
PacketBoxRuntime::PacketBoxRuntime(const NativeDataImage& data)
    : entry_(data.native_cell<std::uintptr_t>(0x4346A7CCu)) {
    require(sizeof(void*) == 4 && entry_,
            "Packet geometry requires its acquired canonical x86 engine entry");
}
network::PacketBoxQuery PacketBoxRuntime::binding() noexcept {
    return {this, invoke};
}
bool PacketBoxRuntime::invoke(void* self, const network::Bounds& moving,
                              const network::Bounds& target, network::Vector& movement,
                              float expansion) {
    return static_cast<PacketBoxRuntime*>(self)->query(moving, target, movement, expansion);
}
bool PacketBoxRuntime::query(const network::Bounds& moving, const network::Bounds& target,
                             network::Vector& movement, float expansion) const {
    require(entry_ != 0, "Packet geometry engine entry is no longer bound");
    return detail::invoke_packet_box(entry_, moving, target, movement, expansion);
}
#if defined(__i386__) && defined(__clang__)
__attribute__((naked, noinline)) bool detail::invoke_packet_box(std::uintptr_t,
                                                                const network::Bounds&,
                                                                const network::Bounds&,
                                                                network::Vector&, float) {
    __asm__ __volatile__("pushl %ebp\n\t"
                         "movl %esp, %ebp\n\t"
                         "movl 8(%ebp), %eax\n\t"
                         "movl 12(%ebp), %ecx\n\t"
                         "movl 16(%ebp), %edx\n\t"
                         "movss 24(%ebp), %xmm3\n\t"
                         "pushl 20(%ebp)\n\t"
                         "call *%eax\n\t"
                         "addl $4, %esp\n\t"
                         "movzbl %al, %eax\n\t"
                         "leave\n\t"
                         "ret");
}
#else
bool detail::invoke_packet_box(std::uintptr_t, const network::Bounds&, const network::Bounds&,
                               network::Vector&, float) {
    throw std::logic_error("Packet geometry requires its Windows x86 SSE ABI bridge");
}
#endif
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_prediction_scratch_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* deduplicated helper */
/* [dedup require] */
constexpr std::array<unsigned char, 0x130> rebuild_bytes{
    0x55, 0x8b, 0xec, 0x83, 0xec, 0x18, 0x57, 0x8b, 0xf9, 0x89, 0x7d, 0xf4, 0x83, 0x7f, 0x14, 0x00,
    0x0f, 0x85, 0x14, 0x01, 0x00, 0x00, 0x53, 0x56, 0xff, 0x35, 0,    0,    0,    0,    0xb9, 0,
    0,    0,    0,    0xe8, 0xe8, 0xd8, 0x65, 0x00, 0x8b, 0xf0, 0x85, 0xf6, 0x74, 0x14, 0x6a, 0x60,
    0x6a, 0x00, 0x56, 0xe8, 0xe8, 0x4b, 0x78, 0x00, 0x83, 0xc4, 0x0c, 0x8b, 0xce, 0xe8, 0x7e, 0x17,
    0x00, 0x00, 0x8d, 0x4d, 0xe8, 0x89, 0x77, 0x14, 0x33, 0xdb, 0xc7, 0x45, 0xe8, 0x00, 0x00, 0x00,
    0x00, 0xc7, 0x45, 0xec, 0x00, 0x00, 0x00, 0x00, 0x33, 0xf6, 0x89, 0x4d, 0xfc, 0x0f, 0x1f, 0x00,
    0x8b, 0x47, 0x14, 0x8b, 0xd1, 0x6a, 0x00, 0x03, 0xc6, 0x8b, 0xcb, 0x50, 0x57, 0x6a, 0x00, 0xe8,
    0x2c, 0xfb, 0xff, 0xff, 0x8b, 0x4d, 0xfc, 0x83, 0xc6, 0x30, 0x83, 0xc1, 0x04, 0x83, 0xc4, 0x10,
    0x43, 0x89, 0x4d, 0xfc, 0x83, 0xfe, 0x60, 0x7c, 0xd7, 0x33, 0xdb, 0x8d, 0x45, 0xe8, 0x89, 0x5d,
    0xf8, 0x89, 0x45, 0xfc, 0x8b, 0x57, 0x14, 0x33, 0xff, 0x8b, 0x08, 0x03, 0xd3, 0x89, 0x4d, 0xf0,
    0x39, 0x7a, 0x0c, 0x7e, 0x29, 0x33, 0xf6, 0x8b, 0xd9, 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x8b, 0x0a, 0x8d, 0x76, 0x3c, 0x66, 0x8b, 0xc3, 0x47, 0x66, 0x2b, 0x44, 0x31, 0xfc, 0x66, 0x89,
    0x44, 0x31, 0xfc, 0x3b, 0x7a, 0x0c, 0x7c, 0xe8, 0x8b, 0x5d, 0xf8, 0x8b, 0x45, 0xfc, 0x8b, 0x7d,
    0xf4, 0x83, 0xc3, 0x30, 0x83, 0xc0, 0x04, 0x89, 0x5d, 0xf8, 0x89, 0x45, 0xfc, 0x83, 0xfb, 0x60,
    0x7c, 0xb2, 0xc7, 0x45, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x33, 0xdb, 0x0f, 0x1f, 0x44, 0x00, 0x00,
    0x8b, 0x77, 0x14, 0x03, 0xf3, 0x68, 0,    0,    0,    0,    0x6a, 0x3c, 0xff, 0x76, 0x0c, 0xff,
    0x36, 0xe8, 0x6a, 0x4f, 0x79, 0x00, 0x8b, 0x4d, 0xf8, 0x83, 0xc4, 0x10, 0x8b, 0xd6, 0xe8, 0x5d,
    0xfc, 0xff, 0xff, 0x83, 0xc3, 0x30, 0x89, 0x45, 0xf8, 0x89, 0x47, 0x10, 0x83, 0xfb, 0x60, 0x7c,
    0xcf, 0x8b, 0xcf, 0xe8, 0x08, 0xfd, 0xff, 0xff, 0x5e, 0x5b, 0x5f, 0x8b, 0xe5, 0x5d, 0xc3, 0xcc};
bool relocated(std::size_t offset) noexcept {
    return (offset >= 0x1a && offset < 0x1e) || (offset >= 0x1f && offset < 0x23) ||
           (offset >= 0xf6 && offset < 0xfa);
}
}
void initialize_build_prediction_scratch_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Prediction scratch acquisition requires x86");
    auto* const base = data.native_cell<std::byte*>(0x4346A55Cu);
    require(base != nullptr, "Prediction scratch requires the acquired client module");
    require(read<std::uint16_t>(base) == 0x5a4d && read<std::uint32_t>(base, 0x3c) == 0x150,
            "Unsupported prediction scratch client DOS/NT layout");
    const auto* const nt = base + 0x150;
    require(
        read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt, 4) == 0x14c &&
            read<std::uint16_t>(nt, 6) == 7 && read<std::uint16_t>(nt, 0x14) == 0xe0 &&
            read<std::uint16_t>(nt, 0x18) == 0x10b && read<std::uint32_t>(nt, 8) == 0x65272025 &&
            read<std::uint32_t>(nt, 0x50) == 0x5509000 && read<std::uint32_t>(nt, 0x58) == 0xfa635f,
        "Unsupported prediction scratch client PE32 build");
    struct prediction_scratch_bindings_Section {
        char name[8];
        std::uint32_t rva, size, flags;
    };
    constexpr std::array<prediction_scratch_bindings_Section, 2> prediction_scratch_bindings_sections{
        {{".text", 0x1000, 0xb38409, 0x60000020}, {".data", 0xda1000, 0x460c3e0, 0xc0000040}}};
    std::array<bool, 2> found{};
    for (unsigned index = 0; index < 7; ++index) {
        const auto* section = nt + 0xf8 + 40 * index;
        const auto size = read<std::uint32_t>(section, 8), rva = read<std::uint32_t>(section, 12);
        require(rva <= 0x5509000 && size <= 0x5509000 - rva,
                "Prediction scratch section exceeds client image");
        for (unsigned wanted = 0; wanted < prediction_scratch_bindings_sections.size(); ++wanted) {
            const auto& expected = prediction_scratch_bindings_sections[wanted];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[wanted] && rva == expected.rva && size == expected.size &&
                        read<std::uint32_t>(section, 36) == expected.flags,
                    "Unsupported prediction scratch section fingerprint");
            found[wanted] = true;
        }
    }
    require(found[0] && found[1], "Prediction scratch client prediction_scratch_bindings_sections are missing");
    auto* const entry = base + 0x346ac0;
    for (std::size_t index = 0; index < rebuild_bytes.size(); ++index)
        require(relocated(index) || std::uint8_t(entry[index]) == rebuild_bytes[index],
                "Prediction map rebuild instruction fingerprint differs");
    auto* const pool = base + 0x52448f8;
    require(read<void*>(entry, 0x1a) == pool && read<void*>(entry, 0x1f) == pool &&
                read<void*>(entry, 0xf6) == base + 0x3467f0,
            "Prediction map rebuild operands lack client provenance");
    data.native_cell<void*>(0x4346D658u) = pool;
    data.native_cell<void*>(0x4346D65Cu) = entry;
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_runtime_type_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
constexpr std::array<BuildRuntimeTypeRecipe, 15> build_runtime_type_recipes{
    {{BuildRuntimeType::trace_filter_simple, 0x0FC0u, 0xF17DA9A2u, 0x434758B4u, 0u,
      ".?AVCTraceFilterSimple@@"},
     {BuildRuntimeType::trace_filter_skip_two_entities, 0x0FCCu, 0x0A3AB8DEu, 0x43475B90u, 0u,
      ".?AVCTraceFilterSkipTwoEntities@@"},
     {BuildRuntimeType::base_player_animation_state, 0x0FD8u, 0x73D50E69u, 0x4346A460u, 0u,
      ".?AVCBasePlayerAnimState@@"},
     {BuildRuntimeType::game_rules_proxy, 0x0FE4u, 0x479D7323u, 0x4346A720u, 0u,
      ".?AVC_CSGameRulesProxy@@"},
     {BuildRuntimeType::cs_ragdoll, 0x0FF0u, 0x17843051u, 0x43467DC0u, 0u, ".?AVC_CSRagdoll@@"},
     {BuildRuntimeType::cs_ragdoll_renderable, 0x0FFCu, 0x17843051u, 0x43467190u, 4u,
      ".?AVC_CSRagdoll@@"},
     {BuildRuntimeType::econ_item_attribute, 0x1008u, 0x7E00C86Du, 0x4346848Cu, 0u,
      ".?AVCEconItemAttribute@@"},
     {BuildRuntimeType::viewmodel_attachment_renderable, 0x1014u, 0x0C3B7DAEu, 0x4346A81Cu, 4u,
      ".?AVC_ViewmodelAttachmentModel@@"},
     {BuildRuntimeType::predicted_viewmodel_renderable, 0x1020u, 0x17CEECD0u, 0x43468D28u, 4u,
      ".?AVC_PredictedViewModel@@"},
     {BuildRuntimeType::weapon_world_model_renderable, 0x102Cu, 0x24C25B1Au, 0x4346A444u, 4u,
      ".?AVC_BaseWeaponWorldModel@@"},
     {BuildRuntimeType::econ_wearable_renderable, 0x1038u, 0x0AE8CF3Cu, 0x43468908u, 4u,
      ".?AVC_EconWearable@@"},
     {BuildRuntimeType::cs_player, 0x1044u, 0x1B1A0D29u, 0x43467D54u, 0u, ".?AVC_CSPlayer@@"},
     {BuildRuntimeType::base_cs_grenade, 0x1050u, 0x581536C7u, 0x4346A43Cu, 0u,
      ".?AVC_BaseCSGrenade@@"},
     {BuildRuntimeType::player_addon_model_renderable, 0x105Cu, 0xFB0368F4u, 0x43469284u, 4u,
      ".?AVC_PlayerAddonModel@@"},
     {BuildRuntimeType::weapon_cs_base, 0x1068u, 0x05F64981u, 0x43467F84u, 0u,
      ".?AVC_WeaponCSBase@@"}}};
/* [dedup read/write] */
struct runtime_type_bindings_Section {
    std::uint32_t rva, size, flags;
};
constexpr runtime_type_bindings_Section text_section{0x1000u, 0xB38409u, 0x60000020u};
constexpr runtime_type_bindings_Section rdata_section{0xB3A000u, 0x26629Cu, 0x40000040u};
constexpr runtime_type_bindings_Section data_section{0xDA1000u, 0x460C3E0u, 0xC0000040u};
struct runtime_type_bindings_ClientImage {
    std::byte* base;
    bool runtime_type_bindings_within(const void* pointer, std::size_t size, const runtime_type_bindings_Section& section) const noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto first = reinterpret_cast<std::uintptr_t>(base) + section.rva;
        return address >= first && size <= section.size && address - first <= section.size - size;
    }
};
runtime_type_bindings_ClientImage runtime_type_bindings_checked_image(std::byte* module, NativeImageEnvironment images) {
    if (!module || !images.image_nt_header)
        throw std::invalid_argument(
            "Runtime RTTI requires the published client module and native PE-header import");
    const auto* nt = images.image_nt_header(module);
    if (!nt || nt != module + 0x150 || read<std::uint16_t>(module) != 0x5A4Du ||
        read<std::uint32_t>(module + 0x3C) != 0x150u || read<std::uint32_t>(nt) != 0x4550u ||
        read<std::uint16_t>(nt + 4) != 0x14Cu || read<std::uint16_t>(nt + 6) != 7u ||
        read<std::uint16_t>(nt + 0x14) != 0xE0u || read<std::uint16_t>(nt + 0x18) != 0x10Bu ||
        read<std::uint32_t>(nt + 8) != 0x65272025u ||
        read<std::uint32_t>(nt + 0x50) != 0x5509000u || read<std::uint32_t>(nt + 0x58) != 0xFA635Fu)
        throw std::runtime_error("Runtime RTTI requires the supported client PE32 build");
    constexpr std::array<const char*, 3> names{".text", ".rdata", ".data"};
    constexpr std::array<runtime_type_bindings_Section, 3> runtime_type_expected{text_section, rdata_section, data_section};
    std::array<bool, 3> found{};
    for (std::size_t index = 0; index < 7; ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        const auto rva = read<std::uint32_t>(section + 12), size = read<std::uint32_t>(section + 8);
        if (rva > 0x5509000u || size > 0x5509000u - rva)
            throw std::runtime_error("Runtime RTTI section exceeds its client image");
        for (std::size_t wanted = 0; wanted < names.size(); ++wanted) {
            std::array<char, 8> name{};
            std::memcpy(name.data(), names[wanted], std::strlen(names[wanted]));
            if (std::memcmp(section, name.data(), name.size()))
                continue;
            const auto& profile = runtime_type_expected[wanted];
            if (found[wanted] || rva != profile.rva || size != profile.size ||
                read<std::uint32_t>(section + 36) != profile.flags)
                throw std::runtime_error("Runtime RTTI client section fingerprint differs");
            found[wanted] = true;
        }
    }
    if (!found[0] || !found[1] || !found[2])
        throw std::runtime_error("Runtime RTTI client data sections are unavailable");
    return {module};
}
std::size_t index_of(BuildRuntimeType type) {
    const auto index = static_cast<std::size_t>(type);
    if (index >= build_runtime_type_recipes.size())
        throw std::out_of_range("Unknown build runtime RTTI identity");
    return index;
}
std::string runtime_type_bindings_failure_message(std::size_t index, const BuildRuntimeTypeRecipe& row,
                            BuildRuntimeTypeFailureReason reason) {
    std::ostringstream message;
    message << "Runtime RTTI " << index << " '" << row.decorated_name << "' (hash0x" << std::hex
            << row.type_hash << ", offset0x" << row.object_offset << ") ";
    switch (reason) {
    case BuildRuntimeTypeFailureReason::missing_type:
        message << "has no type descriptor";
        break;
    case BuildRuntimeTypeFailureReason::unexpected_type_name:
        message << "has an unexpected descriptor name";
        break;
    case BuildRuntimeTypeFailureReason::missing_vtable:
        message << "has no matching vtable";
        break;
    case BuildRuntimeTypeFailureReason::invalid_vtable:
        message << "has invalid table provenance";
        break;
    }
    return message.str();
}
}

std::span<const BuildRuntimeTypeRecipe, 15> build_runtime_type_manifest() noexcept {
    return build_runtime_type_recipes;
}
BuildRuntimeTypeFailure::BuildRuntimeTypeFailure(std::size_t failed_index,
                                                 const BuildRuntimeTypeRecipe& row,
                                                 BuildRuntimeTypeFailureReason classification)
    : std::runtime_error(runtime_type_bindings_failure_message(failed_index, row, classification)), index(failed_index),
      identity(row.identity), operand(row.operand), type_hash(row.type_hash),
      destination_cell(row.destination_cell), object_offset(row.object_offset),
      decorated_name(row.decorated_name), reason(classification) {}
BuildRuntimeTypeBindings::BuildRuntimeTypeBindings(NativeDataImage& data,
                                                   const std::array<RttiTypeDescriptor*, 15>& types)
    : data_(data), types_(types) {}
RttiTypeDescriptor* BuildRuntimeTypeBindings::type(BuildRuntimeType identity) const {
    return types_[index_of(identity)];
}
void** BuildRuntimeTypeBindings::vtable(BuildRuntimeType identity) const {
    return data_.native_cell<void**>(build_runtime_type_recipes[index_of(identity)].destination_cell);
}
BuildRuntimeTypeBindings initialize_build_runtime_type_bindings(NativeDataImage& data,
                                                                NativeImageEnvironment images) {
    std::array<void***, build_runtime_type_recipes.size()> destinations;
    for (std::size_t index = 0; index < build_runtime_type_recipes.size(); ++index) {
        const auto& row = build_runtime_type_recipes[index];
        if (index_of(row.identity) != index || row.operand != 0xFC0u + index * 12u ||
            netvar_name_hash(row.decorated_name) != row.type_hash)
            throw std::logic_error("Runtime RTTI manifest identity or hash differs");
        destinations[index] = &data.native_cell<void**>(row.destination_cell);
    }
    const auto image = runtime_type_bindings_checked_image(data.native_cell<std::byte*>(0x4346A55Cu), images);
    std::array<RttiTypeDescriptor*, build_runtime_type_recipes.size()> types;
    for (std::size_t index = 0; index < build_runtime_type_recipes.size(); ++index) {
        const auto& row = build_runtime_type_recipes[index];
        types[index] = find_rtti_type(image.base, row.type_hash, images);
        if (!types[index])
            throw BuildRuntimeTypeFailure(index, row, BuildRuntimeTypeFailureReason::missing_type);
        const auto length = std::strlen(row.decorated_name) + 1u;
        if (!image.runtime_type_bindings_within(types[index], offsetof(RttiTypeDescriptor, name) + length,
                          data_section) ||
            std::memcmp(types[index]->name, row.decorated_name, length))
            throw BuildRuntimeTypeFailure(index, row,
                                          BuildRuntimeTypeFailureReason::unexpected_type_name);
        auto** table =
            find_rtti_vtable_at_offset(image.base, types[index], row.object_offset, images);
        if (!table)
            throw BuildRuntimeTypeFailure(index, row,
                                          BuildRuntimeTypeFailureReason::missing_vtable);
        if (!image.runtime_type_bindings_within(table, sizeof(void*), rdata_section) ||
            !image.runtime_type_bindings_within(*table, 1u, text_section))
            throw BuildRuntimeTypeFailure(index, row,
                                          BuildRuntimeTypeFailureReason::invalid_vtable);
        *destinations[index] = table;
    }
    return {data, types};
}
BuildRuntimeTypeBindings initialize_build_runtime_type_bindings(NativeDataImage& data) {
    return initialize_build_runtime_type_bindings(data, native_image_environment());
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_scene_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
constexpr std::array<BuildSceneClassRecipe, 20> classes{
    {{0x390u, 0x009E3601u, 0x4346A440u, "CBreakableSurface"},
     {0x398u, 0x0F0EDE49u, 0x43467478u, "CBaseEntity"},
     {0x3A0u, 0x8CBFEC01u, 0x43467E68u, "CCascadeLight"},
     {0x3A8u, 0xF89B2C53u, 0x43468CA0u, "CDynamicProp"},
     {0x3B0u, 0x52563AF3u, 0x43468E30u, "CBaseCSGrenadeProjectile"},
     {0x3B8u, 0x144AF01Cu, 0x43468DF8u, "CBaseCSGrenade"},
     {0x3C0u, 0xFD66DD64u, 0x434682ACu, "CWeaponCSBase"},
     {0x3C8u, 0x30F3B77Eu, 0x43467E88u, "CMolotovProjectile"},
     {0x3D0u, 0x392B5938u, 0x434674E8u, "CInferno"},
     {0x3D8u, 0x1DA776D4u, 0x4346A348u, "CParticleSystem"},
     {0x3E0u, 0xAF29E891u, 0x43467008u, "CPredictedViewModel"},
     {0x3E8u, 0xAEEB25F3u, 0x4346A7D8u, "CBaseWeaponWorldModel"},
     {0x3F0u, 0xB79BF5CAu, 0x43467C44u, "CKnife"},
     {0x3F8u, 0xD5A32634u, 0x43467DBCu, "CBaseAnimating"},
     {0x400u, 0xB66F52E5u, 0x434684C4u, "CSun"},
     {0x408u, 0xDBD65E05u, 0x4346841Cu, "CEconWearable"},
     {0x410u, 0xB4890975u, 0x4346A5A0u, "CFuncBrush"},
     {0x418u, 0x71D79C49u, 0x43468238u, "CSmokeGrenadeProjectile"},
     {0x420u, 0xD2B93652u, 0x434685FCu, "CBaseAnimatingOverlay"},
     {0x428u, 0x4A945FC9u, 0x4346719Cu, "CFuncLadder"}}};
/* [dedup read/write] */
/* [dedup require] */
struct scene_bindings_Section {
    char name[8];
    std::uint32_t rva, size, flags;
};
constexpr std::array<scene_bindings_Section, 3> scene_bindings_sections{{{".text", 0x1000, 0xB38409, 0x60000020},
                                           {".rdata", 0xB3A000, 0x26629C, 0x40000040},
                                           {".data", 0xDA1000, 0x460C3E0, 0xC0000040}}};
struct scene_bindings_Image {
    std::byte* base;
    bool scene_bindings_within(const void* pointer, std::size_t size, std::size_t section) const noexcept {
        const auto& region = scene_bindings_sections[section];
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto first = reinterpret_cast<std::uintptr_t>(base) + region.rva;
        return address >= first && size <= region.size && address - first <= region.size - size;
    }
};
scene_bindings_Image scene_bindings_checked_image(std::byte* module) {
    require(module && read<std::uint16_t>(module) == 0x5A4D &&
                read<std::uint32_t>(module + 0x3C) == 0x150,
            "Scene bindings require the supported loaded client image");
    const auto* nt = module + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 0x14) == 0xE0 &&
                read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x65272025 &&
                read<std::uint32_t>(nt + 0x50) == 0x5509000 &&
                read<std::uint32_t>(nt + 0x58) == 0xFA635F,
            "Scene client PE32 build identity differs");
    std::array<bool, 3> found{};
    for (std::size_t i = 0; i < 7; ++i) {
        const auto* section = nt + 0xF8 + i * 40;
        const auto rva = read<std::uint32_t>(section + 12), size = read<std::uint32_t>(section + 8);
        require(rva <= 0x5509000 && size <= 0x5509000 - rva,
                "Scene client section exceeds its image");
        for (std::size_t j = 0; j < scene_bindings_sections.size(); ++j) {
            const auto& expected = scene_bindings_sections[j];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[j] && rva == expected.rva && size == expected.size &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Scene client section fingerprint differs");
            found[j] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Required scene client section is missing");
    return {module};
}
/* [dedup matches] */
void scene_bindings_expect(const scene_bindings_Image& image, const std::byte* code, std::initializer_list<int> pattern) {
    require(image.scene_bindings_within(code, pattern.size(), 0) && matches(code, pattern),
            "Scene native instruction fingerprint differs");
}
const std::byte* scene_bindings_unique(const scene_bindings_Image& image, std::initializer_list<int> pattern, std::uint32_t rva) {
    const std::byte* found = nullptr;
    for (std::size_t i = 0; i <= scene_bindings_sections[0].size - pattern.size(); ++i) {
        const auto* code = image.base + scene_bindings_sections[0].rva + i;
        if (!matches(code, pattern))
            continue;
        require(!found, "Scene native code anchor is ambiguous");
        found = code;
    }
    require(found == image.base + rva, "Scene native code anchor is missing or unsupported");
    return found;
}
const std::byte* scene_bindings_absolute(const scene_bindings_Image& image, const std::byte* operand, std::size_t size,
                          std::size_t section) {
    const auto* pointer =
        reinterpret_cast<const std::byte*>(std::uintptr_t(read<std::uint32_t>(operand)));
    require(image.scene_bindings_within(pointer, size, section),
            "Decoded scene pointer has invalid section provenance");
    return pointer;
}
void string(const scene_bindings_Image& image, const std::byte* pointer, const char* expected,
            std::size_t section) {
    const auto size = std::strlen(expected) + 1;
    require(image.scene_bindings_within(pointer, size, section) && std::memcmp(pointer, expected, size) == 0,
            "Scene native name operand differs");
}
void rtti_operand(const scene_bindings_Image& image, const std::byte* operand, const char* expected) {
    const auto* type = scene_bindings_absolute(image, operand, 8 + std::strlen(expected) + 1, 2);
    string(image, type + 8, expected, 2);
}
const std::byte* call_target(const scene_bindings_Image& image, const std::byte* instruction, std::uint32_t rva) {
    scene_bindings_expect(image, instruction, {0xE8, -1, -1, -1, -1});
    const auto offset =
        std::int64_t(instruction - image.base) + 5 + read<std::int32_t>(instruction + 1);
    require(offset == rva && offset >= 0 && offset < 0x5509000,
            "Scene native helper call target differs");
    const auto* target = image.base + static_cast<std::size_t>(offset);
    require(image.scene_bindings_within(target, 1, 0), "Scene helper call leaves executable text");
    return target;
}
const std::byte* breakable_function(const scene_bindings_Image& image) {
    const auto* function =
        scene_bindings_unique(image, {0x55, 0x8B, 0xEC, 0x51, 0x56, 0x8B, 0xF1, 0x85, 0xF6, 0x74, 0x68, 0x83,
                       0xBE, 0,    0x01, 0,    0,    0,    0x7D, 0x10, 0x8B, 0x06, 0xFF, 0x90,
                       0xEC, 0x01, 0,    0,    0x85, 0xC0, 0x0F, 0x8F, 0xE2, 0,    0,    0,
                       0x80, 0xBE, 0x80, 0x02, 0,    0,    0x02, 0x75, 0x46},
               0x320BD0);
    scene_bindings_expect(image, function + 0x4D, {0x6A, 0, 0x68, -1,   -1, -1, -1, 0x68, -1,   -1,   -1,  -1,
                                    0x6A, 0, 0x56, 0xE8, -1, -1, -1, -1,   0x83, 0xC4, 0x14});
    rtti_operand(image, function + 0x50, ".?AVIMultiplayerPhysics@@");
    rtti_operand(image, function + 0x55, ".?AVC_BaseEntity@@");
    call_target(image, function + 0x5C, 0xAC99C5);
    scene_bindings_expect(image, function + 0x7A,
           {0xBA, -1, -1, -1, -1, 0x8B, 0xCE, 0xE8, -1, -1, -1, -1, 0x84, 0xC0, 0x75, 0x28});
    scene_bindings_expect(image, function + 0x8A,
           {0xBA, -1, -1, -1, -1, 0x8B, 0xCE, 0xE8, -1, -1, -1, -1, 0x84, 0xC0, 0x75, 0x18});
    string(image, scene_bindings_absolute(image, function + 0x7B, 15, 1), "func_breakable", 1);
    string(image, scene_bindings_absolute(image, function + 0x8B, 20, 1), "func_breakable_surf", 1);
    call_target(image, function + 0x81, 0x1A7480);
    call_target(image, function + 0x91, 0x1A7480);
    scene_bindings_expect(image, function + 0xD2, {0x6A, 0,  0x68, -1, -1,   -1,   -1, 0x68, -1, -1,
                                    -1,   -1, 0x6A, 0,  0x56, 0xE8, -1, -1,   -1, -1});
    rtti_operand(image, function + 0xD5, ".?AVIBreakableWithPropData@@");
    rtti_operand(image, function + 0xDA, ".?AVC_BaseEntity@@");
    call_target(image, function + 0xE1, 0xAC99C5);
    scene_bindings_expect(image, function + 0x106, {0xB0, 0x01, 0x5E, 0x8B, 0xE5, 0x5D, 0xC3});
    return function;
}
void validate_class_list(const scene_bindings_Image& image, void* base_client) {
    require(image.scene_bindings_within(base_client, sizeof(void*), 2),
            "Scene bindings require the actual VClient018 receiver");
    const auto* table = read<const std::byte*>(base_client);
    // A source hook may own the table allocation. Its unchanged virtual8 must
    // still be the verified genuine client GetAllClasses getter.
    const auto* getter = read<const std::byte*>(table + 8 * sizeof(void*));
    scene_bindings_expect(image, getter, {0xA1, -1, -1, -1, -1, 0xC3});
    require(getter == image.base + 0x260A50, "Scene GetAllClasses entry differs");
    auto* node = read<ClientClass*>(scene_bindings_absolute(image, getter + 1, 4, 2));
    for (std::size_t i = 0; i < 289; ++i) {
        require(image.scene_bindings_within(node, sizeof(ClientClass), 2),
                "Scene class list is incomplete or has invalid provenance");
        require((node->create || node->create_event) &&
                    (!node->create || image.scene_bindings_within(node->create, 1, 0)) &&
                    (!node->create_event || image.scene_bindings_within(node->create_event, 1, 0)) &&
                    image.scene_bindings_within(node->table, sizeof(RecvTable), 2) &&
                    image.scene_bindings_within(node->network_name, 1, 1),
                "Scene ClientClass fields have invalid provenance");
        const auto name_end = image.base + scene_bindings_sections[1].rva + scene_bindings_sections[1].size;
        require(std::memchr(node->network_name, 0,
                            static_cast<std::size_t>(name_end - reinterpret_cast<const std::byte*>(
                                                                    node->network_name))) !=
                    nullptr,
                "Scene ClientClass network name is not terminated");
        node = node->next;
    }
    require(!node, "Scene class list count differs from the supported build");
}
}

std::span<const BuildSceneClassRecipe, 20> build_scene_class_manifest() noexcept {
    return classes;
}
BuildSceneClassFailure::BuildSceneClassFailure(std::size_t failed, const BuildSceneClassRecipe& row)
    : std::runtime_error("ClientClass startup could not resolve " + std::string(row.network_name)),
      index(failed), operand(row.operand), class_hash(row.class_hash),
      destination_cell(row.destination_cell), network_name(row.network_name) {}
void initialize_build_scene_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4 && data.size() >= NativeDataImage::image_size,
            "Scene bindings require the complete canonical x86 data image");
    const auto image = scene_bindings_checked_image(data.native_cell<std::byte*>(0x4346A55Cu));
    auto* const base_client = data.native_cell<void*>(0x434683E0u);
    validate_class_list(image, base_client);
    for (std::size_t i = 0; i < classes.size(); ++i) {
        const auto& row = classes[i];
        require(row.operand == 0x390u + i * 8u &&
                    netvar_name_hash(row.network_name) == row.class_hash,
                "Scene class recipe identity differs");
        auto* const found = find_client_class(base_client, row.class_hash);
        if (!found)
            throw BuildSceneClassFailure(i, row);
        require(std::strcmp(found->network_name, row.network_name) == 0,
                "Scene ClientClass hash collision differs from the requested name");
        data.native_cell<ClientClass*>(row.destination_cell) = found;
    }
    const auto* breakable = breakable_function(image);
    data.native_cell<EngineBreakableFunction>(0x43468F30u) =
        reinterpret_cast<EngineBreakableFunction>(const_cast<std::byte*>(breakable));
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_script_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct script_bindings_Image {
    std::byte* base;
    std::uint32_t text_size, rdata_begin, rdata_size, data_begin, data_size;
    bool contains(std::uint32_t address, std::size_t size, std::uint32_t begin,
                  std::uint32_t length) const {
        const auto first = reinterpret_cast<std::uintptr_t>(base) + begin;
        return address >= first && size <= length && address - first <= length - size;
    }
    void code(std::uint32_t rva, std::initializer_list<int> pattern) const {
        require(rva >= 0x1000u && pattern.size() <= text_size &&
                    rva - 0x1000u <= text_size - pattern.size(),
                "Script binding code exceeds its module text");
        std::size_t index = 0;
        for (const auto value : pattern) {
            require(value < 0 || static_cast<unsigned char>(base[rva + index]) == value,
                    "Script binding instruction fingerprint differs");
            ++index;
        }
    }
    std::byte* operand(std::uint32_t rva, std::uint32_t target) const {
        require(read<std::uint32_t>(base + rva) == reinterpret_cast<std::uintptr_t>(base + target),
                "Script binding operand has different module provenance");
        return base + target;
    }
};
script_bindings_Image script_bindings_checked_image(void* module, bool client) {
    require(module != nullptr, "Script bindings require the initialized client/engine modules");
    auto* const base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == 0x150,
            "Script binding module DOS/NT-header layout differs");
    const auto* nt = base + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 7 && read<std::uint16_t>(nt + 20) == 0xE0 &&
                read<std::uint16_t>(nt + 24) == 0x10B &&
                read<std::uint32_t>(nt + 8) == (client ? 0x65272025u : 0x64EE7E86u) &&
                read<std::uint32_t>(nt + 80) == (client ? 0x5509000u : 0x39C0000u) &&
                read<std::uint32_t>(nt + 88) == (client ? 0xFA635Fu : 0x64174Eu),
            "Script binding module build identity differs");
    const script_bindings_Image image{base,
                      client ? 0xB38409u : 0x451C32u,
                      client ? 0xB3A000u : 0x453000u,
                      client ? 0x26629Cu : 0xD4766u,
                      client ? 0xDA1000u : 0x528000u,
                      client ? 0x460C3E0u : 0x3427638u};
    struct script_bindings_Section {
        char name[8];
        std::uint32_t rva, size, flags;
    };
    const std::array<script_bindings_Section, 3> expected{
        {{".text", 0x1000, image.text_size, 0x60000020},
         {".rdata", image.rdata_begin, image.rdata_size, 0x40000040},
         {".data", image.data_begin, image.data_size, 0xC0000040}}};
    for (const auto& wanted : expected) {
        unsigned matches = 0;
        for (unsigned index = 0; index < 7; ++index) {
            const auto* section = nt + 0xF8 + index * 40;
            if (std::memcmp(section, wanted.name, 8))
                continue;
            require(read<std::uint32_t>(section + 12) == wanted.rva &&
                        read<std::uint32_t>(section + 8) == wanted.size &&
                        read<std::uint32_t>(section + 36) == wanted.flags,
                    "Script binding module section differs");
            ++matches;
        }
        require(matches == 1, "Script binding module section is missing or duplicated");
    }
    return image;
}
}
BuildScriptBindings initialize_build_script_bindings(NativeDataImage& data) {
    require(sizeof(void*) == 4, "Script module bindings require the original x86 ABI");
    const auto engine = script_bindings_checked_image(data.native_cell<void*>(0x43468D20u), false);
    const auto client = script_bindings_checked_image(data.native_cell<void*>(0x4346A55Cu), true);
    // GetCommandAck reads the same raw split-screen record used by the other
    // client-state aliases. The public current-state pointer is raw+8.
    engine.code(0xB6830, {0xA1, -1, -1, -1, -1, 0x8B, 0x80, 0x3C, 0x4D, 0, 0, 0xC3});
    const auto record = read<std::uint32_t>(engine.operand(0xB6831, 0x59F19C));
    require(record && !(record & 3u) &&
                record <= std::numeric_limits<std::uint32_t>::max() - 0x4D40u &&
                data.native_cell<void*>(0x43468E44u) ==
                    reinterpret_cast<void*>(std::uintptr_t(record + 8u)),
            "Script globals do not share the canonical current client state");
    engine.code(0x286110, {0x55, 0x8B, 0xEC, 0x8B, 0x45, 8,    0x89, 0x81, 0x14, 1,    0, 0, 0x8B,
                           0x45, 0x0C, 0x89, 0x81, 0x34, 0x4D, 0,    0,    0x5D, 0xC2, 8, 0});
    engine.code(0x286146, {0x8B, 0x86, 0x34, 0x4D, 0,    0,    0x8B, 0xD0, 0x2B, 0x96,
                           0x2C, 0x4D, 0,    0,    0x89, 0x8E, 0x30, 0x4D, 0,    0});
    engine.code(0xD779C, {0x8B, 0x86, 0x3C, 0x4D, 0,    0,    0xFF, 0xB6, 0x1C, 1,    0,    0,
                          0x8B, 0x11, 0x2B, 0x86, 0x34, 0x4D, 0,    0,    0x50, 0xFF, 0x52, 0x10});

    // The inline CClassMap object is initialized by this actual constructor.
    // Its fourth virtual compares registered class names and returns size.
    client.code(0x26ACF0, {0xC7, 5, -1, -1, -1, -1,   -1, -1, -1, -1, 0xB8, -1, -1, -1, -1,
                           0xC7, 5, -1, -1, -1, -1,   0,  0,  0,  0,  0xC7, 5,  -1, -1, -1,
                           -1,   0, 0,  0,  0,  0xC7, 5,  -1, -1, -1, -1,   0,  0,  0,  0});
    auto* const classes = client.operand(0x26ACF2, 0x535E4CC);
    auto* const table = client.operand(0x26ACF6, 0xBC162C);
    client.operand(0x26ACFB, 0x535E4CC);
    client.operand(0x26AD01, 0x535E4D4);
    client.operand(0x26AD0B, 0x535E4D8);
    client.operand(0x26AD15, 0x535E4DC);
    require(read<std::uint32_t>(classes) == reinterpret_cast<std::uintptr_t>(table) &&
                client.contains(reinterpret_cast<std::uint32_t>(table), 5 * sizeof(void*),
                                client.rdata_begin, client.rdata_size),
            "Script class-size object has a different live vtable");
    client.operand(0xBC163C, 0x26AF90);
    client.code(0x26AF90, {0x55, 0x8B, 0xEC, 0x51, 0x53, 0x0F, 0xB7, 0x59, 0x16, 0x56, 0x33, 0xF6,
                           0x57, 0x85, 0xDB, 0x7E, 0x51, 0x8B, 0x51, 8,    0x89, 0x55, 0xFC});
    client.code(0x26AFF2, {0x5F, 0x5E, 0x83, 0xC8, 0xFF, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 4, 0,
                           0x8B, 0x47, 4,    0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 4, 0});
    client.operand(0xBC1628, 0xD53B3C);
    const auto* const locator = client.base + 0xD53B3C;
    require(read<std::uint32_t>(locator) == 0 && read<std::uint32_t>(locator + 4) == 0 &&
                read<std::uint32_t>(locator + 8) == 0,
            "Script class-map RTTI primary locator differs");
    auto* const type = client.operand(0xD53B48, 0xE1EBD8);
    require(std::memcmp(type + 8, ".?AVCClassMap@@", 16) == 0,
            "Script class-map RTTI name differs");
    const BuildScriptBindings result{
        data, reinterpret_cast<const std::int32_t*>(std::uintptr_t(record + 0x4D34u)),
        reinterpret_cast<const std::int32_t*>(std::uintptr_t(record + 0x4D3Cu)), classes};
    data.native_cell<const std::int32_t*>(0x43475794u) = result.old_command_ack;
    data.native_cell<const std::int32_t*>(0x43475BA4u) = result.command_ack;
    data.native_cell<void*>(0x43469304u) = result.class_size_interface;
    return result;
}
}

// ============================================================================
// Subcomponent: build_shared_esp_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct shared_esp_bindings_Region {
    std::uint32_t rva, size, flags;
};
constexpr shared_esp_bindings_Region shared_esp_text{0x1000, 0x451C32, 0x60000020}, shared_esp_rdata{0x453000, 0xD4766, 0x40000040},
    shared_esp_objects{0x528000, 0x3427638, 0xC0000040};
struct Engine {
    std::byte* base;
    bool contains(const void* pointer, std::size_t size, shared_esp_bindings_Region region) const noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer),
                   first = reinterpret_cast<std::uintptr_t>(base) + region.rva;
        return address >= first && size <= region.size && address - first <= region.size - size;
    }
    void shared_esp_bindings_expect(const std::byte* code, std::initializer_list<int> pattern) const {
        require(contains(code, pattern.size(), shared_esp_text),
                "Shared ESP instruction is outside genuine engine shared_esp_text");
        std::size_t index = 0;
        for (const auto value : pattern) {
            require(value < 0 || static_cast<unsigned char>(code[index]) == value,
                    "Shared ESP engine instruction fingerprint differs");
            ++index;
        }
    }
    const std::byte* shared_esp_bindings_unique(std::initializer_list<int> pattern) const {
        const std::byte* result = nullptr;
        for (std::uint32_t offset = 0; offset <= shared_esp_text.size - pattern.size(); ++offset) {
            const auto* code = base + shared_esp_text.rva + offset;
            bool match = true;
            std::size_t index = 0;
            for (const auto value : pattern) {
                if (value >= 0 && static_cast<unsigned char>(code[index]) != value) {
                    match = false;
                    break;
                }
                ++index;
            }
            if (!match)
                continue;
            require(!result, "Shared ESP engine CALL anchor is ambiguous");
            result = code;
        }
        require(result != nullptr, "Shared ESP engine CALL anchor is missing");
        return result;
    }
    const std::byte* call(const std::byte* instruction) const {
        shared_esp_bindings_expect(instruction, {0xE8, -1, -1, -1, -1});
        const auto* result = instruction + 5 + read<std::int32_t>(instruction + 1);
        require(contains(result, 1, shared_esp_text), "Shared ESP CALL target leaves genuine engine shared_esp_text");
        return result;
    }
};
Engine shared_esp_bindings_checked(NativeDataImage& data, NativeImageEnvironment images) {
    auto* module = data.native_cell<std::byte*>(0x43468D20);
    require(module && images.image_nt_header,
            "Shared ESP requires the loaded engine and PE-header service");
    const auto* nt = images.image_nt_header(module);
    require(nt == module + 0x150 && read<std::uint16_t>(module) == 0x5A4D &&
                read<std::uint32_t>(module + 0x3C) == 0x150 && read<std::uint32_t>(nt) == 0x4550 &&
                read<std::uint16_t>(nt + 4) == 0x14C && read<std::uint16_t>(nt + 6) == 7 &&
                read<std::uint16_t>(nt + 20) == 0xE0 && read<std::uint16_t>(nt + 24) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x64EE7E86 &&
                read<std::uint32_t>(nt + 80) == 0x39C0000 &&
                read<std::uint32_t>(nt + 88) == 0x64174E,
            "Unsupported Shared ESP engine PE32 build");
    constexpr std::array<shared_esp_bindings_Region, 3> shared_esp_regions{shared_esp_text, shared_esp_rdata, shared_esp_objects};
    for (std::size_t index = 0; index < shared_esp_regions.size(); ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        require(read<std::uint32_t>(section + 12) == shared_esp_regions[index].rva &&
                    read<std::uint32_t>(section + 8) == shared_esp_regions[index].size &&
                    read<std::uint32_t>(section + 36) == shared_esp_regions[index].flags,
                "Shared ESP engine section geometry differs");
    }
    return {module};
}
void require_class(const Engine& engine, const std::byte* table, const char* name) {
    require(engine.contains(table - 4, 4, shared_esp_rdata) && engine.contains(table, 4, shared_esp_rdata),
            "Shared ESP engine message table is outside read-only data");
    const auto* locator = read<const std::byte*>(table - 4);
    require(engine.contains(locator, 20, shared_esp_rdata) && read<std::uint32_t>(locator) == 0 &&
                read<std::uint32_t>(locator + 4) == 0 && read<std::uint32_t>(locator + 8) == 0,
            "Shared ESP engine message RTTI locator is invalid");
    const auto* type = read<const std::byte*>(locator + 12);
    require(engine.contains(type, 8 + std::strlen(name) + 1, shared_esp_objects) &&
                !std::strcmp(reinterpret_cast<const char*>(type + 8), name),
            "Shared ESP engine message RTTI identity differs");
}
}
void initialize_build_shared_esp_bindings(NativeDataImage& data, NativeImageEnvironment images) {
    const auto engine = shared_esp_bindings_checked(data, images);
    const auto* message = engine.shared_esp_bindings_unique({0x8D, 0x4C, 0x24, 0x18, 0xE8, -1, -1, -1, -1,
                                         0x56, 0x8D, 0x84, 0x24, 0xCC, 1,  0,  0,  0x50,
                                         0x8D, 0x4C, 0x24, 0x28, 0xE8, -1, -1, -1, -1}) +
                          4;
    const auto* construct = engine.call(message);
    const auto* destroy = engine.call(message + 0xB2);
    engine.shared_esp_bindings_expect(construct,
                  {0x56, 0x57, 0x8B, 0xF9, 0x8D, 0x4F, 8,    0xC7, 7,    -1,   -1,   -1,
                   -1,   0xE8, -1,   -1,   -1,   -1,   0xC7, 7,    -1,   -1,   -1,   -1,
                   0x8B, 0xC7, 0xC7, 0x47, 8,    -1,   -1,   -1,   -1,   0xC6, 0x47, 0x38,
                   0,    0xC7, 0x47, 0x50, 0x0F, 0,    0,    0,    0xC7, 0x47, 0x4C, 0,
                   0,    0,    0,    0xC6, 0x47, 0x3C, 0,    0x5F, 0x5E, 0xC3});
    engine.shared_esp_bindings_expect(destroy, {0x53, 0x8B, 0xD9, 0x56, 0x8D, 0x73, 0x3C, 0x57, 0xC7, 3, -1,
                            -1,   -1,   -1,   0xC7, 0x43, 8,    -1,   -1,   -1,   -1});
    engine.shared_esp_bindings_expect(destroy + 0x72, {0xC7, 3, -1, -1, -1, -1, 0x5B, 0xC3});
    const auto* voice_table = read<const std::byte*>(construct + 0x14);
    require(voice_table == read<const std::byte*>(destroy + 0xA) &&
                read<const std::byte*>(construct + 0x1D) == read<const std::byte*>(destroy + 0x11),
            "Shared ESP voice constructor/destructor tables disagree");
    require_class(engine, voice_table, ".?AV?$CNetMessagePB@$09VCCLCMsg_VoiceData@@$08$0A@@@");
    (void)engine.call(construct + 0xD);

    const auto* receiver = engine.shared_esp_bindings_unique({0x8D, 0x4D, 0xCC, 0xE8, -1,   -1, -1,   -1,   0x8B,
                                          0xD6, 0x8D, 0x4D, 0xD0, 0xE8, -1, -1,   -1,   -1,
                                          0x8B, 0x8F, 0x94, 0,    0,    0,  0x8D, 0x55, 0xCC}) +
                           3;
    construct = engine.call(receiver);
    destroy = engine.call(receiver + 0x28);
    engine.shared_esp_bindings_expect(construct,
                  {0x56, 0x8B, 0xF1, 0xC7, 6,    -1,   -1,   -1,   -1,   0x8D, 0x4E, 8,
                   0xC7, 0x46, 4,    -1,   -1,   -1,   -1,   0xE8, -1,   -1,   -1,   -1,
                   0xC7, 0x46, 0x10, 0,    0,    0,    0,    0x8B, 0xC6, 0xC7, 0x46, 0x0C,
                   -1,   -1,   -1,   -1,   0xC7, 0x46, 0x14, 0,    0,    0,    0,    0xC7,
                   6,    -1,   -1,   -1,   -1,   0xC7, 0x46, 4,    -1,   -1,   -1,   -1,
                   0xC6, 0x46, 0x18, 1,    0xC7, 0x46, 0x30, 0x0F, 0,    0,    0,    0xC7,
                   0x46, 0x2C, 0,    0,    0,    0,    0xC6, 0x46, 0x1C, 0,    0x5E, 0xC3});
    engine.shared_esp_bindings_expect(destroy, {0x53, 0x8B, 0xD9, 0x56, 0x8D, 0x73, 0x1C, 0x57, 0xC7, 3, -1,
                            -1,   -1,   -1,   0xC7, 0x43, 4,    -1,   -1,   -1,   -1});
    engine.shared_esp_bindings_expect(destroy + 0x72, {0xC7, 3, -1, -1, -1, -1, 0x5B, 0xC3});
    const auto* key_values_table = read<const std::byte*>(construct + 0x31);
    require(key_values_table == read<const std::byte*>(destroy + 0xA) &&
                read<const std::byte*>(construct + 0x38) == read<const std::byte*>(destroy + 0x11),
            "Shared ESP key-values constructor/destructor tables disagree");
    require_class(engine, key_values_table,
                  ".?AV?$CNetMessagePB@$0BC@VCCLCMsg_CmdKeyValues@@$0A@$00@@");
    (void)engine.call(construct + 0x13);
    // The complete feature constructor consumes and clears these CALL cells
    // before deriving its four genuine engine functions in native order.
    data.native_cell<const std::byte*>(0x4346D668) = message;
    data.native_cell<const std::byte*>(0x4346D66C) = receiver;
}
void initialize_build_shared_esp_bindings(NativeDataImage& data) {
    initialize_build_shared_esp_bindings(data, native_image_environment());
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_skin_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
struct CodeRecipe {
    std::uint32_t cell, rva;
    std::array<std::int16_t, 64> pattern;
};
constexpr std::array<CodeRecipe, 7> skin_functions{{
    {0x43467040u, 0x70ADB0u, {{0x55, 0x8B, 0xEC, 0x53, 0x56, 0x57, 0x8B, 0xF9, 0x8B, 0x87, 0x98,
                               0x02, 0x00, 0x00, 0x83, 0xF8, 0xFF, 0x74, 0x28, 0x8B, 0x9F, 0x8C,
                               0x02, 0x00, 0x00, 0x8B, 0x75, 0x08, 0x0F, 0x1F, 0x40, 0x00, 0x8D,
                               0x0C, 0x40, 0x8B, 0x54, 0xCB, 0x10, 0x3B, 0xD6, 0x7E, 0x05, 0x8B,
                               0x04, 0xCB, 0xEB, 0x06, 0x7D, 0x09, 0x8B, 0x44, 0xCB, 0x04, 0x83,
                               0xF8, 0xFF, 0x75, 0xE5, 0x85, 0xC0, 0x78, 0x43, 0x3B}}},
    {0x4346A884u, 0x2A78A0u, {{0x55, 0x8B, 0xEC, 0x53, 0x8B, 0x5D, 0x08, 0x8B, 0xC3, 0x56, 0x57,
                               0x8B, 0xF9, 0xC1, 0xE0, 0x04, 0x8D, 0x77, 0x04, 0x03, 0xF0, 0x8B,
                               0x0E, 0x85, 0xC9, 0x0F, 0x84, 0x81, 0x00, 0x00, 0x00, 0xC7, 0x45,
                               0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x55, 0x08, 0x8B, 0x01, 0x52,
                               0xFF, 0x50, 0x04, 0x8B, 0x46, 0x04, 0x51, 0x8B, 0xCC, 0xC1, 0xE0,
                               0x10, 0x0B, 0xC3, 0x89, 0x01, 0x8B, 0xCF, 0x8B, 0x07}}},
    {0x43468E88u, 0x2A77E0u, {{0x55, 0x8B, 0xEC, 0x51, 0x53, 0x8B, 0x99, 0x0C, 0x00, 0x02, 0x00,
                               0x89, 0x4D, 0xFC, 0x56, 0x57, 0x85, 0xDB, 0x75, 0x20, 0x68, -1,
                               -1,   -1,   -1,   0xFF, 0x15, -1,   -1,   -1,   -1,   0x8B, 0x45,
                               0x08, 0x83, 0xC4, 0x04, 0xC7, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x5F,
                               0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 0x08, 0x00, 0x8B, 0x53, 0x08,
                               0x3B, 0xD3, 0x74, 0x2D, 0x8B, 0x43, 0x0C, 0x85, 0xD2}}},
    {0x4346D644u, 0x1E8100u, {{0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x28, 0x56, 0x57,
                               0x8B, 0x7D, 0x08, 0x8B, 0xF1, 0x85, 0xFF, 0x74, 0x0B, 0x8B, 0x07,
                               0x8B, 0xCF, 0xFF, 0x50, 0x08, 0x8B, 0x00, 0xEB, 0x03, 0x83, 0xC8,
                               0xFF, 0x3B, 0x86, 0x08, 0x03, 0x00, 0x00, 0x0F, 0x84, 0x40, 0x01,
                               0x00, 0x00, 0x8B, 0x06, 0x8B, 0xCE, 0xFF, 0x50, 0x28, 0x8B, 0xCE,
                               0xF3, 0x0F, 0x7E, 0x00, 0x8B, 0x40, 0x08, 0x89, 0x44}}},
    {0x434688A4u, 0x215080u, {{0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x83, 0xEC, 0x2C, 0x53, 0x8B,
                               0xD9, 0x56, 0x57, 0x8B, 0x03, 0xFF, 0x90, 0xF0, 0x03, 0x00, 0x00,
                               0x8B, 0xF8, 0x89, 0x7C, 0x24, 0x0C, 0x85, 0xFF, 0x0F, 0x84, 0xA7,
                               0x03, 0x00, 0x00, 0x8B, 0x17, 0x8B, 0xCF, 0x8B, 0x92, 0x78, 0x02,
                               0x00, 0x00, 0xFF, 0xD2, 0x84, 0xC0, 0x0F, 0x84, 0x93, 0x03, 0x00,
                               0x00, 0x8B, 0x07, 0x8B, 0xCF, 0xFF, 0x90, 0x30, 0x04}}},
    {0x4346D64Cu, 0x4364D0u, {{0x53, 0x8B, 0xD9, 0x56, 0x57, 0x85, 0xDB, 0x74, 0x26, 0x33, 0xFF,
                               0x33, 0xF6, 0x0F, 0x1F, 0x00, 0xFF, 0xB6, -1,   -1,   -1,   -1,
                               0x53, 0xE8, 0xC4, 0x04, 0x56, 0x00, 0x83, 0xC4, 0x08, 0x85, 0xC0,
                               0x75, 0x15, 0x83, 0xC6, 0x14, 0x47, 0x81, 0xFE, 0x04, 0x06, 0x00,
                               0x00, 0x72, 0xE1, 0x5F, 0x5E, 0xB8, -1,   -1,   -1,   -1,   0x5B,
                               0xC3, 0x8D, 0x04, 0xBF, 0x5F, 0x5E, 0x8D, 0x04, 0x85}}},
    {0x4346A374u, 0x2D3A40u, {{0x55, 0x8B, 0xEC, 0x53, 0x8B, 0x5D, 0x08, 0x56, 0x57, 0x8B, 0xF9,
                               0x33, 0xF6, 0x39, 0x77, 0x28, 0x7E, 0x1E, 0x8B, 0x47, 0x1C, 0x8B,
                               0x0C, 0xB0, 0x8B, 0x01, 0xFF, 0x50, 0x30, 0x8B, 0xD3, 0x8B, 0xC8,
                               0xE8, 0x5A, 0x28, 0x6C, 0x00, 0x85, 0xC0, 0x74, 0x23, 0x46, 0x3B,
                               0x77, 0x28, 0x7C, 0xE2, 0x53, 0xFF, 0x77, 0x78, 0x68, -1,   -1,
                               -1,   -1,   0x6A, 0x01, 0xFF, 0x15, -1,   -1,   -1}}},
}};
constexpr std::array<CodeRecipe, 4> operands{{
    {0x43468C94u, 0x195F65u, {{0x8B, 0x3D, -1,   -1,   -1,   -1,   0x85, 0xFF, 0x74, 0x1D, 0x8B,
                               0x07, 0x8B, 0xCF, 0x8B, 0x80, 0x78, 0x02, 0x00, 0x00, 0xFF, 0xD0,
                               0x84, 0xC0, 0x74, 0x0D, 0x80, 0xBF, 0x4D, 0x1A, 0x01, 0x00, 0x00,
                               0x0F, 0x85, 0x9F, 0x02, 0x00, 0x00, 0x53, 0x8B, 0x5D, 0x0C, 0x83,
                               0xFB, 0x01, 0x0F, 0x8D, 0x91, 0x02, 0x00, 0x00, 0x8B, 0x45, 0x08,
                               0x8D, 0x3C, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x2B, 0xFB}}},
    {0x43468994u, 0x20AF16u, {{0xB9, -1,   -1,   -1,   -1,   0xE8, 0x20, 0x8B, 0x0C, 0x00, 0x85,
                               0xC0, 0x0F, 0x84, 0x8C, 0x00, 0x00, 0x00, 0x83, 0xC0, 0xEC, 0x0F,
                               0x84, 0x83, 0x00, 0x00, 0x00, 0x80, 0xB8, 0x48, 0x01, 0x00, 0x00,
                               0x00, 0x74, 0x7A, 0x83, 0x3D, -1,   -1,   -1,   -1,   0x00, 0x74,
                               0x44, 0xA1, -1,   -1,   -1,   -1,   0xA8, 0x01, 0x75, 0x19, 0x6A,
                               0x00, 0x83, 0xC8, 0x01, 0xB9, -1,   -1,   -1,   -1}}},
    {0x43467EF4u, 0x4062E4u, {{0x8B, 0x3C, 0xB5, -1,   -1,   -1,   -1,   0x85, 0xFF, 0x75, 0x04,
                               0x32, 0xC0, 0xEB, 0x16, 0x8B, 0x07, 0x8B, 0xCF, 0xFF, 0x10, 0xB9,
                               0x00, 0x01, 0x00, 0x00, 0x66, 0x3B, 0xC1, 0x73, 0xEC, 0x8B, 0x07,
                               0x8B, 0xCF, 0xFF, 0x10, 0x0F, 0xB6, 0xC0, 0x89, 0x83, 0xFC, 0x04,
                               0x00, 0x00, 0x8B, 0x04, 0xB5, -1,   -1,   -1,   -1,   0x5F, 0x8B,
                               0x84, 0x83, 0x0C, 0x05, 0x00, 0x00, 0x89, 0x83, 0x00}}},
    {0x434670C4u, 0x3CBB2Bu, {{0xBA, -1,   -1,   -1,   -1,   0xB8, -1,   -1,   -1,   -1,   0x8B,
                               0x08, 0x83, 0x3C, 0x8A, 0x00, 0x74, 0x12, 0x83, 0xC0, 0x04, 0x3D,
                               -1,   -1,   -1,   -1,   0x7C, 0xEE, 0x51, 0x8B, 0xCE, 0xE8, 0x41,
                               0x8A, 0xDD, 0xFF, 0x5E, 0x5D, 0xC2, 0x04, 0x00, 0xCC, 0xCC, 0xCC,
                               0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xA1, -1,
                               -1,   -1,   -1,   0x56, 0x68, 0x90, 0x00, 0x00, 0x00}}},
}};
/* [dedup read/write] */
/* [dedup require] */
struct skin_bindings_ClientImage {
    std::byte* base;
    bool contains(const void* pointer, std::size_t size, std::uint32_t rva,
                  std::uint32_t length) const noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin = reinterpret_cast<std::uintptr_t>(base) + rva;
        return address >= begin && size <= length && address - begin <= length - size;
    }
    bool text(const void* pointer, std::size_t size) const noexcept {
        return contains(pointer, size, 0x1000u, 0xB38409u);
    }
    bool data(const void* pointer, std::size_t size) const noexcept {
        return contains(pointer, size, 0xDA1000u, 0x460C3E0u);
    }
    bool skin_bindings_rdata(const void* pointer, std::size_t size) const noexcept {
        return contains(pointer, size, 0xB3A000u, 0x26629Cu);
    }
};
skin_bindings_ClientImage skin_bindings_checked(std::byte* module, NativeImageEnvironment images) {
    require(module && images.image_nt_header,
            "Skin binding discovery requires its client module and PE import");
    const auto* nt = images.image_nt_header(module);
    require(nt == module + 0x150 && read<std::uint16_t>(module) == 0x5A4Du &&
                read<std::uint32_t>(nt) == 0x4550u && read<std::uint16_t>(nt + 4) == 0x14Cu &&
                read<std::uint16_t>(nt + 6) == 7u && read<std::uint16_t>(nt + 0x14) == 0xE0u &&
                read<std::uint16_t>(nt + 0x18) == 0x10Bu &&
                read<std::uint32_t>(nt + 8) == 0x65272025u &&
                read<std::uint32_t>(nt + 0x50) == 0x5509000u &&
                read<std::uint32_t>(nt + 0x58) == 0xFA635Fu,
            "Unsupported skin client PE32 build");
    constexpr std::array<const char*, 3> names{".text", ".rdata", ".data"};
    constexpr std::array<std::array<std::uint32_t, 3>, 3> skin_expected{
        {{0x1000u, 0xB38409u, 0x60000020u},
         {0xB3A000u, 0x26629Cu, 0x40000040u},
         {0xDA1000u, 0x460C3E0u, 0xC0000040u}}};
    for (std::size_t wanted = 0; wanted < names.size(); ++wanted) {
        std::array<char, 8> name{};
        std::memcpy(name.data(), names[wanted], std::strlen(names[wanted]));
        const std::byte* match = nullptr;
        for (std::size_t index = 0; index < 7; ++index) {
            const auto* section = nt + 0xF8 + index * 40;
            if (std::memcmp(section, name.data(), 8))
                continue;
            require(!match, "Duplicate skin client section");
            match = section;
        }
        const auto& value = skin_expected[wanted];
        require(match && read<std::uint32_t>(match + 12) == value[0] &&
                    read<std::uint32_t>(match + 8) == value[1] &&
                    read<std::uint32_t>(match + 36) == value[2],
                "Unsupported skin client section fingerprint");
    }
    return {module};
}
const std::byte* skin_bindings_unique(const skin_bindings_ClientImage& image, const CodeRecipe& recipe) {
    const std::byte* found = nullptr;
    for (std::uint32_t offset = 0x1000u; offset <= 0xB39409u - recipe.pattern.size(); ++offset) {
        const auto* candidate = image.base + offset;
        bool matches = true;
        for (std::size_t i = 0; i < recipe.pattern.size(); ++i)
            if (recipe.pattern[i] >= 0 &&
                static_cast<unsigned char>(candidate[i]) != recipe.pattern[i]) {
                matches = false;
                break;
            }
        if (!matches)
            continue;
        require(!found, "Skin client instruction signature is ambiguous");
        found = candidate;
    }
    require(found == image.base + recipe.rva,
            "Skin client instruction signature is missing or unsupported");
    return found;
}
const void* operand(const skin_bindings_ClientImage& image, const std::byte* instruction, std::size_t offset,
                    std::uint32_t skin_expected, std::size_t size) {
    const auto* pointer = reinterpret_cast<const std::byte*>(
        std::uintptr_t(read<std::uint32_t>(instruction + offset)));
    require(pointer == image.base + skin_expected && image.data(pointer, size),
            "Skin static operand has wrong client provenance");
    return pointer;
}
ClientClass* klass(void* client, const skin_bindings_ClientImage& image, const char* name) {
    auto* value = find_client_class(client, netvar_name_hash(name));
    require(value && image.data(value, sizeof(ClientClass)) &&
                image.skin_bindings_rdata(value->network_name, std::strlen(name) + 1) &&
                std::strcmp(value->network_name, name) == 0,
            "Required skin client class is unavailable");
    return value;
}
std::byte* random_export(std::byte* module, const char* name, NativeImageEnvironment images) {
    require(module != nullptr, "Skin random imports require the loaded vstdlib module");
    auto* value = resolve_pe_export(module, netvar_name_hash(name), 0, true, images);
    const auto* nt = images.image_nt_header(module);
    require(nt && read<std::uint16_t>(nt + 4) == 0x14Cu && read<std::uint16_t>(nt + 0x18) == 0x10Bu,
            "Skin random imports require vstdlib PE32");
    const auto* section = find_pe_section(nt, netvar_name_hash(".text"));
    require(section != nullptr, "Skin random imports require executable vstdlib text");
    const auto first = reinterpret_cast<std::uintptr_t>(module) + read<std::uint32_t>(section + 12);
    const auto address = reinterpret_cast<std::uintptr_t>(value);
    require(value && address >= first && address - first < read<std::uint32_t>(section + 8),
            "Genuine skin vstdlib export is unavailable");
    return value;
}
}

BuildSkinBindings initialize_build_skin_bindings(NativeDataImage& data,
                                                 NativeImageEnvironment images) {
    const auto image = skin_bindings_checked(data.native_cell<std::byte*>(0x4346A55Cu), images);
    std::array<const std::byte*, skin_functions.size()> entries;
    for (std::size_t i = 0; i < entries.size(); ++i)
        entries[i] = skin_bindings_unique(image, skin_functions[i]);
    std::array<const std::byte*, operands.size()> sources;
    for (std::size_t i = 0; i < sources.size(); ++i)
        sources[i] = skin_bindings_unique(image, operands[i]);
    auto* override_slot =
        const_cast<void*>(operand(image, sources[0], 2, 0x52BF6D8u, sizeof(void*)));
    auto* hud = const_cast<void*>(operand(image, sources[1], 1, 0x523BC98u, 0x80u));
    const auto* rows =
        static_cast<const std::byte*>(operand(image, sources[2], 3, 0xE047DCu, 0x1Cu));
    const auto* counters =
        static_cast<const std::int32_t*>(operand(image, sources[3], 1, 0x525EBC4u, 0x134u * 4u));
    // The HUD data anchor actually calls this same discovered FindHudElement.
    const auto hud_call = sources[1] + 5;
    require(hud_call[0] == std::byte{0xE8} &&
                hud_call + 5 + read<std::int32_t>(hud_call + 1) == entries[6],
            "Skin HUD data and lookup function do not share their actual call site");
    auto* base_client = data.native_cell<void*>(0x434683E0u);
    require(base_client != nullptr, "Skin classes require the canonical VClient018 interface");
    auto* glove = klass(base_client, image, "CEconWearable");
    auto* viewmodel = klass(base_client, image, "CPredictedViewModel");
    auto* knife = klass(base_client, image, "CKnife");
    auto* vstdlib = data.native_cell<std::byte*>(0x43468A7Cu);
    const auto seed = reinterpret_cast<BuildSkinBindings::RandomSeed>(
        random_export(vstdlib, "RandomSeed", images));
    const auto random = reinterpret_cast<BuildSkinBindings::RandomInteger>(
        random_export(vstdlib, "RandomInt", images));
    using Cosmetic = skins::NativeCosmeticBindings;
    data.native_cell<skins::NativeSchemaBindings::PaintkitDefinition>(0x43467040u) =
        reinterpret_cast<skins::NativeSchemaBindings::PaintkitDefinition>(
            const_cast<std::byte*>(entries[0]));
    data.native_cell<Cosmetic::RegisterEntity>(0x4346A884u) =
        reinterpret_cast<Cosmetic::RegisterEntity>(const_cast<std::byte*>(entries[1]));
    data.native_cell<Cosmetic::UnregisterEntity>(0x43468E88u) =
        reinterpret_cast<Cosmetic::UnregisterEntity>(const_cast<std::byte*>(entries[2]));
    data.native_cell<Cosmetic::SetParent>(0x4346D644u) =
        reinterpret_cast<Cosmetic::SetParent>(const_cast<std::byte*>(entries[3]));
    data.native_cell<Cosmetic::InvalidateBones>(0x434688A4u) =
        reinterpret_cast<Cosmetic::InvalidateBones>(const_cast<std::byte*>(entries[4]));
    data.native_cell<skins::NativeAgentBindings::SupportModels>(0x4346D64Cu) =
        reinterpret_cast<skins::NativeAgentBindings::SupportModels>(
            const_cast<std::byte*>(entries[5]));
    data.native_cell<skins::RefreshBindings::FindHudElement>(0x4346A374u) =
        reinterpret_cast<skins::RefreshBindings::FindHudElement>(
            const_cast<std::byte*>(entries[6]));
    data.native_cell<void**>(0x43468C94u) = static_cast<void**>(override_slot);
    data.native_cell<void*>(0x43468994u) = hud;
    data.native_cell<const std::byte*>(0x43467EF4u) = rows;
    data.native_cell<const std::int32_t*>(0x434670C4u) = counters;
    data.native_cell<ClientClass*>(0x4346841Cu) = glove;
    return {data, seed, random, viewmodel, knife};
}
BuildSkinBindings initialize_build_skin_bindings(NativeDataImage& data) {
    return initialize_build_skin_bindings(data, native_image_environment());
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_skin_receive_patches.cpp
// ============================================================================

namespace skeet::engine {
namespace {
constexpr std::array<BuildSkinReceivePatchRecipe, 5> build_skin_receive_patch_recipes{{
    {0xA48u, 0xDD75A186u, 0xB05C42F2u, 0xA5BF8ECDu, 0x434769BCu, "CBaseViewModel",
     "DT_BaseViewModel", "m_nSequence"},
    {0xA5Cu, 0xDD75A186u, 0xB05C42F2u, 0x0BA094FBu, 0x434769ACu, "CBaseViewModel",
     "DT_BaseViewModel", "m_nModelIndex"},
    {0xA70u, 0xDD75A186u, 0xB05C42F2u, 0x91E87C84u, 0x43476B0Cu, "CBaseViewModel",
     "DT_BaseViewModel", "m_nNewSequenceParity"},
    {0xA84u, 0xAEEB25F3u, 0x6B34DA8Fu, 0x623C927Eu, 0x43476A6Cu, "CBaseWeaponWorldModel",
     "DT_BaseWeaponWorldModel", "m_hCombatWeaponParent"},
    {0xAFCu, 0x0F0EDE49u, 0x8C24DA3Du, 0x0BA094FBu, 0x43476A8Cu, "CBaseEntity", "DT_BaseEntity",
     "m_nModelIndex"},
}};
static_assert(sizeof(client::EntityUpdatePatch) == 16);
static_assert(offsetof(client::EntityUpdatePatch, target) == 4);
static_assert(offsetof(client::EntityUpdatePatch, after) == 8);
static_assert(offsetof(client::EntityUpdatePatch, before) == 12);
}
std::span<const BuildSkinReceivePatchRecipe, 5> build_skin_receive_patch_manifest() noexcept {
    return build_skin_receive_patch_recipes;
}
BuildSkinReceivePatchFailure::BuildSkinReceivePatchFailure(std::size_t at,
                                                           const BuildSkinReceivePatchRecipe& row,
                                                           BuildSkinReceivePatchFailureReason why)
    : std::runtime_error("Required skin receive patch cannot be initialized"), index(at),
      recipe(row), reason(why) {}
BuildSkinReceivePatches::BuildSkinReceivePatches(NativeDataImage& data)
    : data_(data),
      sequence_{{&data.native_cell<client::EntityUpdatePatch>(build_skin_receive_patch_recipes[0].patch_address),
                 &data.native_cell<client::EntityUpdatePatch>(build_skin_receive_patch_recipes[1].patch_address),
                 &data.native_cell<client::EntityUpdatePatch>(build_skin_receive_patch_recipes[2].patch_address),
                 &data.native_cell<client::EntityUpdatePatch>(build_skin_receive_patch_recipes[3].patch_address)}},
      agent_(data.native_cell<client::EntityUpdatePatch>(build_skin_receive_patch_recipes[4].patch_address)) {}
BuildSkinReceivePatches initialize_build_skin_receive_patches(NativeDataImage& data) {
    auto* client = data.native_cell<void*>(0x434683E0u);
    if (!client)
        throw std::invalid_argument("Skin receive patches require canonical VClient018");
    struct Capture {
        client::EntityUpdatePatch* node;
        RecvProp* property;
        RecvVarProxy original;
    };
    std::array<Capture, 5> captures;
    const auto sequence = skins::sequence_receive_descriptors();
    const auto& agent = skins::agent_model_receive_descriptor();
    for (std::size_t i = 0; i < build_skin_receive_patch_recipes.size(); ++i) {
        const auto& row = build_skin_receive_patch_recipes[i];
        // Verify the receiver and discovery manifests cannot silently drift.
        const auto class_hash = i < 4 ? sequence[i].class_hash : agent.class_hash;
        const auto table_hash = i < 4 ? sequence[i].table_hash : agent.table_hash;
        const auto property_hash = i < 4 ? sequence[i].property_hash : agent.property_hash;
        const auto patch_address = i < 4 ? sequence[i].patch_address : agent.patch_address;
        if (row.class_hash != class_hash || row.table_hash != table_hash ||
            row.property_hash != property_hash || row.patch_address != patch_address ||
            netvar_name_hash(row.class_name) != row.class_hash ||
            netvar_name_hash(row.table_name) != row.table_hash ||
            netvar_name_hash(row.property_name) != row.property_hash)
            throw std::logic_error("Skin receive discovery and callback manifests differ");
        auto& node = data.native_cell<client::EntityUpdatePatch>(row.patch_address);
        if (node.target || node.after || node.before)
            throw BuildSkinReceivePatchFailure(
                i, row, BuildSkinReceivePatchFailureReason::already_initialized);
        auto* property =
            find_receive_property(client, row.class_hash, row.table_hash, row.property_hash);
        if (!property)
            throw BuildSkinReceivePatchFailure(
                i, row, BuildSkinReceivePatchFailureReason::missing_property);
        if (!property->proxy)
            throw BuildSkinReceivePatchFailure(
                i, row, BuildSkinReceivePatchFailureReason::missing_original);
        captures[i] = {&node, property, property->proxy};
    }
    for (const auto& capture : captures) {
        capture.node->target = reinterpret_cast<volatile std::int32_t*>(&capture.property->proxy);
        capture.node->after = std::bit_cast<std::int32_t>(
            std::uint32_t(reinterpret_cast<std::uintptr_t>(capture.original)));
    }
    return BuildSkinReceivePatches(data);
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_startup_convars.cpp
// ============================================================================

namespace skeet::engine {
namespace {
// Ordered fixed-build data, not captured heap pointers or a VM getter.
// Every exact name/hash and decoded object was checked against the working
// ICvar registry. Each key is the pinned static word XOR35F76E47, matching
// the native producer and independent existing consumer decoders.
constexpr std::array<BuildStartupConVarSlot, 105> k_build_startup_convar_slots{{
    {0xB70u, 0x4346794Cu, 0x0D26DB34u, 0xC429E1BFu, "weapon_recoil_scale"},
    {0xB78u, 0x434683ACu, 0x0E327003u, 0x0AF7B93Cu, "cl_interpolate"},
    {0xB80u, 0x434688A8u, 0x1347018Fu, 0xCDF25CE4u, "cl_extrapolate"},
    {0xB88u, 0x43467A5Cu, 0x1FF3E017u, 0xBA5BE8A8u, "name"},
    {0xB90u, 0x43468528u, 0xDD9E7FF3u, 0xF75367FEu, "sv_max_usercmd_future_ticks"},
    {0xB98u, 0x4346747Cu, 0x77619B7Du, 0x03B4819Cu, "cl_updaterate"},
    {0xBA0u, 0x434671ACu, 0x072F875Eu, 0xD8BFAEFFu, "cl_cmdrate"},
    {0xBA8u, 0x43468BA8u, 0x8656D6BCu, 0x30911EABu, "cl_smooth"},
    {0xBB0u, 0x4346A748u, 0x9263E2C4u, 0x7D9DC86Fu, "cl_interp"},
    {0xBB8u, 0x4346A330u, 0x3F0981F4u, 0xBE64D223u, "cl_interp_ratio"},
    {0xBC0u, 0x43467A54u, 0xCA292AF8u, 0xB882FC15u, "sv_gravity"},
    {0xBC8u, 0x43468C2Cu, 0x85E0EDEBu, 0x3FE3AE0Cu, "sv_maxvelocity"},
    {0xBD0u, 0x4346748Cu, 0x8F6F36C9u, 0x69283E00u, "sv_maxspeed"},
    {0xBD8u, 0x43467E28u, 0x72D57DC4u, 0xE80C95F9u, "sv_accelerate_use_weapon_speed"},
    {0xBE0u, 0x43467048u, 0x53E4D214u, 0x983081BFu, "molotov_throw_detonate_time"},
    {0xBE8u, 0x43467088u, 0x882577C9u, 0x8FD87158u, "weapon_molotov_maxdetonateslope"},
    {0xBF0u, 0x43469008u, 0x3EC4CEFDu, 0xAAA77B8Au, "sv_footsteps"},
    {0xBF8u, 0x43468F44u, 0x64E9041Cu, 0x684EE759u, "mat_postprocess_enable"},
    {0xC00u, 0x43468F98u, 0xC875E387u, 0xADB120F0u, "cl_csm_shadows"},
    {0xC08u, 0x43467950u, 0xC18F6179u, 0x66BFCA30u, "m_yaw"},
    {0xC10u, 0x43467988u, 0x80666B20u, 0x01EAAD2Fu, "m_pitch"},
    {0xC18u, 0x43467BB4u, 0x89CFF0F4u, 0x0998CE5Du, "weapon_recoil_decay2_exp"},
    {0xC20u, 0x43468D8Cu, 0x959D7974u, 0x9709DCB1u, "weapon_recoil_decay2_lin"},
    {0xC28u, 0x4346A480u, 0x02E9B330u, 0x195B42E3u, "weapon_recoil_vel_decay"},
    {0xC30u, 0x43467104u, 0x7CC02A76u, 0xE974CD65u, "weapon_recoil_decay_coefficient"},
    {0xC38u, 0x43468C44u, 0xB3A95638u, 0x1D07014Du, "cl_modelfastpath"},
    {0xC40u, 0x43467124u, 0x3A884E45u, 0xC5C9925Eu, "sv_stopspeed"},
    {0xC48u, 0x4346A820u, 0x3F40AE2Eu, 0xEAAEC541u, "sv_friction"},
    {0xC50u, 0x43467990u, 0xE8B4AAECu, 0xAF1F2C93u, "sv_jump_impulse"},
    {0xC58u, 0x4346A568u, 0x5D844595u, 0x9190E2E6u, "mp_teammates_are_enemies"},
    {0xC60u, 0x43468CACu, 0x3D9AD583u, 0xD8ED8A94u, "mp_solid_teammates"},
    {0xC68u, 0x43467FA0u, 0xACE01EB4u, 0x4B52A9BFu, "sv_enablebunnyhopping"},
    {0xC70u, 0x4346A4E4u, 0xF946C584u, 0x14F449DFu, "sv_autobunnyhopping"},
    {0xC78u, 0x434674E4u, 0xBD9F590Bu, 0xDCEAEAC8u, "sv_airaccelerate"},
    {0xC80u, 0x434683BCu, 0xE1D49A6Bu, 0xE853BE80u, "r_jiggle_bones"},
    {0xC88u, 0x43467080u, 0x9425368Fu, 0x62444FD4u, "r_player_visibility_mode"},
    {0xC90u, 0x434693BCu, 0x89AD8BCEu, 0x2729DCC3u, "r_eyegloss"},
    {0xC98u, 0x4346A780u, 0x849C73B8u, 0x359CF7E3u, "cl_lagcomp_errorcheck"},
    {0xCA0u, 0x43467A78u, 0xF748DD16u, 0x3AE06513u, "cl_lagcompensation"},
    {0xCA8u, 0x43467968u, 0x84060A5Du, 0x1632541Cu, "sv_accelerate"},
    {0xCB0u, 0x434670D0u, 0xFF4A4615u, 0x1320FFAAu, "sv_infinite_ammo"},
    {0xCB8u, 0x43468254u, 0xDF660FEEu, 0xCF54AEB3u, "sv_skirmish_id"},
    {0xCC0u, 0x43467FBCu, 0x3C49C196u, 0x96BEB183u, "rate"},
    {0xCC8u, 0x4346A39Cu, 0x746AD238u, 0x738FE7C5u, "sv_skyname"},
    {0xCD0u, 0x43468DFCu, 0x7EBAA819u, 0x18D4EC2Eu, "cam_snapto"},
    {0xCD8u, 0x43467D8Cu, 0x7C368E82u, 0x41632A61u, "cam_collision"},
    {0xCE0u, 0x43467AA0u, 0x579E58FBu, 0x8985EFCEu, "cam_idealdist"},
    {0xCE8u, 0x4346A3B0u, 0x3C7F8DBCu, 0xE66A127Fu, "developer"},
    {0xCF0u, 0x4346A694u, 0x8955D95Au, 0xD9328889u, "mp_free_armor"},
    {0xCF8u, 0x434685A0u, 0x10DA1A89u, 0xDE58CE34u, "sv_min_jump_landing_sound"},
    {0xD00u, 0x43467638u, 0x17F42093u, 0xDEAE3FAEu, "sv_stepsize"},
    {0xD08u, 0x43468E94u, 0xDCB5D665u, 0x2DC3CC20u, "bot_mimic"},
    {0xD10u, 0x43468EA4u, 0xFAF01779u, 0xFCD2DA7Au, "weapon_accuracy_nospread"},
    {0xD18u, 0x4346A478u, 0x7EA31282u, 0x3D5F961Fu, "mp_c4timer"},
    {0xD20u, 0x43468520u, 0x20E2FB4Fu, 0x394BFF56u, "weapon_accuracy_shotgun_spread_patterns"},
    {0xD28u, 0x43469238u, 0x4F986B7Cu, 0xF7E8922Fu, "cl_predictweapons"},
    {0xD30u, 0x4346A7DCu, 0x8F4F6833u, 0xB2005B9Cu, "cl_righthand"},
    {0xD38u, 0x43468C30u, 0xB4E39538u, 0x876ADF97u, "mp_flinch_punch_scale"},
    {0xD40u, 0x43469068u, 0xCDAAD240u, 0x7B3564BBu, "sv_deadtalk"},
    {0xD48u, 0x43468D88u, 0x19F7F06Bu, 0x584C3C72u, "sv_full_alltalk"},
    {0xD50u, 0x4346A850u, 0xA14C43A9u, 0x478720C2u, "cl_predict"},
    {0xD58u, 0x4346896Cu, 0xE3376ED8u, 0xA7B82DE9u, "joy_autosprint"},
    {0xD60u, 0x43467D7Cu, 0xD982577Fu, 0x4A285142u, "sv_timebetweenducks"},
    {0xD68u, 0x43467A8Cu, 0xA5D1AF91u, 0xAC9299CCu, "game_type"},
    {0xD70u, 0x4346A42Cu, 0x400F5DEAu, 0x672DFE69u, "game_mode"},
    {0xD78u, 0x4346A37Cu, 0xDFD900D9u, 0x38DF1D5Eu, "r_skybox"},
    {0xD80u, 0x4346925Cu, 0x8B3A7803u, 0x89E7CF90u, "r_drawskybox"},
    {0xD88u, 0x434693A8u, 0x33B5FAE3u, 0x27F664A6u, "r_3dsky"},
    {0xD90u, 0x43468A94u, 0xABBC3D6Du, 0x24CE2F36u, "gl_clear"},
    {0xD98u, 0x43467130u, 0x726DD366u, 0x99854835u, "net_maxroutable"},
    {0xDA0u, 0x43467A48u, 0x80F57EB3u, 0x1EEEF408u, "zoom_sensitivity_ratio_mouse"},
    {0xDA8u, 0x4346A448u, 0x5273AB65u, 0xBB72C450u, "cl_clock_correction"},
    {0xDB0u, 0x4346859Cu, 0xE008AEDEu, 0x8510F1A9u, "tv_enable"},
    {0xDB8u, 0x434674DCu, 0x542EE5CBu, 0xE3F1DF58u, "tv_enable1"},
    {0xDC0u, 0x434671B0u, 0xDED404CCu, 0x9628DBC9u, "host_timescale"},
    {0xDC8u, 0x43467C3Cu, 0xB239EC33u, 0x211963A6u, "mp_friendlyfire"},
    {0xDD0u, 0x4346A88Cu, 0xD39FDE1Du, 0xD9D937D8u, "sv_staminamax"},
    {0xDD8u, 0x43468F4Cu, 0x575B8D6Bu, 0xB0CF25E8u, "sv_staminalandcost"},
    {0xDE0u, 0x43468FB4u, 0x3F8F7262u, 0xC9B16EBBu, "sv_staminajumpcost"},
    {0xDE8u, 0x43467E4Cu, 0x6BAE0E86u, 0xFC23F1E7u, "sv_staminarecoveryrate"},
    {0xDF0u, 0x434685DCu, 0x288CA67Au, 0x1A59159Fu, "occlusion_test_margins"},
    {0xDF8u, 0x434679D0u, 0xFF62558Au, 0x3F95B88Du, "occlusion_test_camera_margins"},
    {0xE00u, 0x43468338u, 0x10E146F2u, 0x24D1CE57u, "occlusion_test_jump_margin"},
    {0xE08u, 0x434671CCu, 0x402E015Cu, 0xEDA30A71u, "occlusion_test_shadow_length"},
    {0xE10u, 0x43468A78u, 0x1AEFDFFCu, 0xA6BBEFA3u, "cl_clock_correction_adjustment_min_offset"},
    {0xE18u, 0x4346A6FCu, 0x7A564282u, 0x856034BDu, "cl_clock_correction_adjustment_max_offset"},
    {0xE20u, 0x43467588u, 0xB7A90EBDu, 0xBEF53DACu, "cl_clock_correction_adjustment_max_amount"},
    {0xE28u, 0x434670F8u, 0x6281AD55u, 0x00329FD4u, "net_fakelag"},
    {0xE30u, 0x4346A67Cu, 0x61F7DF53u, 0x3155EA46u, "cl_ragdoll_physics_enable"},
    {0xE38u, 0x4346A5B4u, 0x322214A2u, 0x7CBE81A1u, "cl_ragdoll_workaround_threshold"},
    {0xE40u, 0x434671ECu, 0xD7F1E079u, 0x3C0C5F1Cu, "ff_damage_grenade_hit"},
    {0xE48u, 0x4346A7ECu, 0xFBDF805Du, 0x6B6A9C24u, "ff_damage_reduction_grenade"},
    {0xE50u, 0x434685C8u, 0xC85EB685u, 0xD5F0540Cu, "ff_damage_reduction_other"},
    {0xE58u, 0x4346A4FCu, 0xA48831A2u, 0x02081323u, "ff_damage_reduction_grenade_self"},
    {0xE60u, 0x43468C90u, 0x42215707u, 0x98D588D4u, "ff_damage_bullet_penetration"},
    {0xE68u, 0x43467084u, 0x10C2C88Au, 0x9C2756F5u, "ff_damage_reduction_bullets"},
    {0xE70u, 0x434670E8u, 0xDE1DAA9Cu, 0x308C7E6Bu, "mp_buy_anywhere"},
    {0xE78u, 0x43468E24u, 0x0DEB6867u, 0x41FA9A70u, "cl_rebuy"},
    {0xE80u, 0x43468510u, 0xA1215558u, 0x3273B755u, "cl_mouseenable"},
    {0xE88u, 0x43468EA8u, 0x4B0F60A9u, 0xAC34F836u, "sv_talk_enemy_living"},
    {0xE90u, 0x43467A4Cu, 0x86AB32B6u, 0xFA2DD713u, "sv_talk_enemy_dead"},
    {0xE98u, 0x4346A648u, 0x494D4282u, 0x66E946DBu, "singlestep"},
    {0xEA0u, 0x43467128u, 0x619A6A03u, 0xE78AB46Au, "cl_clanid"},
    {0xEA8u, 0x43469028u, 0x7847F897u, 0x0EFD12C6u, "mp_damage_headshot_only"},
    {0xEB0u, 0x4346A534u, 0x2153ADA1u, 0xB739EB42u, "viewmodel_recoil"},
}};
class EngineBuildConVarProvider final : public BuildStartupConVarProvider {
public:
    void* find_variable(void* cvar, const char* name) override {
        return invoke<void*>(cvar, 15, name);
    }
    bool is_command(void* candidate) override {
        return invoke<bool>(candidate, 1);
    }
};
std::string startup_convars_failure_message(std::size_t index, const BuildStartupConVarSlot& slot,
                            BuildStartupConVarFailureReason reason) {
    std::ostringstream message;
    message << "Build ConVar " << index << " '" << slot.name << "' (hash0x" << std::hex
            << slot.name_hash << ") ";
    if (reason == BuildStartupConVarFailureReason::missing_candidate)
        message << "is unavailable";
    else
        message << "resolves to a command";
    return message.str();
}
}
BuildStartupConVarFailure::BuildStartupConVarFailure(std::size_t failed_index,
                                                     const BuildStartupConVarSlot& slot,
                                                     BuildStartupConVarFailureReason classification)
    : std::runtime_error(startup_convars_failure_message(failed_index, slot, classification)), index(failed_index),
      operand(slot.operand), address(slot.address), name_hash(slot.name_hash), name(slot.name),
      reason(classification) {}
std::span<const BuildStartupConVarSlot, 105> build_startup_convar_slots() noexcept {
    return k_build_startup_convar_slots;
}
NativeConVarReference BuildStartupConVarBindings::at(std::uint32_t address) const {
    for (const auto& slot : k_build_startup_convar_slots)
        if (slot.address == address)
            return {data_.native_cell<std::uint32_t>(address), slot.xor_key};
    throw std::out_of_range("Address is not a fixed-build startup ConVar cell");
}
StartupConVarBindings BuildStartupConVarBindings::startup_bindings() const noexcept {
    return StartupConVarBindings(data_);
}
client::StartupConVars BuildStartupConVarBindings::startup_variables() const {
    return startup_bindings().startup_variables();
}
NativeConVarReference BuildStartupConVarBindings::accuracy_variable() const {
    return startup_bindings().accuracy_variable();
}
BuildStartupConVarBindings discover_build_startup_convars(NativeDataImage& data, void* cvar,
                                                          BuildStartupConVarProvider& provider) {
    if (!cvar)
        throw std::invalid_argument("Build ConVar discovery requires the real ICvar interface");
    std::array<std::uint32_t*, k_build_startup_convar_slots.size()> destinations;
    for (std::size_t index = 0; index < k_build_startup_convar_slots.size(); ++index)
        destinations[index] = &data.native_cell<std::uint32_t>(k_build_startup_convar_slots[index].address);
    std::array<void*, k_build_startup_convar_slots.size()> found;
    for (std::size_t index = 0; index < k_build_startup_convar_slots.size(); ++index) {
        found[index] = provider.find_variable(cvar, k_build_startup_convar_slots[index].name);
        // Native NULL reaches4337F065 with code10 after any earlier stores.
        // The command-candidate path reaches erased4353F942. Acquisition and
        // typed failure before any publication are explicit source policy.
        if (!found[index])
            throw BuildStartupConVarFailure(index, k_build_startup_convar_slots[index],
                                            BuildStartupConVarFailureReason::missing_candidate);
        if (provider.is_command(found[index]))
            throw BuildStartupConVarFailure(index, k_build_startup_convar_slots[index],
                                            BuildStartupConVarFailureReason::candidate_is_command);
    }
    for (std::size_t index = 0; index < k_build_startup_convar_slots.size(); ++index)
        *destinations[index] =
            std::uint32_t(reinterpret_cast<std::uintptr_t>(found[index])) ^ k_build_startup_convar_slots[index].xor_key;
    return BuildStartupConVarBindings(data);
}
BuildStartupConVarBindings discover_build_startup_convars(NativeDataImage& data, void* cvar) {
    EngineBuildConVarProvider provider;
    return discover_build_startup_convars(data, cvar, provider);
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: build_visual_interface_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct visual_interface_bindings_Section {
    std::uint32_t rva, size, flags;
};
constexpr visual_interface_bindings_Section visual_interface_text{0x1000, 0xB38409, 0x60000020};
constexpr visual_interface_bindings_Section visual_interface_rdata{0xB3A000, 0x26629C, 0x40000040};
constexpr visual_interface_bindings_Section visual_interface_objects{0xDA1000, 0x460C3E0, 0xC0000040};
struct visual_interface_bindings_Client {
    std::byte* base;
    bool contains(const void* pointer, std::size_t size, visual_interface_bindings_Section section) const noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto first = reinterpret_cast<std::uintptr_t>(base) + section.rva;
        return address >= first && size <= section.size && address - first <= section.size - size;
    }
    void visual_interface_bindings_expect(const std::byte* code, std::span<const int> bytes) const {
        require(contains(code, bytes.size(), visual_interface_text), "Visual method is outside genuine client visual_interface_text");
        for (std::size_t index = 0; index < bytes.size(); ++index)
            require(bytes[index] < 0 || static_cast<unsigned char>(code[index]) == bytes[index],
                    "Visual method does not match the supported client build");
    }
    void visual_interface_bindings_expect(const std::byte* code, std::initializer_list<int> bytes) const {
        visual_interface_bindings_expect(code, {bytes.begin(), bytes.size()});
    }
    const std::byte* visual_interface_bindings_unique(std::span<const int> pattern) const {
        const std::byte* result = nullptr;
        for (std::uint32_t offset = 0; offset <= visual_interface_text.size - pattern.size(); ++offset) {
            const auto* code = base + visual_interface_text.rva + offset;
            bool match = true;
            for (std::size_t index = 0; index < pattern.size(); ++index)
                if (pattern[index] >= 0 &&
                    static_cast<unsigned char>(code[index]) != pattern[index]) {
                    match = false;
                    break;
                }
            if (!match)
                continue;
            require(!result, "Visual client instruction anchor is ambiguous");
            result = code;
        }
        require(result != nullptr, "Visual client instruction anchor is missing");
        return result;
    }
};
visual_interface_bindings_Client checked_client(NativeDataImage& data, NativeImageEnvironment images) {
    auto* module = data.native_cell<std::byte*>(0x4346A55C);
    require(module && images.image_nt_header,
            "Visual visual_interface_objects require the actual client module and PE-header service");
    const auto* nt = images.image_nt_header(module);
    require(nt == module + 0x150 && read<std::uint16_t>(module) == 0x5A4D &&
                read<std::uint32_t>(module + 0x3C) == 0x150 && read<std::uint32_t>(nt) == 0x4550 &&
                read<std::uint16_t>(nt + 4) == 0x14C && read<std::uint16_t>(nt + 6) == 7 &&
                read<std::uint16_t>(nt + 20) == 0xE0 && read<std::uint16_t>(nt + 24) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x65272025 &&
                read<std::uint32_t>(nt + 80) == 0x5509000 &&
                read<std::uint32_t>(nt + 88) == 0xFA635F,
            "Unsupported visual-object client PE32 build");
    constexpr std::array<visual_interface_bindings_Section, 3> visual_interface_expected{visual_interface_text, visual_interface_rdata, visual_interface_objects};
    for (std::size_t index = 0; index < visual_interface_expected.size(); ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        require(read<std::uint32_t>(section + 12) == visual_interface_expected[index].rva &&
                    read<std::uint32_t>(section + 8) == visual_interface_expected[index].size &&
                    read<std::uint32_t>(section + 36) == visual_interface_expected[index].flags,
                "Visual client section geometry differs from the supported build");
    }
    return {module};
}
void** table(const visual_interface_bindings_Client& image, const char* name, std::size_t slots,
             NativeImageEnvironment images, std::uint32_t object_offset = 0) {
    auto* type = find_rtti_type(image.base, netvar_name_hash(name), images);
    require(type && image.contains(type, 8 + std::strlen(name) + 1, visual_interface_objects) &&
                !std::strcmp(type->name, name),
            "Required genuine visual client RTTI type is missing");
    auto** result = find_rtti_vtable_at_offset(image.base, type, object_offset, images);
    require(result && image.contains(result, slots * 4, visual_interface_rdata) &&
                image.contains(result - 1, 4, visual_interface_rdata),
            "Required visual client vtable is missing or outside read-only data");
    auto* locator = read<const std::byte*>(reinterpret_cast<const std::byte*>(result - 1));
    require(image.contains(locator, 20, visual_interface_rdata) && read<std::uint32_t>(locator) == 0 &&
                read<std::uint32_t>(locator + 4) == object_offset &&
                read<std::uint32_t>(locator + 8) == 0 &&
                read<RttiTypeDescriptor*>(locator + 12) == type,
            "Visual client RTTI locator is inconsistent");
    for (std::size_t slot = 0; slot < slots; ++slot)
        require(image.contains(result[slot], 1, visual_interface_text),
                "Visual vtable method is outside genuine client visual_interface_text");
    return result;
}
std::byte* unique_instance(const visual_interface_bindings_Client& image, void** vtable, std::size_t extent) {
    std::byte* result = nullptr;
    for (std::uint32_t offset = 0; offset <= visual_interface_objects.size - extent; offset += 4) {
        auto* object = image.base + visual_interface_objects.rva + offset;
        if (read<void**>(object) != vtable)
            continue;
        require(!result, "Visual client inline instance is ambiguous");
        result = object;
    }
    require(result != nullptr, "Required visual client inline instance has not been constructed");
    return result;
}
}
BuildVisualInterfaceBindings
initialize_build_visual_interface_bindings(NativeDataImage& data, NativeImageEnvironment images) {
    const auto image = checked_client(data, images);
    auto** view_table = table(image, ".?AVCCSViewRender@@", 13, images);
    auto* view = unique_instance(image, view_table, 0x5A8);
    // Real static initialization writes the same CCSViewRender table and its
    // three following fields. The object is derived from the store operand.
    std::array<int, 17> view_anchor{0xC7, 0x05, -1,   -1, -1, -1, -1, -1, -1,
                                    -1,   0xC6, 0x05, -1, -1, -1, -1, 0};
    const auto table_value = reinterpret_cast<std::uintptr_t>(view_table);
    for (unsigned index = 0; index < 4; ++index)
        view_anchor[6 + index] = int((table_value >> (index * 8)) & 255u);
    const auto* initialization = image.visual_interface_bindings_unique(view_anchor);
    require(read<std::byte*>(initialization + 2) == view &&
                read<std::byte*>(initialization + 12) == view + 0x5A0,
            "CCSViewRender initialization operands disagree with the genuine inline instance");
    const auto* get_view = static_cast<const std::byte*>(view_table[12]);
    image.visual_interface_bindings_expect(get_view, {0x55, 0x8B, 0xEC, 0x8B, 0x45, 8, 0x83, 0xF8, 0xFF, 0x75, 7,
                            0x8D, 0x41, 4,    0x5D, 0xC2, 4, 0,    0x69, 0xC0, 0x4C, 1,
                            0,    0,    0x83, 0xC0, 4,    3, 0xC1, 0x5D, 0xC2, 4,    0});

    auto** temp_table = table(image, ".?AVC_TempEntsSystem@@", 10, images);
    auto* temp = unique_instance(image, temp_table, 0x14);
    // The same inline object is both ECX and the absolute vtable read in this
    // genuine player-effect call. This also distinguishes its static instance
    // from an unrelated object that happens to share a class table.
    static constexpr int temp_anchor[]{0xB9, -1,   -1,   -1,   -1,   0x50, 0xA1, -1,   -1,   -1,
                                       -1,   0xFF, 0x50, 0x7C, 0x8D, 0x4D, 0xE4, 0xC7, 0x45, 0xDC};
    const auto* temp_use = image.visual_interface_bindings_unique(temp_anchor);
    require(read<std::byte*>(temp_use + 1) == temp && read<std::byte*>(temp_use + 7) == temp,
            "Temporary-entity code operands disagree with the genuine inline instance");
    const auto* ring = static_cast<const std::byte*>(temp_table[9]);
    image.visual_interface_bindings_expect(ring,
                 {0x55, 0x8B, 0xEC, 0x8B, 1,    0xFF, 0x75, 8,    0x8B, 0x40, 4,    0xFF, 0xD0,
                  0x84, 0xC0, 0x75, 0x5F, 0xFF, 0x75, 0x50, 0xF3, 0x0F, 0x10, 0x45, 0x38});
    image.visual_interface_bindings_expect(ring + 0x70, {0x5D, 0xC2, 0x4C, 0}); //19 original stack arguments

    auto** beams_table = table(image, ".?AVCViewRenderBeams@@", 13, images);
    auto* beams = unique_instance(image, beams_table, 0x20);
    std::array<int, 20> beams_anchor{0xC7, 5,  -1, -1, -1, -1,   -1, -1, -1, -1,
                                     0xB9, -1, -1, -1, -1, 0xE9, -1, -1, -1, -1};
    const auto beams_value = reinterpret_cast<std::uintptr_t>(beams_table);
    for (unsigned index = 0; index < 4; ++index)
        beams_anchor[6 + index] = int((beams_value >> (index * 8)) & 255u);
    const auto* beams_init = image.visual_interface_bindings_unique(beams_anchor);
    require(read<std::byte*>(beams_init + 2) == beams && read<std::byte*>(beams_init + 11) == beams,
            "Beam-renderer constructor operands disagree with the genuine inline instance");
    image.visual_interface_bindings_expect(static_cast<const std::byte*>(beams_table[12]),
                 {0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 8,    0x0F, 0x57, 0xC9,
                  0x57, 0x8B, 0xF9, 0xF3, 0x0F, 0x10, 0x46, 0x40, 0x0F, 0x2E,
                  0xC1, 0x9F, 0xF6, 0xC4, 0x44, 0x7B, 0x17, 0x6A, 1});

    //43467B54 is this class's IWeaponSystem subobject at complete-object+C,
    // not its primary game-system endpoint. Its real RTTI COL has offsetC.
    auto** weapon_table = table(image, ".?AVCCSWeaponSystem@@", 3, images, 0xC);
    auto* weapon_system = unique_instance(image, weapon_table, 0x30);
    std::array<int, 20> weapon_anchor{0xC7, 5, -1, -1, -1, -1, -1, -1, -1, -1,
                                      0xC7, 5, -1, -1, -1, -1, 0,  0,  0,  0};
    const auto weapon_value = reinterpret_cast<std::uintptr_t>(weapon_table);
    for (unsigned index = 0; index < 4; ++index)
        weapon_anchor[6 + index] = int((weapon_value >> (index * 8)) & 255u);
    const auto* weapon_init = image.visual_interface_bindings_unique(weapon_anchor);
    require(read<std::byte*>(weapon_init + 2) == weapon_system &&
                read<std::byte*>(weapon_init + 12) == weapon_system + 8,
            "Weapon-system constructor operands disagree with the genuine inline instance");
    image.visual_interface_bindings_expect(static_cast<const std::byte*>(weapon_table[2]),
                 {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x0C, 0x53, 0x8B, 0x5D, 8,
                  0x89, 0x4D, 0xF4, 0x81, 0xFB, 0xFF, 0xFF, 0,    0,    0x72,
                  9,    0x33, 0xC0, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 4,    0});
    // Source acquisition publishes this pair after validation; it does not
    // claim a separate recovered ordering for the loader's extra() writes.
    data.native_cell<void*>(0x4346A624) = view;
    data.native_cell<void*>(0x434679EC) = temp;
    data.native_cell<void*>(0x4346A898) = beams;
    data.native_cell<void*>(0x43467B54) = weapon_system;
    return {view, temp, beams, weapon_system};
}
BuildVisualInterfaceBindings initialize_build_visual_interface_bindings(NativeDataImage& data) {
    return initialize_build_visual_interface_bindings(data, native_image_environment());
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_window_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct window_bindings_SectionProfile {
    char name[8];
    std::uint32_t window_bindings_rva, size, flags;
};
struct window_bindings_BuildProfile {
    std::uint32_t nt_offset, timestamp, image_size, checksum;
    std::uint16_t section_count;
    std::array<window_bindings_SectionProfile, 3> sections;
};
// Only the three sections used below are fingerprinted; every section's
// virtual extent is also checked against the declared image size.
constexpr window_bindings_BuildProfile window_engine_build{0x150,
                                    0x64EE7E86,
                                    0x39C0000,
                                    0x64174E,
                                    7,
                                    {{{".text", 0x1000, 0x451C32, 0x60000020},
                                      {".rdata", 0x453000, 0xD4766, 0x40000040},
                                      {".data", 0x528000, 0x3427638, 0xC0000040}}}};
constexpr window_bindings_BuildProfile studio_build{0x110,
                                    0x63DC118B,
                                    0x45E000,
                                    0x9DB8B,
                                    6,
                                    {{{".text", 0x1000, 0x717A2, 0x60000020},
                                      {".rdata", 0x73000, 0x1698E, 0x40000040},
                                      {".data", 0x8A000, 0x3C576C, 0xC0000040}}}};
struct window_bindings_Image {
    std::byte* base;
    const window_bindings_BuildProfile& build;
    bool contains(const void* pointer, std::size_t length, std::size_t section) const noexcept {
        const auto& region = build.sections[section];
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin = reinterpret_cast<std::uintptr_t>(base) + region.window_bindings_rva;
        return address >= begin && length <= region.size && address - begin <= region.size - length;
    }
    std::uint32_t window_bindings_rva(const void* pointer) const noexcept {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer) -
                                          reinterpret_cast<std::uintptr_t>(base));
    }
};
window_bindings_Image window_bindings_checked_image(void* module, const window_bindings_BuildProfile& profile) {
    require(module != nullptr, "Required engine/studiorender module is not loaded");
    auto* const base = static_cast<std::byte*>(module);
    require(read<std::uint16_t>(base) == 0x5A4D, "Build binding requires a DOS image header");
    require(read<std::uint32_t>(base + 0x3C) == profile.nt_offset,
            "Unsupported engine/studiorender NT-header position");
    const auto* nt = base + profile.nt_offset;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == profile.section_count &&
                read<std::uint16_t>(nt + 0x14) == 0xE0 && read<std::uint16_t>(nt + 0x18) == 0x10B,
            "Unsupported engine/studiorender PE32 layout");
    require(read<std::uint32_t>(nt + 8) == profile.timestamp &&
                read<std::uint32_t>(nt + 0x50) == profile.image_size &&
                read<std::uint32_t>(nt + 0x58) == profile.checksum,
            "Unsupported engine/studiorender build identity");
    std::array<bool, 3> found{};
    for (std::uint16_t index = 0; index < profile.section_count; ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        const auto window_bindings_rva = read<std::uint32_t>(section + 12), size = read<std::uint32_t>(section + 8);
        require(window_bindings_rva <= profile.image_size && size <= profile.image_size - window_bindings_rva,
                "Loaded PE section exceeds its image");
        for (std::size_t wanted = 0; wanted < profile.sections.size(); ++wanted) {
            const auto& expected = profile.sections[wanted];
            if (std::memcmp(section, expected.name, 8))
                continue;
            require(!found[wanted] && window_bindings_rva == expected.window_bindings_rva && size == expected.size &&
                        read<std::uint32_t>(section + 36) == expected.flags,
                    "Unsupported engine/studiorender section fingerprint");
            found[wanted] = true;
        }
    }
    require(found[0] && found[1] && found[2], "Required engine/studiorender section is missing");
    return {base, profile};
}
/* [dedup matches] */
void window_bindings_expect_code(const window_bindings_Image& image, const std::byte* code, std::initializer_list<int> pattern) {
    require(image.contains(code, pattern.size(), 0),
            "Build binding code is outside executable text");
    require(matches(code, {pattern.begin(), pattern.size()}),
            "Loaded engine/studiorender instruction fingerprint differs");
}
const std::byte* window_bindings_unique_code(const window_bindings_Image& image, std::initializer_list<int> pattern,
                             std::uint32_t expected_rva) {
    const auto& text = image.build.sections[0];
    const std::byte* found = nullptr;
    for (std::uint32_t offset = 0; offset <= text.size - pattern.size(); ++offset) {
        const auto* code = image.base + text.window_bindings_rva + offset;
        if (!matches(code, {pattern.begin(), pattern.size()}))
            continue;
        require(found == nullptr, "Loaded engine/studiorender instruction anchor is ambiguous");
        found = code;
    }
    require(found && image.window_bindings_rva(found) == expected_rva,
            "Loaded engine/studiorender instruction anchor is unsupported");
    return found;
}
const std::byte* window_bindings_absolute(const window_bindings_Image& image, const std::byte* operand, std::size_t section,
                          std::size_t length) {
    const auto* pointer =
        reinterpret_cast<const std::byte*>(std::uintptr_t(read<std::uint32_t>(operand)));
    require(image.contains(pointer, length, section),
            "Decoded engine/studiorender pointer has invalid section provenance");
    return pointer;
}
const std::byte* relative_call(const window_bindings_Image& image, const std::byte* instruction,
                               std::uint32_t expected_rva) {
    window_bindings_expect_code(image, instruction, {0xE8, -1, -1, -1, -1});
    const auto offset =
        std::int64_t(image.window_bindings_rva(instruction)) + 5 + read<std::int32_t>(instruction + 1);
    require(offset >= 0 && offset < image.build.image_size,
            "Decoded engine/studiorender call leaves its image");
    const auto* target = image.base + static_cast<std::size_t>(offset);
    require(image.contains(target, 1, 0) && image.window_bindings_rva(target) == expected_rva,
            "Decoded engine/studiorender call target is unsupported");
    return target;
}
const std::byte* range_validator(const window_bindings_Image& image, std::uint32_t expected_rva) {
    return window_bindings_unique_code(image,
                       {0x55, 0x8B, 0xEC, 0x56, 0x8B, 0xF1, 0x33, 0xC0, 0x57, 0x8B, 0x7D, 0x08,
                        0x8B, 0x8E, 0x80, 0x02, 0,    0,    0x85, 0xC9, 0x7E, 0x16, 0x3B, 0x3C},
                       expected_rva);
}
void module_name_operand(const window_bindings_Image& image, const std::byte* operand, const char* name) {
    const auto length = std::strlen(name) + 1;
    const auto* string = window_bindings_absolute(image, operand, 1, length);
    require(std::memcmp(string, name, length) == 0,
            "Range initializer has an unexpected module-name operand");
}
struct window_bindings_Acquired {
    const std::byte* guard;
    void** interface_slot;
    std::uintptr_t rtti;
    const std::byte* engine_initialize;
    const std::byte* engine_get;
    const std::byte* studio_initialize;
    ModuleAddressRange** studio_holder;
};
window_bindings_Acquired window_bindings_acquire(const window_bindings_Image& engine, const window_bindings_Image& studio) {
    const auto* window =
        window_bindings_unique_code(engine, {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14, 0xA1, -1,   -1,   -1,   -1,   0xA3,
                             -1,   -1,   -1,   -1,   0x8B, 0x0D, -1,   -1,   -1,   -1,   0x89, 0x0D,
                             -1,   -1,   -1,   -1,   0x8B, 0x15, -1,   -1,   -1,   -1,   0x8B, 0x02,
                             0x8B, 0x0D, -1,   -1,   -1,   -1,   0x8B, 0x50, 0x24, 0xFF, 0xD2},
                    0x22F060);
    const auto* guard = window + 0x2F;
    const auto* slot = window_bindings_absolute(engine, window + 0x1E, 2, 4);
    require(read<std::uint32_t>(window + 0x26) == reinterpret_cast<std::uintptr_t>(slot) &&
                engine.window_bindings_rva(slot) == 0x52C810,
            "Game-window caller does not use one verified IGame global");
    const std::array<std::byte, 4> epilogue{std::byte{0x8B}, std::byte{0xE5}, std::byte{0x5D},
                                            std::byte{0xC3}};
    // CRC32 on four bytes is injective. This exact epilogue has the native
    // B94D4C7E checksum; locating its first occurrence mirrors the valid scan.
    auto* continuation = guard;
    while (engine.contains(continuation, 4, 0) && std::memcmp(continuation, epilogue.data(), 4))
        ++continuation;
    require(engine.contains(continuation, 4, 0) && engine.window_bindings_rva(continuation) == 0x22F15F,
            "Game-window caller has an unsupported CRC continuation");
    const auto* object = window_bindings_absolute(engine, slot, 2, 12);
    const auto* table = window_bindings_absolute(engine, object, 1, 25 * 4);
    require(engine.contains(table - 4, 4, 1), "IGame vtable lacks its RTTI prefix");
    const auto* locator = window_bindings_absolute(engine, table - 4, 1, 20);
    require(read<std::uint32_t>(locator) == 0 && read<std::uint32_t>(locator + 4) == 0 &&
                read<std::uint32_t>(locator + 8) == 0,
            "IGame RTTI locator is not the observed CGame layout");
    const auto* type = window_bindings_absolute(engine, locator + 12, 2, 20);
    window_bindings_absolute(engine, locator + 16, 1, 16);
    require(std::memcmp(type + 8, ".?AVCGame@@", 12) == 0, "IGame RTTI type is not CGame");
    const auto* get_window = window_bindings_absolute(engine, table + 9 * 4, 0, 4);
    window_bindings_expect_code(engine, get_window, {0x8B, 0x41, 0x08, 0xC3});

    const auto* engine_validator = range_validator(engine, 0x2BD710);
    const auto* engine_initialize = relative_call(engine, engine_validator + 0x61, 0x2BDC20);
    window_bindings_expect_code(engine, engine_initialize,
                {0x55, 0x8B, 0xEC, 0x81, 0xEC, 0x08, 0x01, 0, 0, 0x56, 0x57, 0x8B, 0x3D, -1, -1, -1,
                 -1, 0x33, 0xF6});
    window_bindings_absolute(engine, engine_initialize + 0xD, 1, 4);
    module_name_operand(engine, engine_initialize + 0x19, "client.dll");
    module_name_operand(engine, engine_initialize + 0x23, "engine.dll");
    const auto* engine_append = relative_call(engine, engine_initialize + 0x25F, 0x2BDFF0);
    window_bindings_expect_code(engine, engine_append, {0x56, 0x8B, 0x71, 0x3C, 0xB8, 0x0B, 0x01, 0,    0,    0x03,
                                        0xF1, 0x66, 0x39, 0x46, 0x18, 0x75, 0x4A, 0x53, 0x8B, 0x5E,
                                        0x50, 0x57, 0x8B, 0x7E, 0x2C, 0x03, 0xF9, 0x03, 0xDF});
    const auto* engine_get = relative_call(engine, engine_append + 0x1D, 0x2BD020);
    window_bindings_expect_code(engine, engine_get,
                {0xA1, -1,   -1,   -1,   -1,   0x85, 0xC0, 0x75, 0x45, 0xA1, -1,   -1,   -1,   -1,
                 0x68, 0x0C, 0x03, 0,    0,    0x8B, 0x08, 0x8B, 0x01, 0xFF, 0x50, 0x04, 0x85, 0xC0,
                 0x74, 0x24, 0xC7, 0x80, 0x80, 0x02, 0,    0,    0,    0,    0,    0,    0xC7, 0x80,
                 0x04, 0x03, 0,    0,    0,    0,    0,    0,    0xC7, 0x80, 0x08, 0x03, 0,    0,
                 0,    0,    0,    0,    0xA3, -1,   -1,   -1,   -1,   0xC3, 0xC7, 0x05, -1,   -1,
                 -1,   -1,   0,    0,    0,    0,    0x33, 0xC0, 0xC3});
    const auto* engine_holder = window_bindings_absolute(engine, engine_get + 1, 2, 4);
    require(read<std::uint32_t>(engine_get + 0x3D) ==
                    reinterpret_cast<std::uintptr_t>(engine_holder) &&
                read<std::uint32_t>(engine_get + 0x44) ==
                    reinterpret_cast<std::uintptr_t>(engine_holder),
            "Engine range getter does not publish its own range holder");
    window_bindings_absolute(engine, engine_get + 0xA, 1, 4);

    const auto* studio_validator = range_validator(studio, 0x57830);
    const auto* studio_initialize = relative_call(studio, studio_validator + 0x61, 0x57DF0);
    window_bindings_expect_code(studio, studio_initialize,
                {0x55, 0x8B, 0xEC, 0x81, 0xEC, 0x08, 0x01, 0,  0,    0x53,
                 0x56, 0x57, 0x8B, 0x3D, -1,   -1,   -1,   -1, 0x33, 0xF6});
    window_bindings_absolute(studio, studio_initialize + 0xE, 1, 4);
    module_name_operand(studio, studio_initialize + 0x1A, "client.dll");
    module_name_operand(studio, studio_initialize + 0x29, "engine.dll");
    const auto* studio_append = relative_call(studio, studio_initialize + 0x27C, 0x570B0);
    window_bindings_expect_code(studio, studio_append,
                {0x53, 0x56, 0x8B, 0x35, -1,   -1,   -1,   -1,   0x8B, 0xDA, 0x57, 0x8B, 0xF9, 0x85,
                 0xF6, 0x75, 0x3F, 0xA1, -1,   -1,   -1,   -1,   0x68, 0x0C, 0x03, 0,    0,    0x8B,
                 0x08, 0x8B, 0x01, 0xFF, 0x50, 0x04, 0x8B, 0xF0, 0x85, 0xF6, 0x74, 0x20, 0xC7, 0x86,
                 0x80, 0x02, 0,    0,    0,    0,    0,    0,    0xC7, 0x86, 0x04, 0x03, 0,    0,
                 0,    0,    0,    0,    0xC7, 0x86, 0x08, 0x03, 0,    0,    0,    0,    0,    0,
                 0xEB, 0x02, 0x33, 0xF6, 0x89, 0x35, -1,   -1,   -1,   -1});
    const auto* studio_holder = window_bindings_absolute(studio, studio_append + 4, 2, 4);
    require(studio.window_bindings_rva(studio_holder) == 0x44E2EC &&
                read<std::uint32_t>(studio_append + 0x4C) ==
                    reinterpret_cast<std::uintptr_t>(studio_holder),
            "Studio range appender does not publish its verified range holder");
    window_bindings_absolute(studio, studio_append + 0x12, 1, 4);
    return {guard,
            reinterpret_cast<void**>(const_cast<std::byte*>(slot)),
            reinterpret_cast<std::uintptr_t>(locator),
            engine_initialize,
            engine_get,
            studio_initialize,
            reinterpret_cast<ModuleAddressRange**>(const_cast<std::byte*>(studio_holder))};
}
}

void initialize_build_window_bindings(NativeDataImage& data,
                                      const LoadedModuleEnvironment& modules) {
    require(sizeof(void*) == 4, "Loaded game-window bindings require the original x86 ABI");
    require(data.size() >= NativeDataImage::image_size,
            "Loaded game-window bindings require the complete canonical data image");
    require(modules.loader && modules.acquire_peb_lock && modules.release_peb_lock,
            "Loaded game-window bindings require the actual module list and lock imports");
    const auto engine = window_bindings_checked_image(
        find_loaded_module(modules, loaded_module_name_hash(u"engine.dll", -1), true, true),
        window_engine_build);
    const auto studio = window_bindings_checked_image(
        find_loaded_module(modules, loaded_module_name_hash(u"studiorender.dll", -1), true, true),
        studio_build);
    const auto bindings = window_bindings_acquire(engine, studio);
    data.native_cell<const std::byte*>(0x43467288u) = bindings.guard;
    data.native_cell<void**>(0x43468B2Cu) = bindings.interface_slot;
    data.native_cell<std::uintptr_t>(0x43469288u) = bindings.rtti;
    // These are literal inactive rows in both pinned payload and working dump.
    // Their companion cells are never read; no captured value is installed.
    data.native_cell<std::uint32_t>(0x434675D8u) = 1;
    data.native_cell<std::uint32_t>(0x4346A58Cu) = 1;
    data.native_cell<ModuleRangeInitialize>(0x4346837Cu) =
        reinterpret_cast<ModuleRangeInitialize>(const_cast<std::byte*>(bindings.engine_initialize));
    data.native_cell<ModuleRangeGet>(0x43467570u) =
        reinterpret_cast<ModuleRangeGet>(const_cast<std::byte*>(bindings.engine_get));
    data.native_cell<ModuleRangeInitialize>(0x43468234u) =
        reinterpret_cast<ModuleRangeInitialize>(const_cast<std::byte*>(bindings.studio_initialize));
    data.native_cell<ModuleAddressRange**>(0x43468C98u) = bindings.studio_holder;
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: build_world_bindings.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
/* [dedup require] */
struct world_bindings_Region {
    std::uint32_t rva, size, flags;
};
struct world_bindings_Profile {
    std::uint32_t nt, timestamp, size, checksum;
    std::uint16_t sections;
    std::array<world_bindings_Region, 3> regions;
};
constexpr world_bindings_Profile client_profile{0x150,
                                 0x65272025,
                                 0x5509000,
                                 0xFA635F,
                                 7,
                                 {{{0x1000, 0xB38409, 0x60000020},
                                   {0xB3A000, 0x26629C, 0x40000040},
                                   {0xDA1000, 0x460C3E0, 0xC0000040}}}};
constexpr world_bindings_Profile panorama_profile{0x160,
                                   0x651F4636,
                                   0x2C6000,
                                   0x26E92E,
                                   8,
                                   {{{0x1000, 0x1E5C8D, 0x60000020},
                                     {0x1E7000, 0x51D7A, 0x40000040},
                                     {0x239000, 0x65DD8, 0xC0000040}}}};
struct world_bindings_Image {
    std::byte* base;
    const world_bindings_Profile& profile;
    bool contains(std::uint32_t at, std::size_t length, unsigned kind) const {
        const auto& r = profile.regions[kind];
        return at >= r.rva && length <= r.size && at - r.rva <= r.size - length;
    }
    void code(std::uint32_t at, std::initializer_list<int> bytes) const {
        require(contains(at, bytes.size(), 0), "World binding code exceeds supported module text");
        for (const int value : bytes) {
            require(value < 0 || std::uint8_t(base[at]) == value,
                    "World binding instruction/ABI differs");
            ++at;
        }
    }
    std::byte* operand(std::uint32_t at, std::uint32_t target, std::size_t size,
                       unsigned kind) const {
        require(contains(at, 4, 0) && contains(target, size, kind),
                "World binding operand has wrong provenance");
        require(read<std::uint32_t>(base + at) == reinterpret_cast<std::uintptr_t>(base + target),
                "World binding operand reaches another module object");
        return base + target;
    }
    void pointer(std::uint32_t at, std::uint32_t target, std::size_t size, unsigned kind) const {
        require(contains(at, 4, 1) && contains(target, size, kind),
                "World RTTI pointer has wrong provenance");
        require(read<std::uint32_t>(base + at) == reinterpret_cast<std::uintptr_t>(base + target),
                "World RTTI pointer identity differs");
    }
    void call(std::uint32_t at, std::uint32_t target) const {
        code(at, {0xE8, -1, -1, -1, -1});
        require(std::int64_t(at) + 5 + read<std::int32_t>(base + at + 1) == target,
                "World constructor/getter calls another engine body");
    }
};
world_bindings_Image world_bindings_checked(void* pointer, const world_bindings_Profile& profile) {
    require(pointer != nullptr, "World bindings require acquired client and panorama modules");
    auto* base = static_cast<std::byte*>(pointer);
    require(read<std::uint16_t>(base) == 0x5A4D && read<std::uint32_t>(base + 0x3C) == profile.nt,
            "World binding DOS/NT layout differs");
    const auto* nt = base + profile.nt;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == profile.sections &&
                read<std::uint16_t>(nt + 20) == 0xE0 && read<std::uint16_t>(nt + 24) == 0x10B &&
                read<std::uint32_t>(nt + 8) == profile.timestamp &&
                read<std::uint32_t>(nt + 80) == profile.size &&
                read<std::uint32_t>(nt + 88) == profile.checksum,
            "Unsupported world binding module build");
    std::array<bool, 3> found{};
    for (unsigned i = 0; i < profile.sections; ++i) {
        const auto* s = nt + 0xF8 + i * 40;
        const auto rva = read<std::uint32_t>(s + 12), size = read<std::uint32_t>(s + 8);
        require(rva <= profile.size && size <= profile.size - rva,
                "World module section exceeds image");
        for (unsigned kind = 0; kind < found.size(); ++kind) {
            constexpr std::array<const char*, 3> names{".text\0\0", ".rdata\0", ".data\0\0"};
            if (std::memcmp(s, names[kind], 8))
                continue;
            const auto& expected = profile.regions[kind];
            require(!found[kind] && rva == expected.rva && size == expected.size &&
                        read<std::uint32_t>(s + 36) == expected.flags,
                    "World module section profile differs");
            found[kind] = true;
        }
    }
    require(found[0] && found[1] && found[2], "World binding module is missing a required section");
    return {base, profile};
}
}
void initialize_build_map_game_system_bindings(NativeDataImage& data) {
    const auto client = world_bindings_checked(data.native_cell<void*>(0x4346A55Cu), client_profile);
    struct List {
        std::uint32_t constructor, append, size, begin, cell;
    };
    constexpr std::array lists{List{0x29628u, 0x2F2FC0u, 0xDF14C0u, 0xDF14B4u, 0x43468D0Cu},
                               List{0x29658u, 0x2F3090u, 0xDF14D4u, 0xDF14C8u, 0x43467A70u}};
    std::array<std::int32_t*, 2> sizes{};
    for (std::size_t index = 0; index < lists.size(); ++index) {
        const auto& list = lists[index];
        client.code(list.constructor, {0xC7, 5, -1, -1, -1, -1, 0, 0, 0, 0, 0xA3, -1, -1, -1, -1});
        sizes[index] =
            reinterpret_cast<std::int32_t*>(client.operand(list.constructor + 2, list.size, 4, 2));
        client.operand(list.constructor + 11, list.begin + 0x10, 4, 2);
        client.code(list.append, {0x55, 0x8B, 0xEC, 0x8B, 0x15, -1,   -1,   -1, -1,   0x8B,
                                  0x0D, -1,   -1,   -1,   -1,   0x8D, 0x42, 1,  0x3B, 0xC1});
        client.operand(list.append + 5, list.size, 4, 2);
        client.operand(list.append + 11, list.begin + 4, 4, 2);
        client.code(list.append + 0x25,
                    {0xA1, -1, -1, -1, -1, 0x42, 0x56, 0x8B, 0x75, 8, 0x89, 0x15, -1, -1, -1, -1});
        client.operand(list.append + 0x26, list.begin, 0x14, 2);
        client.operand(list.append + 0x31, list.size, 4, 2);
    }
    for (std::size_t index = 0; index < lists.size(); ++index)
        data.native_cell<std::int32_t*>(lists[index].cell) = sizes[index];
}
BuildWorldBindings initialize_build_world_bindings(NativeDataImage& data) {
    const auto client = world_bindings_checked(data.native_cell<void*>(0x4346A55C), client_profile);
    const auto panorama = world_bindings_checked(data.native_cell<void*>(0x43469234), panorama_profile);
    // ClientModeCSNormal: actual CRT initializer, constructor stores, live
    // inline vptr and COL/type identity all agree on the same object.
    client.code(0x3F390, {0xB9, -1, -1, -1, -1});
    client.call(0x3F395, 0x3F80A0);
    auto* const mode = client.operand(0x3F391, 0x525E110, 0x34, 2);
    client.code(0x3F8121, {0xC7, 6, -1, -1, -1, -1, 0xC7, 0x46, 4, -1, -1, -1, -1});
    client.operand(0x3F8123, 0xC04980, 0x94, 1);
    client.operand(0x3F812A, 0xC04A70, 4, 1);
    require(read<std::uint32_t>(mode) == reinterpret_cast<std::uintptr_t>(client.base + 0xC04980),
            "ClientModeCSNormal has not been initialized by its real constructor");
    client.pointer(0xC0497C, 0xD61D9C, 20, 1);
    client.pointer(0xD61DA8, 0xE25B60, 32, 2);
    constexpr char type_name[] = ".?AVClientModeCSNormal@@";
    require(read<std::uint32_t>(client.base + 0xD61DA0) == 0 &&
                std::memcmp(client.base + 0xE25B68, type_name, sizeof type_name) == 0,
            "ClientModeCSNormal RTTI differs");
    client.code(0x263150, {0x55, 0x8B, 0xEC, 0xA1, -1, -1, -1, -1, 0xB9, -1, -1, -1, -1});
    client.operand(0x263154, 0x525E110, 4, 2);
    client.operand(0x263159, 0x525E110, 4, 2);
    client.code(0x263165, {0x8B, 0x80, 0x90, 0, 0, 0, 0x57, 0x56, 0xFF, 0xD0});

    // The lazy engine getter returns an object through this slot. Borrow the
    // slot, including its native null state; do not run/skip its allocator.
    client.code(0x27A4A0, {0xA1, -1, -1, -1, -1, 0x85, 0xC0, 0x75, 0x2E, 0xA1});
    auto** const rank = reinterpret_cast<void**>(client.operand(0x27A4A1, 0x5334848, 4, 2));
    client.code(0x27A4AE, {0x68, 0x18, 0x6E, 0, 0, 0x8B, 8, 0x8B, 1, 0xFF, 0x50, 4, 0x85, 0xC0,
                           0x74, 0x0D, 0x8B, 0xC8});
    client.call(0x27A4C0, 0x5A93C0);
    client.code(0x27A4C5, {0xA3, -1, -1, -1, -1, 0xC3, 0xC7, 5, -1, -1, -1, -1, 0, 0, 0, 0, 0x33,
                           0xC0, 0xC3});
    client.operand(0x27A4C6, 0x5334848, 4, 2);
    client.operand(0x27A4CD, 0x5334848, 4, 2);

    client.code(0x32110, {0xA1, -1, -1, -1, -1, 0xA3, -1, -1, -1, -1,  0xC7,
                          5,    -1, -1, -1, -1, -1,   -1, -1, -1, 0xC3});
    auto** const head =
        reinterpret_cast<GameSystemRegistrationNode**>(client.operand(0x32111, 0x5334E08, 4, 2));
    client.operand(0x32116, 0xDF7AE0, 4, 2);
    client.operand(0x3211C, 0x5334E08, 4, 2);
    client.operand(0x32120, 0xDF7AD8, 12, 2);
    client.code(0x35FD40, {0x56, 0x8B, 0x35, -1, -1, -1, -1, 0x85, 0xF6, 0x74, 0x18});
    client.operand(0x35FD43, 0x5334E08, 4, 2);
    client.code(0x35FD50, {0x8B, 0x4E, 4,    0x85, 0xC9, 0x74, 5,    0x8B, 1,    0xFF, 0x50,
                           4,    0x8B, 0x76, 8,    0x85, 0xF6, 0x75, 0xED, 0x5E, 0xC3});

    // The complete push/pop pair shares the genuine counter and panel stack.
    // Begin uses RET4; the typed source caller must not clean that word again.
    panorama.code(0x3DFE0, {0x55, 0x8B, 0xEC, 0x8B, 0x0D, -1, -1, -1, -1, 0x83, 0xF9, 0x40, 0x7C,
                            0x12, 0x68});
    panorama.operand(0x3DFE5, 0x286FD0, 4, 2);
    panorama.operand(0x3DFEF, 0x1FC660, 1, 1);
    panorama.code(0x3DFF3, {0xFF, 0x15, -1, -1, -1, -1, 0x83, 0xC4, 4, 0x5D, 0xC2, 4, 0});
    panorama.operand(0x3DFF5, 0x1E72D8, 4, 1);
    panorama.code(0x3E000, {0x8B, 0x45, 8,  0x41, 0x89, 0x0D, -1,   -1,   -1, -1, 0x89,
                            4,    0x8D, -1, -1,   -1,   -1,   0x5D, 0xC2, 4,  0});
    panorama.operand(0x3E006, 0x286FD0, 4, 2);
    panorama.operand(0x3E00D, 0x274388, 65 * 4, 2);
    panorama.code(0x3E020, {0xA1, -1, -1, -1, -1, 0x85, 0xC0, 0x7E, 0x11, 0xC7, 4,  0x85, -1,  -1,
                            -1,   -1, 0,  0,  0,  0,    0x48, 0xA3, -1,   -1,   -1, -1,   0xC3});
    panorama.operand(0x3E021, 0x286FD0, 4, 2);
    panorama.operand(0x3E02C, 0x274388, 65 * 4, 2);
    panorama.operand(0x3E036, 0x286FD0, 4, 2);
    BuildWorldBindings result{
        data,
        mode,
        rank,
        head,
        reinterpret_cast<script::PanoramaBeginEngineCall>(panorama.base + 0x3DFE0),
        reinterpret_cast<script::PanoramaEndEngineCall>(panorama.base + 0x3E020)};
    // Publish only after both modules and every real dependency have passed.
    data.native_cell<void*>(0x4346D664) = result.rank_context;
    data.native_cell<void**>(0x43468250) = result.rank_data;
    data.native_cell<GameSystemRegistrationNode**>(0x43468D50) = result.registry_head;
    data.native_cell<script::PanoramaBeginEngineCall>(0x4346D628) = result.begin_engine_call;
    data.native_cell<script::PanoramaEndEngineCall>(0x4346D62C) = result.end_engine_call;
    return result;
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: client_class_factory_hooks.cpp
// ============================================================================

namespace skeet::engine {
namespace {
ClientClassFactoryRuntime* active_class_factory_runtime = nullptr;
ClientClassFactoryRuntime& runtime() {
    if (!active_class_factory_runtime)
        throw std::logic_error("ClientClass creation hooks require their actual source runtime");
    return *active_class_factory_runtime;
}
void* create_player(std::int32_t index, std::int32_t serial) {
    return runtime().create(ClientClassFactoryKind::player, index, serial);
}
void* create_ragdoll(std::int32_t index, std::int32_t serial) {
    return runtime().create(ClientClassFactoryKind::ragdoll, index, serial);
}
void* create_incendiary(std::int32_t index, std::int32_t serial) {
    return runtime().create(ClientClassFactoryKind::incendiary, index, serial);
}
void* create_molotov(std::int32_t index, std::int32_t serial) {
    return runtime().create(ClientClassFactoryKind::molotov, index, serial);
}
void* create_viewmodel(std::int32_t index, std::int32_t serial) {
    return runtime().create(ClientClassFactoryKind::viewmodel, index, serial);
}
void* create_inferno(std::int32_t index, std::int32_t serial) {
    return runtime().create(ClientClassFactoryKind::inferno, index, serial);
}
constexpr std::uint32_t hash(const char* name) {
    std::uint32_t result = 0x811C9DC5u;
    for (; *name; ++name)
        result = (result ^ static_cast<std::uint8_t>(*name)) * 0x1000193u;
    return result ^ 0x92CA5DF1u;
}
constexpr std::array<ClientClassFactoryDescriptor, 6> client_class_factory_recipes{
    {{0x480, "CCSPlayer", hash("CCSPlayer"), 0x4336BD6C, 0x43476AEC, create_player},
     {0x48C, "CCSRagdoll", hash("CCSRagdoll"), 0x433B147B, 0x4347695C, create_ragdoll},
     {0x498, "CIncendiaryGrenade", hash("CIncendiaryGrenade"), 0x4336ACFB, 0x43476B1C,
      create_incendiary},
     {0x4A4, "CMolotovGrenade", hash("CMolotovGrenade"), 0x433D5A0F, 0x43476A9C, create_molotov},
     {0x4B0, "CPredictedViewModel", hash("CPredictedViewModel"), 0x433F20C5, 0x43476ADC,
      create_viewmodel},
     {0x4BC, "CInferno", hash("CInferno"), 0x433F8F74, 0x43476A0C, create_inferno}}};
/* [dedup read/write] */
/* [dedup read/write] */
std::ptrdiff_t offset(const NativeOffsetGlobals& globals, std::uint32_t pointer, std::uint32_t key,
                      std::uint32_t salt) {
    return DecodedOffset(globals, pointer, key, salt, 0);
}
}
std::span<const ClientClassFactoryDescriptor, 6> client_class_factory_descriptors() noexcept {
    return client_class_factory_recipes;
}
ClientClassFactoryRuntime::ClientClassFactoryRuntime(client::ClientRuntimeStorage& storage,
                                                     const client::EngineRuntimeBindings& engine,
                                                     client::PlayerRuntimeOwner& players,
                                                     client::EntityCommandRuntimeOwner& entity,
                                                     HookRuntimeOwner& hooks)
    : ClientClassFactoryRuntime(storage, engine, players, entity, hooks.registry(),
                                hooks.descriptor_storage()) {
    if (&hooks.storage() != &storage)
        throw std::invalid_argument("ClientClass hooks require the canonical hook owner");
}
ClientClassFactoryRuntime::ClientClassFactoryRuntime(client::ClientRuntimeStorage& storage,
                                                     const client::EngineRuntimeBindings& engine,
                                                     client::PlayerRuntimeOwner& players,
                                                     client::EntityCommandRuntimeOwner& entity,
                                                     VtableHookRegistry& registry,
                                                     HookDescriptorStorage& descriptors)
    : storage_(storage), players_(players), entity_(entity), registry_(registry),
      descriptors_(descriptors) {
    if (&engine.variables.data() != &storage.data() || &players.storage() != &storage ||
        &entity.storage() != &storage || &descriptors.image() != &storage.data() ||
        !storage.data().native_cell<void*>(0x4346A514u))
        throw std::invalid_argument(
            "ClientClass hooks require the actual engine/player/prediction/descriptor owners");
}
ClientClassFactoryRuntime::~ClientClassFactoryRuntime() {
    if (active_class_factory_runtime == this)
        active_class_factory_runtime = nullptr;
}
void bind_client_class_factory_runtime(ClientClassFactoryRuntime& value) {
    if (active_class_factory_runtime && active_class_factory_runtime != &value)
        throw std::logic_error("ClientClass creation hooks already have a live source runtime");
    active_class_factory_runtime = &value;
}
void ClientClassFactoryRuntime::install(void* receiver, std::uint32_t descriptor) {
    install_vtable_hook(registry_, receiver, descriptors_.at(descriptor));
}
void* ClientClassFactoryRuntime::create(ClientClassFactoryKind kind, std::int32_t index,
                                        std::int32_t serial) {
    const auto recipe_index = static_cast<std::size_t>(kind);
    const auto original_word =
        storage_.data()
            .native_cell<client::EntityUpdatePatch>(client_class_factory_recipes.at(recipe_index).patch_address)
            .after;
    const auto original = reinterpret_cast<ClientClassCreate>(
        static_cast<std::uintptr_t>(std::bit_cast<std::uint32_t>(original_word)));
    auto* result = original(index, serial);
    if (!result)
        return result;
    if (kind == ClientClassFactoryKind::player)
        initialize_player(result, index);
    else if (kind == ClientClassFactoryKind::ragdoll) {
        auto* engine = storage_.data().native_cell<void*>(0x4346A514u);
        if (!invoke<bool>(engine, 82) &&
            !invoke<bool>(storage_.data().native_cell<void*>(0x4346A514u), 93))
            install(result, 0x43475AEC);
    } else {
        auto* unknown = invoke<void*>(result, 0);
        auto* entity = invoke<void*>(unknown, 7);
        constexpr std::array<std::uint32_t, 4> descriptor{0x43475B18, 0x43475A08, 0x43475B34,
                                                          0x43475AC4};
        install(entity, descriptor.at(recipe_index - 2));
    }
    return result;
}
void ClientClassFactoryRuntime::restore_local_movement_flags(void* player) { //433FD037
    auto& data = storage_.data();
    if (data.native_cell<std::int32_t>(0x4347C9ECu) != 2 ||
        !invoke<bool>(data.native_cell<void*>(0x4346A514u), 26))
        return;
    if (*data.native_cell<const std::int32_t*>(0x434758D8u) != -1 &&
        invoke<std::int32_t>(data.native_cell<void*>(0x4346A514u), 205) !=
            data.native_cell<std::int32_t>(0x4346D4B8u))
        return;
    if (invoke<std::int32_t>(data.native_cell<void*>(0x4346A514u), 205) <= 10)
        return;
    const auto command_number = static_cast<std::uint32_t>(invoke<std::int32_t>(
                                    data.native_cell<void*>(0x4346A514u), 205)) -
                                10u;
    const auto* commands = read<const client::Command*>(data.native_cell<void*>(0x4346A894u), 0xF0);
    if (static_cast<std::uint32_t>(commands[command_number % 150u].number) != command_number)
        return;
    const auto* movement = read<const void*>(data.native_cell<void*>(0x4346A7F8u), 8);
    if (!movement)
        return;
    const auto handle = read<std::uint32_t>(movement, 4);
    const auto player_index = handle == UINT32_MAX ? 0x1FFFu : static_cast<std::uint16_t>(handle);
    if (player_index !=
        static_cast<std::uint32_t>(invoke<std::int32_t>(data.native_cell<void*>(0x4346A514u), 12)))
        return;
    const auto flags = read<std::uint32_t>(movement, 0x28);
    if (flags & 0x400000u)
        write(player, offset(storage_.offsets().globals, 0x4346A3E4u, 0x43467AF0u, 0xD1C1093Eu),
              flags);
}
void ClientClassFactoryRuntime::initialize_player(void* networkable,
                                                  std::int32_t index) { //4336BD6C
    auto& data = storage_.data();
    if (invoke<bool>(data.native_cell<void*>(0x4346A514u), 82) ||
        invoke<bool>(data.native_cell<void*>(0x4346A514u), 93))
        return;
    auto* unknown = invoke<void*>(networkable, 0);
    auto* player = invoke<void*>(unknown, 7);
    auto* thinkable = invoke<void*>(unknown, 8);
    auto* owner = players_.cache().get(static_cast<std::uint32_t>(index));
    auto* owner_bytes = reinterpret_cast<std::byte*>(&owner->native);
    if (data.native_cell<std::int32_t>(0x4347C9ECu) == 0) {
        lagcomp::clear_history_records(owner->history, players_.records());
        std::memset(&owner->native, 0, sizeof owner->native);
        owner->history = {};
        owner->publish_history();
        write(owner_bytes, 0xAA, std::uint16_t(0xFFFF));
    }
    const auto& globals = storage_.offsets().globals;
    write(player, offset(globals, 0x43468DC4u, 0x4346895Cu, 0x1E54010Cu), std::uint8_t(0));
    std::array<std::byte, 0x158> info;
    if (invoke<bool>(data.native_cell<void*>(0x4346A514u), 8, index, info.data())) {
        const auto identity =
            data.native_cell<std::uint32_t>(0x4347BE00u + static_cast<std::uint32_t>(index) * 32u);
        if (identity != read<std::uint32_t>(info.data(), 8))
            players_.adjustments()
                .records[static_cast<std::uint32_t>(index)]
                .adjustments.correction_active = 0;
    }
    write(owner_bytes, 0x17C,
          read<std::uint8_t>(player, offset(globals, 0x43468DBCu, 0x434689C8u, 0x8481EA04u)));
    if (data.native_cell<std::int32_t>(0x4347C9ECu) == 0)
        std::memset(owner_bytes + 0x2FC, 0, 0x16C);
    std::memset(owner_bytes + 0x468, 0, 0x16C);
    if (index != invoke<std::int32_t>(data.native_cell<void*>(0x4346A514u), 12)) {
        install(networkable, 0x43475A28);
        install(player, 0x43475830);
        install(thinkable, 0x43475B60);
        if (auto* renderable = invoke<void*>(unknown, 5))
            install(renderable, 0x4347589C);
    } else {
        restore_local_movement_flags(player);
        entity_.initialize_prediction_scratch(true);
        install(networkable, 0x43475B48);
        install(player, 0x43475954);
        install(thinkable, 0x43475878);
        data.native_cell<void*>(0x43475808u) = player;
        if (auto* renderable = invoke<void*>(unknown, 5))
            install(renderable, 0x43475998);
    }
    if (!descriptors_.at(0x43475864).replacement_table)
        return;
    auto& listener_offset = data.native_cell<std::int32_t>(0x43479CE4u);
    if (!listener_offset) {
        auto** table = read<void**>(player, 0);
        const auto* locator = static_cast<const RttiObjectLocator*>(table[-1]);
        for (std::uint32_t i = 0; i < locator->hierarchy->count; ++i) {
            const auto* base = locator->hierarchy->bases[i];
            const auto name_hash = netvar_name_hash(base->type->name);
            if (name_hash == 0x440C879Du || name_hash == 0xC7BCB2A1u) {
                listener_offset = base->member_displacement;
                break;
            }
        }
    }
    if (listener_offset)
        install(static_cast<std::byte*>(player) + listener_offset, 0x43475864);
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: common_controls_initialization.cpp
// ============================================================================

namespace skeet::engine {
// Devirtualized from VM entry 0x4354DB7E: consume three ordinal/destination
// pairs, in operand order 0xEB8..0xECC. The ordinal identities also match the
// actual comctl32 export addresses stored in the working process snapshot.
void initialize_startup_common_controls(CommonControlsExports& imports,
                                        const ForwardedExportEnvironment& environment) {
    auto* module =
        static_cast<std::byte*>(find_loaded_module(environment.modules, 0xc0ba92c9u, true, true));
    if (!module)
        return;
    struct Import {
        std::uint32_t ordinal;
        std::byte** destination;
    };
    const std::array<Import, 3> entries{{{413, &imports.default_subclass_proc},
                                         {410, &imports.set_window_subclass},
                                         {412, &imports.remove_window_subclass}}};
    for (const auto& entry : entries) {
        // The native loop also stores null results and continues with the
        // following pair; failure handling belongs to the enclosing callback.
        *entry.destination =
            resolve_forwarded_export(module, 0, entry.ordinal, true, true, environment);
    }
}
}

// ============================================================================
// Subcomponent: executable_pattern.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup field] */
}
std::byte* executable_pattern(std::byte* module, const std::uint8_t* pattern, std::uint32_t length,
                              std::uint32_t occurrence, std::uint8_t wildcard) {
    if (!module || field<std::uint16_t>(module, 0) != 0x5A4D)
        return nullptr;
    auto* nt = module + field<std::uint32_t>(module, 0x3C);
    if (field<std::uint32_t>(nt, 0) != 0x4550)
        return nullptr;
    const auto count = field<std::uint16_t>(nt, 6);
    auto* section = nt + 0x18 + field<std::uint16_t>(nt, 0x14);
    for (std::uint32_t index = 0; index < count; ++index, section += 0x28) {
        if ((field<std::uint32_t>(section, 0x24) & 0x22000000u) != 0x20000000u)
            continue;
        auto remaining = field<std::uint32_t>(section, 8) - length;
        auto* first = module + field<std::uint32_t>(section, 0xC);
        auto* const boundary = first + remaining;
        for (std::uint32_t found_index = 0;; ++found_index) {
            auto* found = find_byte_pattern(first, remaining, pattern, length, wildcard);
            if (!found)
                break;
            if (found_index == occurrence)
                return found;
            first = found + length;
            remaining = static_cast<std::uint32_t>(boundary - first);
        }
    }
    return nullptr;
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: first_startup_services.cpp
// ============================================================================

namespace skeet::engine {
namespace {
constexpr std::array<FirstStartupServiceSlot, 9> k_first_startup_service_slots{
    {{0x43467F6Cu, 0xE288FE8Cu, "NtOpenFile"},
     {0x4346823Cu, 0x3B0FE868u, "NtCreateFile"},
     {0x434691D4u, 0x957C7302u, "NtQueryInformationFile"},
     {0x43468C08u, 0x83B1BB6Fu, "NtQueryDirectoryFile"},
     {0x4346835Cu, 0xE0F057C2u, "NtSetInformationFile"},
     {0x43469228u, 0xC0532A13u, "NtQueryAttributesFile"},
     {0x4346A318u, 0x8DE5D376u, "NtTerminateProcess"},
     {0x43467228u, 0x5903BC5Fu, "NtMapViewOfSection"},
     {0x43468380u, 0x2C842BEEu, "NtQueryVirtualMemory"}}};
}
std::span<const FirstStartupServiceSlot, 9> first_startup_service_slots() noexcept {
    return k_first_startup_service_slots;
}

void initialize_first_startup_services(const FirstStartupServiceBindings& bindings) {
    if (!bindings.modules.loader)
        throw std::invalid_argument("First startup requires the actual loaded-module list");
    const auto* entry =
        reinterpret_cast<const LoaderModule*>(bindings.modules.loader->modules.next->next);
    if (reinterpret_cast<std::uintptr_t>(entry->base) != 0x77BF0000u)
        return;
    if (!bindings.images.image_nt_header || !bindings.image_directory_entry ||
        !bindings.terminate_process)
        throw std::invalid_argument(
            "First startup requires its real native image and process imports");
    NativeServiceTable table{}; //the original zeroed3004-byte stack object
    initialize_native_service_table(table, *bindings.modules.loader, bindings.images,
                                    bindings.image_directory_entry);
    for (const auto& slot : k_first_startup_service_slots) {
        auto& value = bindings.data.native_cell<std::uint32_t>(slot.address);
        value = resolve_native_service_number(table, value, bindings.terminate_process);
    }
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: focus_overlay_hooks.cpp
// ============================================================================

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_FOCUS_ABI __fastcall
#else
#define SKEET_FOCUS_ABI
#endif
namespace skeet::engine {
namespace {
using SourceEntry = void(SKEET_FOCUS_ABI*)(void*, void*);
FocusOverlayHooks* active_focus_overlay_hooks;
void SKEET_FOCUS_ABI first_entry(void* receiver, void*) {
    active_focus_overlay_hooks->first_invocation(receiver);
}
void SKEET_FOCUS_ABI initialize_entry(void*, void*) {
    active_focus_overlay_hooks->initialize();
}
void SKEET_FOCUS_ABI draw_entry(void* receiver, void*) {
    active_focus_overlay_hooks->draw(receiver);
}
/* [dedup read/write] */
/* [dedup require] */
bool in_region(const std::byte* module, const void* pointer, std::size_t size, std::uint32_t rva,
               std::uint32_t extent) {
    const auto address = reinterpret_cast<std::uintptr_t>(pointer);
    const auto begin = reinterpret_cast<std::uintptr_t>(module) + rva;
    return address >= begin && size <= extent && address - begin <= extent - size;
}
const std::uint32_t& accent(client::ClientRuntimeStorage& storage) {
    // Private persistence descriptors are unlinked before focus publication.
    // Rendering borrows the same live color cell as the actual menu widget.
    return storage.menu_startup_data().settings_values().menu_color;
}
VtableHookMetadata& installed_descriptor(void* receiver) {
    return *static_cast<VtableHookMetadata*>((*static_cast<void***>(receiver))[-2]);
}
}

void* find_build_focus_overlay_receiver(const NativeDataImage& data,
                                        const VtableHookMetadata& descriptor) {
    auto* module = data.native_cell<std::byte*>(0x43468D20);
    require(module && read<std::uint16_t>(module) == 0x5A4D &&
                read<std::uint32_t>(module + 0x3C) == 0x150,
            "Focus overlay requires the loaded engine build");
    const auto* nt = module + 0x150;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint32_t>(nt + 8) == 0x64EE7E86 &&
                read<std::uint32_t>(nt + 80) == 0x39C0000 &&
                read<std::uint32_t>(nt + 88) == 0x64174E,
            "Unsupported focus overlay engine build");
    auto* engine_vgui = data.native_cell<void*>(0x4346712C);
    require(in_region(module, engine_vgui, 0x48, 0x528000, 0x3427638),
            "VEngineVGui001 is outside its actual module data");
    auto** table = *static_cast<void***>(engine_vgui);
    require(in_region(module, table, 40 * sizeof(void*), 0x453000, 0xD4766),
            "VEngineVGui001 vtable is outside engine RTTI data");
    const auto* method = static_cast<const std::byte*>(table[39]);
    require(in_region(module, method, 0x13E, 0x1000, 0x451C32),
            "VEngineVGui001 focus field use is outside executable engine text");
    const auto* code = method + 0x134;
    static constexpr unsigned char field_use[]{0x8B, 0x4B, 0x44, 0x8B, 0x01, 0xFF,
                                               0x90, 0xE8, 0,    0,    0};
    require(!std::memcmp(code, field_use, sizeof field_use),
            "VEngineVGui001 does not contain the proven focus panel field use");
    auto* receiver = read<void*>(static_cast<const std::byte*>(engine_vgui) +
                                 static_cast<unsigned char>(code[2]));
    require(receiver != nullptr, "The engine focus overlay panel has not been created");
    auto** current = *static_cast<void***>(receiver);
    require(current && descriptor.original_table &&
                (current == descriptor.original_table || current == descriptor.replacement_table),
            "Engine focus receiver does not match the canonical CFocusOverlayPanel descriptor");
    return receiver;
}
void bind_focus_overlay_first_entry(VtableHookMetadata& descriptor) noexcept {
    descriptor.replacement_table[131] = reinterpret_cast<void*>(&first_entry);
}

FocusOverlayHooks::FocusOverlayHooks(client::CrtRuntimeOwner& crt,
                                     client::PostInitializationOwner& post,
                                     ui::StartupRenderer& renderer,
                                     client::VisualRuntimeOwner& visual)
    : crt_(crt), storage_(crt.storage()), post_(post), renderer_(renderer), visual_(visual),
      scripts_(post.menu().script_modules()), metadata_(crt.descriptor_storage().at(0x434757C8)),
      unload_(scripts_.unload_state()), accent_(accent(storage_)) {
    auto& data = storage_.data();
    if (&post.data() != &data || &visual.storage() != &storage_ ||
        &visual.startup_renderer() != &renderer ||
        &renderer.dpi() != &storage_.menu_startup_data().dpi ||
        data.native_cell<ui::RendererContainer*>(0x4347690C) != &renderer.renderer() ||
        data.native_cell<ui::RootMenu*>(0x43476924) != &post.menu())
        throw std::invalid_argument(
            "Focus hooks require the canonical source menu, renderer and startup data");
    if (active_focus_overlay_hooks && active_focus_overlay_hooks != this)
        throw std::logic_error("A canonical focus hook owner already exists");
    active_focus_overlay_hooks = this;
    // Static native producer:434675D0=4341F57F;43467F78=43466D60.
    // Loader extra() and the mapped working dump prove that43466D60's time
    // guard branches unconditionally to ordinary4338A69A. Bind those actual
    // source operations, without copying either native executable address.
    data.native_cell<SourceEntry>(0x434675D0) = &initialize_entry;
    data.native_cell<SourceEntry>(0x43467F78) = &draw_entry;
}
FocusOverlayHooks::~FocusOverlayHooks() {
    restore();
    if (active_focus_overlay_hooks == this) {
        auto& before = storage_.data().native_cell<SourceEntry>(0x434675D0);
        auto& after = storage_.data().native_cell<SourceEntry>(0x43467F78);
        if (before == &initialize_entry)
            before = nullptr;
        if (after == &draw_entry)
            after = nullptr;
        active_focus_overlay_hooks = nullptr;
    }
}
void FocusOverlayHooks::prepare() {
    if (!metadata_.replacement_table) {
        auto* module = storage_.data().native_cell<std::byte*>(0x43468D20);
        crt_.descriptors().prepare(
            metadata_, resolve_rtti_vtable(module, 0xBA6192DD, 0, crt_.descriptors().images()));
    }
    require(metadata_.replacement_table && metadata_.original_table,
            "Focus descriptor preparation failed");
    bind_focus_overlay_first_entry(metadata_);
}
void FocusOverlayHooks::install() {
    if (installed_)
        throw std::logic_error("The focus overlay hook is already installed");
    prepare();
    auto* receiver = find_build_focus_overlay_receiver(storage_.data(), metadata_);
    metadata_.target = receiver;
    unload_.first_patch = {static_cast<void**>(receiver), metadata_.original_table,
                           reinterpret_cast<std::uintptr_t>(metadata_.replacement_table)};
    std::atomic_ref<void**>(*static_cast<void***>(receiver)).exchange(metadata_.replacement_table);
    installed_ = true;
}
void FocusOverlayHooks::restore() {
    if (!installed_)
        return;
    installed_ = false;
    auto& patch = unload_.first_patch;
    if (!patch.enabled)
        return;
    // Normal unload restores the canonical patch before CRT releases its
    // cloned table. The engine receiver can retire before this outer owner.
    auto** replacement = metadata_.replacement_table;
    if (!replacement)
        return;
    if (patch.target != metadata_.target || patch.saved != metadata_.original_table ||
        patch.enabled != reinterpret_cast<std::uintptr_t>(replacement))
        return;
    auto* expected = static_cast<void*>(replacement);
    std::atomic_ref<void*>(*patch.target).compare_exchange_strong(expected, patch.saved);
    patch.enabled = 0;
}
void dispatch_focus_first_invocation(NativeDataImage& data, void* receiver) {
    auto& first = data.native_cell<SourceEntry>(0x434675D0);
    const auto before = first;
    first = nullptr;
    auto& second = data.native_cell<SourceEntry>(0x43467F78);
    const auto after = second;
    second = nullptr;
    before(receiver, nullptr);
    //43416A0F rereads the receiver after post-initialization may run callbacks.
    auto& current = installed_descriptor(receiver);
    std::atomic_ref<void*>(current.replacement_table[131]).exchange(reinterpret_cast<void*>(after));
    after(receiver, nullptr);
}
void FocusOverlayHooks::first_invocation(void* receiver) {
    dispatch_focus_first_invocation(storage_.data(), receiver);
}
void FocusOverlayHooks::initialize() {
    post_.runtime().initialize();
}
void FocusOverlayHooks::draw(void* receiver) {
    auto& data = storage_.data();
    // Source HUD bindings borrow semantic screen/icon views. Do not touch a
    // released renderer during the native blocked/null-context rejection.
    if (!data.native_cell<std::int32_t>(0x4347C9FC) &&
        data.native_cell<ui::RendererContainer*>(0x4347690C) &&
        renderer_.renderer().backend_if_alive())
        visual_.refresh();
    const hud::CompositeFrameContext context{data.native_cell<ui::RendererContainer*>(0x4347690C),
                                             data.native_cell<std::int32_t>(0x4347C9FC),
                                             renderer_.dpi().active_scale,
                                             data.native_cell<void*>(0x4346A514),
                                             data.native_cell<void*>(0x4346A890),
                                             storage_.owner_registries().at(0x43475A9C),
                                             scripts_.bridge(),
                                             data.native_cell<ui::MenuWindow*>(0x4347691C),
                                             accent_};
    (void)hud::draw_composite_frame(context);
    reinterpret_cast<MemberFunction<void>>(installed_descriptor(receiver).original_table[131])(
        receiver);
}
}
#undef SKEET_FOCUS_ABI

// ============================================================================
// Subcomponent: forwarded_exports.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
namespace {
/* [dedup read/write] */
std::uint32_t module_hash(const char* name) noexcept { // 0x4334DEFF
    std::uint32_t hash = 0x811c9dc5;
    for (auto value = static_cast<std::uint8_t>(*name); value;
         value = static_cast<std::uint8_t>(*++name)) {
        if (static_cast<std::uint8_t>(value - 'A') <= 'Z' - 'A')
            value += 'a' - 'A';
        hash = (hash ^ value) * 0x1000193;
    }
    return hash ^ 0x92ca5df1;
}
std::uint32_t export_string_hash(const ExportAnsiString& name) noexcept { // 0x4333D6A2
    std::uint32_t hash = 0x811c9dc5;
    for (std::uint32_t i = 0; i != name.length; ++i)
        hash = (hash ^ static_cast<std::uint8_t>(name.buffer[i])) * 0x1000193;
    return hash ^ 0x92ca5df1;
}

std::byte* resolve_directory_export(std::byte* module, std::uint32_t hash,
                                    std::uint32_t ordinal_token, std::byte* directory,
                                    std::uint32_t directory_size, bool salted_hash,
                                    bool lock_loader,
                                    const ForwardedExportEnvironment& environment) {
    // Native 0x433EC666 keeps the selected export index in a 16-bit value.
    std::uint16_t index;
    if (ordinal_token & 0x80000000u) {
        index = static_cast<std::uint16_t>(ordinal_token - read<std::uint16_t>(directory + 0x10));
    } else {
        const auto wanted = salted_hash ? hash ^ 0x92ca5df1u : hash;
        const auto names = module + read<std::uint32_t>(directory + 0x20);
        const auto ordinals = module + read<std::uint32_t>(directory + 0x24);
        const auto count = read<std::uint32_t>(directory + 0x18);
        index = 0xffff;
        for (std::uint32_t i = 0; i != count; ++i) {
            const auto name =
                reinterpret_cast<const char*>(module + read<std::uint32_t>(names + i * 4));
            if ((netvar_name_hash(name) ^ 0x92ca5df1u) == wanted) {
                index = read<std::uint16_t>(ordinals + i * 2);
                break;
            }
        }
    }
    if (index >= read<std::uint32_t>(directory + 0x14))
        return nullptr;
    const auto functions = module + read<std::uint32_t>(directory + 0x1c);
    const auto function_rva = read<std::uint32_t>(functions + index * 4);
    auto* const function = module + function_rva;
    // The native lower bound is strict: the directory's first byte is not
    // classified as a forwarder, unlike an address strictly inside it.
    if (function <= directory || function >= directory + directory_size)
        return function_rva ? function : nullptr;

    const auto* forwarder = reinterpret_cast<const char*>(function);
    const auto* separator = std::strchr(forwarder, '.');
    const auto module_length = static_cast<std::uint16_t>(separator - forwarder);
    std::string target_name(forwarder, module_length);
    target_name += ".dll";
    auto* target = static_cast<std::byte*>(find_loaded_module(
        environment.modules, module_hash(target_name.c_str()), true, lock_loader));
    if (!target)
        return nullptr;

    ExportAnsiString symbol{};
    environment.initialize_string(&symbol, forwarder + module_length + 1);
    std::uint32_t target_ordinal = 0;
    std::uint32_t target_hash = 0;
    if (symbol.length > 1 && symbol.buffer[0] == '#') {
        if (environment.ordinal_from_string(symbol.buffer + 1, 0, &target_ordinal) < 0)
            return nullptr;
    } else {
        target_hash = export_string_hash(symbol);
    }
    return resolve_forwarded_export(target, target_hash, target_ordinal, true, lock_loader,
                                    environment);
}
}

std::byte* resolve_forwarded_export(std::byte* module, std::uint32_t hash, std::uint32_t ordinal,
                                    bool salted_hash, bool lock_loader,
                                    const ForwardedExportEnvironment& environment) { // 0x43320B0A
    if (!hash && !ordinal)
        return nullptr;
    const auto ordinal_token = hash ? 0u : ordinal | 0x80000000u;
    std::uint32_t directory_size = ordinal;
    auto* directory = environment.image_directory_entry(module, 1, 0, &directory_size);
    if (!directory)
        return nullptr;
    return resolve_directory_export(module, hash, ordinal_token, directory, directory_size,
                                    salted_hash, lock_loader, environment);
}

ForwardedExportEnvironment native_forwarded_export_environment() {
#if defined(_WIN32)
    const auto native = GetModuleHandleA("ntdll.dll");
    if (!native)
        throw std::runtime_error("ntdll.dll is not loaded");
    const auto directory = GetProcAddress(native, "RtlImageDirectoryEntryToData");
    const auto string = GetProcAddress(native, "RtlInitAnsiString");
    const auto ordinal = GetProcAddress(native, "RtlCharToInteger");
    if (!directory || !string || !ordinal)
        throw std::runtime_error("Native forwarded-export dependency is missing");
    return {native_loaded_module_environment(), reinterpret_cast<ImageDirectoryEntry>(directory),
            reinterpret_cast<InitializeExportString>(string),
            reinterpret_cast<ExportOrdinalFromString>(ordinal)};
#else
    throw std::runtime_error("Native forwarded-export resolution requires Windows");
#endif
}
}

// ============================================================================
// Subcomponent: frame_update.cpp
// ============================================================================

namespace skeet::engine {
namespace {
FrameUpdateRuntime* frame_runtime;
}
FrameUpdateEndpoint* register_frame_update(OwnerRegistryBinding owners,
                                           FrameUpdateEndpoint* endpoint) {
    void* value =
        reinterpret_cast<std::uintptr_t>(endpoint) + std::uintptr_t{4} == 0 ? nullptr : endpoint;
    append_owner(owners.registry, &value, owners.memory);
    return endpoint;
}
void dispatch_frame_update(const OwnerRegistry& owners, std::int32_t& counter,
                           std::uint8_t update) {
    if (update) {
        auto next = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(counter) - 1u);
        if (next < 0)
            next = 0;
        counter = next;
    }
    const auto entries = owners.entries();
    for (auto* object : entries) {
        auto* endpoint = static_cast<FrameUpdateEndpoint*>(object);
        endpoint->vtable[0](object, update);
    }
}
void bind_frame_update_runtime(FrameUpdateRuntime& runtime) noexcept {
    frame_runtime = &runtime;
}
#if defined(_M_IX86) || defined(__i386__)
void __stdcall frame_update_hook(std::uint8_t update) {
#else
void frame_update_hook(std::uint8_t update) {
#endif
    dispatch_frame_update(frame_runtime->callbacks, frame_runtime->counter, update);
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: game_system.cpp
// ============================================================================

namespace skeet::engine {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SYSTEM_MEMBER __thiscall
#else
#define SYSTEM_MEMBER
#endif
using namespace game_system_detail;
GameSystemRuntime& runtime(void* endpoint) {
    return *owner_of<GameSystemOwner>(static_cast<GameSystemEndpoint*>(endpoint),
                                      offsetof(GameSystemOwner, endpoint))
                .runtime;
}
void SYSTEM_MEMBER game_system_empty(void*) {} //433CCE8E
bool SYSTEM_MEMBER false0(void*) {
    return false;
} //43408189
bool SYSTEM_MEMBER game_system_true0(void*) {
    return true;
} //433422C6
bool SYSTEM_MEMBER false1(void*, std::uintptr_t) {
    return false;
} //43336939, RET4
bool SYSTEM_MEMBER true1(void*, std::uintptr_t) {
    return true;
} //4334C25E, RET4
void SYSTEM_MEMBER empty2(void*, std::uintptr_t, std::uintptr_t) {} //4334A269, RET8
bool SYSTEM_MEMBER false2(void*, std::uintptr_t, std::uintptr_t) {
    return false;
} //43382057
bool SYSTEM_MEMBER true3(void*, std::uintptr_t, std::uintptr_t, std::uintptr_t) {
    return true;
} //433ADABB
std::uintptr_t SYSTEM_MEMBER identity(void*, std::uintptr_t value) {
    return value;
} //433E9F3B
std::uintptr_t SYSTEM_MEMBER zero1(void*, std::uintptr_t) {
    return 0;
} //43418593
std::uint32_t SYSTEM_MEMBER zero0(void*) {
    return 0;
} //43336942
const char* SYSTEM_MEMBER game_system_name(void*) {
    static const char empty_name[1] = {0};
    return empty_name;
}
void SYSTEM_MEMBER shutdown(void* endpoint) {
    runtime(endpoint).shutdown.detach();
}
void SYSTEM_MEMBER frame(void* endpoint, std::uint8_t update) {
    auto& state = runtime(endpoint);
    dispatch_frame_update(state.frames, state.frame_counter, update);
}
void SYSTEM_MEMBER owner_phase6(void* endpoint) {
    dispatch_owner_slot_six(runtime(endpoint).shutdown.owners);
}
void SYSTEM_MEMBER owner_phase8(void* endpoint) {
    dispatch_owner_slot_eight(runtime(endpoint).shutdown.owners);
}
void SYSTEM_MEMBER update2(void* endpoint) {
    network::dispatch_update_phase2(runtime(endpoint).updates);
}
void SYSTEM_MEMBER update3(void* endpoint) {
    network::dispatch_update_phase3(runtime(endpoint).updates);
}
void SYSTEM_MEMBER update0(void* endpoint) {
    auto& state = runtime(endpoint);
    network::dispatch_update_phase0(state.updates, state.update_stage);
}
void SYSTEM_MEMBER update1(void* endpoint) {
    auto& state = runtime(endpoint);
    network::dispatch_update_phase1(state.updates, state.update_stage);
}
void SYSTEM_MEMBER update4(void* endpoint) {
    network::dispatch_update_phase4(runtime(endpoint).updates);
}
void SYSTEM_MEMBER camera_capture(void* endpoint, std::uintptr_t, std::uintptr_t,
                                  const std::int32_t* width, const std::int32_t* height) {
    runtime(endpoint).capture_camera(width, height);
}
bool SYSTEM_MEMBER view(void* endpoint, float* origin, float* angles, float* fov) {
    return effects::dispatch_view(runtime(endpoint).updates, origin, angles, fov);
}
void SYSTEM_MEMBER before(void* endpoint, std::int32_t stage) {
    runtime(endpoint).before_stage(stage);
}
void SYSTEM_MEMBER after(void* endpoint, std::int32_t stage) {
    runtime(endpoint).after_stage(stage);
}
void SYSTEM_MEMBER pure_files(void* endpoint) {
    runtime(endpoint).apply_pure_file_override();
}
const GameSystemVtable system_table{game_system_name,
                                    false0,
                                    shutdown,
                                    false1,
                                    false1,
                                    game_system_empty,
                                    game_system_empty,
                                    true1,
                                    empty2,
                                    frame,
                                    {game_system_empty, game_system_empty, game_system_empty, game_system_empty, game_system_empty, game_system_empty, game_system_empty, game_system_empty},
                                    identity,
                                    zero1,
                                    owner_phase6,
                                    game_system_empty,
                                    owner_phase8,
                                    game_system_empty,
                                    update2,
                                    update3,
                                    camera_capture,
                                    view,
                                    false1,
                                    game_system_true0,
                                    false0,
                                    game_system_true0,
                                    false0,
                                    zero1,
                                    game_system_empty,
                                    game_system_empty,
                                    false2,
                                    true3,
                                    update0,
                                    update1,
                                    game_system_empty,
                                    game_system_empty,
                                    before,
                                    after,
                                    pure_files,
                                    update4,
                                    zero0};
void SYSTEM_MEMBER destroy_owner(void* object) {
    auto* owner = static_cast<GameSystemOwner*>(object);
    const auto memory = owner->memory;
    owner->~GameSystemOwner();
    release(object, memory);
}
void SYSTEM_MEMBER install_owner(void* object) {
    static_cast<GameSystemOwner*>(object)->install();
}
void SYSTEM_MEMBER remove_owner(void* object) {
    static_cast<GameSystemOwner*>(object)->remove();
}
const std::array<GameSystemEntry<void>, 9> game_system_owner_table{
    destroy_owner, game_system_empty, game_system_empty, install_owner, remove_owner, game_system_empty, game_system_empty, game_system_empty, game_system_empty};

std::int32_t add(std::int32_t a, std::int32_t b) {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(a) +
                                       static_cast<std::uint32_t>(b));
}
std::int32_t subtract(std::int32_t a, std::int32_t b) {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(a) -
                                       static_cast<std::uint32_t>(b));
}
void grow(GameSystemVector& vector, std::int32_t amount, void* allocator) { //4337CF0C
    if (vector.grow_size < 0)
        return;
    const auto requested = add(vector.capacity, amount);
    std::int32_t capacity;
    if (vector.grow_size) { //433C050D, signed IDIV rounding
        const auto preceding = subtract(requested, 1);
        capacity = add(subtract(preceding, preceding % vector.grow_size), vector.grow_size);
    } else {
        capacity = vector.capacity ? vector.capacity : 8; //(31+native sizeof(pointer))/4
        while (capacity < requested)
            capacity = add(capacity, capacity);
    }
    if (capacity < requested) {
        if (!capacity && requested <= -1)
            capacity = -1;
        else
            do {
                const auto sum = add(capacity, requested);
                capacity = add(sum, sum < 0 ? 1 : 0) >> 1;
            } while (capacity < requested);
    }
    vector.capacity = capacity; //published before the allocator call, including failure
    const auto bytes =
        static_cast<std::uint32_t>(capacity) * std::uint32_t(sizeof(GameSystemEndpoint*));
    vector.data = vector.data ? invoke<GameSystemEndpoint**>(allocator, 3, vector.data, bytes)
                              : invoke<GameSystemEndpoint**>(allocator, 1, bytes);
}
void append(GameSystemVector& vector, GameSystemEndpoint* const* value,
            void* allocator) { //433F4786
    const auto insertion = vector.size;
    auto count = vector.size;
    if (add(insertion, 1) > vector.capacity) {
        grow(vector, add(subtract(insertion, vector.capacity), 1), allocator);
        count = vector.size; //the genuine allocator may reenter the manager
    }
    vector.size = add(count, 1);
    const auto moved = subtract(count, insertion);
    vector.elements = vector.data;
    if (moved > 0)
        std::memmove(vector.data + insertion + 1, vector.data + insertion,
                     static_cast<std::size_t>(moved) * sizeof(GameSystemEndpoint*));
    vector.data[insertion] = *value; //read the caller's pointer after all allocation/copy
}
bool remove(GameSystemVector& vector, GameSystemEndpoint* const* value) { //433B9059
    auto count = vector.size;
    if (count <= 0)
        return false;
    const auto target = *value;
    std::int32_t index = 0;
    while (vector.data[index] != target)
        if (++index >= count)
            return false;
    const auto moved = subtract(subtract(count, index), 1);
    if (moved > 0) {
        std::memmove(vector.data + index, vector.data + index + 1,
                     static_cast<std::size_t>(moved) * sizeof(GameSystemEndpoint*));
        count = vector.size;
    }
    vector.size = subtract(count, 1);
    return true;
}
float multiply(float first, float second) {
    return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(first), _mm_set_ss(second)));
}
}

GameSystemRuntime::GameSystemRuntime(client::ShutdownRuntime& shutdown_runtime,
                                     const OwnerRegistry& update_registry,
                                     const OwnerRegistry& frame_registry, std::int32_t& counter,
                                     std::int32_t& stage, GameSystemBindings native,
                                     GameSystemCameraState state)
    : shutdown(shutdown_runtime), updates(update_registry), frames(frame_registry),
      frame_counter(counter), update_stage(stage), bindings(native), view_state(state) {}

void GameSystemRuntime::capture_camera(const std::int32_t* width, const std::int32_t* height) {
    if (!bindings.view_provider)
        return;
    auto* setup = invoke<void*>(bindings.view_provider, 12, -1);
    if (!setup)
        return;
    auto* engine = bindings.engine;
    // Six MOVSD copies precede the FOV load and the external aspect-ratio call.
    const auto* bytes = static_cast<const std::byte*>(setup);
    for (std::ptrdiff_t i = 0; i < 3; ++i)
        std::memcpy(&view_state.origin[i], bytes + 0xB8 + i * 4, 4);
    for (std::ptrdiff_t i = 0; i < 3; ++i)
        std::memcpy(&view_state.angles[i], bytes + 0xC4 + i * 4, 4);
    std::memcpy(&view_state.field_of_view, bytes + 0xB0, 4);
    const auto table = *static_cast<void***>(engine);
    const auto aspect_entry =
        reinterpret_cast<MemberFunction<float, std::int32_t, std::int32_t>>(table[101]);
    const auto screen_height = *height;
    const auto screen_width = *width;
    const auto aspect = aspect_entry(engine, screen_width, screen_height);
    const auto aspect_scale = static_cast<float>(static_cast<long double>(aspect) * 0.75L);
    auto angle = multiply(field<float>(setup, 0xB0), std::bit_cast<float>(0x3C8EFA35u));
    angle = multiply(angle, 0.5f);
    angle = bindings.tangent(angle);
    angle = multiply(angle, aspect_scale);
    angle = bindings.arctangent(angle);
    angle = multiply(angle, std::bit_cast<float>(0x42652EE0u));
    view_state.aspect_field_of_view =
        _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(angle), _mm_set_ss(angle)));
}
void GameSystemRuntime::capture_projection() {
    auto* matrix = invoke<const float*>(bindings.engine, 37); //433C0939
    if (!matrix)
        return;
    std::int32_t width = 0, height = 0;
    invoke<void>(bindings.engine, 5, &width, &height);
    // Four original MOVUPS loads precede all destination stores.
    std::array<float, 16> snapshot;
    std::memcpy(snapshot.data(), matrix, sizeof snapshot);
    std::memcpy(view_state.projection.matrix.data(), snapshot.data(), sizeof snapshot);
    const auto half = [](std::int32_t dimension) {
        const auto value = _mm_cvtepi32_ps(_mm_cvtsi32_si128(dimension));
        return _mm_cvtss_f32(_mm_mul_ss(value, _mm_set_ss(0.5f)));
    };
    view_state.projection.horizontal_scale = half(width);
    view_state.projection.vertical_scale = half(height);
}
void GameSystemRuntime::before_stage(std::int32_t stage) {
    if (stage == 2)
        capture_projection();
    else if (stage == 1) {
        const auto& patch = shutdown.hooks.state.first_patch;
        if (patch.enabled && patch.target)
            std::atomic_ref<void*>(*patch.target)
                .exchange(reinterpret_cast<void*>(patch.enabled), std::memory_order_seq_cst);
    }
}
void GameSystemRuntime::after_stage(std::int32_t stage) {
    if (stage != 1)
        return;
    const auto& patch = shutdown.hooks.state.first_patch;
    if (patch.enabled && patch.target)
        std::atomic_ref<void*>(*patch.target).exchange(patch.saved, std::memory_order_seq_cst);
}
void GameSystemRuntime::apply_pure_file_override() {
    if (!bindings.disable_pure.enabled.value)
        return;
    auto* object = bindings.filesystem;
    const auto table = *static_cast<void***>(object);
    const auto* instruction = table[128];
    if (!instruction || field<std::uint16_t>(instruction, 0) != 0x818Bu)
        return;
    const auto displacement = field<std::uint32_t>(instruction, 2);
    if (displacement - 0xC0u > 0x5Fu)
        return;
    const std::uint32_t enabled = 1;
    std::memcpy(static_cast<std::byte*>(object) + displacement, &enabled, 4);
}

void GameSystemOwner::install() {
    if (!manager)
        return;
    installed = this;
    saved_index = manager->current;
    auto* value = &endpoint;
    append(manager->systems, &value, *allocator);
    manager->current = subtract(manager->systems.size, 1);
}
void GameSystemOwner::remove() {
    if (!installed || manager->systems.size == 0)
        return;
    manager->current = saved_index;
    auto* value = &endpoint;
    engine::remove(manager->systems, &value);
    // Native leaves installed/saved_index and the unused tail slot untouched.
}
GameSystemOwner* create_game_system_owner(OwnerRegistryBinding owners, GameSystemManager* manager,
                                          void* const* allocator, GameSystemRuntime& state) {
    constexpr auto game_system_owner_size = sizeof(GameSystemOwner) > 0x44u ? sizeof(GameSystemOwner) : 0x44u;
    auto* storage = allocate(owners.memory, game_system_owner_size);
    if (!storage)
        return nullptr;
    auto* owner = ::new (storage) GameSystemOwner;
    owners.append(owner);
    owner->vtable = game_system_owner_table.data();
    owner->manager = manager;
    owner->saved_index = -1;
    owner->installed = nullptr;
    owner->endpoint.vtable = &system_table;
    owner->runtime = &state;
    owner->allocator = allocator;
    owner->memory = owners.memory;
    return owner;
}
#undef SYSTEM_MEMBER
} //namespace skeet::engine

// ============================================================================
// Subcomponent: game_system_patch.cpp
// ============================================================================

namespace skeet::engine {
// Devirtualized from VM entry 0x4351CF96; both null and nonnull paths verified.
bool capture_game_system_patch_target(void**& target_slot, void** candidate) noexcept {
    if (!candidate)
        return false;
    target_slot = candidate;
    return true;
}
}

// ============================================================================
// Subcomponent: game_system_registration.cpp
// ============================================================================

namespace skeet::engine {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SYSTEM_MEMBER __thiscall
#else
#define SYSTEM_MEMBER
#endif
using namespace game_system_detail;
void SYSTEM_MEMBER reg_system_empty(void*) {} //433CCE8E
void SYSTEM_MEMBER empty1(void*, std::uintptr_t) {} //433C95B7
bool SYSTEM_MEMBER reg_system_true0(void*) {
    return true;
} //433422C6
void SYSTEM_MEMBER dispatch(void* endpoint, std::uintptr_t first, std::uintptr_t second,
                            std::uintptr_t third, std::uintptr_t fourth) {
    auto& owner = owner_of<GameSystemRegistrationOwner>(
        static_cast<GameSystemRegistrationEndpoint*>(endpoint),
        offsetof(GameSystemRegistrationOwner, endpoint));
    dispatch_registered_client_callbacks(*owner.callbacks, first, second, third, fourth);
}
const GameSystemRegistrationVtable registration_table{reg_system_empty,    reg_system_empty,  empty1,
                                                      dispatch, empty1, reg_system_true0};
void SYSTEM_MEMBER reg_system_destroy(void* object) {
    auto* owner = static_cast<GameSystemRegistrationOwner*>(object);
    const auto memory = owner->memory;
    owner->~GameSystemRegistrationOwner();
    release(object, memory);
}
void SYSTEM_MEMBER install(void* object) {
    static_cast<GameSystemRegistrationOwner*>(object)->install();
}
void SYSTEM_MEMBER reg_system_remove(void* object) {
    static_cast<GameSystemRegistrationOwner*>(object)->remove();
}
const std::array<GameSystemEntry<void>, 9> reg_system_owner_table{reg_system_destroy, reg_system_empty, install, reg_system_empty, reg_system_remove,
                                                       reg_system_empty,   reg_system_empty, reg_system_empty,   reg_system_empty};
}
void dispatch_registered_client_callbacks(const OwnerRegistry& callbacks, std::uintptr_t first,
                                          std::uintptr_t second, std::uintptr_t third,
                                          std::uintptr_t fourth) {
    const auto entries = callbacks.entries(); //native captures both begin and end
    for (auto* object : entries) {
        auto* endpoint = static_cast<RegisteredClientEndpoint*>(object);
        endpoint->vtable[0](object, first, second, third, fourth);
    }
}
void GameSystemRegistrationOwner::install() {
    if (!*head_slot)
        return;
    node.endpoint = &endpoint;
    node.next = **head_slot;
    **head_slot = &node;
    registered = 1;
}
void GameSystemRegistrationOwner::remove() {
    if (unload->unloading || !*head_slot || !registered)
        return;
    registered = 0;
    auto** link = *head_slot;
    for (auto* current = *link; current; current = *link) {
        if (current == &node) {
            *link = node.next;
            node.next = nullptr;
            return;
        }
        link = &current->next;
    }
}
GameSystemRegistrationOwner*
create_game_system_registration(OwnerRegistryBinding owners,
                                GameSystemRegistrationNode** const* head_slot,
                                const OwnerRegistry& callbacks, UnloadState& unload) {
    auto* storage = allocate(owners.memory, sizeof(GameSystemRegistrationOwner));
    if (!storage)
        return nullptr;
    auto* owner = ::new (storage) GameSystemRegistrationOwner;
    owners.append(owner);
    owner->node.endpoint = nullptr;
    owner->node.next = nullptr;
    owner->registered = 0;
    owner->empty_name = 0;
    owner->vtable = reg_system_owner_table.data();
    owner->endpoint.vtable = &registration_table;
    owner->node.name = &owner->empty_name;
    owner->head_slot = head_slot;
    owner->callbacks = &callbacks;
    owner->unload = &unload;
    owner->memory = owners.memory;
    return owner;
}
#undef SYSTEM_MEMBER
} //namespace skeet::engine

// ============================================================================
// Subcomponent: game_window_proxy.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
#define WINDOW_MEMBER __thiscall
#define WINDOW_SCALAR __vectorcall
#define WINDOW_TEXT __fastcall
#else
#define WINDOW_MEMBER
#define WINDOW_SCALAR
#define WINDOW_TEXT
#endif
using WindowOwnerEntry = void(WINDOW_MEMBER*)(void*);
struct GameWindowProxy {
    const std::uintptr_t* table = nullptr;
    void* original = nullptr;
    GameWindowProxyOwner* owner = nullptr;
};
struct GameWindowProxyCells {
    const std::byte* const* guarded_return_site;
    void** const* game_interface_slot;
    std::uintptr_t* original_rtti;
    GameWindowProxyImports imports;
};
namespace {
void WINDOW_MEMBER window_proxy_destroy(void*);
void WINDOW_MEMBER initialize(void*);
void WINDOW_MEMBER restore(void*);
void WINDOW_MEMBER window_proxy_empty(void*) {} //433CCE8E
void WINDOW_SCALAR install(void*, float);
void WINDOW_TEXT map_text_empty(void*, const char*) {} //433CCE8E
const std::array<WindowOwnerEntry, 9> primary_table{window_proxy_destroy, initialize, window_proxy_empty, window_proxy_empty, restore,
                                                    window_proxy_empty,   window_proxy_empty,      window_proxy_empty, window_proxy_empty};
const esp::MapEntityParserTable map_table{install, map_text_empty};
std::array<std::uintptr_t, 26> source_proxy_table();
}

struct GameWindowProxyOwner {
    const WindowOwnerEntry* table = nullptr;
    std::uint32_t untouched_4 = 0;
    esp::MapEntityParserEndpoint maps{};
    const std::byte* continuation = nullptr;
    GameWindowProxy proxy{};
    std::uint32_t untouched_1c = 0;
    void** target = nullptr;
    void* saved = nullptr;
    GameWindowProxy* installed = nullptr;
    // Source context follows the complete native2C-byte object prefix.
    GameWindowProxyCells bindings;
    JobMemory memory;
    std::array<std::uintptr_t, 26> proxy_table;
    GameWindowProxyOwner(const GameWindowProxyRegistries& registries,
                         GameWindowProxyBindings context)
        : bindings{&context.guarded_return_site, &context.game_interface_slot,
                   &context.original_rtti, context.imports},
          memory(registries.lifecycle.memory), proxy_table(source_proxy_table()) {
        registries.lifecycle.append(this);
        esp::register_map_entity_parser(registries.maps, &maps);
        table = primary_table.data();
        maps.vtable = &map_table;
    }
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(GameWindowProxy) == 0xC);
static_assert(offsetof(GameWindowProxyOwner, maps) == 8);
static_assert(offsetof(GameWindowProxyOwner, continuation) == 0xC);
static_assert(offsetof(GameWindowProxyOwner, proxy) == 0x10);
static_assert(offsetof(GameWindowProxyOwner, proxy) + offsetof(GameWindowProxy, original) == 0x14);
static_assert(offsetof(GameWindowProxyOwner, proxy) + offsetof(GameWindowProxy, owner) == 0x18);
static_assert(offsetof(GameWindowProxyOwner, target) == 0x20);
static_assert(offsetof(GameWindowProxyOwner, saved) == 0x24);
static_assert(offsetof(GameWindowProxyOwner, installed) == 0x28);
static_assert(offsetof(GameWindowProxyOwner, bindings) == 0x2C);
#endif

namespace {
void exchange_interface(GameWindowProxyOwner& owner, void** target, void* value) {
    if (!owner.bindings.imports.exchange)
        throw std::logic_error("Game-window proxy requires the genuine InterlockedExchange import");
    const auto raw = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
    owner.bindings.imports.exchange(reinterpret_cast<volatile std::int32_t*>(target),
                                    std::bit_cast<std::int32_t>(raw));
}
void WINDOW_MEMBER window_proxy_destroy(void* object) { //433B1A7A ->433837D0
    if (!object)
        return;
    auto* const owner = static_cast<GameWindowProxyOwner*>(object);
    const auto memory = owner->memory;
    owner->~GameWindowProxyOwner();
    release_owner_storage(memory, object);
}
void WINDOW_MEMBER initialize(void* object) {
    initialize_game_window_proxy(*static_cast<GameWindowProxyOwner*>(object));
}
void WINDOW_MEMBER restore(void* object) {
    restore_game_window_proxy(*static_cast<GameWindowProxyOwner*>(object));
}
void WINDOW_SCALAR install(void* receiver, float) {
    auto* const owner = reinterpret_cast<GameWindowProxyOwner*>(
        static_cast<std::byte*>(receiver) - offsetof(GameWindowProxyOwner, maps));
    install_game_window_proxy(*owner);
}
#if defined(_WIN32)
template <class Function> Function required_export(const char* module_name, const char* name) {
    const auto module = GetModuleHandleA(module_name);
    const auto address = module ? GetProcAddress(module, name) : nullptr;
    if (!address)
        throw std::runtime_error("Required game-window proxy export is missing");
    return reinterpret_cast<Function>(address);
}
#endif
}

// The helper receives the actual caller stack word. The x86 veneer then tail
// jumps into the real virtual9 with its original receiver and argument stack.
// Engine callers provide the Win32 four-byte stack contract. Realign only the
// helper's private frame when the source compiler uses SSE stack alignment.
#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
#define WINDOW_ALIGN __attribute__((force_align_arg_pointer))
#else
#define WINDOW_ALIGN
#endif
extern "C" WINDOW_ALIGN void* __cdecl skeet_game_window_proxy_prepare(void* receiver,
                                                                      std::uintptr_t* return_slot) {
    auto& proxy = *static_cast<GameWindowProxy*>(receiver);
    auto* owner = proxy.owner;
    if (*return_slot == reinterpret_cast<std::uintptr_t>(*owner->bindings.guarded_return_site)) {
        if (owner->target && owner->saved) {
            exchange_interface(*owner, owner->target, owner->saved);
            owner = proxy.owner; //the native entry reloads parent after the import
        }
        *return_slot = reinterpret_cast<std::uintptr_t>(owner->continuation);
    }
    return proxy.original;
}
#undef WINDOW_ALIGN

#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
#define WINDOW_FORWARD(slot, offset)                                                               \
    extern "C" __attribute__((naked)) void skeet_game_window_forward_##slot() {                    \
        __asm__("movl 4(%ecx),%ecx\n\tmovl (%ecx),%eax\n\tjmp *" #offset "(%eax)");                \
    }
extern "C" __attribute__((naked)) void skeet_game_window_slot_nine() {
    __asm__("pushl %esi\n\t"
            "pushl %edi\n\t"
            "leal 8(%esp),%edi\n\t"
            "movl %ecx,%esi\n\t"
            "pushl %edi\n\t"
            "pushl %esi\n\t"
            "call _skeet_game_window_proxy_prepare\n\t"
            "addl $8,%esp\n\t"
            "movl %eax,%ecx\n\t"
            "popl %edi\n\t"
            "popl %esi\n\t"
            "movl (%ecx),%eax\n\t"
            "jmp *36(%eax)");
}
#elif defined(_M_IX86) && defined(_MSC_VER)
#define WINDOW_FORWARD(slot, offset)                                                               \
    extern "C" __declspec(naked) void skeet_game_window_forward_##slot() {                         \
        __asm mov ecx, dword ptr[ecx + 4] __asm mov eax,                                           \
            dword ptr[ecx] __asm jmp dword ptr[eax + offset]                                       \
    }
extern "C" __declspec(naked) void skeet_game_window_slot_nine() {
    __asm {
        push esi
        push edi
        lea edi,[esp+8]
        mov esi,ecx
        push edi
        push esi
        call skeet_game_window_proxy_prepare
        add esp,8
        mov ecx,eax
        pop edi
        pop esi
        mov eax,dword ptr[ecx]
        jmp dword ptr[eax+36]
    }
}
#endif
#if defined(_M_IX86) || defined(__i386__)
WINDOW_FORWARD(0, 0)
WINDOW_FORWARD(1, 4)
WINDOW_FORWARD(2, 8)
WINDOW_FORWARD(3, 12)
WINDOW_FORWARD(4, 16)
WINDOW_FORWARD(5, 20)
WINDOW_FORWARD(6, 24)
WINDOW_FORWARD(7, 28)
WINDOW_FORWARD(8, 32)
WINDOW_FORWARD(10, 40)
WINDOW_FORWARD(11, 44)
WINDOW_FORWARD(12, 48)
WINDOW_FORWARD(13, 52)
WINDOW_FORWARD(14, 56)
WINDOW_FORWARD(15, 60)
WINDOW_FORWARD(16, 64)
WINDOW_FORWARD(17, 68)
WINDOW_FORWARD(18, 72)
WINDOW_FORWARD(19, 76)
WINDOW_FORWARD(20, 80)
WINDOW_FORWARD(21, 84)
WINDOW_FORWARD(22, 88)
WINDOW_FORWARD(23, 92)
WINDOW_FORWARD(24, 96)
#undef WINDOW_FORWARD
#endif

namespace {
std::array<std::uintptr_t, 26> source_proxy_table() {
#if defined(_M_IX86) || defined(__i386__)
#define WINDOW_ENTRY(slot) reinterpret_cast<std::uintptr_t>(&skeet_game_window_forward_##slot)
    return {0,
            WINDOW_ENTRY(0),
            WINDOW_ENTRY(1),
            WINDOW_ENTRY(2),
            WINDOW_ENTRY(3),
            WINDOW_ENTRY(4),
            WINDOW_ENTRY(5),
            WINDOW_ENTRY(6),
            WINDOW_ENTRY(7),
            WINDOW_ENTRY(8),
            reinterpret_cast<std::uintptr_t>(&skeet_game_window_slot_nine),
            WINDOW_ENTRY(10),
            WINDOW_ENTRY(11),
            WINDOW_ENTRY(12),
            WINDOW_ENTRY(13),
            WINDOW_ENTRY(14),
            WINDOW_ENTRY(15),
            WINDOW_ENTRY(16),
            WINDOW_ENTRY(17),
            WINDOW_ENTRY(18),
            WINDOW_ENTRY(19),
            WINDOW_ENTRY(20),
            WINDOW_ENTRY(21),
            WINDOW_ENTRY(22),
            WINDOW_ENTRY(23),
            WINDOW_ENTRY(24)};
#undef WINDOW_ENTRY
#else
    throw std::logic_error("Game-window proxy requires the source x86 tail-transfer ABI");
#endif
}
}

GameWindowProxyImports native_game_window_proxy_imports() {
#if defined(_WIN32)
    return {required_export<GameWindowCrc32>("ntdll.dll", "RtlComputeCrc32"),
            required_export<GameWindowExchange>("kernel32.dll", "InterlockedExchange")};
#else
    throw std::logic_error("Game-window proxy requires genuine Windows imports");
#endif
}
GameWindowProxyBindings native_game_window_proxy_bindings(NativeDataImage& data,
                                                          GameWindowProxyImports imports) {
    return {data.native_cell<const std::byte*>(0x43467288u), data.native_cell<void**>(0x43468B2Cu),
            data.native_cell<std::uintptr_t>(0x43469288u), imports};
}
std::size_t game_window_proxy_owner_size() noexcept {
    return sizeof(GameWindowProxyOwner);
}
GameWindowProxyOwner& construct_game_window_proxy_owner(void* storage,
                                                        const GameWindowProxyRegistries& registries,
                                                        GameWindowProxyBindings bindings) {
    return *::new (storage) GameWindowProxyOwner(registries, bindings);
}
GameWindowProxyOwner* create_game_window_proxy_owner(const GameWindowProxyRegistries& registries,
                                                     GameWindowProxyBindings bindings) {
    auto* const storage = allocate_owner_storage(
        registries.lifecycle.memory, std::max(std::size_t{0x2C}, game_window_proxy_owner_size()));
    if (!storage)
        return nullptr;
    return &construct_game_window_proxy_owner(storage, registries, bindings);
}
void initialize_game_window_proxy(GameWindowProxyOwner& owner) { //4348DD2A
    owner.proxy.owner = &owner;
    const auto& bindings = owner.bindings;
    auto* start = *bindings.guarded_return_site;
    if (!start)
        return;
    if (!bindings.imports.crc32)
        throw std::logic_error("Game-window proxy requires the genuine RtlComputeCrc32 import");
    std::uint32_t offset = 0;
    const std::byte* candidate;
    for (;;) {
        candidate =
            reinterpret_cast<const std::byte*>(reinterpret_cast<std::uintptr_t>(start) + offset);
        if (bindings.imports.crc32(0, candidate, 4) == 0xB94D4C7Eu)
            break;
        ++offset;
        start = *bindings.guarded_return_site;
    }
    owner.continuation = candidate;
    auto* slot = *bindings.game_interface_slot;
    if (!candidate || !slot)
        return;
    const auto* original_table = *static_cast<const std::uintptr_t**>(*slot);
    *bindings.original_rtti = original_table[-1];
    owner.proxy_table[0] = *bindings.original_rtti;
    owner.proxy.original = *slot;
    owner.proxy.table = owner.proxy_table.data() + 1;
    slot = *bindings.game_interface_slot;
    owner.target = slot;
    owner.saved = *slot;
    owner.installed = &owner.proxy;
}
void install_game_window_proxy(GameWindowProxyOwner& owner) { //433689F9, receiver owner+8
    if (owner.target && owner.installed)
        exchange_interface(owner, owner.target, owner.installed);
}
void restore_game_window_proxy(GameWindowProxyOwner& owner) { //4339436B
    if (owner.target && owner.installed && owner.saved)
        exchange_interface(owner, owner.target, owner.saved);
}
#undef WINDOW_MEMBER
#undef WINDOW_SCALAR
#undef WINDOW_TEXT
} // namespace skeet::engine

// ============================================================================
// Subcomponent: hook_descriptors.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
namespace {
HookDescriptorRuntime* active_runtime = nullptr;
void* allocate_table(JobMemory memory, std::size_t size) {
    if (!size)
        return nullptr;
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - address);
    const auto allocation_size = static_cast<std::uint32_t>(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void free_table(JobMemory memory, void* object) {
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}

HookDescriptorSystem native_hook_descriptor_system() {
#if defined(_WIN32)
    const auto native = GetModuleHandleA("ntdll.dll");
    if (!native)
        throw std::runtime_error("ntdll.dll is not loaded");
    const auto query = GetProcAddress(native, "ZwQueryVirtualMemory");
    if (!query)
        throw std::runtime_error("ZwQueryVirtualMemory is not available");
    return {reinterpret_cast<HookQueryMemory>(query)};
#else
    throw std::runtime_error("Native hook descriptor queries require Windows");
#endif
}

void prepare_vtable_hook(VtableHookMetadata& metadata, void** original_table, JobMemory memory,
                         HookDescriptorSystem system) { // 433AB42B
    if (!original_table)
        return;
    std::size_t count = 0;
    while (original_table[count]) {
        HookMemoryRegion region;
        const auto status =
            system.query_memory(reinterpret_cast<void*>(~std::uintptr_t(0)), original_table[count],
                                0, &region, sizeof(region), nullptr);
        if (status < 0 || !(region.protection & 0xf0u))
            break;
        ++count;
    }
    if (!count)
        return;
    auto** const allocation =
        static_cast<void**>(allocate_table(memory, (count + 2) * sizeof(void*)));
    if (!allocation)
        return;
    allocation[0] = &metadata;
    std::memcpy(allocation + 1, original_table - 1, (count + 1) * sizeof(void*));
    metadata.original_table = original_table;
    metadata.replacement_table = allocation + 2;
}

void clear_vtable_hook(VtableHookMetadata& metadata, JobMemory memory) { // 433D2DC1
    void** previous;
#if defined(_WIN32)
    previous = static_cast<void**>(InterlockedExchangePointer(
        reinterpret_cast<void* volatile*>(&metadata.replacement_table), nullptr));
#else
    previous = __atomic_exchange_n(&metadata.replacement_table, static_cast<void**>(nullptr),
                                   __ATOMIC_SEQ_CST);
#endif
    if (previous)
        free_table(memory, previous - 2);
}

void prepare_rtti_vtable_hook(VtableHookMetadata& metadata, std::byte* module,
                              std::uint32_t type_hash, std::uint32_t base_hash,
                              NativeImageEnvironment images, JobMemory memory,
                              HookDescriptorSystem system) {
    prepare_vtable_hook(metadata, resolve_rtti_vtable(module, type_hash, base_hash, images), memory,
                        system);
}

void bind_hook_descriptor_runtime(HookDescriptorRuntime& runtime) noexcept {
    active_runtime = &runtime;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall prepare_vtable_hook_native(VtableHookMetadata* metadata, void*,
                                           void** original_table) {
#else
void prepare_vtable_hook_native(VtableHookMetadata* metadata, void*, void** original_table) {
#endif
    // Native433AB42B: ECX descriptor, one stack argument, RET4.
    active_runtime->prepare(*metadata, original_table);
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall clear_vtable_hook_native(VtableHookMetadata* metadata, void*) {
#else
void clear_vtable_hook_native(VtableHookMetadata* metadata, void*) {
#endif
    // Native433D2DC1: ECX descriptor, no stack arguments.
    active_runtime->clear(*metadata);
}
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_DESCRIPTOR_NATIVE __fastcall
#else
#define SKEET_DESCRIPTOR_NATIVE
#endif
const std::byte* SKEET_DESCRIPTOR_NATIVE find_pe_section_native(const std::byte* nt_header,
                                                                std::uint32_t hash) {
    return find_pe_section(nt_header, hash); //433A1085 ECX/EDX,RET0
}
RttiTypeDescriptor* SKEET_DESCRIPTOR_NATIVE find_rtti_type_native(std::byte* module,
                                                                  std::uint32_t hash) {
    return find_rtti_type(module, hash, active_runtime->images()); //433B305B ECX/EDX,RET0
}
void** SKEET_DESCRIPTOR_NATIVE find_rtti_vtable_native(std::byte* module, RttiTypeDescriptor* type,
                                                       std::uint32_t base_hash) {
    return find_rtti_vtable(module, type, base_hash,
                            active_runtime->images()); //4332CE3A ECX/EDX,RET4
}
void** SKEET_DESCRIPTOR_NATIVE find_rtti_vtable_at_offset_native(std::byte* module,
                                                                 RttiTypeDescriptor* type,
                                                                 std::uint32_t offset) {
    return find_rtti_vtable_at_offset(module, type, offset,
                                      active_runtime->images()); //433EC4BD ECX/EDX,RET4
}
#undef SKEET_DESCRIPTOR_NATIVE
}

// ============================================================================
// Subcomponent: hook_descriptor_storage.cpp
// ============================================================================

namespace skeet::engine {
HookDescriptorStorage::HookDescriptorStorage(PreservedInitialization& initialization)
    : HookDescriptorStorage(initialization.image_owner()) {}
HookDescriptorStorage::HookDescriptorStorage(std::shared_ptr<NativeDataImage> image)
    : image_(image) {
    (void)require_native_data_image(image_);
    if (sizeof(void*) != 4 || sizeof(VtableHookMetadata) != 0x14)
        throw std::logic_error(
            "Native hook descriptor storage requires its original x86 pointer layout");
}

VtableHookMetadata& HookDescriptorStorage::at(std::uint32_t address) const {
    if (std::find(hook_descriptor_cells.begin(), hook_descriptor_cells.end(), address) ==
        hook_descriptor_cells.end())
        throw std::out_of_range("Address is not a recovered hook descriptor cell");
    // No construction, reset, relocation or copy occurs here. Source family
    // preparation and CRT clear functions share the same canonical bytes.
    return image_->native_cell<VtableHookMetadata>(address);
}
}

// ============================================================================
// Subcomponent: hook_runtime_owner.cpp
// ============================================================================

namespace skeet::engine {
namespace {
client::ClientRuntimeStorage& checked_network_storage(client::CrtRuntimeOwner& crt,
                                                      client::MiscRuntimeOwner& owner) {
    auto& storage = crt.storage();
    auto& data = storage.data();
    auto& feature = owner.runtime();
    auto& descriptor = crt.descriptor_storage().at(0x43475B04);
    if (&owner.storage() != &storage || &owner.channel() != &feature.channel ||
        &owner.channel().expected_table != &descriptor.original_table ||
        &owner.channel().replacement_table != &descriptor.replacement_table ||
        &owner.channel().installed_channel != &descriptor.target ||
        &owner.auto_accept_patch().metadata !=
            &crt.descriptor_storage().at(misc::auto_accept_hook_descriptor) ||
        &feature.interfaces.local_player != &data.native_cell<void*>(0x43475808) ||
        feature.interfaces.engine != data.native_cell<void*>(0x4346A514) ||
        feature.ping_spike.latency_4347a508 != &data.native_cell<double>(0x4347A508) ||
        feature.ping_spike.latency_4347b120 != &data.native_cell<double>(0x4347B120))
        throw std::invalid_argument(
            "Channel hooks must borrow the canonical misc feature and descriptor state");
    if (!data.native_cell<misc::PingSpikeNetworkClock>(0x43468C84) ||
        !data.native_cell<const float*>(0x4346A334))
        throw std::invalid_argument(
            "Channel hooks require the actual tier0 clock and engine-time alias");
    return storage;
}
struct NetworkContexts {
    client::ClientRuntimeStorage& storage;
    client::MiscRuntimeOwner& miscellaneous;
    misc::PingSpikeChannelRuntime channel;
    misc::NetworkMessageRuntime messages;
    NetworkContexts(client::CrtRuntimeOwner& crt, client::MiscRuntimeOwner& owner)
        : storage(checked_network_storage(crt, owner)), miscellaneous(owner),
          channel(owner.runtime().ping_spike,
                  {storage.data().native_cell<misc::PingSpikeNetworkHistory>(0x4347A500),
                   storage.data().native_cell<std::uint32_t>(0x4347C9F4),
                   storage.data().native_cell<float>(0x43476928),
                   storage.data().native_cell<std::array<client::StartupAcknowledgement, 150>>(
                       0x43475BE0),
                   storage.data().native_cell<void*>(0x4346A514),
                   storage.data().native_cell<misc::PingSpikeNetworkClock>(0x43468C84)}),
          messages{storage.owner_registries().at(0x43475B94),
                   {storage.data().native_cell<std::uint8_t>(0x43475BD2),
                    storage.data().native_cell<const std::int32_t*>(0x434758D8),
                    storage.data().native_cell<std::uint8_t>(0x4347C9F0),
                    storage.data().native_cell<std::int32_t>(0x4347C9EC),
                    storage.data().native_cell<float>(0x43479924),
                    storage.data().native_cell<const float*>(0x4346A334),
                    storage.data().native_cell<void (*)()>(0x43468BF0)},
                   storage.data().native_cell<const std::byte*>(0x4346A55C)} {}
};
}
struct HookRuntimeOwner::Data {
    client::CrtRuntimeOwner& crt;
    script::SourceScriptRuntimeOwner& scripts;
    ui::StartupRenderer& renderer;
    VtableHookRegistry registry;
    misc::AutoAcceptPatchState auto_accept_patch;
    misc::AutoAcceptHookFamily auto_accept_hooks;
    script::ConsoleInputHook console;
    std::unique_ptr<NetworkContexts> network;
    std::unique_ptr<BuildHookCallbacks> callbacks;
    std::unique_ptr<effects::PlayerAuxiliaryHooks> auxiliary;
    std::unique_ptr<network::ClientStateRuntimeOwner> client_state;
    std::unique_ptr<FocusOverlayHooks> focus;

    Data(client::CrtRuntimeOwner& crt_owner, script::SourceScriptRuntimeOwner& script_owner,
         ui::StartupRenderer& renderer_owner)
        : crt(crt_owner), scripts(script_owner), renderer(renderer_owner),
          registry{script_owner.unload_state().hooks, crt.storage().memory()},
          auto_accept_patch{crt.descriptor_storage().at(misc::auto_accept_hook_descriptor)},
          auto_accept_hooks(
              auto_accept_patch, registry,
              crt.storage().data().native_cell<std::byte*>(misc::auto_accept_hook_source_global),
              crt.descriptors().images(), crt.hook_system()),
          console(crt.descriptor_storage(), script_owner.bridge()) {
        auto& storage = crt.storage();
        auto& image = storage.data();
        if (&scripts.storage() != &storage || &scripts.ui_types() != &crt.ui_types() ||
            image.native_cell<ui::RendererContainer*>(0x4347690C) != &renderer.renderer() ||
            &renderer.dpi() != &storage.menu_startup_data().dpi ||
            &crt.descriptor_storage().image() != &image)
            throw std::invalid_argument(
                "Hook runtime must borrow the canonical script, renderer and descriptor owners");
    }
};
HookRuntimeOwner::HookRuntimeOwner(client::CrtRuntimeOwner& crt,
                                   script::SourceScriptRuntimeOwner& scripts,
                                   ui::StartupRenderer& renderer)
    : data_(std::make_unique<Data>(crt, scripts, renderer)) {}
HookRuntimeOwner::~HookRuntimeOwner() = default;
VtableHookRegistry& HookRuntimeOwner::registry() noexcept {
    return data_->registry;
}
client::ClientRuntimeStorage& HookRuntimeOwner::storage() const noexcept {
    return data_->crt.storage();
}
UnloadState& HookRuntimeOwner::unload_state() const noexcept {
    return data_->scripts.unload_state();
}
HookDescriptorStorage& HookRuntimeOwner::descriptor_storage() noexcept {
    return data_->crt.descriptor_storage();
}
HookDescriptorSystem HookRuntimeOwner::descriptor_system() const noexcept {
    return data_->crt.hook_system();
}
NativeImageEnvironment HookRuntimeOwner::descriptor_images() const noexcept {
    return data_->crt.descriptors().images();
}
misc::AutoAcceptPatchState& HookRuntimeOwner::auto_accept_patch() noexcept {
    return data_->auto_accept_patch;
}
misc::AutoAcceptHookFamily& HookRuntimeOwner::auto_accept_hooks() noexcept {
    return data_->auto_accept_hooks;
}
script::ConsoleInputHook& HookRuntimeOwner::console_input() noexcept {
    return data_->console;
}
script::ScriptOwner& HookRuntimeOwner::script_owner() const {
    return data_->scripts.require_owner();
}
void HookRuntimeOwner::initialize_network(client::MiscRuntimeOwner& miscellaneous) {
    if (data_->network)
        throw std::logic_error("The canonical channel hook contexts already exist");
    data_->network = std::make_unique<NetworkContexts>(data_->crt, miscellaneous);
}
misc::PingSpikeChannelRuntime& HookRuntimeOwner::channel() {
    if (!data_->network)
        throw std::logic_error("The actual misc owner has not initialized channel hooks");
    return data_->network->channel;
}
client::ChannelHookState& HookRuntimeOwner::channel_state() {
    if (!data_->network)
        throw std::logic_error("The actual misc owner has not initialized channel hooks");
    return data_->network->miscellaneous.channel();
}
misc::NetworkMessageRuntime& HookRuntimeOwner::messages() {
    if (!data_->network)
        throw std::logic_error("The actual misc owner has not initialized network messages");
    return data_->network->messages;
}
void HookRuntimeOwner::initialize_callbacks(animation::FixRuntimeOwner& animations) {
    if (data_->callbacks)
        throw std::logic_error("The canonical build hook callbacks already exist");
    if (&animations.storage() != &storage() || &animations.backup().detach.hooks != &registry())
        throw std::invalid_argument(
            "Build hook callbacks must borrow the actual animation and hook owners");
    data_->callbacks = std::make_unique<BuildHookCallbacks>(
        descriptor_storage(), registry(), storage().offsets(), animations.animations(),
        animations.backup(), channel(), data_->renderer.frame());
}
BuildHookCallbacks& HookRuntimeOwner::callbacks() {
    if (!data_->callbacks)
        throw std::logic_error(
            "The actual animation owner has not initialized build hook callbacks");
    return *data_->callbacks;
}
void HookRuntimeOwner::initialize_player_auxiliary(const client::EngineRuntimeBindings& engine,
                                                   client::ConfigurationRuntimeOwner& configuration,
                                                   client::HistoryRuntimeOwner& history,
                                                   animation::RuntimeOwner& animation) {
    if (data_->auxiliary)
        throw std::logic_error("The canonical player auxiliary hook contexts already exist");
    data_->auxiliary = std::make_unique<effects::PlayerAuxiliaryHooks>(
        storage(), engine, configuration, history, animation, *this);
}
effects::PlayerAuxiliaryHooks& HookRuntimeOwner::player_auxiliary() {
    if (!data_->auxiliary)
        throw std::logic_error(
            "The actual history owner has not initialized auxiliary player hooks");
    return *data_->auxiliary;
}
void HookRuntimeOwner::initialize_client_state(const client::EngineRuntimeBindings& engine,
                                               client::ConfigurationRuntimeOwner& configuration,
                                               client::HistoryRuntimeOwner& history,
                                               client::CommandCaptureRuntimeOwner& commands,
                                               script::SourceScriptRuntimeOwner& scripts,
                                               ui::MainMenuWindowOwner& window) {
    if (data_->client_state)
        throw std::logic_error("The canonical ClientState hook contexts already exist");
    data_->client_state = std::make_unique<network::ClientStateRuntimeOwner>(
        storage(), engine, configuration, history, commands, *this, scripts, window);
}
network::ClientStateRuntimeOwner& HookRuntimeOwner::client_state() {
    if (!data_->client_state)
        throw std::logic_error(
            "The actual menu and history owners have not initialized ClientState hooks");
    return *data_->client_state;
}
misc::AutoAcceptOwnerBindings HookRuntimeOwner::auto_accept_owner_bindings() const {
    auto& image = data_->crt.storage().data();
    return {data_->auto_accept_patch,
            image.native_cell<misc::AutoAcceptLifecycleOwner*>(0x43475970),
            image.native_cell<void*>(0x43468E98), image.native_cell<std::byte*>(0x43469234),
            image.native_cell<void*>(0x4346D554)};
}
void HookRuntimeOwner::initialize_focus(client::PostInitializationOwner& post,
                                        client::VisualRuntimeOwner& visual) {
    if (data_->focus)
        throw std::logic_error("The canonical focus hook owner is already initialized");
    if (&post.menu().script_owner() != &data_->scripts.require_owner())
        throw std::invalid_argument(
            "Focus startup must use the hook runtime's actual script owner");
    auto focus = std::make_unique<FocusOverlayHooks>(data_->crt, post, data_->renderer, visual);
    focus->install();
    data_->focus = std::move(focus);
}
FocusOverlayHooks& HookRuntimeOwner::focus() {
    if (!data_->focus)
        throw std::logic_error(
            "The actual post-initialization focus owner has not been constructed");
    return *data_->focus;
}
}

// ============================================================================
// Subcomponent: initialization_failure.cpp
// ============================================================================

namespace skeet::engine {
std::int32_t fail_initialization(InitializationFailureState state,
                                 TerminateInitializationThread terminate_thread,
                                 std::int32_t exit_code) {
    // Native 0x4337F065 reads both image fields before clearing the buffer.
    const auto size = state.image_size;
    const auto base = state.image_base;
    if (size)
        std::memset(reinterpret_cast<void*>(base), 0, size);
    state.requested_mode = 0x100;
    return terminate_thread(reinterpret_cast<void*>(std::intptr_t(-2)), exit_code);
}
}

// ============================================================================
// Subcomponent: interface_discovery.cpp
// ============================================================================

namespace skeet::engine {
namespace {
// Fixed-build interface data; these integers are logical data-cell identifiers.
constexpr std::array<BuildInterfaceRecipe, 41> build_requests{{
    {0x4C8u,
     0x4346A650u,
     0xA90F554Cu,
     0x43468A4Cu,
     0u,
     {"studio_renderer", "studiorender.dll", "VStudioRender026"}},
    {0x4D4u,
     0x4346A55Cu,
     0x66DCB316u,
     0x434683E0u,
     0u,
     {"base_client", "client.dll", "VClient018"}},
    {0x4E0u,
     0x4346A55Cu,
     0x530147E3u,
     0x4346A890u,
     0u,
     {"entity_list", "client.dll", "VClientEntityList003"}},
    {0x4ECu,
     0x4346A55Cu,
     0x2CEEF853u,
     0x43469264u,
     0u,
     {"prediction", "client.dll", "VClientPrediction001"}},
    {0x4F8u,
     0x4346A55Cu,
     0xE620BB3Cu,
     0x4346A7F8u,
     0u,
     {"game_movement", "client.dll", "GameMovement001"}},
    {0x504u,
     0x4346A55Cu,
     0x370B6789u,
     0x43467D84u,
     0u,
     {"game_console", "client.dll", "GameConsole004"}},
    {0x510u,
     0x4346A55Cu,
     0xB49164FAu,
     0x4346A3A0u,
     0u,
     {"client_leaf_system", "client.dll", "ClientLeafSystem002"}},
    {0x51Cu,
     0x434670ACu,
     0x5C0F884Bu,
     0x43468C0Cu,
     0u,
     {"match_framework", "matchmaking.dll", "MATCHFRAMEWORK_001"}},
    {0x528u,
     0x43467DACu,
     0x6A62EA4Cu,
     0x4346A89Cu,
     0u,
     {"input_system", "inputsystem.dll", "InputSystemVersion001"}},
    {0x534u,
     0x43467DACu,
     0xCB2A089Au,
     0x43468D24u,
     0u,
     {"input_stack_system", "inputsystem.dll", "InputStackSystemVersion001"}},
    {0x540u,
     0x43468D20u,
     0xFC552D7Cu,
     0x4346A514u,
     0u,
     {"engine_client", "engine.dll", "VEngineClient014"}},
    {0x54Cu,
     0x43468D20u,
     0x6A2E45E8u,
     0x43468304u,
     0u,
     {"engine_trace", "engine.dll", "EngineTraceClient004"}},
    {0x558u,
     0x43468D20u,
     0xE9DDF952u,
     0x4346A614u,
     0u,
     {"model_info", "engine.dll", "VModelInfoClient004"}},
    {0x564u,
     0x43468D20u,
     0x22DC4D99u,
     0x43468260u,
     0u,
     {"debug_overlay", "engine.dll", "VDebugOverlay004"}},
    {0x570u,
     0x43468D20u,
     0xCFE960F2u,
     0x43468974u,
     0u,
     {"model_render", "engine.dll", "VEngineModel016"}},
    {0x57Cu,
     0x43468D20u,
     0x1D91BB6Eu,
     0x4346790Cu,
     0u,
     {"render_view", "engine.dll", "VEngineRenderView014"}},
    {0x588u,
     0x43468D20u,
     0xE8D5CFDEu,
     0x434693F8u,
     0u,
     {"engine_sound", "engine.dll", "IEngineSoundClient003"}},
    {0x594u,
     0x43468D20u,
     0xF8DA3AF4u,
     0x4346712Cu,
     0u,
     {"engine_vgui", "engine.dll", "VEngineVGui001"}},
    {0x5A0u,
     0x43468D20u,
     0x59E431D7u,
     0x43467198u,
     0u,
     {"string_tables", "engine.dll", "VEngineClientStringTable001"}},
    {0x5ACu,
     0x43468D20u,
     0x8D6D3467u,
     0x43467014u,
     0u,
     {"static_props", "engine.dll", "StaticPropMgrClient005"}},
    {0x5B8u,
     0x43468D20u,
     0x22078B6Eu,
     0x43467FB0u,
     0u,
     {"tool_framework", "engine.dll", "VTOOLFRAMEWORKVERSION002"}},
    {0x5C4u,
     0x43468D20u,
     0x70F6F65Bu,
     0x43467F10u,
     0u,
     {"spatial_partition", "engine.dll", "SpatialPartition001"}},
    {0x5D0u,
     0x43468D20u,
     0x5A17F0FFu,
     0x43469214u,
     0u,
     {"engine_tool", "engine.dll", "VENGINETOOL003"}},
    {0x5DCu,
     0x43468D20u,
     0xDEF79758u,
     0x43468364u,
     0u,
     {"game_event_manager", "engine.dll", "GAMEEVENTSMANAGER002"}},
    {0x5E8u,
     0x43467AFCu,
     0xFF7E6279u,
     0x434689C0u,
     0u,
     {"surface", "vguimatsurface.dll", "VGUI_Surface031"}},
    {0x5F4u,
     0x4346889Cu,
     0xF7986847u,
     0x43468898u,
     0u,
     {"vgui_input", "vgui2.dll", "VGUI_Input005"}},
    {0x600u,
     0x4346889Cu,
     0xE4271189u,
     0x4346A44Cu,
     0u,
     {"vgui_panel", "vgui2.dll", "VGUI_Panel009"}},
    {0x60Cu,
     0x4346889Cu,
     0x0F80C440u,
     0x43467118u,
     0u,
     {"vgui_system", "vgui2.dll", "VGUI_System010"}},
    {0x618u, 0x43468A7Cu, 0x9B263577u, 0x4346A87Cu, 0u, {"cvar", "vstdlib.dll", "VEngineCvar007"}},
    {0x624u,
     0x43468B74u,
     0xF0196491u,
     0x43469268u,
     0u,
     {"physics_surface_props", "vphysics.dll", "VPhysicsSurfaceProps001"}},
    {0x630u,
     0x4346A600u,
     0x945D3F26u,
     0x4346A4E0u,
     0u,
     {"material_system", "materialsystem.dll", "VMaterialSystem080"}},
    {0x63Cu,
     0x43468D98u,
     0x2CF5E2E3u,
     0x43469330u,
     0u,
     {"model_cache", "datacache.dll", "MDLCache004"}},
    {0x648u,
     0x434670E4u,
     0x64884D32u,
     0x43467470u,
     0u,
     {"server_game_dll", "server.dll", "ServerGameDLL005"}},
    {0x654u,
     0x43468D40u,
     0x7F31CFBDu,
     0x43467010u,
     0u,
     {"sound_system", "soundsystem.dll", "SoundSystem001"}},
    {0x660u,
     0x43469234u,
     0x877DC74Cu,
     0x43468E98u,
     0u,
     {"panorama_engine", "panorama.dll", "PanoramaUIEngine001"}},
    {0x66Cu,
     0x4346762Cu,
     0x15869CDFu,
     0x43467F14u,
     0u,
     {"panorama_ui_client", "panoramauiclient.dll", "PanoramaUIClient001"}},
    {0x678u,
     0x4346A604u,
     0xD5500FAEu,
     0x4346A470u,
     0u,
     {"shader_device_manager", "shaderapidx9.dll", "ShaderDeviceMgr001"}},
    {0x684u,
     0x4346A604u,
     0xA9097514u,
     0x4346A8A4u,
     0u,
     {"shader_api", "shaderapidx9.dll", "ShaderApi029"}},
    {0x690u,
     0x4346A604u,
     0x54E48848u,
     0x4346A33Cu,
     0u,
     {"shader_device", "shaderapidx9.dll", "ShaderDevice001"}},
    {0x69Cu,
     0x43467B74u,
     0x1E300843u,
     0x43467F68u,
     0u,
     {"filesystem", "filesystem_stdio.dll", "VFileSystem017"}},
    {0x6A8u,
     0x43468B74u,
     0x786E227Au,
     0x43467208u,
     0u,
     {"physics_collision", "vphysics.dll", "VPhysicsCollision007"}},
}};
constexpr std::array<InterfaceRequest, 7> requests{{
    {"engine_client", "engine.dll", "VEngineClient014"},
    {"entity_list", "client.dll", "VClientEntityList003"},
    {"base_client", "client.dll", "VClient018"},
    {"prediction", "client.dll", "VClientPrediction001"},
    {"surface", "vguimatsurface.dll", "VGUI_Surface031"},
    {"cvar", "vstdlib.dll", "VEngineCvar007"},
    {"engine_trace", "engine.dll", "EngineTraceClient004"},
}};
void validate_build_destinations(NativeDataImage& data) {
    for (const auto& recipe : build_requests) {
        (void)data.native_cell<void*>(recipe.module_cell);
        (void)data.native_cell<void*>(recipe.destination_cell);
        if (netvar_name_hash(recipe.request.version.data()) != recipe.interface_hash)
            throw std::logic_error(
                "Build interface name does not match its recovered registration hash");
    }
}
void publish_build_interface(NativeDataImage& data, const BuildInterfaceRecipe& recipe,
                             void* object) {
    // All 41 observed destination masks are zero. Keep the recovered storage
    // representation explicit, without replacing live results by dump values.
    const auto encoded = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object)) ^
                         recipe.destination_xor;
    std::memcpy(&data.native_cell<void*>(recipe.destination_cell), &encoded, sizeof encoded);
}
[[noreturn]] void missing_build_interface(const BuildInterfaceRecipe& recipe,
                                          BuildInterfaceFailureStage stage) {
    throw BuildInterfaceDiscoveryFailure(recipe, stage);
}
}
std::span<const InterfaceRequest, 7> core_interface_requests() noexcept {
    return requests;
}
CoreEngineInterfaces discover_core_interfaces(FactoryProvider& provider) {
    InterfaceBindings interfaces;
    interfaces.bind(requests, provider);
    return {interfaces.at("engine_client"), interfaces.at("entity_list"),
            interfaces.at("base_client"),   interfaces.at("prediction"),
            interfaces.at("surface"),       interfaces.at("cvar"),
            interfaces.at("engine_trace")};
}
CoreEngineInterfaces discover_core_interfaces(FactoryProvider& provider, CoreInterfaceCells cells) {
    const auto interfaces = discover_core_interfaces(provider);
    cells.engine_client = interfaces.engine_client;
    cells.entity_list = interfaces.entity_list;
    cells.base_client = interfaces.base_client;
    cells.prediction = interfaces.prediction;
    cells.surface = interfaces.surface;
    cells.cvar = interfaces.cvar;
    cells.engine_trace = interfaces.engine_trace;
    return interfaces;
}
CoreEngineInterfaces discover_core_interfaces(CoreInterfaceCells cells) {
    LoadedModuleFactories provider;
    return discover_core_interfaces(provider, cells);
}
std::span<const BuildInterfaceRecipe, 41> build_interface_manifest() noexcept {
    return build_requests;
}
BuildInterfaceDiscoveryFailure::BuildInterfaceDiscoveryFailure(
    const BuildInterfaceRecipe& recipe, BuildInterfaceFailureStage failure_stage)
    : std::runtime_error("Missing build interface: " + std::string(recipe.request.module) + "!" +
                         std::string(recipe.request.version)),
      operand(recipe.operand), destination_cell(recipe.destination_cell), stage(failure_stage) {}
void* BuildEngineInterfaces::at(std::string_view role) const {
    for (std::size_t index = 0; index < build_requests.size(); ++index)
        if (build_requests[index].request.name == role)
            return objects[index];
    throw std::out_of_range("Name is not a recovered build interface role");
}
CoreEngineInterfaces BuildEngineInterfaces::core() const noexcept {
    return {objects[10], objects[2], objects[1], objects[3], objects[24], objects[28], objects[11]};
}
BuildEngineInterfaces discover_build_interfaces(NativeDataImage& data,
                                                NativeImageEnvironment environment) {
    validate_build_destinations(data);
    BuildEngineInterfaces interfaces;
    for (std::size_t index = 0; index < build_requests.size(); ++index) {
        const auto& recipe = build_requests[index];
        auto* const module = static_cast<std::byte*>(data.native_cell<void*>(recipe.module_cell));
        auto* const registry = find_interface_registry(module, environment);
        if (!registry)
            missing_build_interface(recipe, BuildInterfaceFailureStage::module_registry);
        auto* const registration = find_interface_registration(registry, recipe.interface_hash);
        if (!registration || !registration->create)
            missing_build_interface(recipe, BuildInterfaceFailureStage::registration);
        auto* const object = registration->create();
        if (!object)
            missing_build_interface(recipe, BuildInterfaceFailureStage::creator);
        interfaces.objects[index] = object;
        publish_build_interface(data, recipe, object);
    }
    return interfaces;
}
BuildEngineInterfaces discover_build_interfaces(NativeDataImage& data) {
    return discover_build_interfaces(data, native_image_environment());
}
BuildEngineInterfaces discover_build_interfaces(FactoryProvider& provider, NativeDataImage& data) {
    validate_build_destinations(data);
    BuildEngineInterfaces interfaces;
    for (std::size_t index = 0; index < build_requests.size(); ++index) {
        const auto& recipe = build_requests[index];
        InterfaceFactory factory;
        try {
            factory = provider.factory(recipe.request.module);
        } catch (const std::runtime_error&) {
            missing_build_interface(recipe, BuildInterfaceFailureStage::factory);
        }
        if (!factory)
            missing_build_interface(recipe, BuildInterfaceFailureStage::factory);
        auto* const object = factory(recipe.request.version.data(), nullptr);
        if (!object)
            missing_build_interface(recipe, BuildInterfaceFailureStage::creator);
        interfaces.objects[index] = object;
        publish_build_interface(data, recipe, object);
    }
    return interfaces;
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: jobs.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
// Microsoft documents this exact CRT export without a public header declaration:
// https://learn.microsoft.com/cpp/c-runtime-library/reference/purecall
extern "C" int __cdecl _purecall();
#endif

namespace skeet::engine {
namespace {
std::uint32_t add_word(std::uint32_t& target, std::uint32_t value) {
#if defined(_WIN32)
    return static_cast<std::uint32_t>(InterlockedExchangeAdd(
        reinterpret_cast<volatile long*>(&target), static_cast<long>(value)));
#else
    return __atomic_fetch_add(&target, value, __ATOMIC_SEQ_CST);
#endif
}
std::uint32_t load_word(std::uint32_t& target) {
#if defined(_WIN32)
    return static_cast<std::uint32_t>(
        InterlockedCompareExchange(reinterpret_cast<volatile long*>(&target), 0, 0));
#else
    return __atomic_load_n(&target, __ATOMIC_SEQ_CST);
#endif
}
std::uintptr_t add_pointer(std::uintptr_t& target, std::uintptr_t value) {
#if defined(_WIN64)
    return static_cast<std::uintptr_t>(InterlockedExchangeAdd64(
        reinterpret_cast<volatile long long*>(&target), static_cast<long long>(value)));
#elif defined(_WIN32)
    return static_cast<std::uintptr_t>(InterlockedExchangeAdd(
        reinterpret_cast<volatile long*>(&target), static_cast<long>(value)));
#else
    return __atomic_fetch_add(&target, value, __ATOMIC_SEQ_CST);
#endif
}
std::uintptr_t load_pointer(std::uintptr_t& target) {
#if defined(_WIN32)
    return reinterpret_cast<std::uintptr_t>(InterlockedCompareExchangePointer(
        reinterpret_cast<void* volatile*>(&target), nullptr, nullptr));
#else
    return __atomic_load_n(&target, __ATOMIC_SEQ_CST);
#endif
}
void store_pointer(std::uintptr_t& target, std::uintptr_t value) {
#if defined(_WIN32)
    InterlockedExchangePointer(reinterpret_cast<void* volatile*>(&target),
                               reinterpret_cast<void*>(value));
#else
    __atomic_exchange_n(&target, value, __ATOMIC_SEQ_CST);
#endif
}
void consume(JobTask& task, std::uint32_t stride) {
    const auto end = task.end;
    auto position = load_pointer(task.cursor);
    while (position < end) {
        position = add_pointer(task.cursor, stride);
        const auto next = position + stride;
        if (position < std::min(end, next)) {
            task.work(reinterpret_cast<void*>(position));
            add_word(task.completed, 1);
            position = next;
        }
    }
    invoke<std::int32_t>(&task, 1);
}
#if defined(_M_IX86) || defined(__i386__)
#define TASK_MEMBER __thiscall
#else
#define TASK_MEMBER
#endif
std::int32_t TASK_MEMBER retain_entry(void* task) {
    return retain_job(*static_cast<JobTask*>(task));
}
std::int32_t TASK_MEMBER release_entry(void* task) {
    return release_job(*static_cast<JobTask*>(task));
}
void TASK_MEMBER consume_eight_entry(void* task) {
    consume_eight_byte_jobs(*static_cast<JobTask*>(task));
}
void TASK_MEMBER consume_thirty_six_entry(void* task) {
    consume_thirty_six_byte_jobs(*static_cast<JobTask*>(task));
}
}

std::int32_t retain_job(JobTask& task) {
    return static_cast<std::int32_t>(add_word(task.references, 1) + 1);
}
std::int32_t release_job(JobTask& task) {
    const auto remaining = add_word(task.references, 0xffffffff) - 1;
    if (remaining == 0)
        invoke<JobTask*, std::uint8_t>(&task, 2, 1);
    return static_cast<std::int32_t>(remaining);
}
void consume_eight_byte_jobs(JobTask& task) {
    consume(task, 8);
}
void consume_thirty_six_byte_jobs(JobTask& task) {
    consume(task, 0x24);
}

JobMemory native_job_memory() {
#if defined(_WIN32)
    const auto native = GetModuleHandleA("ntdll.dll");
    if (!native)
        throw std::runtime_error("ntdll.dll is not loaded");
    const auto allocate = GetProcAddress(native, "RtlAllocateHeap");
    const auto free = GetProcAddress(native, "RtlFreeHeap");
    if (!allocate || !free)
        throw std::runtime_error("Native job memory export is missing");
    // Bind this source build's CRT directly; prior loading of a different CRT
    // module is not a requirement of source-owned process-heap storage.
    return {reinterpret_cast<JobMemory::AllocateHeap>(allocate),
            reinterpret_cast<JobMemory::FreeHeap>(free), GetProcessHeap(),
            reinterpret_cast<void*>(&_purecall)};
#else
    throw std::runtime_error("Native job memory requires Windows");
#endif
}
void* native_thread_pool() {
#if defined(_WIN32)
    const auto library = GetModuleHandleA("vstdlib.dll");
    if (!library)
        throw std::runtime_error("vstdlib.dll is not loaded");
    const auto symbol = GetProcAddress(library, "g_pThreadPool");
    if (!symbol)
        throw std::runtime_error("vstdlib!g_pThreadPool export is missing");
    auto* pool = *reinterpret_cast<void**>(symbol);
    if (!pool)
        throw std::runtime_error("vstdlib!g_pThreadPool is null");
    return pool;
#else
    throw std::runtime_error("Native thread pool requires Windows");
#endif
}

JobDispatcher::JobDispatcher(void* thread_pool, JobMemory memory)
    : eight_{}, thirty_six_{}, destroyed_{}, pool_(thread_pool), memory_(memory) {
    if (!pool_ || !memory_.allocate || !memory_.free || !memory_.purecall)
        throw std::invalid_argument(
            "Job dispatcher requires the actual thread pool and heap exports");
    const auto destructor = &destruction_entry;
    eight_ = {
        {{reinterpret_cast<void*>(&retain_entry), reinterpret_cast<void*>(&release_entry),
          reinterpret_cast<void*>(destructor), reinterpret_cast<void*>(&consume_eight_entry)}},
        this};
    thirty_six_ = {
        {{reinterpret_cast<void*>(&retain_entry), reinterpret_cast<void*>(&release_entry),
          reinterpret_cast<void*>(destructor), reinterpret_cast<void*>(&consume_thirty_six_entry)}},
        this};
    destroyed_ = {memory_.purecall, memory_.purecall, reinterpret_cast<void*>(destructor),
                  memory_.purecall};
}

JobTask* JobDispatcher::create(JobWork work, TaskTable& table) {
    // Complete43387448 allocation shape, specialized to native job size0x1C.
    const auto size = sizeof(JobTask);
    auto* allocation = memory_.allocate(memory_.process_heap, 8, size + 0x20);
    if (!allocation)
        return nullptr;
    const auto raw = reinterpret_cast<std::uintptr_t>(allocation);
    const auto header = (raw + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - raw);
    const auto allocation_size = static_cast<std::uint32_t>(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return new (reinterpret_cast<void*>(header + 16))
        JobTask{table.methods.data(), 0, 1, work, 0, 0, 0};
}
JobTask* JobDispatcher::create_eight(JobWork work) {
    return create(work, eight_);
}
JobTask* JobDispatcher::create_thirty_six(JobWork work) {
    return create(work, thirty_six_);
}
JobTask* JobDispatcher::destroy(JobTask& task, std::uint8_t flags) {
    const auto* table = reinterpret_cast<TaskTable*>(task.vtable);
    auto& owner = *table->owner;
    task.vtable = owner.destroyed_.data();
    if (flags & 1) {
        auto* object = reinterpret_cast<std::byte*>(&task);
        std::uint32_t displacement;
        std::memcpy(&displacement, object - 8, 4);
        owner.memory_.free(owner.memory_.process_heap, 0, object - 16 - displacement);
    }
    return &task;
}
JobTask* TASK_MEMBER JobDispatcher::destruction_entry(void* object, std::uint8_t flags) {
    return destroy(*static_cast<JobTask*>(object), flags);
}

void JobDispatcher::dispatch(JobTask& task, void* items, std::uint32_t count,
                             std::uint32_t stride) {
    if (count == 0)
        return;
    const auto first = reinterpret_cast<std::uintptr_t>(items);
    store_pointer(task.cursor, first);
    task.end = first + static_cast<std::uintptr_t>(count) * stride;
    const auto available = invoke<std::int32_t>(pool_, 8);
    auto workers = std::min(static_cast<std::int32_t>(count - 1), available);
    if (workers < 1) {
        invoke<std::int32_t>(&task, 0);
        consume(task, stride);
        return;
    }
    workers = std::min(workers, 32);
    std::array<void*, 32> handles{};
    for (std::int32_t i = 0; i < workers; ++i) {
        void* handle = nullptr;
        invoke<void, JobTask*, void**, int, int>(pool_, 15, &task, &handle, 0, 8);
        if (handle) {
            invoke<std::int32_t>(&task, 0);
            handles[static_cast<std::size_t>(i)] = handle;
        }
    }
    invoke<std::int32_t>(&task, 0);
    consume(task, stride);
    for (std::int32_t i = 0; i < workers; ++i)
        if (auto* handle = handles[static_cast<std::size_t>(i)])
            invoke<void>(handle, 1);
    while (load_word(task.completed) != count) {
#if defined(_WIN32)
        _mm_pause();
#else
        __builtin_ia32_pause();
#endif
    }
}
void JobDispatcher::dispatch_eight(JobTask& task, void* items, std::uint32_t count) {
    dispatch(task, items, count, 8);
}
void JobDispatcher::dispatch_thirty_six(JobTask& task, void* items, std::uint32_t count) {
    dispatch(task, items, count, 0x24);
}

#undef TASK_MEMBER
} // namespace skeet::engine

// ============================================================================
// Subcomponent: map_game_system_owner.cpp
// ============================================================================

namespace skeet::engine {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define MAP_SYSTEM_MEMBER __thiscall
#else
#define MAP_SYSTEM_MEMBER
#endif
void MAP_SYSTEM_MEMBER map_system_empty(void*) {} //433CCE8E, RET0
bool MAP_SYSTEM_MEMBER true_entry(void*) {
    return true;
} //433422C6
const char* MAP_SYSTEM_MEMBER map_system_name(void*) {
    static const char text[1] = {0};
    return text;
}
/* [dedup add] */
/* [dedup subtract] */
void grow(MapGameSystemVector& vector, std::int32_t amount, void* allocator) { //4337CF0C
    if (vector.grow_size < 0)
        return;
    const auto requested = add(vector.capacity, amount);
    std::int32_t capacity;
    if (vector.grow_size) {
        const auto preceding = subtract(requested, 1);
        capacity = add(subtract(preceding, preceding % vector.grow_size), vector.grow_size);
    } else {
        capacity = vector.capacity ? vector.capacity : 8;
        while (capacity < requested)
            capacity = add(capacity, capacity);
    }
    if (capacity < requested) {
        if (!capacity && requested <= -1)
            capacity = -1;
        else
            do {
                const auto sum = add(capacity, requested);
                capacity = add(sum, sum < 0 ? 1 : 0) >> 1;
            } while (capacity < requested);
    }
    vector.capacity = capacity;
    const auto bytes = std::uint32_t(capacity) * std::uint32_t(sizeof(MapGameSystemEndpoint*));
    vector.data = vector.data ? invoke<MapGameSystemEndpoint**>(allocator, 3, vector.data, bytes)
                              : invoke<MapGameSystemEndpoint**>(allocator, 1, bytes);
}
void append(MapGameSystemVector& vector, MapGameSystemEndpoint* const* value,
            void* allocator) { //433F4786
    const auto insertion = vector.size;
    auto count = vector.size;
    if (add(insertion, 1) > vector.capacity) {
        grow(vector, add(subtract(insertion, vector.capacity), 1), allocator);
        count = vector.size;
    }
    const auto data = vector.data;
    vector.size = add(count, 1);
    vector.elements = data;
    const auto moved = subtract(count, insertion);
    if (moved > 0)
        std::memmove(data + insertion + 1, data + insertion,
                     std::size_t(moved) * sizeof(MapGameSystemEndpoint*));
    vector.data[insertion] = *value; //native reloads data after the external memmove
}
void remove(MapGameSystemVector& vector, MapGameSystemEndpoint* const* value) { //433B9059
    auto count = vector.size;
    if (count <= 0)
        return;
    std::int32_t index = 0;
    while (vector.data[index] != *value)
        if (++index >= count)
            return;
    const auto moved = subtract(subtract(count, index), 1);
    if (moved > 0) {
        std::memmove(vector.data + index, vector.data + index + 1,
                     std::size_t(moved) * sizeof(MapGameSystemEndpoint*));
        count = vector.size;
    }
    vector.size = subtract(count, 1);
}
MapGameSystemVector* list_of(std::int32_t* size) {
    return reinterpret_cast<MapGameSystemVector*>(reinterpret_cast<std::byte*>(size) -
                                                  offsetof(MapGameSystemVector, size));
}
void factory_entry(void* opaque) {
    static_cast<MapGameSystemRuntime*>(opaque)->create();
}
} //namespace

MapGameSystemRuntime::MapGameSystemRuntime(OwnerRegistryBinding owners,
                                           MapGameSystemBindings bindings)
    : primary_{{destroy_entry, initialize_entry, append_entry, map_system_empty, remove_entry, map_system_empty, map_system_empty,
                map_system_empty, map_system_empty},
               this},
      endpoint_{this,
                {{map_system_name, true_entry, map_system_empty, map_system_empty, map_system_empty, map_entity_text_entry, map_system_empty, map_system_empty, map_system_empty,
                  map_system_empty, map_system_empty, true_entry, destroy_endpoint_entry},
                 map_system_empty,
                 frame_update_entry,
                 map_system_empty}},
      destroyed_{this,
                 {reinterpret_cast<GameSystemEntry<const char*>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<bool>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<void>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<void>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<void>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<void>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<void>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<void>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<void>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<void>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<void>>(owners.memory.purecall),
                  reinterpret_cast<GameSystemEntry<bool>>(owners.memory.purecall),
                  destroy_endpoint_entry}},
      owners_(owners), bindings_(bindings) {}

MapGameSystemRuntime& MapGameSystemRuntime::runtime(MapGameSystemOwner& owner) {
    static_assert(offsetof(PrimaryTable, entries) == 0);
    return *reinterpret_cast<const PrimaryTable*>(owner.vtable)->runtime;
}
MapGameSystemRuntime& MapGameSystemRuntime::runtime(MapGameSystemEndpoint& endpoint) {
    static_assert(offsetof(EndpointTable, entries) == sizeof(void*));
    static_assert(offsetof(DestroyedEndpointTable, entries) == sizeof(void*));
    MapGameSystemRuntime* context;
    std::memcpy(&context, reinterpret_cast<const std::byte*>(endpoint.vtable) - sizeof(context),
                sizeof(context));
    return *context;
}
MapGameSystemOwner* MapGameSystemRuntime::create() {
    auto* storage = game_system_detail::allocate(owners_.memory, sizeof(MapGameSystemOwner));
    if (!storage)
        return nullptr;
    auto* owner = ::new (storage) MapGameSystemOwner;
    owner->vtable = nullptr;
    owners_.append(owner);
    owner->vtable = primary_.entries.data();
    owner->endpoint.vtable = &endpoint_.entries.base;
    return owner;
}
void MapGameSystemRuntime::initialize(MapGameSystemOwner& owner) {
    owner.initialized = &owner;
    auto* first = bindings_.first_size;
    if (!first || !bindings_.second_size)
        return;
    owner.first = list_of(first);
    if (*first)
        owner.second = list_of(bindings_.second_size);
}
void MapGameSystemRuntime::append(MapGameSystemOwner& owner) {
    if (owner.first && owner.first->size) {
        auto* endpoint = &owner.endpoint;
        engine::append(*owner.first, &endpoint, bindings_.allocator);
    }
    if (owner.second && owner.second->size) {
        auto* endpoint = &owner.endpoint;
        engine::append(*owner.second, &endpoint, bindings_.allocator);
    }
}
void MapGameSystemRuntime::remove(MapGameSystemOwner& owner) {
    if (owner.first) {
        auto* endpoint = &owner.endpoint;
        engine::remove(*owner.first, &endpoint);
    }
    if (owner.second) {
        auto* endpoint = &owner.endpoint;
        engine::remove(*owner.second, &endpoint);
    }
}
void MapGameSystemRuntime::destroy(MapGameSystemOwner* owner) {
    if (!owner)
        return;
    owner->endpoint.vtable = &destroyed_.entries;
    game_system_detail::release(owner, owners_.memory);
}
void MAP_SYSTEM_MEMBER MapGameSystemRuntime::destroy_entry(void* object) {
    if (object)
        runtime(*static_cast<MapGameSystemOwner*>(object))
            .destroy(static_cast<MapGameSystemOwner*>(object));
}
void MAP_SYSTEM_MEMBER MapGameSystemRuntime::initialize_entry(void* object) {
    auto& owner = *static_cast<MapGameSystemOwner*>(object);
    runtime(owner).initialize(owner);
}
void MAP_SYSTEM_MEMBER MapGameSystemRuntime::append_entry(void* object) {
    auto& owner = *static_cast<MapGameSystemOwner*>(object);
    runtime(owner).append(owner);
}
void MAP_SYSTEM_MEMBER MapGameSystemRuntime::remove_entry(void* object) {
    auto& owner = *static_cast<MapGameSystemOwner*>(object);
    runtime(owner).remove(owner);
}
void MAP_SYSTEM_MEMBER MapGameSystemRuntime::map_entity_text_entry(void* object) {
    auto& state = runtime(*static_cast<MapGameSystemEndpoint*>(object));
    esp::dispatch_map_entity_text(state.bindings_.map_entity_parsers, state.bindings_.engine);
}
void MAP_SYSTEM_MEMBER MapGameSystemRuntime::frame_update_entry(void* object, float seconds) {
    auto& state = runtime(*static_cast<MapGameSystemEndpoint*>(object));
    esp::dispatch_map_scalar(state.bindings_.map_entity_parsers, seconds);
}
void* MAP_SYSTEM_MEMBER MapGameSystemRuntime::destroy_endpoint_entry(void* object,
                                                                     std::uint8_t flags) {
    auto& endpoint = *static_cast<MapGameSystemEndpoint*>(object);
    auto& state = runtime(endpoint);
    endpoint.vtable = &state.destroyed_.entries;
    //433B369E passes this exact endpoint to433837D0, without an owner adjustment.
    if (flags & 1)
        game_system_detail::release(object, state.owners_.memory);
    return object;
}
void bind_map_game_system_factory(client::NativeStartupSources& sources,
                                  MapGameSystemRuntime& runtime) {
    sources.bind_initializer({0x4348E6AAu, &runtime, factory_entry});
}
#undef MAP_SYSTEM_MEMBER
} //namespace skeet::engine

// ============================================================================
// Subcomponent: module_range_initialization.cpp
// ============================================================================

namespace skeet::engine {
namespace {
constexpr std::array<ModuleRangeRowCells, 4> cells{{{0x434675D8u, 0x4346A49Cu, 0},
                                                    {0x4346837Cu, 0x43467570u, 0},
                                                    {0x4346A58Cu, 0, 0x43468C20u},
                                                    {0x43468234u, 0, 0x43468C98u}}};
template <class Pointer> bool skipped(Pointer pointer) noexcept {
    return reinterpret_cast<std::uintptr_t>(pointer) == 1;
}
}
std::span<const ModuleRangeRowCells, 4> module_range_row_cells() noexcept {
    return cells;
}

ModuleRangeInitializationBindings native_module_range_bindings(NativeDataImage& data) {
    ModuleRangeInitializationBindings result{{}, &data.native_cell<const std::byte*>(0x4346A55Cu)};
    for (std::size_t index = 0; index < cells.size(); ++index) {
        const auto& cell = cells[index];
        result.rows[index] = {
            cell.initialize ? &data.native_cell<ModuleRangeInitialize>(cell.initialize) : nullptr,
            cell.get ? &data.native_cell<ModuleRangeGet>(cell.get) : nullptr,
            cell.indirect ? &data.native_cell<ModuleAddressRange**>(cell.indirect) : nullptr};
    }
    return result;
}

void initialize_module_ranges(const ModuleRangeInitializationBindings& bindings) {
    for (const auto& row : bindings.rows) {
        if (!row.initialize)
            continue;
        const auto initialize = *row.initialize;
        if (skipped(initialize))
            continue;
        if (!initialize)
            throw std::invalid_argument("Active module range row requires its genuine initializer");
        initialize();

        ModuleAddressRange* range;
        if (row.get) {
            const auto get = *row.get;
            if (skipped(get))
                continue;
            if (!get)
                throw std::invalid_argument("Active module range row requires its genuine getter");
            range = get();
        } else {
            if (!row.indirect)
                continue;
            auto* const holder = *row.indirect;
            if (skipped(holder))
                continue;
            if (!holder)
                throw std::invalid_argument(
                    "Active module range row requires its genuine object holder");
            range = *holder;
        }
        if (!range || range->low == 0x10000u)
            continue;
        if (!bindings.client_module)
            throw std::invalid_argument(
                "Module range initialization requires the current client module cell");
        const auto low =
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(*bindings.client_module)) +
            0x1000u;
        if (range->low != low)
            continue;
        range->low = 0x10000u;
        range->high = 0xFFF00000u;
    }
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: native_data_image.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
NativeDataImage::NativeDataImage(std::shared_ptr<std::byte> mapping, std::size_t size)
    : mapping_(std::move(mapping)), mapped_size_(size) {
    if (!mapping_ || mapped_size_ < image_size ||
        reinterpret_cast<std::uintptr_t>(mapping_.get()) % 16)
        throw std::invalid_argument(
            "Native data storage requires a stable aligned complete address space");
}
std::shared_ptr<NativeDataImage> NativeDataImage::create_source() {
    if (sizeof(void*) != 4)
        throw std::logic_error("Source native data storage requires x86 pointers");
#if defined(_WIN32)
    // VirtualAlloc supplies zeroed writable data without executable access.
    // Source initializers must still construct their real tables and owners.
    auto* const bytes = static_cast<std::byte*>(
        VirtualAlloc(nullptr, image_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!bytes)
        throw std::bad_alloc{};
    std::shared_ptr<std::byte> mapping(
        bytes, [](std::byte* allocation) { (void)VirtualFree(allocation, 0, MEM_RELEASE); });
    return std::shared_ptr<NativeDataImage>(new NativeDataImage(std::move(mapping), image_size));
#else
    throw std::logic_error("Source native data storage requires Windows virtual memory");
#endif
}
NativeDataImage& require_native_data_image(const std::shared_ptr<NativeDataImage>& image) {
    if (!image)
        throw std::invalid_argument("Native data view requires its stable storage owner");
    return *image;
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: native_owner_registry_storage.cpp
// ============================================================================

namespace skeet::engine {
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(OwnerRegistry) == 0xc);
static_assert(offsetof(OwnerRegistry, begin) == 0);
static_assert(offsetof(OwnerRegistry, end) == 4);
static_assert(offsetof(OwnerRegistry, limit) == 8);
#endif

NativeOwnerRegistryStorage::NativeOwnerRegistryStorage(PreservedInitialization& initialization)
    : NativeOwnerRegistryStorage(initialization.image_owner()) {}
NativeOwnerRegistryStorage::NativeOwnerRegistryStorage(std::shared_ptr<NativeDataImage> image)
    : image_(image) {
    (void)require_native_data_image(image_);
    if (sizeof(void*) != 4 || sizeof(OwnerRegistry) != 0xc)
        throw std::logic_error("Native owner registries require their original x86 pointer layout");
}

OwnerRegistry& NativeOwnerRegistryStorage::at(std::uint32_t address) const {
    if (std::find(native_owner_registry_cells.begin(), native_owner_registry_cells.end(),
                  address) == native_owner_registry_cells.end())
        throw std::out_of_range("Address is not a recovered raw owner registry triplet");
    // Viewing an existing native object must not invoke OwnerRegistry's
    // default member initializers or allocate/copy its pointer buffer.
    return image_->native_cell<OwnerRegistry>(address);
}

}

// ============================================================================
// Subcomponent: native_service_table.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* deduplicated helper */
int compare_service_rva(const void* first, const void* second) noexcept { //433F1E1B
    const auto& a = *static_cast<const NativeServiceRecord*>(first);
    const auto& b = *static_cast<const NativeServiceRecord*>(second);
    return std::bit_cast<std::int32_t>(a.function_rva - b.function_rva);
}
}

void initialize_native_service_table(NativeServiceTable& table, const LoaderData& loader,
                                     NativeImageEnvironment images,
                                     ImageDirectoryEntry image_directory_entry) {
    table.count = 0;
    const auto* entry = reinterpret_cast<const LoaderModule*>(loader.modules.next->next);
    auto* const module = static_cast<std::byte*>(entry->base);
    if (!module || !images.image_nt_header(module))
        return;
    std::uint32_t directory_size;
    auto* const directory = image_directory_entry(module, 1, 0, &directory_size);
    if (!directory)
        return;
    const auto* functions = module + read<std::uint32_t>(directory + 0x1C);
    const auto* ordinals = module + read<std::uint32_t>(directory + 0x24);
    const auto* names = module + read<std::uint32_t>(directory + 0x20);
    const auto count = read<std::uint32_t>(directory + 0x18);
    for (std::uint32_t index = 0; index < count; ++index) {
        const auto* name =
            reinterpret_cast<const char*>(module + read<std::uint32_t>(names + index * 4));
        if (name[0] != 'N' || name[1] != 't')
            continue;
        // The native caller reserves exactly1024 records and supplies ntdll's
        // valid export directory. Reject a source-side capacity violation.
        if (table.count == table.records.size())
            throw std::length_error("Native service export table exceeds its 1024-record capacity");
        auto& record = table.records[table.count++];
        record.name_hash = netvar_name_hash(name); //43359C70, case-sensitive FNV1a and salt
        const auto ordinal = read<std::uint16_t>(ordinals + index * 2);
        record.function_rva = read<std::uint32_t>(functions + ordinal * 4);
        record.service_number = 0xFFFFFFFFu;
        const auto* code = module + record.function_rva;
        if (code[0] == std::byte{0xB8})
            record.service_number = read<std::uint32_t>(code + 1);
    }
    std::qsort(table.records.data(), table.count, sizeof(NativeServiceRecord), compare_service_rva);
    std::uint32_t previous = 0;
    for (std::uint32_t index = 0; index < table.count; ++index) {
        auto& record = table.records[index];
        if (record.service_number != 0xFFFFFFFFu)
            previous = record.service_number & 0xFFFu;
        else
            record.service_number = ++previous;
    }
}
std::uint32_t resolve_native_service_number(const NativeServiceTable& table, std::uint32_t hash,
                                            NativeServiceTerminateProcess terminate_process) {
    for (std::uint32_t index = 0; index < table.count; ++index) {
        const auto& record = table.records[index];
        if (record.name_hash != hash)
            continue;
        if (record.service_number != 0xFFFFFFFFu)
            return static_cast<std::uint16_t>(record.service_number);
        break;
    }
    terminate_process(reinterpret_cast<void*>(~std::uintptr_t{0}), -10);
    return 0xFFFFFFFFu;
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: netvars.cpp
// ============================================================================

namespace skeet::engine {

std::uint32_t maximum_property_offset(const RecvTable& table, std::uint32_t base,
                                      std::uint32_t minimum) {
    auto maximum = minimum;
    for (std::int32_t i = 0; i < table.property_count; ++i) {
        const auto& property = table.properties[i];
        const auto offset = base + static_cast<std::uint32_t>(property.offset);
        maximum = std::max(maximum, offset);
        if (property.type == recv_data_table)
            maximum = maximum_property_offset(*property.data_table, offset, maximum);
    }
    return maximum;
}

std::uint32_t netvar_name_hash(const char* name) noexcept {
    std::uint32_t hash = 0x811c9dc5;
    if (name)
        for (const auto* byte = reinterpret_cast<const unsigned char*>(name); *byte; ++byte)
            hash = (hash ^ *byte) * 0x1000193;
    return hash ^ 0x92ca5df1;
}

ClientClass* find_client_class(void* base_client, std::uint32_t hash) {
    // Native 0x433412C5 loads global 0x434683E0; +0x20 is the x86 byte slot.
    auto* node = invoke<ClientClass*>(base_client, 8);
    while (node) {
        if (netvar_name_hash(node->network_name) == hash)
            return node;
        node = node->next;
    }
    return nullptr;
}

RecvTable* find_receive_table(RecvTable& table, std::uint32_t hash) {
    if (netvar_name_hash(table.name) == hash)
        return &table;
    for (std::int32_t i = 0; i < table.property_count; ++i) {
        auto& property = table.properties[i];
        if (property.type == recv_data_table) {
            if (auto* found = find_receive_table(*property.data_table, hash))
                return found;
        }
    }
    return nullptr;
}

RecvProp* find_receive_property(RecvTable& table, std::uint32_t hash) {
    for (std::int32_t i = 0; i < table.property_count; ++i) {
        auto& property = table.properties[i];
        if (netvar_name_hash(property.name) == hash)
            return &property;
        if (property.type == recv_data_table) {
            if (auto* found = find_receive_property(*property.data_table, hash))
                return found;
        }
    }
    return nullptr;
}

std::uint32_t find_receive_offset(RecvTable& table, std::uint32_t hash, std::uint32_t base) {
    for (std::int32_t i = 0; i < table.property_count; ++i) {
        auto& property = table.properties[i];
        if (std::strncmp(property.name, "base", 4) == 0)
            continue;
        const auto offset = base + static_cast<std::uint32_t>(property.offset);
        if (netvar_name_hash(property.name) == hash)
            return offset;
        if (property.type == recv_data_table && property.data_table->property_count > 0) {
            const auto found = find_receive_offset(*property.data_table, hash, offset);
            if (found != std::numeric_limits<std::uint32_t>::max())
                return found;
        }
    }
    return std::numeric_limits<std::uint32_t>::max();
}

RecvProp* find_receive_property(void* base_client, std::uint32_t class_hash,
                                std::uint32_t table_hash, std::uint32_t property_hash) {
    auto* node = find_client_class(base_client, class_hash);
    if (!node)
        return nullptr;
    auto* table = find_receive_table(*node->table, table_hash);
    return table ? find_receive_property(*table, property_hash) : nullptr;
}

void bind_receive_offset(void* base_client, const OffsetBinding& binding, std::uint32_t*& cursor) {
    auto* node = find_client_class(base_client, binding.class_hash);
    if (!node)
        return;
    const auto offset = find_receive_offset(*node->table, binding.property_hash);
    *binding.pointer_output = cursor;
    *binding.xor_key ^= *cursor ^ (offset + binding.addend);
    ++cursor;
}

std::uint32_t decode_receive_offset(const std::uint32_t* pointer, std::uint32_t xor_key,
                                    std::uint32_t consumer_key) noexcept {
    return *pointer ^ xor_key ^ consumer_key;
}

} // namespace skeet::engine

// ============================================================================
// Subcomponent: netvars_datamaps.cpp
// ============================================================================

namespace skeet::engine {

std::uint32_t find_data_map_offset(DataMap& map, std::uint32_t table_hash,
                                   std::uint32_t property_hash) {
    const auto class_hash = netvar_name_hash(map.class_name);
    for (std::int32_t i = 0; i < map.field_count; ++i) {
        const auto& field = map.fields[i];
        if (!field.name || std::strlen(field.name) < 4)
            continue;
        if (class_hash == table_hash && netvar_name_hash(field.name) == property_hash)
            return field.offset;
        if (field.embedded && field.embedded->class_name) {
            const auto result = find_data_map_offset(*field.embedded, table_hash, property_hash);
            if (result != std::numeric_limits<std::uint32_t>::max())
                return result;
        }
    }
    if (map.base && map.base->class_name)
        return find_data_map_offset(*map.base, table_hash, property_hash);
    return std::numeric_limits<std::uint32_t>::max();
}

std::byte* find_byte_pattern(std::byte* first, std::uint32_t size, const std::uint8_t* pattern,
                             std::uint32_t length, std::uint8_t wildcard) {
    if (!first || size < length || !pattern)
        return nullptr;
    const auto last = length - 1;
    auto wildcard_position = last;
    while (wildcard_position && pattern[wildcard_position] != wildcard)
        --wildcard_position;
    auto distance = static_cast<std::uint8_t>(last - wildcard_position);
    if (distance == 0)
        distance = 1;
    std::array<std::uint8_t, 256> shifts;
    shifts.fill(distance);
    for (auto index = last - distance; index < last; ++index)
        shifts[pattern[index]] = static_cast<std::uint8_t>(last - index);
    // The native helper clamps the address addition to UINT32_MAX. Host-side
    // tests have wider addresses; valid ranges cannot overflow their address.
    const auto remaining = static_cast<std::uintptr_t>(~reinterpret_cast<std::uintptr_t>(first));
    const auto effective_size = static_cast<std::uint32_t>(remaining < size ? remaining : size);
    const auto end = first + effective_size - length;
    while (first <= end) {
        std::int32_t index = static_cast<std::int32_t>(last);
        while (index >= 0) {
            const auto expected = pattern[index];
            if (expected != wildcard && std::to_integer<std::uint8_t>(first[index]) != expected)
                break;
            if (index == 0)
                return first;
            --index;
        }
        first += shifts[std::to_integer<std::uint8_t>(first[last])];
    }
    return nullptr;
}

DataMap* find_data_map(std::byte* client_module, std::uint32_t class_hash) {
    if (!client_module)
        return nullptr;
    static constexpr std::array<std::uint8_t, 29> pattern{
        0xcc, 0xcc, 0xcc, 0xcc, 0xc7, 0x05, 0, 0, 0, 0,    0,    0,    0,    0,   0xc7,
        0x05, 0,    0,    0,    0,    0,    0, 0, 0, 0xc3, 0xcc, 0xcc, 0xcc, 0xcc};
    std::uint32_t nt_offset, image_size;
    std::memcpy(&nt_offset, client_module + 0x3c, 4);
    std::memcpy(&image_size, client_module + nt_offset + 0x50, 4);
    auto* end = client_module + image_size - pattern.size();
    auto* first = client_module;
    for (unsigned i = 0; i != 43; ++i) {
        auto* found =
            find_byte_pattern(first, static_cast<std::uint32_t>(end - first), pattern.data(),
                              static_cast<std::uint32_t>(pattern.size()), 0);
        if (!found)
            return nullptr;
        DataMap* map;
        std::memcpy(&map, found + 16, sizeof(map));
        if (map && netvar_name_hash(map->class_name) == class_hash)
            return map;
        first = found + pattern.size();
    }
    return nullptr;
}

void bind_data_map_offset(const DataMapOffsetBinding& binding, std::uint32_t*& cursor) {
    if (!binding.map)
        return;
    *binding.pointer_output = cursor;
    const auto offset =
        find_data_map_offset(*binding.map, binding.table_hash, binding.property_hash);
    *binding.xor_key ^= *cursor ^ offset;
    ++cursor;
}

} // namespace skeet::engine

// ============================================================================
// Subcomponent: netvar_initialization.cpp
// ============================================================================

namespace skeet::engine {
namespace {
// Build constants: observed1179 operand values from analysis/contexts.json.
// Semantic names and all282 output encodings independently match the working
// engine dump. This manifest does not implement the protected operand decoder.
constexpr std::array<ReceiveOffsetRecipe, 268> receive_recipes{{
    {0x1678u, "CBaseCombatWeapon", "m_flNextPrimaryAttack", 0x4346A458u, 0x43467A50u, 0x00000000u,
     0xF71CE331u},
    {0x1688u, "CBaseCombatWeapon", "m_iWorldDroppedModelIndex", 0x43468904u, 0x43467474u,
     0x00000000u, 0x26F3C118u},
    {0x1698u, "CBaseCombatWeapon", "m_iWorldModelIndex", 0x43467DE4u, 0x434679E4u, 0x00000000u,
     0xE7BAED36u},
    {0x16A8u, "CBaseCombatWeapon", "m_flNextSecondaryAttack", 0x43468E9Cu, 0x434670A4u, 0x00000000u,
     0xC0D5F8A1u},
    {0x16B8u, "CBaseCombatWeapon", "m_iPrimaryAmmoType", 0x4346A628u, 0x43467EC8u, 0x00000000u,
     0x0C52DA61u},
    {0x16C8u, "CBaseCombatWeapon", "m_iState", 0x4346A5B0u, 0x43468F40u, 0x00000000u, 0x71A5AC08u},
    {0x16D8u, "CBaseCombatWeapon", "m_iClip1", 0x434678A8u, 0x4346A60Cu, 0x00000000u, 0xCA5EC698u},
    {0x16E8u, "CBaseCombatWeapon", "m_iViewModelIndex", 0x43468E74u, 0x43469370u, 0x00000000u,
     0x1D7092B7u},
    {0x16F8u, "CBaseCombatWeapon", "m_nViewModelIndex", 0x43467E94u, 0x43468D48u, 0x00000000u,
     0xC6C78C82u},
    {0x1708u, "CBaseCombatWeapon", "m_hWeaponWorldModel", 0x43467FA4u, 0x43467180u, 0x00000000u,
     0xCA2B5FFBu},
    {0x1718u, "CBaseCombatWeapon", "m_iPrimaryReserveAmmoCount", 0x43468C14u, 0x4346749Cu,
     0x00000000u, 0x49C770CAu},
    {0x1728u, "CBaseCombatWeapon", "m_hOwner", 0x43467FB4u, 0x434679D8u, 0x00000000u, 0x87907BC3u},
    {0x1738u, "CBaseAnimating", "m_nForceBone", 0x4346A8D4u, 0x43468938u, 0x00000000u, 0x7E62909Fu},
    {0x1748u, "CBaseAnimating", "m_nHitboxSet", 0x4346A84Cu, 0x43468240u, 0x00000000u, 0xC2B6FB10u},
    {0x1758u, "CBaseAnimating", "m_nBody", 0x43467B88u, 0x4346A5F8u, 0x00000000u, 0x950F7B20u},
    {0x1768u, "CBaseAnimating", "m_nSkin", 0x4346898Cu, 0x43468310u, 0x00000000u, 0x5173A531u},
    {0x1778u, "CBaseAnimating", "m_nSequence", 0x4346A56Cu, 0x43467ABCu, 0x00000000u, 0xA25D8271u},
    {0x1788u, "CBaseAnimating", "m_flCycle", 0x43468A88u, 0x4346A608u, 0x00000000u, 0xBEFF3172u},
    {0x1798u, "CBaseAnimating", "m_flModelScale", 0x434683CCu, 0x434682C0u, 0x00000000u,
     0x1952B857u},
    {0x17A8u, "CBaseAnimating", "m_bClientSideAnimation", 0x434689C8u, 0x43468DBCu, 0x00000000u,
     0x8481EA04u},
    {0x17B8u, "CBaseAnimating", "m_nNewSequenceParity", 0x43468414u, 0x43467B4Cu, 0x00000000u,
     0xAEF72A04u},
    {0x17C8u, "CBaseAnimating", "m_nResetEventsParity", 0x43468420u, 0x43468C38u, 0x00000000u,
     0x43D8CA19u},
    {0x17D8u, "CBaseAnimating", "m_bClientSideRagdoll", 0x43467D74u, 0x4346700Cu, 0x00000000u,
     0x1B5F47D5u},
    {0x17E8u, "CBaseAnimating", "m_flFrozen", 0x43467994u, 0x43468494u, 0x00000000u, 0x7ACC2B00u},
    {0x17F8u, "CBaseAnimating", "m_flPlaybackRate", 0x43468E2Cu, 0x43469204u, 0x00000000u,
     0x32ECEB6Du},
    {0x1808u, "CBaseCombatCharacter", "m_hActiveWeapon", 0x434678C0u, 0x43467A74u, 0x00000000u,
     0xCD812654u},
    {0x1818u, "CBaseCombatCharacter", "m_hMyWeapons", 0x43467AF4u, 0x434685D4u, 0x00000000u,
     0xFC3297F7u},
    {0x1828u, "CBaseCombatCharacter", "m_hMyWearables", 0x43467EA0u, 0x43468968u, 0x00000000u,
     0xE96443C6u},
    {0x1838u, "CBaseCombatCharacter", "m_flNextAttack", 0x43469208u, 0x4346A80Cu, 0x00000000u,
     0xDE530A8Bu},
    {0x1848u, "CBaseCombatCharacter", "m_LastHitGroup", 0x43467960u, 0x434682A8u, 0x00000000u,
     0xD4248210u},
    {0x1858u, "CBaseCombatCharacter", "m_flTimeOfLastInjury", 0x4346A3ACu, 0x43468C88u, 0x00000000u,
     0xAE838771u},
    {0x1868u, "CBaseCombatCharacter", "m_nRelativeDirectionOfLastInjury", 0x4346A528u, 0x43468914u,
     0x00000000u, 0xA3ABBB09u},
    {0x1878u, "CBaseCombatCharacter", "m_iAmmo", 0x4346A6D4u, 0x4346A320u, 0x00000000u,
     0x213040ABu}, // Native lookup is UINT32_MAX: property absent from this class.
    {0x1888u, "CBaseEntity", "m_iParentAttachment", 0x434671C0u, 0x43467568u, 0x00000000u,
     0xAD25C138u},
    {0x1898u, "CBaseEntity", "m_flLastMadeNoiseTime", 0x43468B98u, 0x43468990u, 0x00000000u,
     0x15335AACu},
    {0x18A8u, "CBaseEntity", "m_fEffects", 0x43467B48u, 0x43468B50u, 0x00000000u, 0x38CA5228u},
    {0x18B8u, "CBaseEntity", "m_nModelIndex", 0x43468F7Cu, 0x43468978u, 0x00000000u, 0x47877DEBu},
    {0x18C8u, "CBaseEntity", "m_bSpotted", 0x4346722Cu, 0x43467D48u, 0x00000000u, 0xFB1E8DABu},
    {0x18D8u, "CBaseEntity", "m_fadeMinDist", 0x4346919Cu, 0x43467948u, 0x00000000u, 0x68140CD8u},
    {0x18E8u, "CBaseEntity", "m_clrRender", 0x43467904u, 0x434682B0u, 0x00000000u, 0x4999428Du},
    {0x18F8u, "CBaseEntity", "m_Collision", 0x43467018u, 0x43469344u, 0x00000000u, 0x49D93E62u},
    {0x1908u, "CBaseEntity", "m_flSimulationTime", 0x4346A538u, 0x43469198u, 0x00000000u,
     0xD5FD2E2Cu},
    {0x1918u, "CBaseEntity", "m_hOwnerEntity", 0x43467938u, 0x43467FCCu, 0x00000000u, 0x8D1BD8D0u},
    {0x1928u, "CBaseEntity", "m_iTeamNum", 0x4346A518u, 0x434688B0u, 0x00000000u, 0x8B5586F6u},
    {0x1938u, "CBaseEntity", "m_flFadeScale", 0x43469278u, 0x43468BD0u, 0x00000000u, 0x8DBC9874u},
    {0x1948u, "CBaseEntity", "m_CollisionGroup", 0x4346758Cu, 0x434683C8u, 0x00000000u,
     0xE4E2D1A7u},
    {0x1958u, "CBaseEntity", "m_nRenderMode", 0x43468C80u, 0x43467DB0u, 0x00000000u, 0xFC2723F7u},
    {0x1968u, "CBaseEntity", "m_nRenderFX", 0x43467A00u, 0x4346A560u, 0x00000000u, 0x688E0292u},
    {0x1978u, "CBaseEntity", "m_usSolidFlags", 0x4346A3BCu, 0x43467044u, 0x00000000u, 0xB7E9C94Eu},
    {0x1988u, "CBaseEntity", "m_nSolidType", 0x4346A744u, 0x43467964u, 0x00000000u, 0x902A2BFDu},
    {0x1998u, "CBasePlayer", "m_iHealth", 0x4346701Cu, 0x4346A438u, 0x00000000u, 0x9BB80163u},
    {0x19A8u, "CBasePlayer", "m_nTickBase", 0x43468BC4u, 0x434693D8u, 0x00000000u, 0x3D10B94Cu},
    {0x19B8u, "CBasePlayer", "m_flDuckAmount", 0x434675F8u, 0x434679F8u, 0x00000000u, 0x3309C0CFu},
    {0x19C8u, "CBasePlayer", "m_ubEFNoInterpParity", 0x434671F8u, 0x4346934Cu, 0x00000000u,
     0x334B460Cu},
    {0x19D8u, "CBasePlayer", "m_flFallVelocity", 0x43468584u, 0x4346A464u, 0x00000000u,
     0x7E9BA7BCu},
    {0x19E8u, "CBasePlayer", "m_flFOVTime", 0x43468DCCu, 0x43468D10u, 0x00000000u, 0x3F51AA0Au},
    {0x19F8u, "CBasePlayer", "m_vphysicsCollisionState", 0x43469260u, 0x4346A41Cu, 0x00000000u,
     0xB5A66556u},
    {0x1A08u, "CBasePlayer", "m_aimPunchAngleVel", 0x43467A64u, 0x43467F74u, 0x00000000u,
     0x48442721u},
    {0x1A18u, "CBasePlayer", "m_bDucking", 0x4346A564u, 0x4346A398u, 0x00000000u, 0x6B1A59B5u},
    {0x1A28u, "CBasePlayer", "m_flLastDuckTime", 0x43468B34u, 0x43468558u, 0x00000000u,
     0x04B798C6u},
    {0x1A38u, "CBasePlayer", "m_iFOV", 0x43469218u, 0x43468248u, 0x00000000u, 0xFBE8C5EAu},
    {0x1A48u, "CBasePlayer", "m_hLastWeapon", 0x43468B70u, 0x43468D80u, 0x00000000u, 0xF9D209AAu},
    {0x1A58u, "CBasePlayer", "m_iObserverMode", 0x43468EF8u, 0x4346A6CCu, 0x00000000u, 0x2F4E97C4u},
    {0x1A68u, "CBasePlayer", "m_lifeState", 0x434684E4u, 0x43468E78u, 0x00000000u, 0x9D920E35u},
    {0x1A78u, "CBasePlayer", "m_fFlags", 0x43468C24u, 0x434688D0u, 0x00000000u, 0xE2FBE695u},
    {0x1A88u, "CBasePlayer", "m_iDefaultFOV", 0x43467C78u, 0x43468D84u, 0x00000000u, 0xA3153415u},
    {0x1A98u, "CBasePlayer", "m_flStepSize", 0x43468504u, 0x43468988u, 0x00000000u, 0x59CF69F9u},
    {0x1AA8u, "CBasePlayer", "m_aimPunchAngle", 0x43467D90u, 0x434682B8u, 0x00000000u, 0x1BAE1168u},
    {0x1AB8u, "CBasePlayer", "m_flMaxspeed", 0x43468A84u, 0x434685CCu, 0x00000000u, 0x3EB71843u},
    {0x1AC8u, "CBasePlayer", "m_bDucked", 0x43467B98u, 0x43468D9Cu, 0x00000000u, 0x50F9CCC8u},
    {0x1AD8u, "CBasePlayer", "m_nWaterLevel", 0x4346905Cu, 0x434678A4u, 0x00000000u, 0xAC7F2913u},
    {0x1AE8u, "CBasePlayer", "m_hObserverTarget", 0x43467E64u, 0x43468F84u, 0x00000000u,
     0x3B704B41u},
    {0x1AF8u, "CBasePlayer", "m_viewPunchAngle", 0x43468324u, 0x4346A390u, 0x00000000u,
     0x5AE32976u},
    {0x1B08u, "CBasePlayer", "m_iFOVStart", 0x43467BA0u, 0x43468CA8u, 0x00000000u, 0x35B5B2ECu},
    {0x1B18u, "CBasePlayer", "m_flDuckSpeed", 0x4346A52Cu, 0x43467DD4u, 0x00000000u, 0xFB7E3404u},
    {0x1B28u, "CBasePlayer", "m_flDeathTime", 0x434684C8u, 0x43469244u, 0x00000000u, 0x6E4953EBu},
    {0x1B38u, "CBasePlayer", "m_nOldButtons", 0x43467AF0u, 0x4346A3E4u, 0x00000000u, 0xD1C1093Eu},
    {0x1B48u, "CCSPlayer", "m_flStamina", 0x434693ACu, 0x43468B30u, 0x00000000u, 0x2F33C681u},
    {0x1B58u, "CCSPlayer", "m_bGunGameImmunity", 0x4346A610u, 0x43467DD0u, 0x00000000u,
     0xAF649DACu},
    {0x1B68u, "CCSPlayer", "m_bIsWalking", 0x43468DB8u, 0x43468C40u, 0x00000000u, 0x7090D8DDu},
    {0x1B78u, "CCSPlayer", "m_flFlashDuration", 0x434675DCu, 0x4346A70Cu, 0x00000000u, 0xCCC9884Eu},
    {0x1B88u, "CCSPlayer", "m_fMolotovDamageTime", 0x43467A9Cu, 0x434684CCu, 0x00000000u,
     0x8815D240u},
    {0x1B98u, "CCSPlayer", "m_bResumeZoom", 0x434671FCu, 0x43467A90u, 0x00000000u, 0xC07D1150u},
    {0x1BA8u, "CCSPlayer", "m_bWaitForNoAttack", 0x4346A2ECu, 0x4346799Cu, 0x00000000u,
     0xBCB263A5u},
    {0x1BB8u, "CCSPlayer", "m_bHasMovedSinceSpawn", 0x43467B9Cu, 0x4346A618u, 0x00000000u,
     0x28040C16u},
    {0x1BC8u, "CCSPlayer", "m_flLowerBodyYawTarget", 0x4346A3C4u, 0x43468FECu, 0x00000000u,
     0xA2A8247Fu},
    {0x1BD8u, "CCSPlayer", "m_iAddonBits", 0x43469258u, 0x43467584u, 0x00000000u, 0x86A5F8B3u},
    {0x1BE8u, "CCSPlayer", "m_iAccount", 0x4346A7F4u, 0x43467A58u, 0x00000000u, 0x6A00049Eu},
    {0x1BF8u, "CCSPlayer", "m_bIsControllingBot", 0x4346891Cu, 0x43468360u, 0x00000000u,
     0x40C98EF6u},
    {0x1C08u, "CCSPlayer", "m_hRagdoll", 0x434670F0u, 0x434691A0u, 0x00000000u, 0xD5D91363u},
    {0x1C18u, "CCSPlayer", "m_passiveItems", 0x43468DF0u, 0x43467D58u, 0x00000000u, 0x76696C4Bu},
    {0x1C28u, "CCSPlayer", "m_iMoveState", 0x4346853Cu, 0x43468EE4u, 0x00000000u, 0x52899DF3u},
    {0x1C38u, "CCSPlayer", "m_bHasHelmet", 0x43468FE8u, 0x43468C04u, 0x00000000u, 0x7D6280C7u},
    {0x1C48u, "CCSPlayer", "m_iShotsFired", 0x43468BD8u, 0x4346A6DCu, 0x00000000u, 0x8CFC633Au},
    {0x1C58u, "CCSPlayer", "m_bHasHeavyArmor", 0x434671F4u, 0x434671C4u, 0x00000000u, 0xEAA57C2Cu},
    {0x1C68u, "CCSPlayer", "m_szArmsModel", 0x43468C28u, 0x43468E20u, 0x00000000u, 0xAAAE7565u},
    {0x1C78u, "CCSPlayer", "m_flFlashMaxAlpha", 0x43467B78u, 0x434688A0u, 0x00000000u, 0x5461EE58u},
    {0x1C88u, "CCSPlayer", "m_totalHitsOnServer", 0x43467158u, 0x43468D34u, 0x00000000u,
     0xF7DF3858u},
    {0x1C98u, "CCSPlayer", "m_bStrafing", 0x43467164u, 0x434679CCu, 0x00000000u, 0x00A5BD68u},
    {0x1CA8u, "CCSPlayer", "m_bIsDefusing", 0x43468C34u, 0x4346A7E0u, 0x00000000u, 0xA15C3105u},
    {0x1CB8u, "CCSPlayer", "m_ArmorValue", 0x434682E4u, 0x43467908u, 0x00000000u, 0x47A6F50Cu},
    {0x1CC8u, "CCSPlayer", "m_flThirdpersonRecoil", 0x4346852Cu, 0x43468354u, 0x00000000u,
     0x66146802u},
    {0x1CD8u, "CCSPlayer", "m_bHasDefuser", 0x43469024u, 0x4346A4F4u, 0x00000000u, 0xA15F5884u},
    {0x1CE8u, "CCSPlayer", "m_nSurvivalTeam", 0x43467B7Cu, 0x434678ACu, 0x00000000u, 0x430C8993u},
    {0x1CF8u, "CCSPlayer", "m_bIsScoped", 0x434684ECu, 0x43468DA4u, 0x00000000u, 0xCACAEECCu},
    {0x1D08u, "CCSPlayer", "m_flVelocityModifier", 0x43467BD4u, 0x4346791Cu, 0x00000000u,
     0x7F5ED2D0u},
    {0x1D18u, "CCSPlayer", "m_bInBombZone", 0x4346A654u, 0x4346924Cu, 0x00000000u, 0x7FCB7381u},
    {0x1D28u, "CCSPlayer", "m_bIsPlayerGhost", 0x43468430u, 0x4346A5DCu, 0x00000000u, 0xCDE290D2u},
    {0x1D38u, "CCSPlayer", "m_flGroundAccelLinearFracLastTime", 0x43467100u, 0x43467230u,
     0x00000000u, 0xD3D4230Du},
    {0x1D48u, "CCSPlayer", "m_bIsRescuing", 0x43468498u, 0x43467C38u, 0x00000000u, 0xAF6689C2u},
    {0x1D58u, "CCSPlayer", "m_bIsLookingAtWeapon", 0x43467AF8u, 0x43467EC4u, 0x00000000u,
     0x53E7C7D2u},
    {0x1D68u, "CCSPlayer", "m_bInBuyZone", 0x43468514u, 0x4346A3B4u, 0x00000000u, 0xFAEB9B15u},
    {0x1D78u, "CBaseCSGrenade", "m_bPinPulled", 0x43467B1Cu, 0x434670ECu, 0x00000000u, 0xE1DF7E9Bu},
    {0x1D88u, "CBaseCSGrenade", "m_flThrowStrength", 0x43468E34u, 0x43467D1Cu, 0x00000000u,
     0x1B9654D9u},
    {0x1D98u, "CBaseCSGrenade", "m_bIsHeldByPlayer", 0x4346A658u, 0x43467A6Cu, 0x00000000u,
     0xE865A877u},
    {0x1DA8u, "CBaseCSGrenade", "m_bRedraw", 0x4346A6A8u, 0x43467BE4u, 0x00000000u, 0x4013FC03u},
    {0x1DB8u, "CBaseCSGrenade", "m_fThrowTime", 0x43467BF4u, 0x43467AA4u, 0x00000000u, 0x3B93667Bu},
    {0x1DC8u, "CPlayerResource", "m_iPing", 0x4346A708u, 0x43467D30u, 0x00000000u, 0xB6660491u},
    {0x1DD8u, "CPlayerResource", "m_iKills", 0x43468DF4u, 0x43467168u, 0x00000000u, 0x6D5B1E34u},
    {0x1DE8u, "CPlayerResource", "m_iDeaths", 0x43467DB8u, 0x4346A6A0u, 0x00000000u, 0x0896CC62u},
    {0x1DF8u, "CPlayerResource", "m_iTeam", 0x4346A558u, 0x434684F0u, 0x00000000u, 0xC1C49A34u},
    {0x1E08u, "CPlayerResource", "m_bAlive", 0x4346857Cu, 0x4346A724u, 0x00000000u, 0xD266B537u},
    {0x1E18u, "CPlayerResource", "m_iHealth", 0x434675B4u, 0x4346716Cu, 0x00000000u, 0x9BB80163u},
    {0x1E28u, "CPlayerResource", "m_bConnected", 0x4346A814u, 0x43467CB8u, 0x00000000u,
     0x9571D5F5u},
    {0x1E38u, "CCSPlayerResource", "m_iCashSpentThisRound", 0x43468A80u, 0x4346717Cu, 0x00000000u,
     0x5DAAA27Eu},
    {0x1E48u, "CCSPlayerResource", "m_bombsiteCenterB", 0x43467C68u, 0x43467C00u, 0x00000000u,
     0xE92192CAu},
    {0x1E58u, "CCSPlayerResource", "m_bombsiteCenterA", 0x43467DE0u, 0x4346A71Cu, 0x00000000u,
     0xFCCAF99Bu},
    {0x1E68u, "CCSPlayerResource", "m_iPlayerC4", 0x43468D70u, 0x4346A370u, 0x00000000u,
     0x23B532EFu},
    {0x1E78u, "CCSPlayerResource", "m_iCompetitiveWins", 0x43467BF0u, 0x43467C54u, 0x00000000u,
     0x8C7BA015u},
    {0x1E88u, "CCSPlayerResource", "m_iCompetitiveRanking", 0x4346A32Cu, 0x43468F48u, 0x00000000u,
     0x827FEFEAu},
    {0x1E98u, "CCSPlayerResource", "m_iTotalCashSpent", 0x43467DCCu, 0x43467B50u, 0x00000000u,
     0xE8B295CAu},
    {0x1EA8u, "CCSPlayerResource", "m_iMatchStats_CashEarned_Total", 0x43467E6Cu, 0x434670DCu,
     0x00000000u, 0x56F87E51u},
    {0x1EB8u, "CCSPlayerResource", "m_bHasDefuser", 0x4346A468u, 0x4346A590u, 0x00000000u,
     0xA15F5884u},
    {0x1EC8u, "CCSPlayerResource", "m_bHasHelmet", 0x43467194u, 0x434674A4u, 0x00000000u,
     0x7D6280C7u},
    {0x1ED8u, "CCSRagdoll", "m_hPlayer", 0x43468A38u, 0x43467D80u, 0x00000000u, 0x9C14F5A7u},
    {0x1EE8u, "CCSRagdoll", "m_vecRagdollVelocity", 0x4346824Cu, 0x434693A0u, 0x00000000u,
     0x379636EEu},
    {0x1EF8u, "CCSRagdoll", "m_vecRagdollOrigin", 0x43467CBCu, 0x43467998u, 0x00000000u,
     0x258BF82Bu},
    {0x1F08u, "CCSRagdoll", "m_flAbsYaw", 0x434670C8u, 0x43468E84u, 0x00000000u, 0x73DBBB49u},
    {0x1F18u, "CC4", "m_bBombPlacedAnimation", 0x4346A630u, 0x434685D8u, 0x00000000u, 0x5E3393CBu},
    {0x1F28u, "CC4", "m_fArmedTime", 0x434679E8u, 0x43467AD8u, 0x00000000u, 0x9E4FF938u},
    {0x1F38u, "CC4", "m_bStartedArming", 0x4346A848u, 0x4346A8ACu, 0x00000000u, 0x9A6D48BDu},
    {0x1F48u, "CPlantedC4", "m_flC4Blow", 0x4346A880u, 0x4346834Cu, 0x00000000u, 0xD280D4E1u},
    {0x1F58u, "CPlantedC4", "m_flDefuseCountDown", 0x4346A728u, 0x4346A750u, 0x00000000u,
     0x800BB0B1u},
    {0x1F68u, "CPlantedC4", "m_bBombTicking", 0x43468EF4u, 0x4346A6C4u, 0x00000000u, 0x0E5BECE5u},
    {0x1F78u, "CWeaponCSBase", "m_flPostponeFireReadyTime", 0x4346A428u, 0x43468FD8u, 0x00000000u,
     0x43D0EFC4u},
    {0x1F88u, "CWeaponCSBase", "m_flRecoilIndex", 0x4346A774u, 0x43467B44u, 0x00000000u,
     0x6777C9CEu},
    {0x1F98u, "CWeaponCSBase", "m_weaponMode", 0x43469084u, 0x4346796Cu, 0x00000000u, 0x572AA59Fu},
    {0x1FA8u, "CWeaponCSBase", "m_hPrevOwner", 0x43468998u, 0x4346A868u, 0x00000000u, 0x4A3412ECu},
    {0x1FB8u, "CWeaponCSBase", "m_bBurstMode", 0x434678B0u, 0x4346A768u, 0x00000000u, 0xA1AD86BFu},
    {0x1FC8u, "CWeaponCSBase", "m_bReloadVisuallyComplete", 0x43467154u, 0x43468464u, 0x00000000u,
     0x7A4E2019u},
    {0x1FD8u, "CWeaponCSBase", "m_iOriginalTeamNumber", 0x43469334u, 0x43468E5Cu, 0x00000000u,
     0x05FB21BAu},
    {0x1FE8u, "CWeaponCSBase", "m_fAccuracyPenalty", 0x434671E4u, 0x4346A620u, 0x00000000u,
     0xD2563274u},
    {0x1FF8u, "CWeaponCSBase", "m_iIronSightMode", 0x43467DD8u, 0x4346A340u, 0x00000000u,
     0x01AA449Du},
    {0x2008u, "CWeaponCSBase", "m_fLastShotTime", 0x4346A5FCu, 0x43467200u, 0x00000000u,
     0x0176A381u},
    {0x2018u, "CWeaponCSBase", "m_iRecoilIndex", 0x4346A810u, 0x43467E90u, 0x00000000u,
     0x8520C857u},
    {0x2028u, "CWeaponCSBaseGun", "m_iBurstShotsRemaining", 0x43468E64u, 0x43467D40u, 0x00000000u,
     0xF93CEAF4u},
    {0x2038u, "CWeaponCSBaseGun", "m_zoomLevel", 0x43468E8Cu, 0x43467D20u, 0x00000000u,
     0xD6A88AA5u},
    {0x2048u, "CBaseViewModel", "m_hWeapon", 0x43467E78u, 0x43468A98u, 0x00000000u, 0xA116369Au},
    {0x2058u, "CBaseViewModel", "m_nViewModelIndex", 0x4346927Cu, 0x434679A0u, 0x00000000u,
     0xC6C78C82u},
    {0x2068u, "CBaseViewModel", "m_hOwner", 0x434682D8u, 0x434683B0u, 0x00000000u, 0x87907BC3u},
    {0x2078u, "CBaseViewModel", "m_nAnimationParity", 0x4346A6BCu, 0x43468DA0u, 0x00000000u,
     0x695FF325u},
    {0x2088u, "CCSGameRulesProxy", "m_bBombPlanted", 0x43467D50u, 0x4346746Cu, 0x00000000u,
     0xD000A932u},
    {0x2098u, "CCSGameRulesProxy", "m_bBombDropped", 0x43468F24u, 0x4346921Cu, 0x00000000u,
     0x82F5AD08u},
    {0x20A8u, "CCSGameRulesProxy", "m_bIsValveDS", 0x43468244u, 0x43468378u, 0x00000000u,
     0xBF567619u},
    {0x20B8u, "CCSGameRulesProxy", "m_bFreezePeriod", 0x43467C08u, 0x43467910u, 0x00000000u,
     0xD720AEA6u},
    {0x20C8u, "CCSGameRulesProxy", "m_bWarmupPeriod", 0x43468500u, 0x43468D1Cu, 0x00000000u,
     0xF57BEAA9u},
    {0x20D8u, "CCSGameRulesProxy", "m_fRoundStartTime", 0x43468428u, 0x43468D6Cu, 0x00000000u,
     0x9F8E7F71u},
    {0x20E8u, "CCSGameRulesProxy", "m_gamePhase", 0x43468300u, 0x4346926Cu, 0x00000000u,
     0xA5085C33u},
    {0x20F8u, "CCSGameRulesProxy", "m_iRoundTime", 0x4346A378u, 0x43468BE4u, 0x00000000u,
     0x972BC124u},
    {0x2108u, "CCSGameRulesProxy", "m_bIsQueuedMatchmaking", 0x43468614u, 0x4346836Cu, 0x00000000u,
     0xDDA99AB5u},
    {0x2118u, "CCSGameRulesProxy", "m_nQueuedMatchmakingMode", 0x4346A414u, 0x43468BF4u,
     0x00000000u, 0xB7EA8FDAu},
    {0x2128u, "CEconEntity", "m_bInitialized", 0x434679A8u, 0x43469230u, 0x00000000u, 0xA1AF8FFCu},
    {0x2138u, "CEconEntity", "m_iAccountID", 0x43468B78u, 0x4346A358u, 0x00000000u, 0x1136BA97u},
    {0x2148u, "CEconEntity", "m_iEntityQuality", 0x4346A818u, 0x43468940u, 0x00000000u,
     0xADBAA629u},
    {0x2158u, "CEconEntity", "m_iItemDefinitionIndex", 0x43468BA0u, 0x4346908Cu, 0x00000000u,
     0x5C9F0FBBu},
    {0x2168u, "CEconEntity", "m_OriginalOwnerXuidHigh", 0x434683E4u, 0x43468390u, 0x00000000u,
     0xA1493792u},
    {0x2178u, "CEconEntity", "m_nFallbackSeed", 0x43467A44u, 0x43468DC8u, 0x00000000u, 0x2A595D03u},
    {0x2188u, "CEconEntity", "m_nFallbackStatTrak", 0x4346A6C8u, 0x434682E8u, 0x00000000u,
     0xF2DC2C8Au},
    {0x2198u, "CEconEntity", "m_flFallbackWear", 0x43469200u, 0x43468950u, 0x00000000u,
     0x67FC28B7u},
    {0x21A8u, "CEconEntity", "m_Item", 0x43469220u, 0x43467150u, 0x00000000u, 0x587376B7u},
    {0x21B8u, "CEconEntity", "m_iEntityLevel", 0x4346902Cu, 0x4346838Cu, 0x00000000u, 0x8FDF1732u},
    {0x21C8u, "CEconEntity", "m_nFallbackPaintKit", 0x43468568u, 0x4346A8A8u, 0x00000000u,
     0xC6D22EE2u},
    {0x21D8u, "CEconEntity", "m_OriginalOwnerXuidLow", 0x4346798Cu, 0x43469248u, 0x00000000u,
     0x3C1D8AA6u},
    {0x21E8u, "CEconEntity", "m_iItemIDLow", 0x43467188u, 0x43467F9Cu, 0x00000000u, 0xB4056E61u},
    {0x21F8u, "CEconEntity", "m_iItemIDHigh", 0x43468F28u, 0x43468C1Cu, 0x00000000u, 0x3C5CCD57u},
    {0x2208u, "CEconEntity", "m_AttributeManager", 0x43468FA0u, 0x4346A784u, 0x00000000u,
     0xB59B7E97u},
    {0x2218u, "CBaseCSGrenadeProjectile", "m_nExplodeEffectTickBegin", 0x43467630u, 0x43467210u,
     0x00000000u, 0x01E80986u},
    {0x2228u, "CBaseCSGrenadeProjectile", "m_nBounces", 0x43467DECu, 0x43467488u, 0x00000000u,
     0x7418868Fu},
    {0x2238u, "CBaseCSGrenadeProjectile", "m_vInitialVelocity", 0x434693C0u, 0x434683E8u,
     0x00000000u, 0x434E9E95u},
    {0x2248u, "CSmokeGrenadeProjectile", "m_bDidSmokeEffect", 0x43467564u, 0x4346A530u, 0x00000000u,
     0x58AE2423u},
    {0x2258u, "CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", 0x43468E60u, 0x43467AC0u,
     0x00000000u, 0xF267E956u},
    {0x2268u, "CTEFireBullets", "m_weapon", 0x43467290u, 0x43468490u, 0x00000000u, 0xB3832000u},
    {0x2278u, "CTEFireBullets", "m_iMode", 0x434670D8u, 0x43467034u, 0x00000000u, 0xEC1739AAu},
    {0x2288u, "CTEFireBullets", "m_vecOrigin", 0x43468418u, 0x43468910u, 0x00000000u, 0x2A671B42u},
    {0x2298u, "CTEFireBullets", "m_iWeaponID", 0x43468BFCu, 0x4346A6ACu, 0x00000000u, 0xA432EB44u},
    {0x22A8u, "CTEFireBullets", "m_fInaccuracy", 0x4346850Cu, 0x43467480u, 0x00000000u,
     0xC6C62EBAu},
    {0x22B8u, "CTEFireBullets", "m_iSeed", 0x43468B6Cu, 0x434693FCu, 0x00000000u, 0xF300F6B0u},
    {0x22C8u, "CTEFireBullets", "m_fSpread", 0x43468F20u, 0x43468E90u, 0x00000000u, 0xE9392423u},
    {0x22D8u, "CTEFireBullets", "m_iPlayer", 0x434678BCu, 0x43467EC0u, 0x00000000u, 0xB6F85C08u},
    {0x22E8u, "CTEFireBullets", "m_nItemDefIndex", 0x4346A61Cu, 0x43468B28u, 0x00000000u,
     0x571B7D9Au},
    {0x22F8u, "CTEFireBullets", "m_flRecoilIndex", 0x4346A324u, 0x4346A45Cu, 0x00000000u,
     0x6777C9CEu},
    {0x2308u, "CBaseGrenade", "m_hThrower", 0x4346A888u, 0x43468C18u, 0x00000000u, 0xDC743513u},
    {0x2318u, "CBaseGrenade", "m_flDamage", 0x43467120u, 0x4346A6A4u, 0x00000000u, 0x9D63BB7Fu},
    {0x2328u, "CBaseWeaponWorldModel", "m_hCombatWeaponParent", 0x434691D8u, 0x4346922Cu,
     0x00000000u, 0xA4CECBE2u},
    {0x2338u, "CBaseGrenade", "m_bIsLive", 0x4346A450u, 0x43467DA4u, 0x00000000u, 0xD5CB4CF2u},
    {0x2348u, "CBaseGrenade", "m_DmgRadius", 0x4346894Cu, 0x434674ECu, 0x00000000u, 0x727317C4u},
    {0x2358u, "CSun", "m_bOn", 0x43467E2Cu, 0x43468BECu, 0x00000000u, 0x37482375u},
    {0x2368u, "CInferno", "m_fireZDelta", 0x43468DDCu, 0x434671E8u, 0x00000000u, 0xD7600AACu},
    {0x2378u, "CInferno", "m_fireYDelta", 0x4346892Cu, 0x43469058u, 0x00000000u, 0x94E8C7F7u},
    {0x2388u, "CInferno", "m_fireCount", 0x4346893Cu, 0x43468E28u, 0x00000000u, 0x3584E3A5u},
    {0x2398u, "CInferno", "m_bFireIsBurning", 0x4346711Cu, 0x43467D4Cu, 0x00000000u, 0xFC1EF3A1u},
    {0x23A8u, "CInferno", "m_nFireEffectTickBegin", 0x434678B4u, 0x4346A854u, 0x00000000u,
     0x42088943u},
    {0x23B8u, "CInferno", "m_fireXDelta", 0x43467574u, 0x43468A58u, 0x00000000u, 0x24A16FF6u},
    {0x23C8u, "CParticleSystem", "m_vServerControlPoints", 0x43467DF0u, 0x4346A7E8u, 0x00000000u,
     0x11081E9Du},
    {0x23D8u, "CParticleSystem", "m_flStartTime", 0x434679FCu, 0x43469270u, 0x00000000u,
     0xB2C32129u},
    {0x23E8u, "CParticleSystem", "m_iEffectIndex", 0x43468460u, 0x434692F0u, 0x00000000u,
     0x971D6FB6u},
    {0x23F8u, "CParticleSystem", "m_hControlPointEnts", 0x43468DD0u, 0x4346A6B8u, 0x00000000u,
     0x20CA1C2Du},
    {0x2408u, "CParticleSystem", "m_bActive", 0x43468E40u, 0x4346851Cu, 0x00000000u, 0x130502E2u},
    {0x2418u, "CParticleSystem", "m_nStopType", 0x43469340u, 0x43468D04u, 0x00000000u, 0x8189B4C8u},
    {0x2428u, "CParticleSystem", "m_iServerControlPointAssignments", 0x4346A830u, 0x43467B70u,
     0x00000000u, 0x28AAB41Du},
    {0x2438u, "CBreakableSurface", "m_bIsBroken", 0x43468314u, 0x4346A360u, 0x00000000u,
     0xF0374295u},
    {0x2448u, "CPhysicsProp", "m_spawnflags", 0x4346715Cu, 0x434691D0u, 0x00000000u, 0xAB641F3Eu},
    {0x2458u, "CPhysicsProp", "m_bAwake", 0x43469054u, 0x434689B4u, 0x00000000u, 0x33F253F1u},
    {0x2468u, "CPhysicsPropMultiplayer", "m_iPhysicsMode", 0x4346A5F4u, 0x43467490u, 0x00000000u,
     0x9F70B78Bu},
    {0x2478u, "CPhysicsPropMultiplayer", "m_fMass", 0x43467CF4u, 0x4346A8BCu, 0x00000000u,
     0x560EF85Cu},
    {0x2488u, "CPhysicsPropMultiplayer", "m_collisionMins", 0x4346899Cu, 0x434679D4u, 0x00000000u,
     0x61D6746Fu},
    {0x2498u, "CPhysicsPropMultiplayer", "m_collisionMaxs", 0x4346A3C8u, 0x434693F4u, 0x00000000u,
     0xAEF2D371u},
    {0x24A8u, "CPhysPropLootCrate", "m_bRenderInPSPM", 0x43468E38u, 0x4346795Cu, 0x00000000u,
     0x883CDAA1u},
    {0x24B8u, "CPhysPropLootCrate", "m_bRenderInTablet", 0x434691A4u, 0x4346A864u, 0x00000000u,
     0x7614980Bu},
    {0x24C8u, "CPhysPropLootCrate", "m_iMaxHealth", 0x43467DB4u, 0x43468470u, 0x00000000u,
     0x28710ACDu},
    {0x24D8u, "CCascadeLight", "m_shadowDirection", 0x43467484u, 0x434682A4u, 0x00000000u,
     0x31042F87u},
    {0x1390u, "CBaseEntity", "m_clrRender", 0x43467E50u, 0x434683B4u, 0xFFFFFFF8u, 0x57B2D535u},
    {0x13A4u, "CBaseAnimating", "m_hLightingOrigin", 0x43467900u, 0x43467E8Cu, 0x00000008u,
     0x32E4DAF1u},
    {0x13B8u, "CBaseAnimating", "m_nForceBone", 0x43468BE8u, 0x4346A808u, 0x00000004u, 0x2B554AD0u},
    {0x13CCu, "CBaseAnimating", "m_hLightingOrigin", 0x43468538u, 0x434675CCu, 0xFFFFFFE0u,
     0xC7E1ECD2u},
    {0x13E0u, "CBaseAnimating", "m_vecForce", 0x43468FD4u, 0x4346703Cu, 0xFFFFFFF0u, 0xAE5B982Eu},
    {0x13F4u, "CBaseAnimating", "m_nMuzzleFlashParity", 0x4346A7D0u, 0x434688B4u, 0x00000004u,
     0x0471E13Cu},
    {0x1408u, "CBaseAnimating", "m_hLightingOrigin", 0x43467498u, 0x4346A424u, 0xFFFFFFECu,
     0xB3EEFBE0u},
    {0x141Cu, "CBaseAnimating", "m_flFrozen", 0x4346A35Cu, 0x43468540u, 0x00000004u, 0x0FAF9BF0u},
    {0x1430u, "CBaseAnimating", "m_hLightingOrigin", 0x4346895Cu, 0x43468DC4u, 0xFFFFFFE8u,
     0x1E54010Cu},
    {0x1444u, "CBaseAnimating", "m_bSuppressAnimSounds", 0x434679E0u, 0x4346A660u, 0x00000042u,
     0xB538BBA5u},
    {0x1458u, "CBaseEntity", "m_CollisionGroup", 0x43469050u, 0x4346A46Cu, 0xFFFFFFD0u,
     0x8DD50A4Eu},
    {0x146Cu, "CBaseEntity", "m_vecOrigin", 0x43468BCCu, 0x434670CCu, 0x00000000u, 0xA9EE6062u},
    {0x1480u, "CBaseEntity", "m_flUseLookAtAngle", 0x43468298u, 0x4346720Cu, 0xFFFFFFE0u,
     0xA249061Au},
    {0x1494u, "CBaseEntity", "m_angRotation", 0x43467E98u, 0x4346A594u, 0x00000000u, 0xC865F10Eu},
    {0x14A8u, "CBaseEntity", "moveparent", 0x43469280u, 0x43468C9Cu, 0x00000000u, 0x4E36DFD7u},
    {0x14BCu, "CBaseEntity", "m_flShadowCastDistance", 0x43467D18u, 0x43468434u, 0x00000024u,
     0x3CDE8B30u},
    {0x14D0u, "CBaseEntity", "m_hEffectEntity", 0x43467B40u, 0x43468DD4u, 0x0000000Cu, 0x0DCBA89Cu},
    {0x14E4u, "CBasePlayer", "m_iDefaultFOV", 0x43467D88u, 0x43468A8Cu, 0x00000008u, 0x7FD04456u},
    {0x14F8u, "CBasePlayer", "m_vecViewOffset[0]", 0x43468C00u, 0x43468A34u, 0x00000000u,
     0xC246315Eu},
    {0x150Cu, "CBasePlayer", "deadflag", 0x43468BA4u, 0x43467A60u, 0x00000000u, 0xC1B66EE8u},
    {0x1520u, "CBasePlayer", "m_hViewModel[0]", 0x43467AB4u, 0x43468960u, 0x00000000u, 0x71DCEE42u},
    {0x1534u, "CBasePlayer", "m_flLaggedMovementValue", 0x43467EA4u, 0x434688ACu, 0xFFFFFF68u,
     0x5DF1A4BCu},
    {0x1548u, "CBasePlayer", "m_skybox3d.fog.enable", 0x43467F70u, 0x43468DB0u, 0x00000000u,
     0xF24C3E46u},
    {0x155Cu, "CBasePlayer", "m_skybox3d.area", 0x43468294u, 0x434684E8u, 0x00000000u, 0xA2424BD5u},
    {0x1570u, "CBasePlayer", "m_PlayerFog.m_hCtrl", 0x43468384u, 0x43467B6Cu, 0x00000000u,
     0xDC91E6CFu},
    {0x1584u, "CCSPlayer", "m_angEyeAngles[0]", 0x4346A74Cu, 0x43468EA0u, 0x00000000u, 0xE23A9BE1u},
    {0x1598u, "CCSPlayer", "m_nDeathCamMusic", 0x434683B8u, 0x4346A6C0u, 0x00000004u, 0xFAC21EAFu},
    {0x15ACu, "CCSPlayer", "m_cycleLatch", 0x434682D4u, 0x43468EB8u, 0xFFFFFFF8u, 0x668F8B33u},
    {0x15C0u, "CTEFireBullets", "m_vecAngles[0]", 0x43467BECu, 0x43468FACu, 0x00000000u,
     0xF62F1430u},
    {0x15D4u, "CEconEntity", "m_NetworkedDynamicAttributesForDemos", 0x434682B4u, 0x43469080u,
     0xFFFFFFE4u, 0xD172D9BAu},
    {0x15E8u, "CFogController", "m_fog.enable", 0x434679A4u, 0x43468918u, 0x00000000u, 0x60816092u},
    {0x15FCu, "CHostage", "m_nHostageState", 0x43467D44u, 0x434684FCu, 0x00000000u, 0xCB6C59C2u},
    {0x1610u, "CTablet", "m_flUpgradeExpirationTime[0]", 0x434674A0u, 0x43467000u, 0x00000000u,
     0xD6E91C78u},
    {0x1624u, "CTablet", "m_vecNotificationIds[0]", 0x434670D4u, 0x434693B0u, 0x00000000u,
     0x68484E0Du},
    {0x1638u, "CTablet", "m_vecNotificationTimestamps[0]", 0x43467B00u, 0x4346A770u, 0x00000000u,
     0x54E478AAu},
    {0x164Cu, "CTablet", "m_bTabletReceptionIsBlocked", 0x4346A76Cu, 0x43467E74u, 0x00000000u,
     0x581805BBu},
    {0x1660u, "CPlantedC4", "m_bBombDefused", 0x4346A72Cu, 0x43467E54u, 0x00000008u, 0xA7B0989Eu},
}};
constexpr std::array<DataMapOffsetRecipe, 14> data_map_recipes{{
    {0x1278u, DataMapRoot::player, "C_BaseEntity", "m_vecVelocity", 0x434688CCu, 0x4346A8E4u,
     0x36375C23u},
    {0x128Cu, DataMapRoot::player, "C_BaseEntity", "m_vecBaseVelocity", 0x43467970u, 0x43467D2Cu,
     0x0317EEDEu},
    {0x12A0u, DataMapRoot::player, "C_BaseEntity", "m_MoveType", 0x4346A678u, 0x43468BD4u,
     0x7501CB91u},
    {0x12B4u, DataMapRoot::player, "C_BaseEntity", "m_vecAbsVelocity", 0x4346900Cu, 0x434670A0u,
     0x7A2D60C1u},
    {0x12C8u, DataMapRoot::player, "C_BaseEntity", "m_angAbsRotation", 0x43469030u, 0x434688D4u,
     0xF8205EBAu},
    {0x12DCu, DataMapRoot::player, "C_BaseEntity", "m_vecAbsOrigin", 0x43467D28u, 0x434670E0u,
     0x4E66BB34u},
    {0x12F0u, DataMapRoot::player, "C_BaseEntity", "m_iEFlags", 0x4346718Cu, 0x43469224u,
     0x83A6CF2Bu},
    {0x1304u, DataMapRoot::player, "C_BaseEntity", "m_hGroundEntity", 0x4346A6D8u, 0x43468F88u,
     0x6E9852D6u},
    {0x1318u, DataMapRoot::player, "C_BasePlayer", "m_vecPreviouslyPredictedOrigin", 0x434682BCu,
     0x434671C8u, 0x6610746Eu},
    {0x132Cu, DataMapRoot::player, "C_BasePlayer", "m_nButtons", 0x43468BDCu, 0x43468308u,
     0x4746D093u},
    {0x1340u, DataMapRoot::player, "C_BasePlayer", "m_bHasWalkMovedSinceLastJump", 0x4346A6D0u,
     0x43469374u, 0xF3C10E80u},
    {0x1354u, DataMapRoot::player, "C_BasePlayer", "m_afButtonReleased", 0x434689C4u, 0x4346A828u,
     0x356B64E8u},
    {0x1368u, DataMapRoot::player, "C_BasePlayer", "m_afButtonLast", 0x43468B38u, 0x43467554u,
     0xE765ED03u},
    {0x137Cu, DataMapRoot::player, "C_BasePlayer", "m_afButtonPressed", 0x43468ECCu, 0x43467184u,
     0x3DB8EB39u},
}};
}

std::span<const ReceiveOffsetRecipe, 268> build_receive_offset_recipes() noexcept {
    return receive_recipes;
}
std::span<const DataMapOffsetRecipe, 14> build_data_map_offset_recipes() noexcept {
    return data_map_recipes;
}

void initialize_build_netvars(void* base_client, std::byte* client_module,
                              NativeOffsetStorage& storage) {
    if (!storage.cursor)
        throw std::invalid_argument(
            "Netvar initialization requires the seeded canonical key stream");
    for (const auto& recipe : receive_recipes) {
        auto& key =
            storage.globals.keys.try_emplace(recipe.xor_key_identity, recipe.initial_xor_key)
                .first->second;
        auto& output = storage.globals.pointers[recipe.pointer_identity];
        const OffsetBinding binding{netvar_name_hash(recipe.client_class),
                                    netvar_name_hash(recipe.property), &key, &output,
                                    recipe.addend};
        bind_receive_offset(base_client, binding, storage.cursor);
    }
    storage.maps.c69c5c29 = find_data_map(client_module, netvar_name_hash("C_CSPlayer"));
    storage.maps.a9cd7264 = find_data_map(client_module, netvar_name_hash("CWeaponCSBaseGun"));
    for (const auto& recipe : data_map_recipes) {
        auto& key =
            storage.globals.keys.try_emplace(recipe.xor_key_identity, recipe.initial_xor_key)
                .first->second;
        auto& output = storage.globals.pointers[recipe.pointer_identity];
        auto* const map =
            recipe.root == DataMapRoot::player ? storage.maps.c69c5c29 : storage.maps.a9cd7264;
        const DataMapOffsetBinding binding{map, netvar_name_hash(recipe.table),
                                           netvar_name_hash(recipe.property), &key, &output};
        bind_data_map_offset(binding, storage.cursor);
    }
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: owner_allocation.cpp
// ============================================================================

namespace skeet::engine {
void* allocate_owner_storage(JobMemory memory, std::size_t owner_size) {
    if (owner_size > std::numeric_limits<std::uint32_t>::max() - 0x20u)
        throw std::bad_alloc();
    const auto total = static_cast<std::uint32_t>(owner_size + 0x20u);
    auto* allocation = memory.allocate(memory.process_heap, 8, total);
    if (!allocation)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(allocation);
    const auto header = (address + 15u) & ~std::uintptr_t{15};
    const auto displacement = static_cast<std::uint32_t>(header - address);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}
void release_owner_storage(JobMemory memory, void* object) noexcept {
    if (!object)
        return;
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}

// ============================================================================
// Subcomponent: owner_registry.cpp
// ============================================================================

namespace skeet::engine {
namespace {
void* allocate_pointer_storage(JobMemory memory, std::size_t size) {
    // Native43387448, with the requested pointer-vector byte count.
    if (!size)
        return nullptr;
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - address);
    const auto allocation_size = static_cast<std::uint32_t>(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void free_pointer_storage(JobMemory memory, void* object) {
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}

void append_owner(OwnerRegistry& registry, void* const* value, JobMemory memory) {
    if (reinterpret_cast<std::uintptr_t>(registry.end) <
        reinterpret_cast<std::uintptr_t>(registry.limit)) {
        auto* const position = registry.end;
        registry.end = position + 1;
        *position = *value;
        return;
    }
    const auto count = (reinterpret_cast<std::uintptr_t>(registry.end) -
                        reinterpret_cast<std::uintptr_t>(registry.begin)) /
                       sizeof(void*);
    const auto capacity = count ? 2 * count : 1;
    auto** const replacement =
        static_cast<void**>(allocate_pointer_storage(memory, capacity * sizeof(void*)));
    const auto copied_count = (reinterpret_cast<std::uintptr_t>(registry.end) -
                               reinterpret_cast<std::uintptr_t>(registry.begin)) /
                              sizeof(void*);
    std::memmove(replacement, registry.begin, copied_count * sizeof(void*));
    replacement[copied_count] = *value;
    if (registry.begin)
        free_pointer_storage(memory, registry.begin);
    registry.begin = replacement;
    registry.end = replacement + copied_count + 1;
    registry.limit = replacement + capacity;
}

void* register_owner(OwnerRegistry& registry, void* object, JobMemory memory) {
    auto* value =
        reinterpret_cast<std::uintptr_t>(object) == std::numeric_limits<std::uintptr_t>::max() - 3
            ? nullptr
            : object;
    append_owner(registry, &value, memory);
    return object;
}

void dispatch_owner_slot_six(OwnerRegistry& registry) { // 43328BEA
    const auto owners = registry.entries();
    for (auto* owner : owners)
        invoke<void>(owner, 6);
}

void dispatch_owner_slot_eight(OwnerRegistry& registry) { // 433DF17B
    const auto owners = registry.entries();
    for (auto* owner : owners)
        invoke<void>(owner, 8);
}
}

// ============================================================================
// Subcomponent: player_hook_family.cpp
// ============================================================================

namespace skeet::engine {
namespace {
const PlayerPrimaryHookRuntimes* active_runtimes = nullptr;

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_PLAYER_ENTRY __fastcall
#else
#define SKEET_PLAYER_ENTRY
#endif

// Native433EB3F4 receives only ECX. The shared production body resolves the
// current metadata at its original155 call, after all preceding observers.
void SKEET_PLAYER_ENTRY player_simulation_entry(void* player, void*) {
    prediction::simulate_hooked_player_command(player, active_runtimes->simulation);
}

// Native433C82DC receives three raw stack arguments and returns RET12. Its
// update registry is the very same43475988 vector used by FixAnimations.
void SKEET_PLAYER_ENTRY player_view_entry(void* player, void*, std::uintptr_t first,
                                          std::uintptr_t second, std::uintptr_t third) {
    const auto& bindings = active_runtimes->animation_updates.bindings;
    bindings.view.calculate_hooked_view(player, bindings.duck,
                                        active_runtimes->animations.bindings.update_observers,
                                        first, second, third);
}

// Native433CCEA1 calls the original337 first, before its camera/trace work.
void SKEET_PLAYER_ENTRY player_free_look_entry(void* player, void*, void* setup) {
    const auto table = *static_cast<void***>(player);
    const auto* metadata = static_cast<const VtableHookMetadata*>(table[-2]);
    effects::capture_free_look_view_hook(player, metadata->original_table, setup,
                                         active_runtimes->free_look);
}

#undef SKEET_PLAYER_ENTRY
}

std::span<const PlayerPrimaryNativeReplacement>
player_primary_native_replacements(PlayerPrimaryKind kind) {
    switch (kind) {
    case PlayerPrimaryKind::local:
        return local_player_native_replacements;
    case PlayerPrimaryKind::remote:
        return remote_player_native_replacements;
    }
    throw std::invalid_argument("Unknown primary player hook family");
}

PlayerHookFamily::PlayerHookFamily(VtableHookMetadata& local, VtableHookMetadata& remote,
                                   VtableHookRegistry& hooks, std::byte* const& client_module,
                                   NativeImageEnvironment images, HookDescriptorSystem system,
                                   PlayerPrimaryHookRuntimes runtimes) noexcept
    : local_(local), remote_(remote), hooks_(hooks), client_module_(client_module), images_(images),
      system_(system), runtimes_(runtimes) {}

VtableHookMetadata& PlayerHookFamily::metadata(PlayerPrimaryKind kind) const {
    switch (kind) {
    case PlayerPrimaryKind::local:
        return local_;
    case PlayerPrimaryKind::remote:
        return remote_;
    }
    throw std::invalid_argument("Unknown primary player hook family");
}

void PlayerHookFamily::prepare(PlayerPrimaryKind kind) {
    prepare_rtti_vtable_hook(metadata(kind), client_module_, player_primary_rtti_hash,
                             player_primary_rtti_base_hash, images_, hooks_.memory, system_);
    bind_replacements(kind);
}

namespace {
void bind_primary_entries(VtableHookMetadata& descriptor, PlayerPrimaryKind kind,
                          const PlayerPrimaryHookRuntimes* context) {
    for (const auto& replacement : player_primary_native_replacements(kind)) {
        switch (replacement.native_entry) {
        case 0x433842c1: //local78
        case 0x4334d3eb: //local88, hidden caller ESI
        case 0x4340191d: //local113
        case 0x433e97c1: //local170
            if (context)
                context->queries.bind_native_replacement(descriptor, replacement.slot,
                                                         replacement.native_entry);
            else
                descriptor.replacement_table[replacement.slot] =
                    client::PlayerCommandQueries::native_replacement_entry(
                        replacement.slot, replacement.native_entry);
            break;
        case 0x43402307: //shared109
        case 0x4333181e: //shared145
        case 0x4333c562: //shared173
        case 0x433abbf0: //remote78
        case 0x433875be: //remote113
        case 0x43382222: //remote169
            if (context)
                context->physics.bind_native_replacement(descriptor, replacement.slot,
                                                         replacement.native_entry);
            else
                descriptor.replacement_table[replacement.slot] =
                    client::PlayerPhysicsHooks::native_replacement_entry(replacement.slot,
                                                                         replacement.native_entry);
            break;
        case 0x43330f73: //190
        case 0x4335c639: //207
        case 0x43388201: //198
        case 0x4335b798: //local277; not present in the remote recipe
            if (context)
                context->animations.bind_native_replacement(descriptor, replacement.slot,
                                                            replacement.native_entry);
            else
                descriptor.replacement_table[replacement.slot] =
                    animation::FixAnimationsRuntime::native_replacement_entry(
                        replacement.slot, replacement.native_entry);
            break;
        case 0x43341fa7: //local222
        case 0x43329436: //local224
        case 0x433c6689: //local268, hidden caller EBP
        case 0x433e009a: //local294, hidden caller EDI
            if (context)
                context->animation_updates.bind_native_replacement(descriptor, replacement.slot,
                                                                   replacement.native_entry);
            else
                descriptor.replacement_table[replacement.slot] =
                    client::LocalPlayerAnimationHooks::native_replacement_entry(
                        replacement.slot, replacement.native_entry);
            break;
        case 0x4331e412: //local230
        case 0x433ab4b6: //local298
        case 0x433d18b0: //local350
            if (context)
                context->command_hooks.bind_native_replacement(descriptor, replacement.slot,
                                                               replacement.native_entry);
            else
                descriptor.replacement_table[replacement.slot] =
                    client::PlayerCommandHooks::native_replacement_entry(replacement.slot,
                                                                         replacement.native_entry);
            break;
        case 0x433eb3f4:
            descriptor.replacement_table[replacement.slot] =
                reinterpret_cast<void*>(&player_simulation_entry);
            break;
        case 0x4334f53d:
            if (context)
                client::bind_create_move_runtime(context->commands);
#if defined(_M_IX86) || defined(__i386__)
            // The captured outer wrapper hands its saved EBX address to the
            // inner master. The engine's incoming EDX is not that address.
            descriptor.replacement_table[replacement.slot] =
                reinterpret_cast<void*>(&client::player_create_move_hook);
#else
            throw std::logic_error("The saved-EBX player CreateMove hook requires x86");
#endif
            break;
        case 0x433c82dc:
            descriptor.replacement_table[replacement.slot] =
                reinterpret_cast<void*>(&player_view_entry);
            break;
        case 0x433ccea1:
            descriptor.replacement_table[replacement.slot] =
                reinterpret_cast<void*>(&player_free_look_entry);
            break;
        case 0x4336ee16:
            if (context)
                context->visibility.bind_player_table(descriptor);
            else
                descriptor.replacement_table[replacement.slot] =
                    effects::EntityVisibilityRuntime::source_entry();
            break;
        default:
            throw std::logic_error("Unbound captured primary player hook");
        }
    }
}
}
void bind_player_primary_static_entries(VtableHookMetadata& metadata, PlayerPrimaryKind kind) {
    bind_primary_entries(metadata, kind, nullptr);
}
void PlayerHookFamily::bind_replacements(PlayerPrimaryKind kind) {
    // Publish actual contexts before any receiver can use its prepared table,
    // including the native installer's cleanup-node allocation failure path.
    active_runtimes = &runtimes_;
    bind_primary_entries(metadata(kind), kind, &runtimes_);
}

VtableHookNode* PlayerHookFamily::install(PlayerPrimaryKind kind, void* actual_player) {
    bind_replacements(kind);
    return install_vtable_hook(hooks_, actual_player, metadata(kind));
}

void PlayerHookFamily::clear(PlayerPrimaryKind kind) {
    clear_vtable_hook(metadata(kind), hooks_.memory);
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: preserved_initialization.cpp
// ============================================================================

namespace skeet::engine {
namespace {
template <std::size_t Size>
void require_entry(const PreservedInitializationImage& image, std::uint32_t address,
                   const std::array<unsigned char, Size>& expected) {
    const auto offset = address - PreservedInitializationImage::preferred_address;
    if (std::memcmp(image.data() + offset, expected.data(), Size))
        throw std::invalid_argument(
            "Preserved initialization entry bytes do not match the input image");
}
#if defined(_M_IX86) || defined(__i386__)
using OperandEntry = std::uint32_t(__stdcall*)(std::uint32_t);
using StartupEntry = void(__cdecl*)();
#endif
}

PreservedInitializationImage::PreservedInitializationImage(std::shared_ptr<std::byte> mapping,
                                                           std::size_t mapped_size,
                                                           std::shared_ptr<const void> environment)
    : NativeDataImage(std::move(mapping), mapped_size),
      exception_environment_(std::move(environment)) {
    if (reinterpret_cast<std::uintptr_t>(data()) != preferred_address || size() < image_size ||
        !exception_environment_)
        throw std::invalid_argument(
            "Preserved initialization requires its mapped input image and live exception environment");
}

PreservedInitialization::PreservedInitialization(
    std::shared_ptr<PreservedInitializationImage> image)
    : image_(std::move(image)) {
    if (!image_)
        throw std::invalid_argument("Preserved initialization image owner is required");
    // These bytes are identical in supplied skeet.bin and the immutable
    // analysis image. No exception trap or VM jump is patched here.
    require_entry(*image_, 0x43493903,
                  std::array<unsigned char, 7>{0xcc, 4, 0, 0x51, 0xcc, 0x10, 0});
    require_entry(*image_, 0x43484f06, std::array<unsigned char, 5>{0xe9, 0x73, 0x8c, 0x0c, 0});
}

std::uintptr_t PreservedInitialization::operand(std::uint32_t address) {
#if defined(_M_IX86) || defined(__i386__)
    // Ordinary callers push the operand and continue without ADD ESP,4.
    // This specifies the caller-observed effective cleanup/return contract,
    // not a recovered RET4 opcode. The supplied entry is an INT3 transfer,
    // so the retained live environment must implement that contract; the
    // image bytes alone do not provide it.
    const auto entry =
        reinterpret_cast<OperandEntry>(image_->data() + (0x43493903 - image_->preferred_address));
    return entry(address);
#else
    (void)address;
    throw std::logic_error(
        "Preserved initialization requires its original x86 execution environment");
#endif
}
void PreservedInitialization::last_startup_callback() {
#if defined(_M_IX86) || defined(__i386__)
    const auto entry =
        reinterpret_cast<StartupEntry>(image_->data() + (0x43484f06 - image_->preferred_address));
    entry();
#else
    throw std::logic_error(
        "Preserved initialization requires its original x86 execution environment");
#endif
}
}

// ============================================================================
// Subcomponent: rtti.cpp
// ============================================================================

namespace skeet::engine {
namespace {
/* [dedup read/write] */
std::uint32_t section_hash(const char* name) { // 4334DEFF
    std::uint32_t result = 0x811c9dc5;
    for (auto value = static_cast<std::uint8_t>(*name); value;
         value = static_cast<std::uint8_t>(*++name)) {
        if (static_cast<std::uint8_t>(value - 'A') <= 'Z' - 'A')
            value += 'a' - 'A';
        result = (result ^ value) * 0x1000193;
    }
    return result ^ 0x92ca5df1;
}
struct SectionRange {
    std::byte* begin;
    std::byte* end;
};
SectionRange rtti_rdata(std::byte* module, NativeImageEnvironment images) {
    const auto* section = find_pe_section(images.image_nt_header(module), 0x33d66818);
    if (!section)
        return {};
    auto* begin = module + read<std::uint32_t>(section + 0xc);
    return {begin, begin + read<std::uint32_t>(section + 8)};
}
bool rtti_within(const void* pointer, SectionRange section) {
    const auto address = reinterpret_cast<std::uintptr_t>(pointer);
    return address >= reinterpret_cast<std::uintptr_t>(section.begin) &&
           address < reinterpret_cast<std::uintptr_t>(section.end);
}
void** table_after_locator(SectionRange section, const std::byte* locator) {
    for (auto* position = section.begin; position < section.end; position += 4)
        if (read<const std::byte*>(position) == locator)
            return reinterpret_cast<void**>(position + sizeof(void*));
    return nullptr;
}
}

const std::byte* find_pe_section(const std::byte* nt_header, std::uint32_t name_hash) { // 433A1085
    if (!nt_header)
        return nullptr;
    const auto count = read<std::uint16_t>(nt_header + 6);
    auto* section = nt_header + 0x18 + read<std::uint16_t>(nt_header + 0x14);
    for (std::uint16_t i = 0; i < count; ++i, section += 0x28) {
        std::array<char, 9> name{};
        std::memcpy(name.data(), section, 8);
        if (section_hash(name.data()) == name_hash)
            return section;
    }
    return nullptr;
}

RttiTypeDescriptor* find_rtti_type(std::byte* module, std::uint32_t type_hash,
                                   NativeImageEnvironment images) { // 433B305B
    const auto* section = find_pe_section(images.image_nt_header(module), 0xe2d1c070);
    if (!section)
        return nullptr;
    auto* begin = module + read<std::uint32_t>(section + 0xc);
    auto* const end = begin + read<std::uint32_t>(section + 8) - 10;
    for (auto* position = begin; position < end; position += 4) {
        if (read<std::uint32_t>(position) == 0x56413f2e &&
            netvar_name_hash(reinterpret_cast<const char*>(position)) == type_hash)
            return reinterpret_cast<RttiTypeDescriptor*>(position -
                                                         offsetof(RttiTypeDescriptor, name));
    }
    return nullptr;
}

void** find_rtti_vtable(std::byte* module, RttiTypeDescriptor* type, std::uint32_t base_hash,
                        NativeImageEnvironment images) { // 4332CE3A
    const auto section = rtti_rdata(module, images);
    if (!section.begin)
        return nullptr;
    std::array<const std::byte*, 32> candidates;
    std::uint32_t candidate_count = 0;
    auto desired_offset = base_hash ? 0xffffffffu : 0u;
    const std::byte* selected = nullptr;
    for (auto* position = section.begin; position < section.end; position += 4) {
        if (read<RttiTypeDescriptor*>(position) != type)
            continue;
        const auto* locator = position - offsetof(RttiObjectLocator, type);
        const auto* hierarchy =
            read<RttiClassHierarchy*>(locator + offsetof(RttiObjectLocator, hierarchy));
        if (rtti_within(hierarchy, section)) {
            if (desired_offset == 0xffffffffu) {
                if (candidate_count < candidates.size())
                    candidates[candidate_count++] = locator;
            } else if (read<std::uint32_t>(locator + offsetof(RttiObjectLocator, offset)) ==
                       desired_offset) {
                selected = locator;
                break;
            }
            continue;
        }
        // The same type pointer can belong to a BaseClassDescriptor. Native
        // follows that record's hierarchy to obtain the requested base mdisp.
        hierarchy = read<RttiClassHierarchy*>(position + offsetof(RttiBaseClass, hierarchy));
        if (!hierarchy)
            continue;
        const auto* hierarchy_bytes = reinterpret_cast<const std::byte*>(hierarchy);
        auto** bases = read<RttiBaseClass**>(hierarchy_bytes + offsetof(RttiClassHierarchy, bases));
        if (!bases)
            continue;
        const auto count =
            read<std::uint32_t>(hierarchy_bytes + offsetof(RttiClassHierarchy, count));
        for (std::uint32_t i = 0; i < count; ++i) {
            const auto* base = bases[i];
            if (!base)
                continue;
            const auto* bytes = reinterpret_cast<const std::byte*>(base);
            const auto* base_type =
                read<RttiTypeDescriptor*>(bytes + offsetof(RttiBaseClass, type));
            if (base_type && netvar_name_hash(reinterpret_cast<const char*>(base_type) +
                                              offsetof(RttiTypeDescriptor, name)) == base_hash) {
                desired_offset =
                    read<std::uint32_t>(bytes + offsetof(RttiBaseClass, member_displacement));
                break;
            }
        }
        if (desired_offset == 0xffffffffu)
            continue;
        for (std::uint32_t i = 0; i < candidate_count; ++i) {
            if (read<std::uint32_t>(candidates[i] + offsetof(RttiObjectLocator, offset)) ==
                desired_offset) {
                selected = candidates[i];
                break;
            }
        }
        if (selected)
            break;
    }
    return selected ? table_after_locator(section, selected) : nullptr;
}

void** find_rtti_vtable_at_offset(std::byte* module, RttiTypeDescriptor* type, std::uint32_t offset,
                                  NativeImageEnvironment images) { // 433EC4BD
    const auto section = rtti_rdata(module, images);
    if (!section.begin)
        return nullptr;
    for (auto* position = section.begin; position < section.end; position += 4) {
        if (read<RttiTypeDescriptor*>(position) != type)
            continue;
        const auto* locator = position - offsetof(RttiObjectLocator, type);
        if (read<std::uint32_t>(locator + offsetof(RttiObjectLocator, constructor_displacement)) ==
                0 &&
            read<std::uint32_t>(locator + offsetof(RttiObjectLocator, offset)) == offset)
            return table_after_locator(section, locator);
    }
    return nullptr;
}

void** resolve_rtti_vtable(std::byte* module, std::uint32_t type_hash, std::uint32_t base_hash,
                           NativeImageEnvironment images) {
    return find_rtti_vtable(module, find_rtti_type(module, type_hash, images), base_hash, images);
}
}

// ============================================================================
// Subcomponent: scene_queries.cpp
// ============================================================================

namespace skeet::engine {

SceneQueries::SceneQueries(const SceneQueryBindings& source)
    : bindings_(source), movement_bindings_{bindings_.engine_trace, bindings_.surface_properties,
                                            bindings_.skip_filter_vtable},
      ballistic_bindings_{bindings_.engine_trace,
                          bindings_.surface_properties,
                          bindings_.entity_list,
                          bindings_.static_props,
                          bindings_.model_info,
                          bindings_.model_cache,
                          bindings_.maximum_clients,
                          bindings_.skip_filter_vtable,
                          bindings_.entity_filter_vtable,
                          bindings_.offsets,
                          bindings_.player_adjustments,
                          bindings_.primary_breakable_class,
                          bindings_.alternate_breakable_class,
                          bindings_.breakable,
                          bindings_.game_type,
                          bindings_.teammates_are_enemies},
      grenade_bindings_{bindings_.entity_list,
                        bindings_.engine_trace,
                        bindings_.surface_properties,
                        bindings_.spatial_partition,
                        bindings_.static_props,
                        bindings_.skip_filter_vtable,
                        bindings_.entity_filter_vtable,
                        bindings_.player_resource_slot,
                        bindings_.smoke_projectile_class,
                        esp::GrenadeEngineOffsets(bindings_.offsets),
                        bindings_.base_projectile_class,
                        bindings_.molotov_projectile_class,
                        bindings_.inferno_class},
      grenade_environment_{bindings_.tick_interval,
                           bindings_.frame_interval,
                           bindings_.gravity_convar,
                           bindings_.incendiary_duration_convar,
                           bindings_.incendiary_max_slope_convar,
                           bindings_.entity_list,
                           bindings_.primary_breakable_class,
                           bindings_.alternate_breakable_class,
                           bindings_.grenade_alternate_breakable_class,
                           bindings_.grenade_slope_exempt_class,
                           bindings_.inferno_class,
                           esp::GrenadeEnvironmentOffsets(bindings_.offsets)},
      movement_(movement_bindings_), ballistics_(ballistic_bindings_),
      grenades_(grenade_bindings_) {}

} // namespace skeet::engine

// ============================================================================
// Subcomponent: scene_runtime_owner.cpp
// ============================================================================

namespace skeet::engine {
namespace {
SceneQueryBindings compose(client::ClientRuntimeStorage& storage,
                           const client::EngineRuntimeBindings& engine,
                           client::PlayerRuntimeOwner& players) {
    auto& data = storage.data();
    if (&engine.variables.data() != &data || &players.storage() != &storage)
        throw std::invalid_argument("Scene queries require the canonical engine and player owners");
    const auto manifest = build_interface_manifest();
    for (const auto cell :
         std::array<std::uint32_t, 7>{0x43468304u, 0x43469268u, 0x4346A890u, 0x43467014u,
                                      0x4346A614u, 0x43469330u, 0x43467F10u}) {
        bool found = false;
        for (std::size_t index = 0; index < manifest.size(); ++index) {
            if (manifest[index].destination_cell != cell)
                continue;
            const auto current = data.native_cell<void*>(cell);
            if (!current || current != engine.interfaces.objects[index])
                throw std::invalid_argument(
                    "Scene interface does not belong to the initialized engine bundle");
            found = true;
            break;
        }
        if (!found)
            throw std::logic_error("Scene interface is absent from the build acquisition manifest");
    }
    for (const auto cell : std::array<std::uint32_t, 11>{
             0x434758B4u, 0x43475B90u, 0x43468FF0u, 0x4346A440u, 0x43467478u, 0x43468238u,
             0x43468E30u, 0x43467E88u, 0x434674E8u, 0x43468CA0u, 0x4346719Cu})
        if (!data.native_cell<const void*>(cell))
            throw std::invalid_argument(
                "Scene class, resource and trace-filter acquisition must finish first");
    const auto* globals = data.native_cell<const std::byte*>(0x43467B04u);
    const auto* interval = data.native_cell<const float*>(0x4346A338u);
    if (!globals || reinterpret_cast<std::uintptr_t>(globals) % 4 ||
        reinterpret_cast<const std::byte*>(interval) != globals + 0x20 ||
        reinterpret_cast<const std::byte*>(data.native_cell<const std::int32_t*>(0x43469300u)) !=
            globals + 0x18 ||
        !storage.offsets().words || storage.offsets().globals.pointers.empty())
        throw std::invalid_argument(
            "Scene queries require initialized live clock aliases and entity offsets");
    for (const auto cell : {0x43467A8Cu, 0x4346A568u, 0x43467A54u, 0x43467048u, 0x43467088u})
        if (!engine.variables.at(cell).get())
            throw std::invalid_argument("Scene query ConVar is unresolved");
    return {data.native_cell<void*>(0x43468304u),
            data.native_cell<void*>(0x43469268u),
            data.native_cell<void*>(0x4346A890u),
            data.native_cell<void*>(0x43467014u),
            data.native_cell<void*>(0x4346A614u),
            data.native_cell<void*>(0x43469330u),
            data.native_cell<void*>(0x43467F10u),
            data.native_cell<void*>(0x43468FF0u),
            data.native_cell<const std::int32_t*>(0x43469300u),
            data.native_cell<void* const*>(0x434758B4u),
            data.native_cell<void* const*>(0x43475B90u),
            storage.offsets().globals,
            players.adjustments(),
            data.native_cell<void*>(0x4346A440u),
            data.native_cell<void*>(0x43467478u),
            data.native_cell<const void*>(0x43468238u),
            data.native_cell<const void*>(0x43468E30u),
            data.native_cell<const void*>(0x43467E88u),
            data.native_cell<const void*>(0x434674E8u),
            data.native_cell<EngineBreakableFunction>(0x43468F30u),
            engine.variables.at(0x43467A8Cu),
            engine.variables.at(0x4346A568u),
            data.native_cell<void*>(0x43468CA0u),
            data.native_cell<void*>(0x4346719Cu),
            *interval,
            *reinterpret_cast<const float*>(globals + 0x20),
            engine.variables.at(0x43467A54u),
            engine.variables.at(0x43467048u),
            engine.variables.at(0x43467088u)};
}
}
SceneQueryRuntimeOwner::SceneQueryRuntimeOwner(client::ClientRuntimeStorage& storage,
                                               const client::EngineRuntimeBindings& engine,
                                               client::PlayerRuntimeOwner& players)
    : storage_(storage), players_(players), bindings_(compose(storage, engine, players)),
      queries_(bindings_) {}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: source_data_initialization.cpp
// ============================================================================

namespace skeet::engine {
namespace {
template <class Descriptor>
void initialize_static_descriptor(Descriptor& target, const Descriptor& source) {
    target.next = source.next;
    target.previous = source.previous;
    target.hash = source.hash;
    target.serialized_size = source.serialized_size;
    target.type = source.type;
    target.reserved = source.reserved;
    target.value = source.value;
}
}
void initialize_source_configuration_data(NativeDataImage& data) {
#if UINTPTR_MAX == UINT32_MAX
    static_assert(sizeof(misc::AntiUntrustedConfig) == 20);
    static_assert(sizeof(misc::HideFromObsConfig) == 20);
    static_assert(sizeof(misc::CustomEventsConfig) == 20);
    static_assert(sizeof(misc::FreeLookConfig) == 40);
    static_assert(sizeof(client::TargetMaskSettings) == 20);
#endif
    // Each class is a single existing descriptor at the proved native base.
    // Its declaration supplies static hash/type/size/value, not a CRT default
    // or an owner registration. The source-only creation path starts zeroed.
    // Assign fields individually: BooleanConfig's three trailing padding
    // bytes are not initialized by its C++ declaration and must stay zero.
    initialize_static_descriptor(data.native_cell<misc::AntiUntrustedConfig>(0x434693DCu).enabled,
                                 misc::AntiUntrustedConfig{}.enabled);
    initialize_static_descriptor(data.native_cell<misc::HideFromObsConfig>(0x43468E48u).enabled,
                                 misc::HideFromObsConfig{}.enabled);
    initialize_static_descriptor(data.native_cell<misc::CustomEventsConfig>(0x434691A8u).enabled,
                                 misc::CustomEventsConfig{}.enabled);
    initialize_static_descriptor(data.native_cell<misc::FreeLookConfig>(0x43468264u).key,
                                 misc::FreeLookConfig{}.key);
    initialize_static_descriptor(data.native_cell<client::TargetMaskSettings>(0x43467974u).enabled,
                                 client::TargetMaskSettings{}.enabled);
}
void initialize_source_native_data(NativeDataImage& data) {
    // The three SVG texture slots start absent before433283A5 creates them.
    data.native_cell<std::int32_t>(0x43468E3Cu) = -1;
    data.native_cell<std::int32_t>(0x4346842Cu) = -1;
    data.native_cell<std::int32_t>(0x43467DA8u) = -1;
    // Immutable native weapon-classification bits shared by skin/RAGE readers.
    data.native_cell<std::array<std::uint32_t, 3>>(0x43474580u) = {0xFF8B6F9Eu, 0xB00001DFu, 1};
    ui::initialize_source_settings_menu_data(data);
    misc::initialize_source_settings_runtime_data(data);
    initialize_source_configuration_data(data);
    // The two Lua menu flags are enabled in the pinned static image. Source
    // menu bindings borrow these cells before any script owner is created.
    data.native_cell<std::uint8_t>(0x434675D4u) = 1;
    data.native_cell<std::uint8_t>(0x4346D544u) = 1;
    // The independent Lua availability gate starts enabled in static data.
    // Its only native runtime writes are the two shutdown paths clearing it;
    // leaving source BSS zero would suppress every initial Lua-state request.
    data.native_cell<bool>(0x434688BDu) = true;
    // Skins schedules HUD invalidation from menu callbacks and consumes it
    // in command phase4. Both initial values are independent static data.
    data.native_cell<std::uint8_t>(0x4346838Bu) = 1;
    data.native_cell<float>(0x4346A8A0u) = std::numeric_limits<float>::max();
    // These are initial scalar data, not defaults supplied by a feature
    // constructor. Their source consumers distinguish absent handles and
    // unseen command numbers from zero before their first dynamic capture.
    for (const auto cell : {0x43467114u, 0x43468488u, 0x4346D4B4u, 0x4346D4B8u, 0x4346D4BCu,
                            0x4346D648u, 0x4346D650u, 0x4346D674u, 0x4346DB1Cu})
        data.native_cell<std::int32_t>(cell) = -1;
    // The independent camera/ESP FOVs, aim-step interval and uncaptured duck
    // sentinel retain their exact native static bits until their real writers.
    data.native_cell<float>(0x43468E80u) = 90.0f;
    data.native_cell<float>(0x4346D548u) = 90.0f;
    data.native_cell<float>(0x43469254u) = 0.015625f;
    data.native_cell<float>(0x4346D654u) = std::numeric_limits<float>::max();
    data.native_cell<std::uint8_t>(0x4346A65Cu) = 1;
    data.native_cell<std::array<std::uint8_t, 3>>(0x434745E0u) = {2, 0, 1};
    //43369FAA/CA pass this immutable float directly to the native trigonometry
    // boundary. Preserve its recorded bits rather than infer a different angle.
    data.native_cell<float>(0x43472534u) = std::bit_cast<float>(0x7C8EFA34u);
    //43416B4E only compares the caller word with this pinned literal. Neither
    // the ordinary payload nor loader extra() writes it, and the working
    // dump retains it verbatim. It is never called/dereferenced. Preserve the
    // comparison bits; an inferred rebase would invent a different predicate.
    data.native_cell<std::uintptr_t>(0x43467DE8u) = 0x283A0BF0u;
    // Original static hashes consumed, in order, by the devirtualized first
    // startup callback. They become native service numbers only on its
    // matching loader-base branch.
    for (const auto& service : first_startup_service_slots())
        data.native_cell<std::uint32_t>(service.address) = service.initial_hash;

    // Static4346D678 starts at1; the first433EA7BC token read builds the
    // delimiter bitmap and clears it. The source parser shares these cells.
    data.native_cell<std::uint8_t>(0x4346D678u) = 1;
    data.native_cell<std::array<std::uint32_t, 8>>(0x4347C620u).fill(0);

    // All three native one-bucket headers initially use the same null bucket and
    // -1 terminator at4346935C. Store their actual source addresses rather than the
    // original absolute pointer value contained in the captured data image.
    auto& empty_buckets = data.native_cell<std::array<void*, 2>>(0x4346935Cu);
    empty_buckets = {nullptr, reinterpret_cast<void*>(~std::uintptr_t{})};
    data.native_cell<StartupTextBuckets>(0x4346D700u) = {
        0, reinterpret_cast<StartupTextBucketNode**>(empty_buckets.data()), 1, 0};
    data.native_cell<StartupPointerBuckets>(0x4346DA40u) = {
        0, reinterpret_cast<StartupPointerBucketNode**>(empty_buckets.data()), 1, 0};
    data.native_cell<animation::FixAnimationBackupNode**>(0x4346D6E4u) =
        reinterpret_cast<animation::FixAnimationBackupNode**>(empty_buckets.data());
    data.native_cell<std::uint32_t>(0x4346D6E8u) = 1;
    data.native_cell<std::uint32_t>(0x4346D6ECu) = 0;
    data.native_cell<animation::FixAnimationBackupGrowth>(0x4346D6F0u) = {1.0f, 2.0f, 0};
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: source_initialization_imports.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
FirstStartupServiceBindings
SourceInitializationImports::first_startup_bindings(NativeDataImage& data) const noexcept {
    return {data, modules, images, forwarded_exports.image_directory_entry, terminate_process};
}

SourceInitializationImports native_source_initialization_imports() {
    if (sizeof(void*) != 4)
        throw std::logic_error("Source initialization imports require their x86 layouts");
#if defined(_WIN32)
    const auto native = GetModuleHandleA("ntdll.dll");
    if (!native)
        throw std::runtime_error("ntdll.dll is not loaded");
    const auto terminate = GetProcAddress(native, "ZwTerminateProcess");
    if (!terminate)
        throw std::runtime_error("ntdll!ZwTerminateProcess export is missing");

    auto forwarded = native_forwarded_export_environment();
    if (!forwarded.modules.loader || !forwarded.modules.acquire_peb_lock ||
        !forwarded.modules.release_peb_lock)
        throw std::runtime_error(
            "Source initialization requires the real loaded-module environment");
    auto images = native_image_environment();
    auto memory = native_job_memory();

    CommonControlsExports controls;
    initialize_startup_common_controls(controls, forwarded);
    if (!controls.default_subclass_proc || !controls.remove_window_subclass ||
        !controls.set_window_subclass)
        throw std::runtime_error(
            "Required common-controls exports are unavailable in already-loaded modules");
    return {memory,
            forwarded.modules,
            images,
            forwarded,
            reinterpret_cast<NativeServiceTerminateProcess>(terminate),
            controls};
#else
    throw std::runtime_error("Source initialization imports require Windows");
#endif
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: startup_container_cleanup.cpp
// ============================================================================

namespace skeet::engine {
namespace {
const StartupContainerCleanup* active_container_cleanup = nullptr;
void destroy_module(StartupModulePrefix& module, JobMemory memory) { //4334D2C1
    release_owner_storage(memory, module.source);
    misc::destroy_native_text(module.name, memory);
}
template <std::uint32_t Finalizer> void register_finalizer(void* opaque) {
    static_cast<StartupContainerCleanup*>(opaque)->append_finalizer(Finalizer);
}
}

void finalize_startup_owned_pointers(StartupOwnedPointers& vector, JobMemory memory) {
    auto* const end = vector.end;
    auto* cursor = vector.begin;
    if (cursor != end) {
        do {
            if (*cursor) {
                release_owner_storage(memory, *cursor);
                *cursor = nullptr; //after the heap call, including its failure return
            }
        } while (++cursor != end);
        cursor = vector.begin; //native reload after all element destruction
    }
    release_owner_storage(memory, cursor);
}
void finalize_startup_owned_texts(StartupOwnedTexts& vector, JobMemory memory) {
    auto* const end = vector.end;
    auto* cursor = vector.begin;
    if (cursor != end) {
        do {
            misc::destroy_native_text(*cursor, memory);
        } while (++cursor != end);
        cursor = vector.begin;
    }
    release_owner_storage(memory, cursor);
}
void finalize_startup_owned_modules(StartupOwnedModules& vector, JobMemory memory) {
    auto* const end = vector.end;
    auto* cursor = vector.begin;
    if (cursor != end) {
        do {
            auto* const module = *cursor;
            if (module) {
                destroy_module(*module, memory);
                release_owner_storage(memory, module);
                *cursor = nullptr;
            }
        } while (++cursor != end);
        cursor = vector.begin;
    }
    release_owner_storage(memory, cursor);
}
void finalize_startup_text_buckets(StartupTextBuckets& table, JobMemory memory) {
    auto count = table.bucket_count;
    auto** const buckets = table.buckets;
    if (count) {
        for (std::uint32_t i = 0; i < count; ++i) {
            auto* node = buckets[i];
            while (node) {
                auto* const next = node->next; //captured before either destructor call
                misc::destroy_native_text(node->text, memory);
                release_owner_storage(memory, node);
                node = next;
            }
            buckets[i] = nullptr;
        }
        count = table.bucket_count;
    }
    table.size = 0;
    if (count > 1)
        release_owner_storage(memory, table.buckets);
}
void finalize_startup_pointer_buckets(StartupPointerBuckets& table, JobMemory memory) {
    auto count = table.bucket_count;
    auto** const buckets = table.buckets;
    if (count) {
        for (std::uint32_t i = 0; i < count; ++i) {
            auto* node = buckets[i];
            while (node) {
                auto* const next = node->next;
                release_owner_storage(memory, node);
                node = next;
            }
            buckets[i] = nullptr;
        }
        count = table.bucket_count;
    }
    table.size = 0;
    if (count > 1)
        release_owner_storage(memory, table.buckets);
}

StartupContainerCleanup::StartupContainerCleanup(StartupContainerCleanupBindings bindings)
    : bindings_(bindings) {
    if (!bindings_.memory.free || !bindings_.memory.process_heap)
        throw std::invalid_argument(
            "Startup container cleanup requires the canonical process heap");
}
const StartupContainerCleanup& StartupContainerCleanup::bound() {
    if (!active_container_cleanup)
        throw std::logic_error("Startup container cleanup is not bound");
    return *active_container_cleanup;
}
void (*StartupContainerCleanup::source_callback(std::uint32_t target) const)() {
    void (*callback)();
    switch (target) {
    case 0x4333087Cu:
        callback = preset_hotkeys;
        break;
    case 0x433D2E2Bu:
        callback = renderer_listener;
        break;
    case 0x4331ECD4u:
        callback = agent_paths_ct;
        break;
    case 0x433F4BCBu:
        callback = agent_paths_t;
        break;
    case 0x43336C3Cu:
        callback = builtin_modules;
        break;
    case 0x4334D2ABu:
        callback = required_modules;
        break;
    case 0x4335851Bu:
        callback = text_buckets;
        break;
    case 0x433ACF65u:
        callback = pointer_buckets;
        break;
    default:
        throw std::out_of_range("Unknown startup container finalizer");
    }
    if (active_container_cleanup && active_container_cleanup != this)
        throw std::logic_error("Startup container callbacks require one stable source owner");
    active_container_cleanup = this;
    return callback;
}
void StartupContainerCleanup::append_finalizer(std::uint32_t target) const {
    (void)bindings_.callbacks.append(source_callback(target));
}
void StartupContainerCleanup::preset_hotkeys() {
    const auto& source = bound().bindings_;
    finalize_startup_owned_pointers(source.preset_hotkeys, source.memory);
}
void StartupContainerCleanup::renderer_listener() {
    const auto& source = bound().bindings_;
    //433D0304 restores the concrete base vtable before unregistering. Reuse
    // the same source object/storage so the compiler emits its actual table;
    // no captured table pointer is installed into a C++ object.
    source.renderer_listener.restore_base_vtable();
    if (source.unload.listener_registered) {
        source.unload.listener_registered = false;
        invoke<void>(source.unload.event_manager, 21, &source.renderer_listener);
    }
}
void StartupContainerCleanup::agent_paths_ct() {
    const auto& source = bound().bindings_;
    finalize_startup_owned_texts(source.agent_paths_ct, source.memory);
}
void StartupContainerCleanup::agent_paths_t() {
    const auto& source = bound().bindings_;
    finalize_startup_owned_texts(source.agent_paths_t, source.memory);
}
void StartupContainerCleanup::builtin_modules() {
    const auto& source = bound().bindings_;
    finalize_startup_owned_modules(source.builtin_modules, source.memory);
}
void StartupContainerCleanup::required_modules() {
    const auto& source = bound().bindings_;
    finalize_startup_owned_modules(source.required_modules, source.memory);
}
void StartupContainerCleanup::text_buckets() {
    const auto& source = bound().bindings_;
    finalize_startup_text_buckets(source.text_buckets, source.memory);
}
void StartupContainerCleanup::pointer_buckets() {
    const auto& source = bound().bindings_;
    finalize_startup_pointer_buckets(source.pointer_buckets, source.memory);
}
void bind_startup_container_cleanup(client::NativeStartupSources& sources,
                                    StartupContainerCleanup& context) {
    const auto& image = sources.data();
    const auto& bindings = context.bindings();
    if (&bindings.preset_hotkeys != &image.native_cell<StartupOwnedPointers>(0x43478DA8u) ||
        &bindings.agent_paths_ct != &image.native_cell<StartupOwnedTexts>(0x43479088u) ||
        &bindings.agent_paths_t != &image.native_cell<StartupOwnedTexts>(0x43479078u) ||
        &bindings.builtin_modules != &image.native_cell<StartupOwnedModules>(0x434792E0u) ||
        &bindings.required_modules != &image.native_cell<StartupOwnedModules>(0x434792D0u) ||
        &bindings.text_buckets != &image.native_cell<StartupTextBuckets>(0x4346D700u) ||
        &bindings.pointer_buckets != &image.native_cell<StartupPointerBuckets>(0x4346DA40u))
        throw std::invalid_argument(
            "Startup container cleanup must borrow its canonical native cells");
    sources.bind_initializer({0x4337E843u, &context, register_finalizer<0x4333087Cu>});
    sources.bind_initializer({0x4331FDB2u, &context, register_finalizer<0x433D2E2Bu>});
    sources.bind_initializer({0x433C9DFEu, &context, register_finalizer<0x4331ECD4u>});
    sources.bind_initializer({0x4337FFB5u, &context, register_finalizer<0x433F4BCBu>});
    sources.bind_initializer({0x4339791Au, &context, register_finalizer<0x43336C3Cu>});
    sources.bind_initializer({0x43398F0Cu, &context, register_finalizer<0x4334D2ABu>});
    sources.bind_initializer({0x433999F0u, &context, register_finalizer<0x4335851Bu>});
    sources.bind_initializer({0x4337F097u, &context, register_finalizer<0x433ACF65u>});
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: startup_convars.cpp
// ============================================================================

namespace skeet::engine {
namespace {
// The working dump's decoded objects name these exact ConVars at+C. Each
// key is independently present in the original consumers of its native cell.
constexpr std::array<StartupConVarSlot, 9> k_startup_convar_slots{
    {{0x43467FBCu, 0x96BEB183u, "rate"},
     {0x43467130u, 0x99854835u, "net_maxroutable"},
     {0x4346A5B4u, 0x7CBE81A1u, "cl_ragdoll_workaround_threshold"},
     {0x43468BA8u, 0x30911EABu, "cl_smooth"},
     {0x434671ACu, 0xD8BFAEFFu, "cl_cmdrate"},
     {0x4346747Cu, 0x03B4819Cu, "cl_updaterate"},
     {0x4346A748u, 0x7D9DC86Fu, "cl_interp"},
     {0x4346A330u, 0xBE64D223u, "cl_interp_ratio"},
     {0x43468EA4u, 0xFCD2DA7Au, "weapon_accuracy_nospread"}}};
class EngineConVarProvider final : public StartupConVarProvider {
public:
    void* find_variable(void* cvar, const char* name) override {
        return invoke<void*>(cvar, 15, name);
    }
};
NativeConVarReference reference(NativeDataImage& data, std::size_t index) {
    const auto& slot = k_startup_convar_slots[index];
    return {data.native_cell<std::uint32_t>(slot.address), slot.xor_key};
}
}
std::span<const StartupConVarSlot, 9> startup_convar_slots() noexcept {
    return k_startup_convar_slots;
}
client::StartupConVars StartupConVarBindings::startup_variables() const {
    return {reference(data_, 0), reference(data_, 1), reference(data_, 2), reference(data_, 3),
            reference(data_, 4), reference(data_, 5), reference(data_, 6), reference(data_, 7)};
}
NativeConVarReference StartupConVarBindings::accuracy_variable() const {
    return reference(data_, 8);
}

StartupConVarBindings discover_startup_convars(NativeDataImage& data, void* cvar,
                                               StartupConVarProvider& provider) {
    if (!cvar)
        throw std::invalid_argument("Startup ConVar discovery requires the real ICvar interface");
    std::array<std::uint32_t*, k_startup_convar_slots.size()> destinations;
    for (std::size_t index = 0; index < k_startup_convar_slots.size(); ++index)
        destinations[index] = &data.native_cell<std::uint32_t>(k_startup_convar_slots[index].address);
    std::array<void*, k_startup_convar_slots.size()> found;
    for (std::size_t index = 0; index < k_startup_convar_slots.size(); ++index) {
        found[index] = provider.find_variable(cvar, k_startup_convar_slots[index].name);
        if (!found[index])
            throw std::runtime_error(std::string("Required startup ConVar is unavailable: ") +
                                     k_startup_convar_slots[index].name);
    }
    // Every destination and lookup is validated before these nonthrowing
    // stores. Other native consumers continue to read the encoded words.
    for (std::size_t index = 0; index < k_startup_convar_slots.size(); ++index)
        *destinations[index] =
            std::uint32_t(reinterpret_cast<std::uintptr_t>(found[index])) ^ k_startup_convar_slots[index].xor_key;
    return StartupConVarBindings(data);
}
StartupConVarBindings discover_startup_convars(NativeDataImage& data, void* cvar) {
    EngineConVarProvider provider;
    return discover_startup_convars(data, cvar, provider);
}
} //namespace skeet::engine

// ============================================================================
// Subcomponent: startup_operations.cpp
// ============================================================================
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace skeet::engine {
// Devirtualized from VM entry 0x4354DB7E: LOCK BTR on x86 PEB + 0x28, bit 2.
void clear_startup_process_flag(volatile std::uint32_t& process_flags) noexcept {
    static_assert(sizeof(long) == sizeof(process_flags));
    InterlockedAnd(reinterpret_cast<volatile long*>(&process_flags), ~4L);
}

// Devirtualized from VM entry 0x4354DB7E: select an FF 23 match using RDTSC.
// The caller publishes the result to the native jump-target slot 0x43476448.
std::byte* find_startup_jump_target(std::byte* client_module, std::uint32_t timestamp_low) {
    constexpr std::array<std::uint8_t, 2> pattern{0xff, 0x23};
    const auto occurrence = (timestamp_low >> 3) & 31u;
    return executable_pattern(client_module, pattern.data(), 2, occurrence, 0xcc);
}

// Devirtualized from VM entry 0x4354DB7E: initialize the 0x43476B2C sentinel.
// Preserve the third field and the original next-then-previous store order.
void initialize_startup_hook_list(VtableHookNode& sentinel) noexcept {
    sentinel.next = &sentinel;
    sentinel.previous = &sentinel;
}

// Devirtualized from VM entry 0x4354DB7E: subtract in 64 bits, then divide signed.
// The subtraction wraps before interpretation as a signed value.
std::int64_t startup_unix_seconds(std::uint64_t file_time) noexcept {
    constexpr std::uint64_t epoch_offset = 116444736000000000ULL;
    return std::bit_cast<std::int64_t>(file_time - epoch_offset) / 10000000LL;
}

// Devirtualized from VM entry 0x4354DB7E: atomically consume 0x4346D4CC.
// Its initial value is the salted comctl32.dll hash; subsequent calls consume
// the current value, including zero. The native lookup requests the PEB lock.
std::byte* take_startup_common_controls(volatile std::uint32_t& module_hash,
                                        const LoadedModuleEnvironment& modules) {
    static_assert(sizeof(long) == sizeof(module_hash));
    const auto hash = static_cast<std::uint32_t>(
        InterlockedExchange(reinterpret_cast<volatile long*>(&module_hash), 0));
    return static_cast<std::byte*>(find_loaded_module(modules, hash, true, true));
}
}

// ============================================================================
// Subcomponent: startup_vector_cleanup.cpp
// ============================================================================

namespace skeet::engine {
namespace {
StartupVectorCleanupBindings* active_vector_cleanup = nullptr;

/* [dedup release] */
constexpr std::array finalizers{
    0x4332F1FDu, 0x4334DA70u, 0x4339D4FEu, 0x433874D2u, 0x4341EFD3u,
    0x433C8C11u, 0x433AFCE8u, 0x433C956Cu, 0x433FBE15u,
};
constexpr std::array initializers{
    0x433C7FAEu, 0x4334C970u, 0x43325025u, 0x433F47E6u, 0x4340BCC1u, 0x433412AFu, 0x4334115Au,
};
template <std::size_t Index> void finalize_entry() {
    if (!active_vector_cleanup)
        throw std::logic_error("Startup vector cleanup has no stable source binding");
    active_vector_cleanup->finalize(finalizers[Index]);
}
template <std::size_t... Index> constexpr auto finalizer_functions(std::index_sequence<Index...>) {
    return std::array<void (*)(), sizeof...(Index)>{&finalize_entry<Index>...};
}
constexpr auto cleanup_functions = finalizer_functions(std::make_index_sequence<finalizers.size()>{});
template <std::size_t Index> void register_entry(void* owner) {
    auto& binding = *static_cast<StartupVectorCleanupBindings*>(owner);
    (void)binding.callbacks.append(binding.source_finalizer(finalizers[Index]));
}
template <std::size_t... Index>
void bind_entries(client::NativeStartupSources& sources, StartupVectorCleanupBindings& binding,
                  std::index_sequence<Index...>) {
    (sources.bind_initializer({initializers[Index], &binding, &register_entry<Index>}), ...);
}
}

void cleanup_startup_callback_map(StartupCallbackMap& map, JobMemory memory) {
    const auto count = map.bucket_count;
    auto** const buckets = map.buckets;
    for (std::uint32_t index = 0; index < count; ++index) {
        auto* node = buckets[index];
        while (node) {
            auto* const next = node->next;
            release(node, memory);
            node = next;
        }
        buckets[index] = nullptr;
    }
    map.size = 0;
    // The wrapper reloads both fields after the complete clear helper.
    if (map.bucket_count > 1)
        release(map.buckets, memory);
}

void cleanup_startup_v8_slots(StartupV8Slots& slots, DisposeStartupV8Global dispose,
                              JobMemory memory) {
    auto* slot = slots.begin;
    auto* const end = slots.end;
    while (slot != end) {
        if (*slot) {
            dispose(*slot);
            *slot = nullptr;
        }
        ++slot;
    }
    // DisposeGlobal may replace the current header. Native frees the live
    // begin pointer, after traversing the range captured at entry.
    release(slots.begin, memory);
}

void cleanup_startup_v8_handle(StartupV8Handle& global, DisposeStartupV8Global dispose) {
    if (global.owns_handle && global.handle) {
        dispose(global.handle);
        global.handle = nullptr;
    }
}

void cleanup_startup_ui_bindings(StartupVector<StartupUiBinding>& bindings, JobMemory memory) {
    auto* cursor = bindings.begin;
    auto* const end = bindings.end;
    if (cursor != end) {
        do {
            release(cursor->references.begin, memory);
            ++cursor;
        } while (cursor != end);
        cursor = bindings.begin;
    }
    release(cursor, memory);
}

void cleanup_startup_history(lagcomp::LagRecord*& free_head,
                             std::span<client::NativePlayerStateLayout*, 64> players,
                             JobMemory memory) {
    for (auto* const player : players) {
        if (!player)
            continue;
        while (player->history_count) {
            const auto index = (player->history_first + player->history_count - 1u) & 31u;
            auto* record = reinterpret_cast<lagcomp::LagRecord*>(
                std::uintptr_t(player->native_history_links[index]));
            if (record) {
                record->header.field_00 =
                    std::uint32_t(reinterpret_cast<std::uintptr_t>(free_head));
                free_head = record;
            }
            --player->history_count;
        }
    }
    while (free_head) {
        auto* const record = free_head;
        free_head = reinterpret_cast<lagcomp::LagRecord*>(std::uintptr_t(record->header.field_00));
        record->header.field_00 = 0;
        release(record, memory);
    }
    for (std::uint32_t index = 64; index;) {
        auto*& player = players[--index];
        if (player) {
            release(player, memory);
            player = nullptr;
        }
    }
}

void cleanup_startup_reference_strings(StartupInlineVector<StartupReferenceString>& strings,
                                       JobMemory memory) {
    auto* cursor = strings.begin;
    auto* const end = strings.end;
    if (cursor != end) {
        do {
            auto& text = cursor->text;
            const auto byte_capacity = std::uint32_t(reinterpret_cast<std::uintptr_t>(text.limit) -
                                                     reinterpret_cast<std::uintptr_t>(text.begin)) &
                                       0xFFFFFFFEu;
            if (std::bit_cast<std::int32_t>(byte_capacity) > 2 && text.begin)
                release(text.begin, memory);
            ++cursor;
        } while (cursor != end);
        cursor = strings.begin;
    }
    if (cursor && cursor != strings.inline_buffer)
        release(cursor, memory);
}

void cleanup_startup_inline_callbacks(StartupInlineVector<std::uint32_t>& callbacks,
                                      JobMemory memory) {
    auto* const begin = callbacks.begin;
    if (begin && begin != callbacks.inline_buffer)
        release(begin, memory);
}

StartupVectorCleanupBindings::StartupVectorCleanupBindings(PreservedInitialization& initialization,
                                                           client::ExitCallbacks& callbacks,
                                                           JobMemory memory,
                                                           DisposeStartupV8Global dispose)
    : StartupVectorCleanupBindings(initialization.image_owner(), callbacks, memory, dispose) {}
StartupVectorCleanupBindings::StartupVectorCleanupBindings(std::shared_ptr<NativeDataImage> image,
                                                           client::ExitCallbacks& callbacks,
                                                           JobMemory memory,
                                                           DisposeStartupV8Global dispose)
    : data(image), callbacks(callbacks), memory(memory), dispose_global(dispose) {
    (void)require_native_data_image(data);
    if (!memory.free || !memory.process_heap || !dispose_global)
        throw std::invalid_argument("Startup vector cleanup requires process heap and V8 imports");
}

void StartupVectorCleanupBindings::finalize(std::uint32_t address) {
    const auto& image = require_native_data_image(data);
    switch (address) {
    case 0x4332F1FDu:
        cleanup_startup_callback_map(image.native_cell<StartupCallbackMap>(0x4346DA60u), memory);
        break;
    case 0x4334DA70u:
        cleanup_startup_callback_map(image.native_cell<StartupCallbackMap>(0x4346DA80u), memory);
        break;
    case 0x4339D4FEu:
        cleanup_startup_v8_slots(image.native_cell<StartupV8Slots>(0x4347963Cu), dispose_global,
                                 memory);
        break;
    case 0x433874D2u:
        cleanup_startup_v8_slots(image.native_cell<StartupV8Slots>(0x4347960Cu), dispose_global,
                                 memory);
        break;
    case 0x4341EFD3u:
        cleanup_startup_v8_handle(image.native_cell<StartupV8Handle>(0x43479600u), dispose_global);
        break;
    case 0x433C8C11u:
        cleanup_startup_ui_bindings(image.native_cell<StartupVector<StartupUiBinding>>(0x43479760u),
                                    memory);
        break;
    case 0x433AFCE8u:
        cleanup_startup_history(
            image.native_cell<lagcomp::LagRecord*>(0x434797B8u),
            image.native_cell<std::array<client::NativePlayerStateLayout*, 64>>(0x434797BCu),
            memory);
        break;
    case 0x433C956Cu:
        cleanup_startup_reference_strings(
            image.native_cell<StartupInlineVector<StartupReferenceString>>(0x4346D720u), memory);
        break;
    case 0x433FBE15u:
        cleanup_startup_inline_callbacks(
            image.native_cell<StartupInlineVector<std::uint32_t>>(0x4346DAA0u), memory);
        break;
    default:
        throw std::out_of_range("Finalizer is not a recovered startup vector cleanup entry");
    }
}

void (*StartupVectorCleanupBindings::source_finalizer(std::uint32_t address))() {
    for (std::size_t index = 0; index < finalizers.size(); ++index) {
        if (finalizers[index] != address)
            continue;
        if (active_vector_cleanup && active_vector_cleanup != this)
            throw std::logic_error(
                "Startup vector exit callbacks require one stable source binding");
        active_vector_cleanup = this;
        return cleanup_functions[index];
    }
    throw std::out_of_range("Finalizer is not a recovered startup vector cleanup entry");
}

void bind_startup_vector_cleanup(client::NativeStartupSources& sources,
                                 StartupVectorCleanupBindings& binding) {
    if (&sources.data() != binding.data.get())
        throw std::invalid_argument(
            "Startup vector cleanup must borrow the same native data owner");
    bind_entries(sources, binding, std::make_index_sequence<initializers.size()>{});
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: tier0_startup_exports.cpp
// ============================================================================

namespace skeet::engine {
Tier0StartupExports initialize_tier0_startup_exports(NativeDataImage& data,
                                                     const LoadedModuleEnvironment& modules,
                                                     NativeImageEnvironment images) {
    if (!images.image_nt_header)
        throw std::invalid_argument(
            "Tier0 startup exports require the genuine native image-header import");
    auto* const module =
        static_cast<std::byte*>(find_loaded_module(modules, 0xBBFFA7E0u, true, true));
    if (!module)
        throw std::runtime_error("Tier0 startup export loop requires its already loaded module");
    data.native_cell<Tier0FloatTime>(0x43468C84u) =
        reinterpret_cast<Tier0FloatTime>(resolve_pe_export(module, 0xB3D93471u, 0, true, images));
    data.native_cell<void**>(0x43467204u) =
        reinterpret_cast<void**>(resolve_pe_export(module, 0xD708420Au, 0, true, images));
    data.native_cell<Tier0MainThreadPredicate>(0x43476920u) =
        reinterpret_cast<Tier0MainThreadPredicate>(
            resolve_pe_export(module, 0x245D80A2u, 0, true, images));
    // The VM performs this dereference only after the three-row loop finishes.
    auto* const allocator_export = data.native_cell<void**>(0x43467204u);
    if (!allocator_export)
        throw std::runtime_error(
            "Tier0 g_pMemAlloc export is required by the native startup dereference");
    data.native_cell<void*>(0x43467204u) = *allocator_export;
    return {data.native_cell<Tier0FloatTime>(0x43468C84u), data.native_cell<void*>(0x43467204u),
            data.native_cell<Tier0MainThreadPredicate>(0x43476920u)};
}
} // namespace skeet::engine

// ============================================================================
// Subcomponent: unload.cpp
// ============================================================================
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace skeet::engine {
namespace {
#if defined(_WIN32)
std::int32_t WINAPI native_set_long_w(std::uintptr_t window, int index, std::int32_t value) {
    return static_cast<std::int32_t>(
        SetWindowLongW(reinterpret_cast<HWND>(window), index, static_cast<LONG>(value)));
}
std::int32_t WINAPI native_set_long_a(std::uintptr_t window, int index, std::int32_t value) {
    return static_cast<std::int32_t>(
        SetWindowLongA(reinterpret_cast<HWND>(window), index, static_cast<LONG>(value)));
}
std::int32_t WINAPI native_get_long_w(std::uintptr_t window, int index) {
    return static_cast<std::int32_t>(GetWindowLongW(reinterpret_cast<HWND>(window), index));
}
std::int32_t WINAPI native_get_long_a(std::uintptr_t window, int index) {
    return static_cast<std::int32_t>(GetWindowLongA(reinterpret_cast<HWND>(window), index));
}
#endif
template <class Pointer> void exchange_pointer(Pointer* target, Pointer value) {
#if defined(_WIN32)
    InterlockedExchangePointer(reinterpret_cast<void* volatile*>(target),
                               reinterpret_cast<void*>(value));
#else
    __atomic_exchange_n(target, value, __ATOMIC_SEQ_CST);
#endif
}
void restore_patch(const SavedPointerPatch& patch) {
    if (patch.enabled && patch.target)
        exchange_pointer(patch.target, patch.saved);
}
}

UnloadSystemFunctions native_unload_system_functions(const CommonControlsExports& common_controls) {
#if defined(_WIN32)
    const auto native = GetModuleHandleA("ntdll.dll");
    if (!native)
        throw std::runtime_error("Required native unload module is not loaded");
    const auto remove = common_controls.remove_window_subclass;
    const auto free = GetProcAddress(native, "RtlFreeHeap");
    if (!remove || !free)
        throw std::runtime_error("Required native unload export is missing");
    UnloadSystemFunctions result;
    result.set_window_long_w = &native_set_long_w;
    result.set_window_long_a = &native_set_long_a;
    result.get_window_long_w = &native_get_long_w;
    result.get_window_long_a = &native_get_long_a;
    result.remove_window_subclass =
        reinterpret_cast<UnloadSystemFunctions::RemoveWindowSubclass>(remove);
    result.free_heap = reinterpret_cast<UnloadSystemFunctions::FreeHeap>(free);
    result.process_heap = GetProcessHeap();
    return result;
#else
    (void)common_controls;
    throw std::runtime_error("Native unload system functions require Windows");
#endif
}

void UnloadRuntime::restore_window() {
    auto& window = state.window;
    if (!window.window || !window.subclass_id)
        return;
    system.set_window_long_w(window.window, -21, window.expected_wide_procedure);
    if (!system.remove_window_subclass(window.window, window.subclass_procedure,
                                       window.subclass_id))
        return;
    if (system.get_window_long_a(window.window, -4) == window.expected_ansi_procedure)
        system.set_window_long_a(window.window, -4, window.restored_ansi_procedure);
    if (system.get_window_long_w(window.window, -4) == window.expected_wide_procedure)
        system.set_window_long_w(window.window, -4, window.restored_wide_procedure);
    window.window = 0;
    window.subclass_id = 0;
}

void free_native_allocation(void* object, const UnloadSystemFunctions& system) {
    if (!object)
        return;
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof(displacement));
    system.free_heap(system.process_heap, 0, bytes - 16 - displacement);
}

void UnloadRuntime::cleanup() {
    restore_window();
    // The production aliases hold pointer-list representations. Write their
    // bytes without accessing the pointer objects through a DWORD lvalue.
    const std::uint32_t cleared = 0;
    std::memcpy(&state.clear_43476908, &cleared, sizeof(cleared));
    std::memcpy(&state.clear_4347644c, &cleared, sizeof(cleared));
    auto& sentinel = state.hooks;
    while (sentinel.previous != &sentinel) {
        auto* node = sentinel.next;
        sentinel.next = node->next;
        sentinel.next->previous = &sentinel;
        auto** installed = *static_cast<void***>(node->object);
        const auto* metadata = static_cast<const VtableHookMetadata*>(installed[-2]);
        exchange_pointer(static_cast<void***>(node->object), metadata->original_table);
        free_native_allocation(node, system);
    }
    if (state.in_game_callback && invoke<bool>(state.engine, 27))
        state.in_game_callback();
    restore_patch(state.first_patch);
    restore_patch(state.second_patch);
    if (state.cleared_patch.target && state.cleared_patch.saved)
        exchange_pointer(state.cleared_patch.target, state.cleared_patch.saved);
    state.cleared_patch = {};
    const auto objects = state.owners.entries();
    for (auto* object : objects)
        invoke<void>(object, 4);
    restore_patch(state.final_patch);
    if (state.listener_registered) {
        state.listener_registered = false;
        invoke<void, void*>(state.event_manager, 21, state.event_listener);
    }
}

} // namespace skeet::engine
