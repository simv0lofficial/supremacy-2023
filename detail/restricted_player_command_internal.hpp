#pragma once
#include "skeet/client/restricted_player_command.hpp"
#include "util/address.h"

namespace skeet::client::restricted_detail {
using skeet::read;
using skeet::write;
void update_animation_step(RestrictedPlayerCommandOwner&, void* player, void* animation_state,
                           Command&, bool jump_reset, std::int32_t seed, std::int32_t layer_count,
                           const std::array<float, 3>* replay_angles,
                           const RestrictedPlayerCommandBindings&);
}
