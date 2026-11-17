#pragma once
#include "types.hpp"

namespace skeet {
struct EasyStrafeState {
    std::uint32_t previous_buttons = 0; // owner + 0x1E0
    std::uint32_t suppressed_buttons = 0; // owner + 0x1E4
    std::uint32_t saved_buttons = 0; // owner + 0x53C
};

// Complete secondary-interface callback 0x4333F746. The settings are the
// owner+0x130 byte, global 0x43467CEC byte, and owner+0x1C4 magnitude.
void process_easy_strafe(EasyStrafeState& state, UserCommand& command, bool enabled,
                         bool force_bullrush, float default_move = 450.0f) noexcept;
} // namespace skeet
