#pragma once
#include "util/address.h"
#include "skeet/engine/game_system.hpp"
#include <cstring>

namespace skeet::engine::game_system_detail {
inline void* allocate(JobMemory memory, std::size_t size) {
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20); //43387448
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t{15};
    const auto displacement = static_cast<std::uint32_t>(header - address);
    const auto allocation_size = static_cast<std::uint32_t>(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
inline void release(void* object, JobMemory memory) {
    if (!object)
        return;
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement); //433837D0
}
template <class Owner, class Endpoint> Owner& owner_of(Endpoint* endpoint, std::size_t offset) {
    return *reinterpret_cast<Owner*>(reinterpret_cast<std::byte*>(endpoint) - offset);
}
using skeet::field;
} //namespace skeet::engine::game_system_detail
