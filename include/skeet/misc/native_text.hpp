#pragma once
#include "skeet/engine/jobs.hpp"

namespace skeet::misc {
// Native string-vector prefixes. The fourth plain word and the three bytes
// following an encrypted key are preserved by their actual constructors.
struct NativeText {
    char* begin;
    char* end;
    char* limit;
    std::uint32_t preserved;
};
struct NativeEncryptedText {
    char* bytes;
    std::uint8_t key;
    std::array<std::byte, 3> preserved;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeText) == 0x10);
static_assert(offsetof(NativeText, end) == 4 && offsetof(NativeText, limit) == 8);
static_assert(sizeof(NativeEncryptedText) == 8 && offsetof(NativeEncryptedText, key) == 4);
#endif

// Complete433B2213/433C1360 for these native byte-string objects. Empty text
// uses a shared one-byte sentinel and never owns a heap allocation.
void construct_native_text(NativeText&, const char* zero_terminated, engine::JobMemory);
void construct_native_text_range(NativeText&, const char* first, const char* last,
                                 engine::JobMemory);
void destroy_native_text(NativeText&, engine::JobMemory); //4336B1E9; fields unchanged

// Complete4334E935. Allocates length+1, stores a zero terminator,
// retries RDTSC until -(AL&7F) is nonzero, and encodes exactly length bytes.
void construct_native_encrypted_text(NativeEncryptedText&, const char*, std::uint32_t length,
                                     engine::JobMemory);
// Native owner destructors free the pointer and clear only that first word.
void destroy_native_encrypted_text(NativeEncryptedText&, engine::JobMemory);
// Inline accessors433A0D3D..5D/433E0EBA..DE copy until ENCRYPTED zero. The
// caller supplies its actual local buffer; backing bytes/key remain unchanged.
void decode_native_text(const NativeEncryptedText&, char* destination) noexcept;
} //namespace skeet::misc
