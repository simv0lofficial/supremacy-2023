#pragma once
#include "rage_geometry.hpp"

namespace skeet {
struct RageSegment {
    RageVector first;
    RageVector second;
};
struct RageSegmentClosest {
    float first_fraction;
    float second_fraction;
    RageVector first_point;
};
// Complete 0x433B5355; returns the point and writes its clamped fraction.
RageVector project_rage_point_to_segment(const RageSegment&, const RageVector&,
                                         float& fraction) noexcept;
// Complete 0x433F4DAD with 0x433B55A8 and 0x433B5355 dependencies.
RageSegmentClosest closest_rage_segments(const RageSegment&, const RageSegment&) noexcept;
// Complete 0x4339CBE2; used by non-head safe-point validation.
float rage_segment_distance(const RageSegment&, const RageSegment&) noexcept;
// Complete 0x434012F7. The first parameter is a ray with the native normalized
// direction; its parameter is clamped at zero but has no upper bound.
RageSegmentClosest closest_rage_ray_segment(const RageRay&, const RageSegment&) noexcept;
} // namespace skeet
