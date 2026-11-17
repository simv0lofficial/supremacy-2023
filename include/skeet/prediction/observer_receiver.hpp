#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::client {
struct Command;
}
namespace skeet::prediction {
struct NativePredictionObserverReceiver;
struct PlayerCommandContext;
#if defined(_M_IX86) || defined(__i386__)
using NativePredictionCommandEntry = void(__fastcall*)(NativePredictionObserverReceiver*, void*,
                                                       const client::Command*);
using NativePredictionTickEntry = std::int32_t(__fastcall*)(NativePredictionObserverReceiver*,
                                                            std::int32_t, std::int32_t);
using NativePredictionRejectEntry = void(__fastcall*)(NativePredictionObserverReceiver*, void*,
                                                      const PlayerCommandContext*);
#else
using NativePredictionCommandEntry = void (*)(NativePredictionObserverReceiver*, void*,
                                              const client::Command*);
using NativePredictionTickEntry = std::int32_t (*)(NativePredictionObserverReceiver*, std::int32_t,
                                                   std::int32_t);
using NativePredictionRejectEntry = void (*)(NativePredictionObserverReceiver*, void*,
                                             const PlayerCommandContext*);
#endif
// Every slot consumes one stack word. Slots0/1/3 receive the live player in
// EDX; slot3 receives the context base rather than its Command at+4. Slot2
// receives command number in EDX and returns the next chained tick in EAX.
struct NativePredictionObserverVtable {
    NativePredictionCommandEntry capture;
    NativePredictionCommandEntry finish;
    NativePredictionTickEntry tick;
    NativePredictionRejectEntry rejected;
};
struct NativePredictionObserverReceiver {
    const NativePredictionObserverVtable* vtable = nullptr;
    void* source = nullptr; //source backreference; actual native array stores this endpoint
};
static_assert(offsetof(NativePredictionObserverReceiver, vtable) == 0);
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativePredictionObserverVtable) == 0x10);
static_assert(offsetof(NativePredictionObserverVtable, tick) == 8);
static_assert(offsetof(NativePredictionObserverVtable, rejected) == 0xC);
static_assert(sizeof(NativePredictionObserverReceiver) == 8);
#endif
} //namespace skeet::prediction
