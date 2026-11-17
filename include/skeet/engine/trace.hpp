#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::engine {

// Engine Ray_t passed to TraceRay slot5. Each VectorAligned occupies16 bytes;
// the transform and flags follow at40/44 in the native x86 interface.
struct alignas(16) TraceRay {
    std::array<float, 4> start{}, delta{}, start_offset{}, extents{};
    const void* world_axis_transform = nullptr;
    std::uint8_t is_ray = 0, swept = 0;
};

// Common4346EDC0 engine-filter prefix. Its four client entries only jump into
// the current genuine table at434758B4. Source adapters keep that table live.
struct TraceSkipFilterPrefix {
    void* const* vtable;
    void* skip;
    std::int32_t collision_group;
    std::uint32_t extra;
};

inline constexpr std::size_t trace_result_size = 0x54;
inline constexpr std::size_t trace_fraction_offset = 0x2c;
inline constexpr std::size_t trace_entity_offset = 0x4c;
static_assert(offsetof(TraceRay, delta) == 0x10);
static_assert(offsetof(TraceRay, start_offset) == 0x20);
static_assert(offsetof(TraceRay, extents) == 0x30);
static_assert(offsetof(TraceRay, world_axis_transform) == 0x40);
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(TraceRay, is_ray) == 0x44);
static_assert(offsetof(TraceRay, swept) == 0x45);
static_assert(sizeof(TraceRay) == 0x50);
static_assert(offsetof(TraceSkipFilterPrefix, skip) == 4);
static_assert(offsetof(TraceSkipFilterPrefix, collision_group) == 8);
static_assert(offsetof(TraceSkipFilterPrefix, extra) == 0xc);
static_assert(sizeof(TraceSkipFilterPrefix) == 0x10);
#endif

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_TRACE_MEMBER __thiscall
#else
#define SKEET_TRACE_MEMBER
#endif
namespace trace_detail {
inline bool SKEET_TRACE_MEMBER reject_entity(void*, void*, std::uint32_t) noexcept {
    return false; //43382057, RET8
}
inline std::int32_t SKEET_TRACE_MEMBER everything(void*) noexcept {
    return 0; //43336942
}
inline std::int32_t SKEET_TRACE_MEMBER world_only(void*) noexcept {
    return 1; //433686FC
}
inline void* const* reject_table() noexcept {
    static void* const table[]{reinterpret_cast<void*>(reject_entity),
                               reinterpret_cast<void*>(everything)};
    return table;
}
inline void* const* world_table() noexcept {
    static void* const table[]{reinterpret_cast<void*>(reject_entity),
                               reinterpret_cast<void*>(world_only)};
    return table;
}
} //namespace trace_detail
#undef SKEET_TRACE_MEMBER

// These native filters share ShouldHitEntity=false but have different trace
// types. They remain separate because GetTraceType changes engine traversal.
struct TraceRejectEntitiesFilter {
    void* const* vtable = trace_detail::reject_table(); //434727C0
};
struct TraceWorldOnlyFilter {
    void* const* vtable = trace_detail::world_table(); //434701D0
};

} //namespace skeet::engine
