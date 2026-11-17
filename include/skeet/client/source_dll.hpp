#pragma once
#include <cstdint>

namespace skeet::client {
enum class SourceDllStatus : std::uint32_t {
    initializing = 1,
    ready = 2,
    initialization_failed = 3,
    cleanup_failed = 4,
    stopping = 5,
    stopped = 6
};
}

#if defined(SKEET_SOURCE_DLL_BUILD)
#define SKEET_SOURCE_DLL_API __declspec(dllexport)
#else
#define SKEET_SOURCE_DLL_API __declspec(dllimport)
#endif

// Both functions are read-only. The DLL starts its source application on a
// worker after normal Windows loading; unloading belongs to its menu runtime.
extern "C" SKEET_SOURCE_DLL_API std::uint32_t __cdecl SkeetGetStatus() noexcept;
// Returns the required byte count including the terminator. A nonzero
// capacity requires a valid buffer; a short buffer receives a terminated copy.
extern "C" SKEET_SOURCE_DLL_API std::uint32_t __cdecl
SkeetCopyStartupError(char* buffer, std::uint32_t capacity) noexcept;

#undef SKEET_SOURCE_DLL_API
