#pragma once
#include "command_state.hpp"
#include "simulation_capture.hpp"
#include "simulation_environment.hpp"
#include "simulation_tick.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/rage/ballistics.hpp"

namespace skeet {

struct AirWallSteeringInput {
    bool enabled; // movement owner +0x158
    std::int32_t prediction_milliseconds; // movement owner +0x16C
    float angle_scale, maximum_angle, side_magnitude; // +0x1C0/+0x1C8/+0x1C4
    std::uint32_t player;
    MovementVector position, velocity;
    std::uint32_t player_flags;
    std::int32_t weapon_handle;
};

struct AirWallSteeringBindings {
    const MovementSimulationPlayer& snapshot;
    const MovementSimulationRules& rules;
    const MovementSimulationEnvironment& environment;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* entity_list; // actual entity-list virtual +0x10
    void* surface_properties; // actual surface-properties virtual +0x14
};

struct AirWallSteeringLiveTuning {
    const std::int32_t& prediction_milliseconds; // owner+16C, after first hull getters
    const std::int32_t& smoothing; // owner+68, each normalized trial
    const float& angle_scale; // owner+1C0, after constructor
    const float& maximum_angle; // owner+1C8
    const float& side_magnitude; // owner+1C4, each prediction tick
    const float& acceleration_limit; // owner+1D0, after speed getter
    const float& angle_step; // owner+1D4
};

struct AirWallSteeringLiveBindings {
    void* player;
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& capture;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* const& surface_properties; // reload current43469268 after the first trace
    const MovementVector&
        origin; // actual earlier GetAbsOrigin result / secondary simulation position
    const MovementVector& velocity; // raw retained velocity pointer / normalized prepared local
    AirWallSteeringLiveTuning tuning;
};

// Complete raw-basis wall-steering block 0x43379C59..0x4337A829. The caller
// has selected the airborne raw-basis path and projected its input heading.
// False is the 0x4337A829 bypass; true reaches the completed 0x4337A816 edge.
// Only movement_angles.yaw is written. Prediction uses the concrete movement
// simulation; EngineTrace, surface data and entity access remain real boundaries.
bool steer_air_wall(const AirWallSteeringInput&, const MovementFeedback&, const client::Command&,
                    MovementAngleView movement_angles, const AirWallSteeringBindings&);

// Actual command path. The native pre-constructor hull getters, constructor
// captures, per-tick reads, weapon lookup and final mask getter execute live.
// Input configuration values, player/position/player_flags/weapon_handle belong
// only to the typed API above. The live path uses input.enabled and the cached
// velocity seed; constructor velocity and late tuning come from live bindings.
bool steer_live_air_wall(const AirWallSteeringInput&, const MovementFeedback&,
                         const client::Command&, MovementAngleView movement_angles,
                         const AirWallSteeringLiveBindings&);

} // namespace skeet
