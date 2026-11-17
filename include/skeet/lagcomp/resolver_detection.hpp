#pragma once
#include "timeline.hpp"
#include "skeet/resolver_runtime/attempts.hpp"

namespace skeet::lagcomp {
struct ResolverDetectionState {
    bool enabled; // per-player table+6
    float yaw_duration_1c0;
    float pitch_duration_178;
    std::uint16_t field_a6;
    resolver_runtime::SelectionState selection;
};
struct ResolverDetectionInput {
    std::int32_t simulation_ticks;
    float interval;
    float previous_yaw_118;
    float observed_yaw_1ac;
    float pitch;
    const LagTimeline& packets;
};
// Complete4334D33C. Only the most recent packet is inspected.
bool newest_packet_timing_discontinuity(const LagTimeline&) noexcept;
// Complete4340D65E..4340D89F. Sustained yaw change>40 or pitch>70 enables
// resolver handling after1.2 seconds. The native packet condition is a separate
// immediate gate; it does not reset timers or publish a new attempt.
bool update_resolver_detection(ResolverDetectionState&, const ResolverDetectionInput&) noexcept;
} // namespace skeet::lagcomp
