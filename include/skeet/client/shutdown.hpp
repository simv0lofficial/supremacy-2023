#pragma once
#include "skeet/engine/unload.hpp"
#include "skeet/input/runtime.hpp"
#include "skeet/menu/window_lifetime.hpp"
#include "skeet/script/shutdown.hpp"
#include <stdexcept>

namespace skeet::ui {
class RendererContainer;
class MainMenuWindowOwner;
}
namespace skeet::client {
struct ChannelHookState;
struct MemoryRegion {
    void* base_address = nullptr;
    void* allocation_base = nullptr;
    std::uint32_t allocation_protection = 0;
    std::size_t region_size = 0;
    std::uint32_t state = 0, protection = 0, type = 0;
};
struct ShutdownImports {
    using KillTimer = int(__stdcall*)(std::uintptr_t, std::uintptr_t);
    using QueryMemory = std::int32_t(__stdcall*)(void*, const void*, std::uint32_t, MemoryRegion*,
                                                 std::size_t, std::size_t*);
    using UnmapSection = std::int32_t(__stdcall*)(void*, void*);
    KillTimer kill_timer = nullptr;
    QueryMemory query_memory = nullptr;
    UnmapSection unmap_section = nullptr;
};
ShutdownImports native_shutdown_imports();

struct ExitCallbacks {
    std::span<std::uintptr_t> entries;
    // Registration and teardown borrow the same cells; callbacks can observe
    // and change the live count while the terminal pass is running.
    std::uintptr_t& xor_mask; //4347CA08
    std::uint32_t& count; //4347CA0C
    // 433837F5 requires the actual512-slot backing store. Registration is
    // rejected only at count512; duplicates do not increment the count.
    int append(void (*callback)());
    void run(); // 0x433B3987, leaves count=UINT32_MAX after completion.
};

class ShutdownRuntime {
    ChannelHookState* source_channel_ = nullptr;

public:
    ShutdownRuntime(script::ShutdownState& scripts, input::Runtime& input, ShutdownImports imports,
                    bool& scripting_enabled, engine::UnloadRuntime& hooks,
                    ExitCallbacks& exit_callbacks, NativeConfigPath& config_path,
                    ui::MenuWindow& main_window)
        : scripts(scripts), input(input), imports(imports), hooks(hooks),
          native_window(main_window.native_window), scripting_enabled(scripting_enabled),
          owners(hooks.state.owners), window_title(config_path), exit_callbacks(exit_callbacks) {
        if (!native_window)
            throw std::invalid_argument("Client shutdown requires the resolved main window");
    }
    ShutdownRuntime(script::ShutdownState& scripts, input::Runtime& input, ShutdownImports imports,
                    bool& scripting_enabled, engine::UnloadRuntime& hooks,
                    ExitCallbacks& exit_callbacks, NativeConfigPath& config_path,
                    ui::MenuWindow& main_window, ChannelHookState& channel)
        : ShutdownRuntime(scripts, input, imports, scripting_enabled, hooks, exit_callbacks,
                          config_path, main_window) {
        source_channel_ = &channel;
    }
    script::ShutdownState& scripts;
    input::Runtime& input;
    ShutdownImports imports;
    engine::UnloadRuntime& hooks;
    // Borrow the menu foundation's HWND cell. Hook restoration clears its own
    // installation state before the unload timer is created and cancelled.
    const std::uintptr_t& native_window;
    std::uint32_t installed_engine_dispatch = 0, saved_engine_dispatch = 0;
    bool& scripting_enabled; // Native 0x434688BD; bind ScriptRuntime.enabled.
    engine::OwnerRegistry& owners; // same43475B78/7C/80 storage as startup/unload
    ui::WindowCallbackStorage* main_callbacks = nullptr;
    ui::WindowTitleString& window_title; //same native434798CC cell as CONFIG file backend
    ui::WindowLifetimeImports window_imports{};
    ui::RendererContainer* renderer = nullptr;
    ExitCallbacks& exit_callbacks; // same512slots/count/mask as process initialization
    // Source wrappers have a longer outer C++ lifetime than native slot0.
    // Detach RootMenu while its ScriptOwner references are still alive, and
    // publish the native GUI release before later outer-owner destruction.
    ui::MainMenuWindowOwner* source_main_window = nullptr;
    ui::RendererContainer** source_renderer_cell = nullptr;

    void restore_engine_dispatch(); // 0x43365AA1
    void release_cleanup_objects(); // 0x4339D451, virtual slot 0.
    void destroy_gui(); // 0x43336C51, logical ownership release for typed objects.
    void cancel_timer(); // 0x433D7307, shared resolved HWND and timer ID0x332.
    void* finalize(); // 0x433C87F6, returns this code section's allocation base.
    void detach(); // 0x433EF91A, hard teardown; no timer/query/unmap tail.
};

// Bind before installing the unload timer. The callback performs a machine tail
// jump to ZwUnmapViewOfSection, so it never returns through unmapped client code.
void bind_shutdown_runtime(ShutdownRuntime& runtime) noexcept;
ShutdownRuntime* bound_shutdown_runtime() noexcept;
void unbind_shutdown_runtime(ShutdownRuntime&) noexcept;
void* finalize_client(); // Native no-argument entry 0x433C87F6.
void detach_client(); // Native no-argument entry 0x433EF91A.
}
