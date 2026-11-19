#pragma once
#include "animation_math.hpp"
#include "history.hpp"

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
struct HistoryAnimationSeed {
    float previous_update_time; // saved animation6C
    Vector velocity; // saved live velocity before resolver animation changes
    bool active_air_layer; // native low byte of stack1E4
};
// Complete434118B6..43411A37. animation is the nonnull state pointer retained
// by the caller's CachedHistoryFrame. Writes owner180 through433CAFE0 and
// initializes the native ground bytes only on a first discontinuous capture.
HistoryAnimationSeed prepare_history_animation(client::PlayerState&, void* player,
                                               void* animation_state, std::int32_t simulation_ticks,
                                               std::uint32_t update_flags,
                                               const EntityStateRuntime&,
                                               std::uint32_t player_flags_e2fbe695);
} // namespace skeet::lagcomp
