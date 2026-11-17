#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::client {
#if defined(_M_IX86) || defined(__i386__)
using EntityStatusEntry = void(__fastcall*)(void*, const void*);
using EntityUpdateEntry = void(__fastcall*)(void*, void*, std::uintptr_t);
using EntityResetEntry = void(__fastcall*)(void*, std::int32_t);
using EntityIncomingEntry = void(__fastcall*)(void*, void*, std::int32_t);
#else
using EntityStatusEntry = void (*)(void*, const void*);
using EntityUpdateEntry = void (*)(void*, void*, std::uintptr_t);
using EntityResetEntry = void (*)(void*, std::int32_t);
using EntityIncomingEntry = void (*)(void*, void*, std::int32_t);
#endif
// ECX is always the physical secondary receiver. Slots0/2 consume EDX only
// and return0; slots1/3 also consume one stack word and return4.
struct NativeEntityObserverVtable {
    EntityStatusEntry status;
    EntityUpdateEntry update;
    EntityResetEntry reset;
    EntityIncomingEntry incoming;
};
struct NativeEntityObserver {
    const NativeEntityObserverVtable* vtable = nullptr;
};
// This metadata belongs inside the actual whole owner. Its source points to
// that owner's existing state/view; the native buffer stores only the prefix.
struct NativeEntityObserverReceiver : NativeEntityObserver {
    void* source = nullptr;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeEntityObserverVtable) == 0x10);
static_assert(offsetof(NativeEntityObserverVtable, update) == 4);
static_assert(offsetof(NativeEntityObserverVtable, reset) == 8);
static_assert(offsetof(NativeEntityObserverVtable, incoming) == 0xC);
static_assert(sizeof(NativeEntityObserver) == 4);
static_assert(sizeof(NativeEntityObserverReceiver) == 8);
#endif
} // namespace skeet::client
