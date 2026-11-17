#pragma once
#include "skeet/engine/jobs.hpp"
#include <cstring>

namespace skeet::client::entity_command_detail {
inline void* allocate(engine::JobMemory memory, std::size_t size) {
    if (!size)
        return nullptr;
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    const auto total = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}
inline void free(engine::JobMemory memory, void* allocation) {
    if (!allocation)
        return;
    auto* bytes = static_cast<std::byte*>(allocation);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
} // namespace skeet::client::entity_command_detail
