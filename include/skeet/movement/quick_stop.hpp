#pragma once
#include "speed_control.hpp"

namespace skeet {

struct QuickStopSettings {
    bool standalone_quick_stop;
    bool quick_peek_enabled;
    HotkeyBinding quick_peek_hotkey;
};

enum class QuickStopContinuation {
    player_movement, // 0x43372C76
    speed_features, // 0x43371E8F
    complete, // 0x4337CAA4
};

// 0x43371D86..0x43371E8F and shared counter-movement return at 0x433702E8.
// The containing scheduler has already passed its initial ground/jump gate.
// Its ground test immediately before the counter-movement call is retained.
QuickStopContinuation process_quick_stop_stage(const QuickStopSettings& settings, InputBitmap input,
                                               const MovementPhysics& physics, bool has_weapon,
                                               std::uint32_t& scheduler_flags, UserCommand& command,
                                               MovementVector velocity,
                                               MovementAngleView movement_angles);

} // namespace skeet
