#pragma once
#include "air_strafe.hpp"
#include "collision.hpp"
#include "command_state.hpp"

namespace skeet {

struct RawScoutGateSettings {
    bool bunny_hop_enabled; // movement owner +0x194
    float forced_forward_move; // movement owner +0x1C4
};

struct RawScoutGateFrame {
    MovementVector velocity;
    std::uint32_t player_flags;
    bool has_weapon;
    std::int32_t weapon_id;
    // The actual player virtual +0x28 has already returned this reference in
    // the enclosing raw-basis preamble. Copy it only after GetViewAngles.
    const MovementVector& position;
};

struct RawScoutGateBindings {
    void* player;
    void* engine_client; // actual GetViewAngles virtual +0x48
    MovementCollisionEngine& collision;
    std::uint8_t* engine_trace_flag;
};

enum class RawScoutGateResult {
    continue_air,
    ground_speed_tail
};

// Complete raw-basis ground/scout gates 0x43378F05..0x43379B3B, after mode
// selection and before the airborne input-heading stage. Hull access uses the
// actual player's collideable virtual +0x0C, then maximum +8 and minimum +4.
// Both traces and the grounded bunny-hop command mutation stay in source.
RawScoutGateResult process_raw_scout_gate(const RawScoutGateSettings&, const AirStrafeSelection&,
                                          const RawScoutGateFrame&, const MovementFeedback&,
                                          UserCommand&, const RawScoutGateBindings&);

} // namespace skeet
