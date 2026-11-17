#pragma once
#include "skeet/engine/owner_registry.hpp"

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
using FrameUpdateEntry = void(__fastcall*)(void*, std::uint8_t);
#else
using FrameUpdateEntry = void (*)(void*, std::uint8_t);
#endif
// Actual one-entry interface of registry43475AD8/ADC/AE0. Unlike the HUD
// registry it receives only the forwarded byte in DL, with no stack arguments.
struct FrameUpdateEndpoint {
    const FrameUpdateEntry* vtable = nullptr;
};
FrameUpdateEndpoint* register_frame_update(OwnerRegistryBinding, FrameUpdateEndpoint*); //433FCB12
// Complete4334820E..4334824B, including the wrapping counter4346DB2C update.
void dispatch_frame_update(const OwnerRegistry&, std::int32_t& counter, std::uint8_t update);
struct FrameUpdateRuntime {
    const OwnerRegistry& callbacks; //canonical43475AD8/ADC/AE0
    std::int32_t& counter; //canonical4346DB2C
};
void bind_frame_update_runtime(FrameUpdateRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __stdcall frame_update_hook(std::uint8_t update);
#else
void frame_update_hook(std::uint8_t update);
#endif
} //namespace skeet::engine
