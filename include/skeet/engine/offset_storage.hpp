#pragma once
#include "bindings.hpp"
#include "jobs.hpp"
#include <array>
#include <span>
#include <unordered_map>

namespace skeet::engine {
// Concrete source storage for the original dword globals. Keys are their
// recovered identities, not addresses to execute/dereference in the host.
struct NativeOffsetGlobals {
    std::unordered_map<std::uint32_t, std::uint32_t> keys;
    std::unordered_map<std::uint32_t, std::uint32_t*> pointers;
};
struct NativeOffsetStorage {
private:
    std::uint32_t* standalone_words_ = nullptr;

public:
    NativeOffsetStorage() noexcept : words(standalone_words_) {}
    explicit NativeOffsetStorage(std::uint32_t*& shared_words) noexcept : words(shared_words) {}
    NativeOffsetStorage(const NativeOffsetStorage&) = delete;
    NativeOffsetStorage& operator=(const NativeOffsetStorage&) = delete;
    NativeOffsetStorage(NativeOffsetStorage&&) = delete;
    NativeOffsetStorage& operator=(NativeOffsetStorage&&) = delete;
    // The source owner and terminal cleanup share the native434797B0 cell.
    // A view neither adopts nor resets an existing allocation.
    std::uint32_t*& words;
    std::uint32_t* cursor = nullptr;
    NativeOffsetGlobals globals;
    DiscoveredDataMaps maps;
    // Native43477B54,43477B58,43477B48,43476B44; actual proxy-field slots.
    std::array<RecvVarProxy*, 4> property_proxy_slots{};
};

// Complete4336F520 with its RDTSC low-dword supplied as a hardware input.
// Word0 is untouched; words1..1023 use the original32bit LCG.
void seed_native_offset_words(std::span<std::uint32_t, 0x400> words,
                              std::uint32_t timestamp_low) noexcept;
// Complete43400DD4: publish current key-word pointer, XOR the destination key,
// then advance the cursor by one dword.
void publish_native_offset(std::uint32_t& key, std::uint32_t*& pointer,
                           std::uint32_t*& cursor) noexcept;

// Complete native4348D41D..4348D9F0: replace native key allocation, seed it,
// bind receive/datamap offsets and actual proxy slots, derive29 additional
// offsets and publish their key-word pointers. The metadata accessor and
// already decoded globals remain real initialization inputs.
void initialize_native_offset_storage(NativeOffsetStorage& storage, JobMemory memory,
                                      void* base_client, std::byte* client_module,
                                      InitializationMetadata& metadata,
                                      std::uint32_t timestamp_low);
// Same complete ordinary allocation/LCG/proxy/derived-offset phase, using the
// typed fixed-build netvar manifest instead of the protected operand accessor.
void initialize_build_offset_storage(NativeOffsetStorage& storage, JobMemory memory,
                                     void* base_client, std::byte* client_module,
                                     std::uint32_t timestamp_low);
} // namespace skeet::engine
