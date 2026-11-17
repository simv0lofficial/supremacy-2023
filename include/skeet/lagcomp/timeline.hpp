#pragma once
#include "records.hpp"

namespace skeet::lagcomp {
struct PacketTimingSample {
    std::int32_t field_00;
    std::array<std::uint32_t, 5> fields_04;
    std::int32_t field_18;
    std::uint32_t field_1c;
};
struct LagTimeline {
    float simulation_time; // player history +0x110
    std::int32_t last_update_tick; // +0x168
    std::uint32_t packet_head; // +0x234
    std::uint32_t packet_count; // +0x238
    std::array<PacketTimingSample, 6> packets;
    std::uint32_t record_head; // +0x5D4
    std::uint32_t record_count; // +0x5D8
    std::array<const LagRecord*, 32> records;
    const LagRecord& at(std::uint32_t age) const noexcept {
        return *records[(record_head + age) & 31u];
    }
};
struct MissingTickTiming {
    std::int32_t tick_d4;
    std::int32_t tick_18c;
    float interval;
    float receive_interval; // globals +0x20, used for engine[14] time conversion
    // Complete 433A13ED result. Nonpositive configured values are retained.
    std::int32_t prediction_limit;
    void* engine; // engine vtable[12] max clients, [14] time
    void* player; // networkable(+8) vtable[10] index
};
// Complete 433FC6C6 and 433F94AA. A ring contains at most 32 nonnull records;
// packet_count is at most 6. The original callers maintain these invariants.
// The forward extrapolation pattern requires a positive prediction_limit.
bool packet_timing_discontinuity(const LagTimeline&) noexcept;
std::int32_t estimate_missing_ticks(const LagTimeline&, const MissingTickTiming&);
// Complete 433A13ED, after resolving its three entity-property offsets.
std::int32_t prediction_tick_limit(bool local_available, bool field_bf567619, bool field_dda99ab5,
                                   std::int32_t field_b7ea8fda, std::int32_t configured) noexcept;
} // namespace skeet::lagcomp
