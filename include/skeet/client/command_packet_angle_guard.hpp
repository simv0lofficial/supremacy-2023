#pragma once
#include "command_frame.hpp"
#include <span>

namespace skeet::client {
struct CommandPacketAngleBindings {
    const std::uint8_t* enabled; // 43467224
    const std::uint8_t* firing_mode_enabled; // 434684E0
    const std::uint8_t* send_mode; // 43468D7C
    std::ptrdiff_t weapon_identifier; // decoded 5C9F0FBB
    std::ptrdiff_t player_flags; // decoded 38CA5228
    std::ptrdiff_t player_first_byte; // decoded 043FA575
    std::ptrdiff_t player_second_byte; // decoded 334B460C
    std::span<const std::uint32_t, 3> weapon_classification; // 43474580
    void* const& input; // live4346A894, CInput GetUserCmd slot8
};

// Inline 433D4E53..433D4F40. Chokes a changed shot angle for classified
// weapons, then enforces the signed frame choke limit. Phase 5 follows.
void guard_changed_shot_packet(CommandFrame&, const Command&, std::uint8_t& send_packet,
                               const void* weapon, const CommandPacketAngleBindings&) noexcept;

// Inline 433D4F71..433D524E, after phase 5. Enforces the choke limit again,
// applies the choked-shot pitch offset, then limits the pitch/yaw angle step.
// Its maximum step is the native float at frame +1E8. The reference is the
// saved command or actual CInput GetUserCmd(0,number-1), in native gate order.
void guard_packet_angles(CommandFrame&, Command&, std::uint8_t& send_packet, void* player,
                         const void* weapon, const CommandPacketAngleBindings&);
} // namespace skeet::client
