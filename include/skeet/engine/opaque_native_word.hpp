#pragma once
#include <bit>
#include <cstddef>
#include <cstdint>

namespace skeet::engine {
#if defined(_MSC_VER)
#define SKEET_OPAQUE_INLINE __forceinline
#else
#define SKEET_OPAQUE_INLINE [[gnu::always_inline]] inline
#endif
// Some recovered native branches observe their own still-unwritten stack
// slots. Read the actual invocation's bytes through a machine load, rather
// than evaluating an indeterminate C++ float/pointer or inventing initial
// values. These are ordinary current-process loads, never captured bytes.
SKEET_OPAQUE_INLINE std::uint32_t read_opaque_native_word(const void* address) noexcept {
    std::uint32_t value;
#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_IX86)
    __asm {
        mov eax, address
        mov eax, [eax]
        mov value, eax
    }
#elif defined(__i386__) || defined(__x86_64__) || defined(__clang__)
    __asm__ __volatile__("movl (%1),%0" : "=r"(value) : "r"(address) : "memory");
#else
#error Opaque native stack loads require the supported x86 instruction set.
#endif
    return value;
}
SKEET_OPAQUE_INLINE float read_opaque_native_float(const void* address) noexcept {
    return std::bit_cast<float>(read_opaque_native_word(address));
}
SKEET_OPAQUE_INLINE void* read_opaque_native_pointer(const void* address) noexcept {
    std::uintptr_t bits = read_opaque_native_word(address);
    if constexpr (sizeof(std::uintptr_t) == 8)
        bits |= std::uintptr_t(read_opaque_native_word(static_cast<const std::byte*>(address) + 4))
                << 32;
    return reinterpret_cast<void*>(bits);
}
#undef SKEET_OPAQUE_INLINE
} //namespace skeet::engine
