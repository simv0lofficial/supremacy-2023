#pragma once
#include "grenade_prediction.hpp"
#include "skeet/hud/composite_runtime.hpp"

namespace skeet::esp {
// Native secondary owner+18, registered at43486787. The frame and scene are
// the engine's current decoded snapshots, including selected observer inputs.
struct GrenadePredictionHudHandler : hud::HudFrameCallback {
    GrenadePredictionHudHandler(engine::OwnerRegistryBinding, const GrenadeEspConfig&,
                                GrenadeTrajectoryOwner&, const GrenadeTrajectoryFrame&,
                                const GrenadePredictionScene&);
    GrenadePredictionHudHandler(const GrenadePredictionHudHandler&) = delete;
    GrenadePredictionHudHandler& operator=(const GrenadePredictionHudHandler&) = delete;
    const GrenadeEspConfig& config;
    GrenadeTrajectoryOwner& owner;
    const GrenadeTrajectoryFrame& frame;
    const GrenadePredictionScene& scene;
};
} // namespace skeet::esp
