#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
using ImageNtHeader = const std::byte*(__stdcall*)(const std::byte* module);
using InterfaceConstructor = void*(__cdecl*)();
#else
using ImageNtHeader = const std::byte* (*)(const std::byte* module);
using InterfaceConstructor = void* (*)();
#endif
struct NativeImageEnvironment {
    ImageNtHeader image_nt_header;
};
NativeImageEnvironment native_image_environment();

// Complete4334AC0B. PE32 only; hash0 selects the ordinal argument. Forwarded
// exports are rejected as in the native function instead of being followed.
std::byte* resolve_pe_export(std::byte* module, std::uint32_t hash, std::uint32_t ordinal,
                             bool salted_hash, NativeImageEnvironment environment);
std::byte* resolve_pe_export(std::byte* module, std::uint32_t hash, std::uint32_t ordinal,
                             bool salted_hash);

struct InterfaceRegistration {
    InterfaceConstructor create;
    const char* name;
    InterfaceRegistration* next;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(InterfaceRegistration) == 0xc);
#endif
// Complete43328E9C. Resolves CreateInterface by its native hash and recovers
// the actual InterfaceReg list from either observed x86 compiler prologue.
InterfaceRegistration* find_interface_registry(std::byte* module,
                                               NativeImageEnvironment environment);
InterfaceRegistration* find_interface_registry(std::byte* module);

// Native registry loop43329359..43329374, also used byLua create_interface.
// This exact-name hash lookup never probes versions or alternate modules.
InterfaceRegistration* find_interface_registration(InterfaceRegistration* first,
                                                   std::uint32_t hash) noexcept;
} // namespace skeet::engine
