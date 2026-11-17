#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::engine {
struct LoaderList {
    LoaderList* next;
    LoaderList* previous;
};
struct LoaderUnicodeName {
    std::uint16_t length;
    std::uint16_t maximum_length;
    const char16_t* buffer;
};
// Exact native LDR_DATA_TABLE_ENTRY prefix accessed by433F29A5.
struct LoaderModule {
    LoaderList load_order;
    LoaderList memory_order;
    LoaderList initialization_order;
    void* base;
    void* entry;
    std::uint32_t image_size;
    LoaderUnicodeName full_name;
    LoaderUnicodeName base_name;
};
struct LoaderData {
    std::uint32_t length;
    std::uint8_t initialized;
    std::uint8_t reserved[3];
    void* ss_handle;
    LoaderList modules;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(LoaderData, modules) == 0xc);
static_assert(offsetof(LoaderModule, base) == 0x18);
static_assert(offsetof(LoaderModule, base_name) == 0x2c);
static_assert(offsetof(LoaderModule, base_name) + offsetof(LoaderUnicodeName, buffer) == 0x30);
#endif

#if defined(_M_IX86) || defined(__i386__)
using PebLockFunction = void(__stdcall*)();
#else
using PebLockFunction = void (*)();
#endif
struct LoadedModuleEnvironment {
    LoaderData* loader;
    PebLockFunction acquire_peb_lock;
    PebLockFunction release_peb_lock;
};

// Complete43381FE9: UTF16 code units, ASCII A..Z folding, FNV1a andsalt.
std::uint32_t loaded_module_name_hash(const char16_t* name, std::int32_t length) noexcept;
// Complete433F29A5. The boolean selects salted or raw FNV input; loader-lock
// imports are called only when requested and loader is nonnull.
void* find_loaded_module(const LoadedModuleEnvironment& environment, std::uint32_t hash,
                         bool salted_hash, bool lock_loader);
LoadedModuleEnvironment native_loaded_module_environment();
void* find_loaded_module(std::uint32_t hash, bool salted_hash, bool lock_loader);
} // namespace skeet::engine
