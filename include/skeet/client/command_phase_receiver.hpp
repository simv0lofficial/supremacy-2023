#pragma once
#include "skeet/core.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::client {
struct Command;
struct CommandFrame;
struct NativeCommandPhaseReceiver;
struct CommandOwnerDispatch {
    std::uint32_t& scheduler_flags;
    InputBitmap input;
};
#if defined(_M_IX86) || defined(__i386__)
using CommandPhase0Entry = void(__fastcall*)(NativeCommandPhaseReceiver*, void*, Command*);
using CommandPhase1Entry = void(__fastcall*)(NativeCommandPhaseReceiver*, void*, void*, Command*,
                                             std::uint8_t*);
using CommandPhase2Entry = void(__fastcall*)(NativeCommandPhaseReceiver*, void*, void*, Command*,
                                             std::array<float, 3>*, std::uint8_t*);
using CommandFramePhaseEntry = void(__fastcall*)(NativeCommandPhaseReceiver*, CommandFrame*, void*,
                                                 Command*);
using CommandLimitEntry = std::int32_t(__fastcall*)(NativeCommandPhaseReceiver*, std::int32_t);
#else
using CommandPhase0Entry = void (*)(NativeCommandPhaseReceiver*, void*, Command*);
using CommandPhase1Entry = void (*)(NativeCommandPhaseReceiver*, void*, void*, Command*,
                                    std::uint8_t*);
using CommandPhase2Entry = void (*)(NativeCommandPhaseReceiver*, void*, void*, Command*,
                                    std::array<float, 3>*, std::uint8_t*);
using CommandFramePhaseEntry = void (*)(NativeCommandPhaseReceiver*, CommandFrame*, void*,
                                        Command*);
using CommandLimitEntry = std::int32_t (*)(NativeCommandPhaseReceiver*, std::int32_t);
#endif
//433D2E40 supplies ECX=secondaryreceiver throughout. Slots0/1/2 take the
//player in EDX and consume1/3/4 stack words. Slots3..8 take the frame in EDX
//and player,command on the stack. Slot9 chains EDX through the returned EAX.
struct NativeCommandPhaseVtable {
    CommandPhase0Entry phase0;
    CommandPhase1Entry phase1;
    CommandPhase2Entry phase2;
    CommandFramePhaseEntry phase3, phase4, phase5, phase6, phase7, phase8;
    CommandLimitEntry query_limit;
};
// This endpoint lives in its actual whole source owner. The native pointer
//buffer contains its address alone; the backreference borrows the existing
//command state/view and services borrow the one canonical dispatch binding.
struct NativeCommandPhaseReceiver {
    const NativeCommandPhaseVtable* vtable = nullptr;
    void* source = nullptr;
    const CommandOwnerDispatch* services = nullptr;
};
static_assert(offsetof(NativeCommandPhaseReceiver, vtable) == 0);
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeCommandPhaseVtable) == 0x28);
static_assert(offsetof(NativeCommandPhaseVtable, phase3) == 0xC);
static_assert(offsetof(NativeCommandPhaseVtable, query_limit) == 0x24);
static_assert(sizeof(NativeCommandPhaseReceiver) == 0xC);
#endif
} //namespace skeet::client
