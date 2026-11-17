#pragma once
#include "owner_registry.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace skeet::engine {
struct CommonControlsExports;

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_UNLOAD_STDCALL __stdcall
#else
#define SKEET_UNLOAD_STDCALL
#endif

struct WindowHookState {
    std::uintptr_t window = 0; // 0x434797B4
    std::uintptr_t subclass_id = 0; // 0x434798D4
    void* subclass_procedure = nullptr; // original 0x4341F1DF
    std::int32_t expected_ansi_procedure = 0; // 0x43475780
    std::int32_t restored_ansi_procedure = 0; // 0x43475848
    std::int32_t expected_wide_procedure = 0; // 0x43475B5C; also written to -21
    std::int32_t restored_wide_procedure = 0; // 0x4347580C
};

struct UnloadSystemFunctions {
    using SetLongFunction = std::int32_t(SKEET_UNLOAD_STDCALL*)(std::uintptr_t, int, std::int32_t);
    using GetLongFunction = std::int32_t(SKEET_UNLOAD_STDCALL*)(std::uintptr_t, int);
    using RemoveWindowSubclass = int(SKEET_UNLOAD_STDCALL*)(std::uintptr_t, void*, std::uintptr_t);
    using FreeHeap = unsigned char(SKEET_UNLOAD_STDCALL*)(void*, std::uint32_t, void*);
    SetLongFunction set_window_long_w = nullptr;
    SetLongFunction set_window_long_a = nullptr;
    GetLongFunction get_window_long_w = nullptr;
    GetLongFunction get_window_long_a = nullptr;
    RemoveWindowSubclass remove_window_subclass = nullptr;
    FreeHeap free_heap = nullptr;
    void* process_heap = nullptr;
};

// Binds the actual Windows APIs; a missing required loaded export is an error.
UnloadSystemFunctions native_unload_system_functions(const CommonControlsExports&);

struct VtableHookNode {
    VtableHookNode* previous;
    VtableHookNode* next;
    void* object;
};

struct VtableHookMetadata {
    std::uintptr_t unknown_0;
    void* target; // native+4, captured receiver used by manual patch owners
    void** original_table; // native metadata +8, installed vtable[-2]
    void* owner; // native+C, captured callback owner (AutoAccept/ConsoleInput)
    void** replacement_table; // native metadata +0x10, used by4331E834
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(VtableHookMetadata, target) == 4);
static_assert(offsetof(VtableHookMetadata, original_table) == 8);
static_assert(offsetof(VtableHookMetadata, owner) == 0xc);
static_assert(offsetof(VtableHookMetadata, replacement_table) == 0x10);
static_assert(sizeof(VtableHookMetadata) == 0x14);
#endif

struct SavedPointerPatch {
    void** target = nullptr;
    void* saved = nullptr;
    std::uintptr_t enabled = 0;
};

struct UnloadState {
private:
    struct StandaloneGlobals {
        bool unloading = false;
        std::uint32_t clear_43476908 = 0, clear_4347644c = 0;
        void* engine = nullptr;
        void (*in_game_callback)() = nullptr;
        bool listener_registered = false;
        void* event_manager = nullptr;
    } standalone_;

public:
    explicit UnloadState(OwnerRegistry& registry)
        : owners(registry), unloading(standalone_.unloading),
          clear_43476908(standalone_.clear_43476908), clear_4347644c(standalone_.clear_4347644c),
          engine(standalone_.engine), in_game_callback(standalone_.in_game_callback),
          listener_registered(standalone_.listener_registered),
          event_manager(standalone_.event_manager) {}
    UnloadState(OwnerRegistry& registry, bool& actual_unloading,
                std::uint32_t& actual_first_patches, std::uint32_t& actual_second_patches,
                void*& actual_engine, void (*&actual_in_game_callback)(),
                bool& actual_listener_registered, void*& actual_event_manager)
        : owners(registry), unloading(actual_unloading), clear_43476908(actual_first_patches),
          clear_4347644c(actual_second_patches), engine(actual_engine),
          in_game_callback(actual_in_game_callback),
          listener_registered(actual_listener_registered), event_manager(actual_event_manager) {}
    UnloadState(const UnloadState&) = delete;
    UnloadState& operator=(const UnloadState&) = delete;
    UnloadState(UnloadState&&) = delete;
    UnloadState& operator=(UnloadState&&) = delete;
    OwnerRegistry& owners; // canonical43475B78/7C/80 storage
    // Native BSS byte43475895. Only hard detach433EF91A sets it; the normal
    // menu request/timer finalization leaves browser disposal enabled.
    bool& unloading;
    WindowHookState window;
    std::uint32_t& clear_43476908; //native DWORD representation of the entity patch-list head
    std::uint32_t& clear_4347644c; //native DWORD representation of the incoming patch-list head
    VtableHookNode hooks{&hooks, &hooks, nullptr}; // native empty sentinel 0x43476B2C
    void*& engine; // live0x4346A514; virtual entry27
    void (*&in_game_callback)(); //live0x43468BF0
    SavedPointerPatch first_patch; // target 0x434757CC, saved +4, enabled +12
    SavedPointerPatch second_patch; // target 0x43475A40, saved +4, enabled +12
    SavedPointerPatch cleared_patch; // 0x43476A40/+4/+8, all cleared unconditionally
    SavedPointerPatch final_patch; // target 0x43475920, saved +4, enabled +12
    bool& listener_registered; // live0x43468B80
    // These legacy field names identify renderer-device notifications, not the
    // game's event registry. Startup registers the same listener at slot20.
    void*& event_manager; // live0x4346A470; virtual entry21
    void* event_listener = nullptr; // original address 0x43468B7C
};

// Restored native hook lifecycle. Engine virtual calls perform their actual
// operations; only OS-export dependencies are explicit for offline verification.
class UnloadRuntime {
public:
    UnloadRuntime(UnloadState& state, UnloadSystemFunctions system)
        : state(state), system(system) {}
    UnloadState& state;
    UnloadSystemFunctions system;
    void restore_window(); // complete 0x4335B9C9
    void cleanup(); // complete 0x433E445B, includes restore_window
};

// Complete 0x433837D0. Allocations have the native alignment displacement dword
// at object-8 and are freed from object-16-displacement on the process heap.
void free_native_allocation(void* object, const UnloadSystemFunctions& system);

#undef SKEET_UNLOAD_STDCALL
} // namespace skeet::engine
