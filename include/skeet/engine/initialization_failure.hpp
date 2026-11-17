#pragma once
#include <cstdint>

namespace skeet::engine {
struct InitializationFailureState {
    std::uintptr_t& image_base; // Native 0x43467E70.
    std::uint32_t& image_size; // Native 0x434679DC.
    std::int32_t& requested_mode; // Native 0x43475A80.
};

#if defined(_M_IX86) || defined(__i386__)
using TerminateInitializationThread = std::int32_t(__stdcall*)(void*, std::int32_t);
#else
using TerminateInitializationThread = std::int32_t (*)(void*, std::int32_t);
#endif

// Recovered native 0x4337F065, reached by protected startup failure paths.
// The imported call can fail and return; its status is propagated unchanged.
std::int32_t fail_initialization(InitializationFailureState state,
                                 TerminateInitializationThread terminate_thread,
                                 std::int32_t exit_code);
}
