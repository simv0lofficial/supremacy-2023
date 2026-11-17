#pragma once
#include "commands.hpp"

namespace skeet::client {
struct CommandTrigonometry {
    float (*sine)(float);
    float (*cosine)(float);
};
struct CommandButtonPolicy {
    std::span<const std::uint32_t, 150> inverted_commands; // 43476090
    const std::int32_t& mode; // canonical Leg movement434685F8, read at433F1594
    std::int32_t choked; // *434757DC
};
// Complete 433F1594. The final native return value is a scratch pointer and
// is not consumed by the caller; observable command-button writes are retained.
void rebuild_command_direction_buttons(Command&, bool force_refresh,
                                       const CommandButtonPolicy&) noexcept;
// Complete 433B7FEA, including its original early exits, horizontal-basis
// projection, roll-dependent solve, speed scaling and direction-button update.
void reorient_command_movement(Command&, const std::array<float, 3>& original_angles,
                               std::uint8_t movement_type, bool on_ground,
                               bool force_button_refresh, const CommandButtonPolicy&,
                               const CommandTrigonometry&);
// Complete 4332B266, including its real 433240A6 roll guard.
void sanitize_command(Command&, bool unrestricted_angles, const ScriptCommandEnvironment&) noexcept;
} // namespace skeet::client
