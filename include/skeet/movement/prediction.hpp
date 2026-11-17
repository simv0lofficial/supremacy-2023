#pragma once
#include "collision.hpp"
#include "simulation_tick.hpp"
#include "skeet/client/commands.hpp"

namespace skeet {

// Complete non-null entity path of 43359415..433598DE. Entity velocity and
// hull bounds are the decoded field and actual virtual130/134 results.
// Speeds outside the open interval (5,340) preserve every origin component.
void predict_movement_origin(MovementVector& origin, MovementVector velocity,
                             MovementVector hull_minimum, MovementVector hull_maximum,
                             std::int32_t ticks, float interval_per_tick,
                             MovementCollisionEngine& collision, std::uint8_t* engine_trace_flag);

// Complete 433376C3..4333781C. The active command's buttons gate prediction;
// the separate copied command supplies movement and retains the native tick
// sentinel comparison. Both ticks execute the concrete simulation directly.
MovementVector advance_movement_origin(std::uint32_t player, MovementVector position,
                                       MovementVector velocity, std::uint32_t player_flags,
                                       const MovementSimulationPlayer& snapshot,
                                       const MovementSimulationRules& rules,
                                       const MovementSimulationEnvironment& environment,
                                       const client::Command& command, std::uint32_t input_buttons,
                                       float yaw, MovementCollisionEngine& collision);

} // namespace skeet
