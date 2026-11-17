#pragma once
#include "skeet/config.hpp"

namespace skeet {

// Native descriptor offsets below are relative to the movement owner, not its
// secondary prediction interface (owner + 8). Hash-only names have no recovered
// UI label; they are deliberately not assigned speculative gameplay meanings.
struct MovementConfig {
    BooleanConfig air_strafe; // 0x01C
    BooleanConfig config_ad536de0; // 0x030
    IntegerConfig air_strafe_direction; // 0x044
    IntegerConfig air_strafe_smoothing; // 0x058
    BooleanConfig z_hop; // 0x06C
    IntegerConfig config_3027dc11; // 0x080
    IntegerConfig config_3127de82; // 0x094
    BooleanConfig pre_speed; // 0x0A8
    BooleanConfig fast_walk; // 0x0BC
    HotkeyConfig config_712c1936; // 0x0D0
    HotkeyConfig config_2b499e77; // 0x0F8
    BooleanConfig easy_strafe; // 0x120
    BooleanConfig standalone_quick_stop; // 0x134
    BooleanConfig avoid_collisions; // 0x148
    IntegerConfig config_af9ae891; // 0x15C
    BooleanConfig slow_motion; // 0x170
    BooleanConfig bunny_hop; // 0x184: hash 0x18C, value 0x194
    BooleanConfig jump_at_edge; // 0x198: hash 0x1A0, value 0x1A8
};

// Nested at movement owner + 0x1E8. Offsets are relative to this subobject.
struct QuickPeekConfig {
    BooleanConfig enabled; // 0x00
    HotkeyConfig hotkey; // 0x14
    IntegerConfig mode; // 0x3C
    ColorConfig color; // 0x50
    IntegerConfig distance; // 0x64
};

// Config construction portions of 0x4348FA90 and 0x43402969. These functions
// preserve preexisting booleans exactly as the native constructors do. They do
// not claim to initialize the movement object's engine interfaces or state.
void initialize_movement_config(ConfigRegistries& registries, MovementConfig& config) noexcept;
void initialize_quick_peek_config(ConfigRegistries& registries, QuickPeekConfig& config) noexcept;

} // namespace skeet
