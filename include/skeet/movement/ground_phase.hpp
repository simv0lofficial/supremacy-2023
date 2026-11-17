#pragma once
#include "quick_stop.hpp"
#include "slow_motion.hpp"

namespace skeet {

struct GroundPhaseSettings {
    QuickStopSettings quick_stop;
    SlowMotionSettings slow_motion;
};

enum class GroundPhaseContinuation {
    player_movement, // 0x43372C76
    weapon_speed_features, // 0x4337206A
    complete, // 0x4337CAA4
};

// Ground/jump gate at 0x43371D4C and its quick-stop/slow-motion chain.
// crouch_cycle_active is owner + 0x544, reset only by the first gate.
GroundPhaseContinuation
process_ground_phase(const GroundPhaseSettings& settings, InputBitmap input,
                     const MovementPhysics& physics, bool has_weapon, std::int32_t weapon_state,
                     bool weapon_blocked, bool& crouch_cycle_active, std::uint32_t& scheduler_flags,
                     std::int32_t command_number, UserCommand& command, MovementVector velocity,
                     MovementAngles command_angles, MovementAngleView movement_angles);

} // namespace skeet
