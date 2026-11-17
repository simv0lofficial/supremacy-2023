#pragma once
#include "skeet/input/runtime.hpp"
#include "skeet/engine/loaded_modules.hpp"
#include "skeet/engine/unload.hpp"

namespace skeet::input {
using SubclassProcedure = std::intptr_t(__stdcall*)(void*, std::uint32_t, std::uintptr_t,
                                                    std::intptr_t, std::uintptr_t, std::uintptr_t);
struct WindowHookImports {
    using SetSubclass = int(__stdcall*)(std::uintptr_t, SubclassProcedure, std::uintptr_t,
                                        std::uintptr_t);
    using DefaultProcedure = std::intptr_t(__stdcall*)(std::uintptr_t, std::uint32_t,
                                                       std::uintptr_t, std::intptr_t);
    using WindowThread = std::uint32_t(__stdcall*)(std::uintptr_t, std::uint32_t*);
    using FileHeader = void*(__stdcall*)(void*, void**);
    std::uint64_t (*read_timestamp_counter)() = nullptr;
    std::uintptr_t* (*current_thread_id_slot)() = nullptr;
    SetSubclass set_window_subclass = nullptr;
    DefaultProcedure default_subclass_proc = nullptr;
    WindowThread get_window_thread = nullptr;
    FileHeader pc_to_file_header = nullptr;
};
WindowHookImports native_window_hook_imports(const engine::CommonControlsExports&);
std::intptr_t __stdcall window_subclass_callback(void* window, std::uint32_t message,
                                                 std::uintptr_t parameter, std::intptr_t key_data,
                                                 std::uintptr_t identifier,
                                                 std::uintptr_t reference_data);

class WindowSubclassRuntime {
public:
    WindowSubclassRuntime(Runtime& input, engine::UnloadRuntime& hooks,
                          engine::LoadedModuleEnvironment modules, WindowHookImports imports)
        : input(input), hooks(hooks), modules(modules), imports(imports) {}
    Runtime& input;
    engine::UnloadRuntime& hooks;
    engine::LoadedModuleEnvironment modules;
    WindowHookImports imports;
    // Native pointer4346D48C is consumed before SetWindowSubclass, even on failure.
    SubclassProcedure pending_procedure = window_subclass_callback;
    void install(std::uintptr_t window); // 0x433CA545
    std::intptr_t process(std::uintptr_t window, std::uint32_t message, std::uintptr_t parameter,
                          std::intptr_t key_data, std::uintptr_t identifier); // 0x4341F1DF
};
void bind_window_subclass_runtime(WindowSubclassRuntime& runtime) noexcept;
}
