#pragma once
#include "skeet/config_storage/owner.hpp"
#include "skeet/engine/jobs.hpp"

namespace skeet::ui {
struct StartupRegistryObjectAttributes {
    std::uint32_t length;
    void* root;
    NativeConfigPath* name;
    std::uint32_t attributes;
    void* security;
    void* quality;
};
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_REGISTRY_ABI __stdcall
#else
#define SKEET_REGISTRY_ABI
#endif
struct StartupRegistryImports {
    std::int32_t(SKEET_REGISTRY_ABI* current_user_path)(NativeConfigPath*);
    std::int32_t(SKEET_REGISTRY_ABI* append_string)(NativeConfigPath*, const NativeConfigPath*);
    void(SKEET_REGISTRY_ABI* free_string)(NativeConfigPath*);
    std::int32_t(SKEET_REGISTRY_ABI* append_text)(NativeConfigPath*, const char16_t*);
    std::int32_t(SKEET_REGISTRY_ABI* open_key)(void**, std::uint32_t,
                                               const StartupRegistryObjectAttributes*);
    void(SKEET_REGISTRY_ABI* initialize_string)(NativeConfigPath*, const char16_t*);
    std::int32_t(SKEET_REGISTRY_ABI* query_value)(void*, const NativeConfigPath*, std::uint32_t,
                                                  void*, std::uint32_t, std::uint32_t*);
    std::int32_t(SKEET_REGISTRY_ABI* close)(void*);
};
#undef SKEET_REGISTRY_ABI
StartupRegistryImports native_startup_registry_imports();

// Devirtualized ordinary43490C82: current-user path + Software, KEY_READ,
// class2 sizing query, aligned zeroed process-heap buffer, second query/free.
// Return the complete KEY_VALUE_PARTIAL_INFORMATION bytes, including its
// twelve-byte header. A missing value is an actual query result, never a
// fabricated empty settings stream. No registry writes or enumeration occur.
std::optional<ConfigBytes> read_startup_registry_value(const char16_t* terminated_name,
                                                       StartupRegistryImports, engine::JobMemory);

// Devirtualized4348DB32 unlinks only the ten settings descriptors and the
// separate hotkey link. Their constructed values and links remain in storage.
void unlink_startup_settings_records(SettingsPersistenceState&);
// Reciprocal of the proven433EEC8B record writer, consumed through ordinary
//43367569/433CFFB4. The present-value VM continuation434A6FCC is erased; this
// operation reconstructs its persisted-format behavior without executing it.
// Missing records preserve their constructed values and are reported false.
std::array<bool, 10>
restore_startup_settings_records(SettingsPersistenceState&,
                                 std::span<const std::byte> key_value_partial_information);
} //namespace skeet::ui
