#pragma once
#include "skeet/movement/simulation_capture.hpp"
#include "skeet/movement/simulation_tick.hpp"
#include "skeet/client/commands.hpp"

namespace skeet::anti_aim {
using ::skeet::MovementCaptureOffsets;
using ::skeet::MovementCaptureConvars;
using ::skeet::MovementCaptureInput;
using ::skeet::capture_movement_player;
using ::skeet::capture_movement_rules;
using ::skeet::initialize_live_movement_simulation;
// Complete433376C3 with the live constructor reads retained behind its gate.
MovementVector advance_live_movement_origin(void* player, MovementVector position,
                                            MovementVector velocity, std::uint32_t player_flags,
                                            const MovementCaptureInput&,
                                            const MovementEnvironmentBindings&,
                                            const client::Command&, std::uint32_t input_buttons,
                                            float yaw, MovementCollisionEngine&);
} // namespace skeet::anti_aim
