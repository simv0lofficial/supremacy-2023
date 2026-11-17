#pragma once
#include "skeet/engine/loaded_modules.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/pe_exports.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet {
class SettingsPersistence;
}
namespace skeet::client {
class CommandPhaseRegistry;
}
namespace skeet::input {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_WINDOW_STDCALL __stdcall
#define SKEET_WINDOW_CDECL __cdecl
#else
#define SKEET_WINDOW_STDCALL
#define SKEET_WINDOW_CDECL
#endif

struct NativeWindowUnicodeString {
    std::uint16_t length = 0, maximum_length = 0;
    char16_t* buffer = nullptr;
};
struct NativeWindowObjectAttributes {
    std::uint32_t length;
    std::uintptr_t root_directory;
    NativeWindowUnicodeString* object_name;
    std::uint32_t attributes;
    void* security_descriptor;
    void* security_quality_of_service;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeWindowUnicodeString) == 8);
static_assert(sizeof(NativeWindowObjectAttributes) == 0x18);
#endif

// All callbacks are CRT/Windows/NT dependencies. Window enumeration and class
// lookup use the optional win32u exports resolved by the original initializer.
struct NativeWindowLifecycleImports {
    using Foreground = std::uintptr_t(SKEET_WINDOW_STDCALL*)();
    using ClassName = std::int32_t(SKEET_WINDOW_STDCALL*)(std::uintptr_t, std::uint32_t,
                                                          NativeWindowUnicodeString*);
    using InternalText = std::int32_t(SKEET_WINDOW_STDCALL*)(std::uintptr_t, char16_t*,
                                                             std::int32_t);
    using WindowList = std::int32_t(SKEET_WINDOW_STDCALL*)(std::uintptr_t, std::uintptr_t,
                                                           std::uint32_t, std::uint32_t,
                                                           std::uint32_t, std::uint32_t,
                                                           std::uintptr_t*, std::uint32_t*);
    using Crc32 = std::uint32_t(SKEET_WINDOW_STDCALL*)(std::uint32_t, const void*, std::uint32_t);
    using OpenMutexFunction = std::uintptr_t(SKEET_WINDOW_STDCALL*)(std::uint32_t, int,
                                                                    const char16_t*);
    using Close = std::int32_t(SKEET_WINDOW_STDCALL*)(std::uintptr_t);
    using Delete = std::int32_t(SKEET_WINDOW_STDCALL*)(const NativeWindowObjectAttributes*);
    using Time = std::int64_t(SKEET_WINDOW_CDECL*)(std::int64_t*);
    using DefaultProcedure = std::intptr_t(SKEET_WINDOW_STDCALL*)(std::uintptr_t, std::uint32_t,
                                                                  std::uintptr_t, std::intptr_t);
    using ModuleFileName = std::uint32_t(SKEET_WINDOW_STDCALL*)(std::uintptr_t, char*,
                                                                std::uint32_t);
    using TempPath = std::uint32_t(SKEET_WINDOW_STDCALL*)(std::uint32_t, char16_t*);
    using DosToNtPath = unsigned char(SKEET_WINDOW_STDCALL*)(const char16_t*,
                                                             NativeWindowUnicodeString*,
                                                             const char16_t**, void*);
    using FreeUnicode = void(SKEET_WINDOW_STDCALL*)(NativeWindowUnicodeString*);
    using Format = int(SKEET_WINDOW_CDECL*)(char*, std::size_t, const char*, ...);
    using AppendWide = int(SKEET_WINDOW_CDECL*)(char16_t*, std::size_t, const char16_t*);

    Foreground get_foreground_window = nullptr;
    OpenMutexFunction open_mutex = nullptr;
    Close close = nullptr;
    Delete delete_file = nullptr;
    Time time = nullptr;
    DefaultProcedure default_window_procedure = nullptr;
    ModuleFileName module_file_name = nullptr;
    TempPath temp_path = nullptr;
    DosToNtPath dos_to_nt_path = nullptr;
    FreeUnicode free_unicode = nullptr;
    Format format = nullptr;
    AppendWide append_wide = nullptr;
    std::uint32_t (*current_process_id)() = nullptr; // TEB.ClientId.UniqueProcess
};
NativeWindowLifecycleImports native_window_lifecycle_imports();

// This owner detects native window signatures and records the reason through
// SettingsPersistence. OpenMutexW's 0x20000 is READ_CONTROL, not a hook size.
class NativeWindowLifecycle {
public:
    NativeWindowLifecycle(SettingsPersistence&, NativeWindowLifecycleImports,
                          engine::LoadedModuleEnvironment, engine::NativeImageEnvironment);
    ~NativeWindowLifecycle();
    NativeWindowLifecycle(const NativeWindowLifecycle&) = delete;
    NativeWindowLifecycle& operator=(const NativeWindowLifecycle&) = delete;

    void initialize(); // complete4348FD50, including initial csgo.DMP deletion
    void poll(); // complete433E291F, lifecycle owner slot6
    void poll_foreground_changed(); // complete4339879F, update observer phase0

    NativeWindowLifecycleImports::ClassName class_name = nullptr; // native+18
    NativeWindowLifecycleImports::Foreground foreground = nullptr; // native+1C
    NativeWindowLifecycleImports::InternalText internal_text = nullptr; // native+20
    NativeWindowLifecycleImports::WindowList window_list = nullptr; // native+24
    NativeWindowLifecycleImports::Crc32 crc32 = nullptr; // native+28, retained initializer output
    std::uintptr_t last_foreground = 0; // native+2C
    NativeWindowUnicodeString dump_path; // native+40, owned RTL allocation
    bool deletion_pending = false; // native+48
    std::int64_t detection_time = 0; // native+50
    std::array<std::uint32_t, 2> pending_signature{0xCA172992, 0x456383A9}; //+58
    std::array<std::uint32_t, 8> class_signatures{}; //+60
    std::array<std::uint32_t, 12> title_signatures{}; //+80
    std::array<std::array<std::uint32_t, 2>, 2> paired_signatures{}; //+B0
private:
    SettingsPersistence& persistence_;
    NativeWindowLifecycleImports imports_;
    engine::LoadedModuleEnvironment modules_;
    engine::NativeImageEnvironment images_;
    std::uint32_t scan();
    std::int32_t delete_dump();
};

// Constructor4347F555 registers the lifecycle owner, the return-only command
// endpoint4346EED8, and update observer43472808 in their actual caller order.
// Storage must use the native aligned allocation contract when owner teardown
// is dispatched; the lifecycle's destructor frees its RTL path before storage.
std::size_t native_window_lifecycle_owner_size() noexcept;
NativeWindowLifecycle&
construct_native_window_lifecycle(void* storage, engine::OwnerRegistryBinding owners,
                                  client::CommandPhaseRegistry& command_owners,
                                  engine::OwnerRegistryBinding updates, SettingsPersistence&,
                                  NativeWindowLifecycleImports, engine::LoadedModuleEnvironment,
                                  engine::NativeImageEnvironment);

#undef SKEET_WINDOW_STDCALL
#undef SKEET_WINDOW_CDECL
} // namespace skeet::input
