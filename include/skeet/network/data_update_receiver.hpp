#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::network {
struct NativeDataUpdateReceiver;
#if defined(_M_IX86) || defined(__i386__)
using NativeDataUpdateEntry = void(__fastcall*)(NativeDataUpdateReceiver*, void*, std::int32_t);
#else
using NativeDataUpdateEntry = void (*)(NativeDataUpdateReceiver*, void*, std::int32_t);
#endif

// Physical secondary interface: ECX is this endpoint, EDX is the player and
// the update word is the sole stack argument. Both slots therefore return4.
struct NativeDataUpdateVtable {
    NativeDataUpdateEntry capture;
    NativeDataUpdateEntry finish;
};
struct NativeDataUpdateReceiver {
    const NativeDataUpdateVtable* vtable = nullptr;
    void* source = nullptr; // source bridge backreference; native dispatch reads only vtable
};
static_assert(offsetof(NativeDataUpdateReceiver, vtable) == 0);
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeDataUpdateVtable) == 8);
static_assert(offsetof(NativeDataUpdateVtable, finish) == 4);
static_assert(sizeof(NativeDataUpdateReceiver) == 8);
static_assert(offsetof(NativeDataUpdateReceiver, source) == 4);
#endif
} // namespace skeet::network
