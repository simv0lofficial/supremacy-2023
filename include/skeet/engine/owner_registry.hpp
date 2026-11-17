#pragma once
#include "jobs.hpp"
#include <span>

namespace skeet::engine {
// The three native pointer globals43475B78/7C/80. This vector borrows the
// registered owners; its allocation contains only their pointers.
struct OwnerRegistry {
    void** begin = nullptr;
    void** end = nullptr;
    void** limit = nullptr;
    std::span<void* const> entries() const noexcept {
        return {begin, begin == end ? 0u : static_cast<std::size_t>(end - begin)};
    }
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(OwnerRegistry) == 0xc);
#endif

// Complete433F1665 and its4334DC99 growth path. The input pointer can alias
// existing vector storage; its value is read after allocation/copy, beforefree.
void append_owner(OwnerRegistry& registry, void* const* value, JobMemory memory);
// Complete433676A0. Constructors invoke this before their vtables/configuration
// are initialized. Registration order follows those actual constructor calls.
void* register_owner(OwnerRegistry& registry, void* object, JobMemory memory);
// Standalone native dispatchers; each captures its own begin/end once.
void dispatch_owner_slot_six(OwnerRegistry& registry); // 43328BEA
void dispatch_owner_slot_eight(OwnerRegistry& registry); // 433DF17B

struct OwnerRegistryBinding {
    OwnerRegistry& registry;
    JobMemory memory;
    void* append(void* object) const {
        return register_owner(registry, object, memory);
    }
    std::span<void* const> entries() const noexcept {
        return registry.entries();
    }
};
}
