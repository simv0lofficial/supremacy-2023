#pragma once
#include "types.hpp"

namespace skeet {

struct AirStrafeSettings {
    std::int32_t direction;
    bool pre_speed;
    HotkeyBinding pre_speed_hotkey;
};

struct AirStrafeSelection {
    bool continue_solver;
    bool view_angles;
    // The native intermediate retains the upper 24 bits of direction >> 1.
    // Its low byte is the movement-keys selection and can be cleared alone.
    std::uint32_t movement_key_mode;
    bool pre_speed_requested;
    bool pre_speed_effective;
};

// Complete mode-selection block 0x433732C6..0x4337338B. The false continuation
// goes to the ground-speed tail at 0x4337C899. pre_speed_guard_enabled is the
// decoded guard-list comparison at 0x43373317, separate from the hotkey.
AirStrafeSelection select_air_strafe_modes(const AirStrafeSettings& settings, InputBitmap input,
                                           bool pre_speed_guard_enabled,
                                           const UserCommand& command);

} // namespace skeet
