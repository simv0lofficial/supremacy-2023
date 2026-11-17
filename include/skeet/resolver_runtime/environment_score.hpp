#pragma once
#include "skeet/lagcomp/records.hpp"
#include "skeet/rage_geometry.hpp"
#include "skeet/rage_hitboxes.hpp"
#include <cmath>

namespace skeet::resolver_runtime {
struct EnvironmentCapsule {
    std::uint32_t matrix_index;
    RageVector minimum;
    RageVector maximum;
    float radius;
};
struct EnvironmentSelection {
    float score;
    float pose_yaw;
    std::uint32_t attempt;
    bool valid() const noexcept {
        return std::isfinite(score);
    }
};
struct EnvironmentRebase {
    float yaw_delta;
    RageVector original_origin; // native selected record +30
    RageVector current_origin; // current engine entity origin
};
// Native 433492B0..43349D27 with matrices already expressed in the current
// entity space. The environment probe compares first and third hypotheses,
// takes the worse of two capsule projections, then selects the smaller score.
EnvironmentSelection score_environment_hypotheses(const lagcomp::LagRecord&,
                                                  const std::array<EnvironmentCapsule, 2>&,
                                                  const RageVector& direction,
                                                  const RageVector& origin,
                                                  bool strict = false) noexcept;
// The same complete solver including native 433492D1..433497DD matrix rebasing.
EnvironmentSelection score_environment_hypotheses(const lagcomp::LagRecord&,
                                                  const std::array<EnvironmentCapsule, 2>&,
                                                  const RageVector& direction,
                                                  const RageVector& origin,
                                                  const EnvironmentRebase&, const RageTrigonometry&,
                                                  bool strict = false);
} // namespace skeet::resolver_runtime
