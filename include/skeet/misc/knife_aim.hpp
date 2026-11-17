#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/rage_geometry.hpp"

namespace skeet::misc {
struct KnifeMeleeBindings;

// Complete 43397F0A. The target must expose the engine collideable interface;
// its two bound queries retain their original call order. Attack scores use
// the concrete trace/damage implementation in knife_melee, with no target
// approximation after the nearest-AABB direction has been selected.
void evaluate_knife_direction(std::uint32_t modes, const client::CommandFrame&, void* local_player,
                              void* target, bool low_health, RageVector& chosen_direction,
                              std::uint32_t& best_score, const KnifeMeleeBindings&);

struct KnifeAimBindings {
    void* input; // actual CInput, GetUserCmd virtual8(slot=0,number-1)
    std::ptrdiff_t player_flags; // decoded43468B50, mask38CA5228
    std::ptrdiff_t first_player_byte; // decoded4346757C, mask043FA575
    std::ptrdiff_t second_player_byte; // decoded4346934C, mask334B460C
    // Genuine msvcrt!__libm_sse2_atan2 semantics, adapted from its SSE ABI.
    double (*arc_tangent2)(double y, double x);
};

// Complete 4335C256 for the valid-player contract of its knife caller. Writes
// pitch/yaw and clears roll before any angle limit. A limited step returns
// false and retains the packet byte; an accepted step clears it and returns
// true. The native null-player paths consist of unreachable junk bytes.
bool apply_knife_aim(const client::CommandFrame&, void* local_player, client::Command&,
                     const RageVector& direction, std::uint8_t& send_packet,
                     const KnifeAimBindings&);
} // namespace skeet::misc
