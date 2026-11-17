#pragma once
#include "collision.hpp"
#include "skeet/client/commands.hpp"

namespace skeet {

struct NormalizedScoutGateFrame {
    void* player; // actual player; collideable virtual +0x0C
    void* engine_client; // actual GetViewAngles virtual +0x48
    MovementVector original_velocity;
    MovementVector origin; // player virtual +0x28, acquired before this block
    bool has_weapon;
    std::uint16_t weapon_definition;
};

// Complete 4337338B..43373984. A nearly stationary scout with no movement
// keys probes 64 units along the normalized horizontal engine view, then
// repeats the same hull sweep 64 units higher when the first sweep is blocked.
// False selects the shared command tail at 4337C895; true reaches 43373984.
// Collideable and view getters execute only after the native scalar gates.
bool pass_normalized_scout_gate(const NormalizedScoutGateFrame&, const client::Command&,
                                MovementCollisionEngine&, std::uint8_t* engine_trace_flag);

} // namespace skeet
