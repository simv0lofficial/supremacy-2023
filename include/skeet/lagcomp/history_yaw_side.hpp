#pragma once
#include <cstdint>

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
// Genuine msvcrt!_CIfmod7750834A. The x86 import consumes numerator in ST1
// and denominator in ST0 and returns ST0. This binding adapts that external
// ABI to ordinary double arguments; it does not replace an internal helper.
using HistoryYawSideFmod = double (*)(double numerator, double denominator);

// Complete4340E846..4340EACE. Called after the live absolute-origin update,
// before the animation-state-null gate. enemy is the saved4339EF8C result
// (the inverse of native stack+132). Only owner.flags bit4 can change.
// enabled refers to live global4346754C. fmod is a required CRT binding.
void update_history_yaw_side(client::PlayerState&, bool enemy, std::uint32_t update_flags,
                             const std::uint8_t& enabled, HistoryYawSideFmod fmod);
} // namespace skeet::lagcomp
