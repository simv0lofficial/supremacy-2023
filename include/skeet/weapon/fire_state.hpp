#pragma once
#include "skeet/client/commands.hpp"
#include <cstddef>

namespace skeet::weapon {
struct FireState {
    bool primary_ready = false;
    bool secondary_ready = false;
    bool burst_ready = false;
};
struct FireStateOffsets {
    std::ptrdiff_t weapon_definition; // 4346908C / 5C9F0FBB, uint16
    std::ptrdiff_t burst_mode; // 4346A768 / A1AD86BF, byte
    std::ptrdiff_t burst_remaining; // 43467D40 / F93CEAF4, int32
    std::ptrdiff_t next_burst; // 43468D68 / 836CB5DB, float
    std::ptrdiff_t player_block_bcb263a5; // 4346799C, byte
    std::ptrdiff_t team; // 434688B0 / 8B5586F6, int32
    std::ptrdiff_t player_state_a15c3105; // 4346A7E0, byte
    std::ptrdiff_t ammunition; // 4346A60C / CA5EC698, int32
    std::ptrdiff_t player_next_attack; // 4346A80C / DE530A8B, float
    std::ptrdiff_t next_primary; // 43467A50 / F71CE331, float
    std::ptrdiff_t shots_fired; // 4346A6DC / 8CFC633A, int32
    std::ptrdiff_t weapon_state_9e2078a1; // 4346828C, int32
    std::ptrdiff_t postpone_fire; // 43468FD8 / 43D0EFC4, float
    std::ptrdiff_t next_secondary; // 434670A4 / C0D5F8A1, float
    std::ptrdiff_t rule_d720aea6; // 43467910, byte
};
struct FireStateBindings {
    void* const* game_rules; // 4346A3A4; both levels may be null
    const float* current_time; // *43467EB8, prediction time
    FireStateOffsets offsets;
};

// Complete 4333D377, including 433DF22C. The engine supplies a nonnull player
// on the ordinary path. A null weapon returns three cleared outputs.
// Weapon selection calls networkable (+8) slot 10; a prior shot can query
// weapon slot 430. Those actual virtual queries retain their native order.
FireState evaluate_fire_state(const FireStateBindings&, void* player, void* weapon,
                              const client::Command&);
} // namespace skeet::weapon
