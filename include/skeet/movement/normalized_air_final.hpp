#pragma once
#include "air_strafe.hpp"
#include "command_state.hpp"
#include "simulation_environment.hpp"
#include "simulation_capture.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/rage/ballistics.hpp"

namespace skeet {

struct NormalizedAirFinalSettings {
    const std::int32_t& smoothing; // movement owner +0x68
    const float& maximum_angle; // +0x1C8
    const float& forward_magnitude; // +0x1C4
    const float& acceleration_limit; // +0x1D0
    const float& angle_step; // +0x1D4
};

// Settings retain the owner's cells across speed getters, constructors and
// both forecasts. A getter can change later knob reads without changing the
// earlier saved smoothing/difference values in each individual trial.

struct NormalizedAirFinalInput {
    const MovementVector& position; // live GetAbsOrigin pointer / simulation position
    MovementVector velocity; // prepared velocity at ESP+0x114
    float stamina; // prepared ESP+0x14, separate from snapshot
    float velocity_yaw; // original heading at ESP+0x10C
    float speed; // prepared horizontal speed at ESP+0x13C
    AirStrafeSelection selection;
};

struct NormalizedAirFinalBindings {
    void* player;
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& capture;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* const& surface_properties;
    const void* const& air_acceleration; // actual movement owner +0x14 cvar cell
};

// Complete normalized-basis final solver 43376EFE..43378DF9. The pre-speed
// stage has already selected movement_angles.yaw. This stage records feedback,
// compares both full movement forecasts when required, and writes forward move.
// It does not execute the shared command tail at 4337C899.
void finish_normalized_air_strafe(const NormalizedAirFinalSettings&, const NormalizedAirFinalInput&,
                                  const MovementFeedbackHistory&, MovementFeedback&,
                                  client::Command&, MovementAngleView,
                                  const NormalizedAirFinalBindings&);

} // namespace skeet
