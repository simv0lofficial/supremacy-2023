#pragma once
#include "types.hpp"

namespace skeet {

struct PreSpeedLimits {
    float angle_scale; // movement owner + 0x1C0
    float maximum_angle; // movement owner + 0x1C8
};

struct PreSpeedTurn {
    float horizontal_speed;
    float minimum_step;
    float maximum_step;
    float turn_step;
    bool within_speed_limit;
};

// Complete pre-speed initial turn calculation 0x43375124..0x43375207.
PreSpeedTurn prepare_pre_speed_turn(PreSpeedLimits limits, float interval_per_tick,
                                    float player_speed_limit, MovementVector velocity) noexcept;

} // namespace skeet
