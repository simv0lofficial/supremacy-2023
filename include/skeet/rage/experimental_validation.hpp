#pragma once
#include "skeet/rage/targeting.hpp"
#include "skeet/lagcomp/record_validity.hpp"
#include <cmath>

namespace skeet::rage {
// Validate the exact selected point and record again at the firing boundary.
inline bool experimental_target_valid(const TargetCandidate& target,
                                       const lagcomp::LagRecord& record,
                                       const lagcomp::RecordRing& history,
                                       const lagcomp::HistoryTickTiming& timing,
                                       const RageVector& origin, float spread, float inaccuracy,
                                       std::int32_t minimum_damage, float minimum_chance) noexcept {
    const auto& point = target.primary;
    if (!target.entity_index || target.entity_index > 64 || target.health <= 0 ||
        point.hitbox < 0 || point.hitbox >= 19 || point.hitgroup < 1 || point.hitgroup > 8 ||
        point.damage <= 0 || point.damage < minimum_damage ||
        !std::isfinite(point.chance) || point.chance < 0 || point.chance > 1 ||
        !std::isfinite(minimum_chance) || minimum_chance < 0 || minimum_chance > 1 ||
        !std::isfinite(spread) || spread < 0 || !std::isfinite(inaccuracy) || inaccuracy < 0 ||
        !lagcomp::record_time_valid(record, timing) || !lagcomp::record_geometry_valid(record) ||
        lagcomp::record_precedes_discontinuity(history, record.header.simulation_tick))
        return false;
    double distance_squared = 0;
    for (unsigned axis = 0; axis < 3; ++axis) {
        if (!std::isfinite(origin[axis]) || !std::isfinite(point.position[axis]))
            return false;
        const auto delta = double(point.position[axis]) - origin[axis];
        distance_squared += delta * delta;
    }
    return distance_squared > 0.0;
}
}
