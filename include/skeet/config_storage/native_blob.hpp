#pragma once
#include "skeet/engine/jobs.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace skeet {
// The tag12 value's native three-pointer string. Capacity includes its trailing
// zero; the empty representation aliases the shared434759AC byte, not a heap.
// The descriptor's ordinary CRT callback constructs the pointer triple, and its
// registered terminal callback releases it. This view has no automatic release.
struct NativeBlobCells {
    std::uint32_t first, last, limit;
};
static_assert(sizeof(NativeBlobCells) == 12);
class NativeBlobBytes {
public:
    NativeBlobBytes(NativeBlobCells&, engine::JobMemory memory, std::byte& shared_empty);
    NativeBlobBytes(const NativeBlobBytes&) = delete;
    NativeBlobBytes& operator=(const NativeBlobBytes&) = delete;
    NativeBlobCells& cells; //live descriptor+10/+14/+18, never a copied triple

    void reset_empty() noexcept; //434822A0..434822AF; does not free an old buffer
    std::byte* data() noexcept;
    const std::byte* data() const noexcept;
    std::size_t size() const noexcept;
    bool empty() const noexcept {
        return cells.first == cells.last;
    }
    std::byte* begin() noexcept {
        return data();
    }
    std::byte* end() noexcept;
    const std::byte* begin() const noexcept {
        return data();
    }
    const std::byte* end() const noexcept;
    void assign(std::span<const std::byte>); //434015F3, including aliasing order
    void resize(std::size_t);
    void clear() noexcept;
    void terminal_release(); //4336B1E9; leaves first/last/limit unchanged
private:
    engine::JobMemory memory_;
    std::byte& shared_empty_;
    std::size_t capacity() const noexcept;
    std::byte* allocate(std::size_t);
    void free(std::byte*);
    void append(std::span<const std::byte>); //433F4815
};
}
