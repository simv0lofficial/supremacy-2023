#pragma once
#include "speed_control.hpp"

namespace skeet {

struct SlowMotionSettings {
    bool enabled;
    HotkeyBinding hotkey;
};

// 0x43371E8F..0x43372061, including the shared regulator call and return.
// The containing scheduler has already checked ground, jump and weapon presence.
// Returns true when the command is consumed; false continues at 0x4337206A.
// weapon_state is the signed native field at 0x43468488; weapon_blocked is
// the decoded weapon byte tested at 0x43371F12.
bool process_slow_motion_stage(const SlowMotionSettings& settings, InputBitmap input,
                               const MovementPhysics& physics, std::int32_t weapon_state,
                               bool weapon_blocked, std::uint32_t& scheduler_flags,
                               std::int32_t command_number, UserCommand& command,
                               MovementVector velocity, MovementAngles command_angles,
                               MovementAngleView movement_angles);

} // namespace skeet
