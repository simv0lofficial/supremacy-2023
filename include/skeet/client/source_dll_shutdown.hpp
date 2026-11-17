#pragma once
#include <cstdint>

namespace skeet::misc {
class UnloadRuntime;
}
namespace skeet::client {
class ShutdownRuntime;

struct SourceDllShutdownImports {
    // The second GetModuleHandleExW operand is a code address when flag4 is
    // set. Flag2 preserves the existing loader reference count.
    using ModuleFromAddress = int(__stdcall*)(std::uint32_t, const void*, void**);
    using FreeLibrary = int(__stdcall*)(void*);
    ModuleFromAddress module_from_address;
    FreeLibrary free_library;
};
SourceDllShutdownImports native_source_dll_shutdown_imports();

enum class SourceDllShutdownReadiness {
    waiting,
    ready,
    failed
};
struct SourceDllShutdownCompletion {
    void* context = nullptr;
    // Runs after all native finalization, before the loader tail jump. It may
    // delete the application, including this profile and its ShutdownRuntime.
    void (*complete)(void*) noexcept = nullptr;
    // The application publishes ready after its last initialization access,
    // or failed before rollback. Once ready, it cannot transition to failed.
    SourceDllShutdownReadiness (*readiness)(void*) noexcept = nullptr;
};

// Explicit profile for this source DLL when its owner acquired a LoadLibrary
// reference. The supplied HMODULE must contain the actual timer entry below.
// Construction neither binds startup nor schedules any timer. Keep the shared
// shutdown runtime alive through its complete finalization and keep this
// profile alive until the scheduled timer begins finalization.
class SourceDllShutdownProfile {
public:
    SourceDllShutdownProfile(ShutdownRuntime&, void* source_module);
    SourceDllShutdownProfile(ShutdownRuntime&, void* source_module, SourceDllShutdownImports);
    SourceDllShutdownProfile(ShutdownRuntime&, void* source_module, SourceDllShutdownCompletion);
    SourceDllShutdownProfile(ShutdownRuntime&, void* source_module, SourceDllShutdownImports,
                             SourceDllShutdownCompletion);
    ~SourceDllShutdownProfile();
    SourceDllShutdownProfile(const SourceDllShutdownProfile&) = delete;
    SourceDllShutdownProfile& operator=(const SourceDllShutdownProfile&) = delete;
    SourceDllShutdownProfile(SourceDllShutdownProfile&&) = delete;
    SourceDllShutdownProfile& operator=(SourceDllShutdownProfile&&) = delete;
    ShutdownRuntime& runtime() const noexcept {
        return runtime_;
    }
    void* module() const noexcept {
        return module_;
    }
    SourceDllShutdownImports imports() const noexcept {
        return imports_;
    }
    SourceDllShutdownCompletion completion() const noexcept {
        return completion_;
    }
    void cancel_deferred_request() noexcept;

private:
    friend void bind_source_dll_shutdown(SourceDllShutdownProfile&, misc::UnloadRuntime&);
    friend void __stdcall source_dll_deferred_request_timer_callback(void*, std::uint32_t,
                                                                     std::uintptr_t, std::uint32_t);
    bool allow_request_locked();
    void cancel_deferred_request_locked() noexcept;
    ShutdownRuntime& runtime_;
    void* module_;
    SourceDllShutdownImports imports_;
    SourceDllShutdownCompletion completion_;
    misc::UnloadRuntime* request_ = nullptr;
    std::uintptr_t deferred_window_ = 0;
};

// Select only after the source startup graph and its actual shutdown runtime
// exist. This changes this request owner's timer entry explicitly; the retained
// mapped-image profile and its NtUnmapViewOfSection tail are unchanged.
void bind_source_dll_shutdown(SourceDllShutdownProfile&, misc::UnloadRuntime&);
void __stdcall source_dll_unload_timer_callback(void*, std::uint32_t, std::uintptr_t,
                                                std::uint32_t);
void __stdcall source_dll_deferred_request_timer_callback(void*, std::uint32_t, std::uintptr_t,
                                                          std::uint32_t);
}
