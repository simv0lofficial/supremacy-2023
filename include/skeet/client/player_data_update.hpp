#pragma once
#include "player_state.hpp"

namespace skeet::engine {
struct NativeOffsetGlobals;
}
namespace skeet::client {
struct PlayerDataUpdateOffsets {
    std::ptrdiff_t duck_3309c0cf, scaled_recoil_66146802;
    std::ptrdiff_t replicated_time_d3d4230d, body_yaw_a2a8247f;
    std::ptrdiff_t velocity_36375c23, local_serialized_d070684c;
};
PlayerDataUpdateOffsets native_player_data_update_offsets(const engine::NativeOffsetGlobals&);
// Complete4333027B on its valid-player contract. It shifts the canonical
//0x16C cache block and captures the live fields before the engine data update.
void capture_serialized_player_state(NativePlayerStateLayout&, void* player,
                                     const PlayerDataUpdateOffsets&);
} //namespace skeet::client
