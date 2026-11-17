#pragma once
#include "types.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/lagcomp/history.hpp"

namespace skeet {
namespace client {
class PlayerStateCache;
}
struct MovementEnvironmentBindings;

struct MovementExtrapolationCommandPlayer {
    MovementVector velocity;
    float duck_amount;
    std::uint32_t player_flags;
    std::uint8_t field_7090d8dd;
    std::int32_t field_52899df3;
    float field_d3d4230d;
};
struct MovementExtrapolationCommandOffsets {
    std::uint32_t velocity, duck_amount, player_flags;
    std::uint32_t field_7090d8dd, field_52899df3, field_d3d4230d;
};

// Complete 4334D3FF, including the distinct player-state acceleration helper
// 433F3DC7. The command is already zeroed by the enclosing 43385AFD entry.
bool infer_history_movement_command(client::Command&, const lagcomp::RecordRing&,
                                    float history_time_110,
                                    const MovementExtrapolationCommandPlayer&,
                                    const MovementPhysics&) noexcept;

// Complete 43385AFD after its actual entity-index/cache lookup. The original
// valid-player path requires a nonzero index. Both history and live-player
// branches zero the entire 0x64-byte command before reconstructing its inputs.
bool reconstruct_extrapolation_command(client::Command&, const lagcomp::RecordRing&,
                                       float history_time_110, std::int32_t history_tick_168,
                                       const MovementExtrapolationCommandPlayer&,
                                       const MovementPhysics&) noexcept;
// Raw43385AFD/4334D3FF/433F3DC7. Index/cache, player fields, ConVars and
// guarded weapon virtuals are read where the original branches consume them.
bool reconstruct_live_extrapolation_command(client::Command&, void* player,
                                            client::PlayerStateCache&,
                                            const MovementExtrapolationCommandOffsets&,
                                            const MovementEnvironmentBindings&);

} // namespace skeet
