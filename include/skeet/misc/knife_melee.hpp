#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/rage/ballistics.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/rage_hitboxes.hpp"

namespace skeet::misc {
struct KnifeMeleeBindings {
    void* engine_trace; // actual 43468304 interface; TraceRay slot 5
    void* const* trace_filter_vtable; // genuine external table 434758B4
    const rage::TargetBindings& targeting; // eligibility's actual 43475808 local
    std::ptrdiff_t target_origin; // A9EE6062
    std::ptrdiff_t target_armor; // 47A6F50C, int32
    std::ptrdiff_t weapon_next_primary; // F71CE331, float
    const float* current_time; // pointed 43467EB8 prediction clock
    RageTrigonometry trigonometry; // actual imported sine/cosine
    double (*arc_tangent2)(double, double); // imported 77509EE0 atan2(y,x)
};

// Complete 433B6A63. Trace a line first; only an ordered fraction >= 1
// triggers the symmetric {16,16,18} hull trace. The same output is reused.
void trace_knife_melee(void* local_player, const RageVector& start, const RageVector& end,
                       BallisticTrace&, const KnifeMeleeBindings&);

// Complete 433CF073. An initial obstruction permits fourteen further traces,
// alternating +5/-5 through +35/-35 degrees around the initial horizontal yaw.
// The first trace hitting target wins; direction changes only on a rotated hit.
bool find_knife_trace_direction(void* local_player, const RageVector& origin, void* target,
                                RageVector& direction, const KnifeMeleeBindings&);

// Complete 4333CA80. Scores: slash 2, stab 1 (9 for health > 100), back hit
// adds 4, lethal adds 0x10. No eligible hit returns zero. Eligibility delegates
// to the existing complete 43358962 source body, not a supplied predicate.
std::uint32_t score_knife_attack(const client::CommandFrame&, void* local_player,
                                 const RageVector& direction, bool stab, const KnifeMeleeBindings&);
} // namespace skeet::misc
