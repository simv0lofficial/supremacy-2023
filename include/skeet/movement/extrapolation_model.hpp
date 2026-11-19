#pragma once
#include "simulation.hpp"
#include "skeet/lagcomp/history.hpp"
#include <optional>

namespace skeet {

inline constexpr std::int32_t maximum_pose_extrapolation_ticks = 4;

struct ExtrapolatedMotion {
    MovementVector velocity_step;
    bool coast;
    bool braking = false;
};

std::optional<ExtrapolatedMotion> fit_extrapolated_motion(
    const lagcomp::RecordRing&, const MovementSimulationRules&, std::int32_t ticks) noexcept;

// Infer a command whose velocity change agrees with the recorded motion.
// Collision resolution takes place in the full movement tick afterwards.
bool extrapolated_motion_command(const MovementSimulation&, const ExtrapolatedMotion&,
                                const MovementPhysics&, UserCommand&, MovementAngles&) noexcept;

} // namespace skeet
