#pragma once
#include <cstdint>

namespace skeet::anti_aim {
// Named by owner offsets until the downstream consumers establish meaning.
struct State {
    std::uint32_t flags_130 = 0;
    std::int32_t value_158 = 0;
    std::uint32_t value_190 = 0; // phase counter, native INC at4332D396
    float value_1a0 = 0;
    float value_1a8 = 0;
    std::uint32_t value_1ac = 0;
    std::uint32_t value_1b0 = 0;
    std::int32_t value_1b4 = 0;
    std::int32_t value_1c4 = 0;
    const std::uint8_t* build_bones_12c = nullptr; //bound4348E637 to96DB2E1C payload
};

// Complete owner virtual method at 0x433C9EE6.
void reset_state(State&) noexcept;
} // namespace skeet::anti_aim
