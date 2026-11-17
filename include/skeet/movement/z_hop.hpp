#pragma once
#include "types.hpp"

namespace skeet {

struct ZHopSettings {
    bool enabled;
    HotkeyBinding hotkey;
    std::int32_t amplitude;
    std::int32_t frequency;
};

// Complete 0x43376E82..0x43376EF4 branch, also used at 0x4337C63B.
// The caller chooses the pre-speed/Z-Hop branch and supplies engine time.
bool apply_z_hop_heading(const ZHopSettings& settings, InputBitmap input, float engine_time,
                         MovementAngleView movement_angles);

} // namespace skeet
