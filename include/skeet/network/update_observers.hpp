#pragma once
#include "skeet/engine/owner_registry.hpp"
#include <array>

namespace skeet::network {
#if defined(_M_IX86) || defined(__i386__)
using UpdatePhaseEntry = void(__thiscall*)(void*);
using PlayerViewEntry = void(__fastcall*)(void*, void*, std::uintptr_t, std::uintptr_t,
                                          std::uintptr_t);
using ViewEntry = void(__fastcall*)(void*, float*, float*, float*);
using PostViewEntry = void(__fastcall*)(void*, void*);
#else
using UpdatePhaseEntry = void (*)(void*);
using PlayerViewEntry = void (*)(void*, void*, std::uintptr_t, std::uintptr_t, std::uintptr_t);
using ViewEntry = void (*)(void*, float*, float*, float*);
using PostViewEntry = void (*)(void*, void*);
#endif

// Secondary interface registered by433B8C1C in43475988/8C/90. The first
// five entries take only ECX. Slot5 forwards player in EDX and three stack
// words; slot6 forwards origin in EDX, then angles and FOV on the stack.
struct UpdateObserverVtable {
    std::array<UpdatePhaseEntry, 5> phases;
    PlayerViewEntry player_view;
    ViewEntry view;
    PostViewEntry post_view;
};
struct UpdateObserver {
    const UpdateObserverVtable* vtable = nullptr;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(UpdateObserverVtable) == 0x20);
static_assert(offsetof(UpdateObserverVtable, player_view) == 0x14);
static_assert(offsetof(UpdateObserverVtable, view) == 0x18);
static_assert(offsetof(UpdateObserverVtable, post_view) == 0x1C);
#endif

// Calls the same recovered pointer-vector append as the lifecycle registry.
// Constructors supply their actual secondary endpoint; insertion order is
// constructor order and no order between independent constructors is invented.
void* register_update_observer(engine::OwnerRegistryBinding, UpdateObserver&);

// Complete native parameterless dispatchers. Each captures begin/end before
// invoking any observer. The stage word is native43476904.
void dispatch_update_phase0(const engine::OwnerRegistry&, std::int32_t& stage); //4338261F, stage5
void dispatch_update_phase1(const engine::OwnerRegistry&, std::int32_t& stage); //4339D542, stage6
void dispatch_update_phase2(const engine::OwnerRegistry&); //4338225D
void dispatch_update_phase3(const engine::OwnerRegistry&); //4341B45A
void dispatch_update_phase4(const engine::OwnerRegistry&); //433B7744
} // namespace skeet::network
