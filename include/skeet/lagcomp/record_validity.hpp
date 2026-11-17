#pragma once
#include "skeet/lagcomp/history.hpp"
#include <algorithm>
#include <cmath>

namespace skeet::lagcomp {
inline bool record_time_valid(const LagRecord& record, const HistoryTickTiming& input) noexcept {
    const auto timing = resolve_history_tick_timing(input);
    if (!(record.header.flags & 4u) || !(timing.interval > 0.0f) ||
        !std::isfinite(timing.interval) || !std::isfinite(timing.current_time) ||
        !std::isfinite(timing.latency_198) || !std::isfinite(timing.interpolation) ||
        timing.maximum_unlag_ms < 0)
        return false;
    for (float value : record.header.origin)
        if (!std::isfinite(value))
            return false;
    for (float value : record.header.velocity)
        if (!std::isfinite(value))
            return false;
    const double maximum = double(timing.maximum_unlag_ms) * 0.001;
    const double correction =
        std::clamp(double(timing.latency_198) + timing.interpolation, 0.0, maximum);
    const double sample = double(record.header.simulation_tick) * timing.interval;
    if (std::fabs((double(timing.current_time) - sample) - correction) > 0.2)
        return false;
    const auto clock_tick = std::int64_t(timing.tick_d4) + timing.tick_18c;
    const double clock = double(clock_tick) * timing.interval;
    // Keep fractional seconds. Truncating this clock to whole seconds admits
    // up to almost one extra second of history near a second boundary.
    return sample >= clock - maximum && sample <= clock + timing.interval;
}
inline bool record_precedes_discontinuity(const RecordRing& history, std::int32_t tick) noexcept {
    for (std::uint32_t age = 1; age < history.count; ++age) {
        const auto& newer = history.at(age - 1).header;
        const auto& older = history.at(age).header;
        if (tick >= newer.simulation_tick)
            break;
        float squared = 0.0f;
        for (unsigned axis = 0; axis < 3; ++axis) {
            const auto delta = newer.origin[axis] - older.origin[axis];
            squared += delta * delta;
        }
        if (!std::isfinite(squared) || squared > 4096.0f ||
            newer.simulation_tick <= older.simulation_tick)
            return true;
    }
    return false;
}
}
