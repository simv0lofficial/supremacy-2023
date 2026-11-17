#pragma once
#include "speed_control.hpp"

namespace skeet {

struct GroundSpeedTailSettings {
    bool fast_walk;
    // Three independent native gates: 0x4346A740, 0x43467F60 and 0x434685B8.
    // Their association with a menu option has not been established.
    bool periodic_speed_enabled;
    bool periodic_context_a;
    bool periodic_context_b;
};

// Complete final movement-scheduler block 0x4337C899..0x4337CAA4.
// player_state is the decoded byte consumed by 0x433BA215 (state >= 2
// prevents this block). The caller supplies the command's signed sequence
// number and the engine's two distinct command/movement angle vectors.
void process_ground_speed_tail(const GroundSpeedTailSettings& settings,
                               const MovementPhysics& physics, std::uint8_t player_state,
                               std::int32_t command_number, UserCommand& command,
                               MovementVector velocity, MovementAngles command_angles,
                               MovementAngleView movement_angles) noexcept;

} // namespace skeet
