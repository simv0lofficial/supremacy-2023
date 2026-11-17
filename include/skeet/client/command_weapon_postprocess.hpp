#pragma once
#include "command_frame.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <cstddef>

namespace skeet::client {
struct CommandMouseOffsets {
    std::ptrdiff_t player_flags; // 43468B50 / 38CA5228, uint32
    std::ptrdiff_t player_state_043fa575; // 4346757C, byte
    std::ptrdiff_t player_state_334b460c; // 4346934C, byte
};
struct CommandWeaponPostprocessBindings {
    void* const& input; // live4346A894; Command[150] pointer at +F0
    engine::NativeConVarReference yaw_scale; //43467950 XOR66BFCA30
    engine::NativeConVarReference pitch_scale; //43467988 XOR01EAAD2F
    std::array<float, 3>* sent_angles; // 4347A4F0
    CommandMouseOffsets offsets;
};
// Complete 433D543B..433D566A, including 4341999F and the exact-number
// command lookup 433B9184 -> 43331EFC. Publishes a sent command's pitch/yaw
// with zero roll, then reconstructs mouse counts only for a changed command
// while the two player states match and player flag bit 3 is clear.
// The previous command is selected with unsigned command-number modulo 150.
void postprocess_command_after_phase7(const CommandWeaponPostprocessBindings&, const CommandFrame&,
                                      void* player, Command&, std::uint8_t send_packet) noexcept;
} // namespace skeet::client
