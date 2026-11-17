#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet {
struct InputBitmap;
}
namespace skeet::client {
struct Command;
struct NativeCommandEmptyTickReceiver;
#if defined(_M_IX86) || defined(__i386__)
using NativeCommandEmptyTickEntry = void(__vectorcall*)(NativeCommandEmptyTickReceiver*,
                                                        void* player, float sample_time, Command*,
                                                        std::uint8_t* original_result);
#else
using NativeCommandEmptyTickEntry = void (*)(NativeCommandEmptyTickReceiver*, void* player,
                                             float sample_time, Command*,
                                             std::uint8_t* original_result);
#endif
//433D2F6C: ECX=receiver, EDX=player, XMM0=sample time, stack Command* then
//original-result byte*. The single entry returns8; result is not an EAX value.
struct NativeCommandEmptyTickVtable {
    NativeCommandEmptyTickEntry invoke;
};
struct NativeCommandEmptyTickReceiver {
    const NativeCommandEmptyTickVtable* vtable = nullptr;
    void* source = nullptr;
    const InputBitmap* input = nullptr; // genuine global-key boundary for grenade
};
static_assert(offsetof(NativeCommandEmptyTickReceiver, vtable) == 0);
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeCommandEmptyTickVtable) == 4);
static_assert(offsetof(NativeCommandEmptyTickReceiver, source) == 4);
static_assert(sizeof(NativeCommandEmptyTickReceiver) == 0xC);
#endif
} // namespace skeet::client
