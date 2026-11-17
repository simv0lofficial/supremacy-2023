#pragma once
#include <cstdint>

namespace skeet::animation {

// Complete 433CC618. The input is the seven-word layer copy beginning at
// engine-layer +14 (record +4D8 at caller 43411872), not the engine layer base.
// It reads sequence at +04, playback-rate bits at +14 and cycle at +18.
// The result is exactly 0 or 1. A NaN playback rate fails; a NaN cycle passes
// the final native COMISS/JAE branch. The input must be a valid layer view.
std::uint8_t layer_is_active(const void* layer) noexcept;

} // namespace skeet::animation
