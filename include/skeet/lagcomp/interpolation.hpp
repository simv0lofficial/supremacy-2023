#pragma once
#include "history.hpp"

namespace skeet::lagcomp {
using Quaternion = std::array<float, 4>;
struct RotationMath {
    // The actual SSE CRT imports are genuine dependencies. atan2 uses double
    // inputs/result; the other three imports use scalar float XMM arguments.
    float (*sine)(float);
    float (*cosine)(float);
    float (*arc_cosine)(float);
    double (*arc_tangent2)(double, double);
};
Quaternion history_angle_quaternion(const Vector&, const RotationMath&); //4334DF4D
Quaternion align_history_quaternion(const Quaternion&, const Quaternion&) noexcept; //4334A802
Quaternion slerp_history_quaternion(const Quaternion&, const Quaternion&, float,
                                    const RotationMath&); //4334199D
Vector history_quaternion_angles(const Quaternion&, const RotationMath&); //433945FB

// Complete field interpolation4336C968..4336CB3E, before the matrix branch.
// The following native matrix branch reads uninitialized stackA0/A4/A8/B0/B4/B8;
// it is not assigned replacement values by this field interpolation stage.
float interpolate_history_fields(LagRecord& destination, const LagRecord& newer,
                                 const LagRecord& older, float requested_time, float interval,
                                 const RotationMath&, Vector* interpolated_rotation = nullptr);
// Full interpolation eligibility4336C8AE..4336C968. Returns the older source
// record or nullptr. requested_tick is the original caller's tick.
const LagRecord* history_interpolation_source(const RecordRing&, std::uint32_t selected_ordinal,
                                              const LagRecord& selected,
                                              std::int32_t requested_tick,
                                              std::int32_t planned_ticks,
                                              const HistoryTickWindow&) noexcept;
} // namespace skeet::lagcomp
