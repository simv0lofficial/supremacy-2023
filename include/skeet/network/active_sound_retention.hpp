#pragma once
#include "skeet/esp/sound_overlay.hpp"

namespace skeet::network {
// Original43476420 tree primitives. The nodes remain engine-owned; removing
// one preserves its allocation, native free-list link and red/black topology.
std::int32_t first_active_sound(const esp::ActiveSoundTree&) noexcept; //433F4BE4
std::int32_t next_active_sound(const esp::ActiveSoundTree&, std::int32_t) noexcept; //433BFEC6
void release_active_sound(esp::ActiveSoundTree&, std::int32_t) noexcept; //43359947 +caller
} //namespace skeet::network
