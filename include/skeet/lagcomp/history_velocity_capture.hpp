#pragma once
#include "entity_state.hpp"
#include "history_velocity_reconstruction.hpp"

namespace skeet::lagcomp {
struct HistoryVelocityCaptureBindings {
    const EntityStateRuntime& entity;
    const HistoryVelocityReconstructionBindings& reconstruction;
    engine::NativeConVarReference maximum_velocity_convar; //43468C2C xor3FE3AE0C
    engine::NativeConVarReference landing_limit_convar; //43467FA0 xor4B52A9BF
};
// Complete velocity owner4340F047..434118B6, excluding the subsequent yaw
// advance. The large physical reconstruction is a concrete source function.
// Both independent engine clock samples, guard restoration and the publish
// versus continue exits retain their native order.
void capture_history_velocity(client::PlayerState&, void* player,
                              const HistoryVelocityCaptureBindings&);
} // namespace skeet::lagcomp
