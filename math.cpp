// Math Utilities Implementation
#include "skeet/core.hpp"
#include <bit>
#include <cstring>


// ============================================================================
// Subcomponent: math.cpp
// ============================================================================

namespace skeet {
namespace {
bool is_nan(float value) noexcept {
    return (std::bit_cast<std::uint32_t>(value) & 0x7fffffffU) > 0x7f800000U;
}
bool negative(float value) noexcept {
    return (std::bit_cast<std::uint32_t>(value) >> 31) != 0;
}
}

// 0x43367633..0x43367696. Keep NaN payloads and signed-zero selection.
float minimum_number(float first, float second) noexcept {
    if (is_nan(first))
        return second;
    if (is_nan(second))
        return first;
    if (negative(first) != negative(second))
        return negative(first) ? first : second;
    return first < second ? first : second;
}

// 0x4332FC35..0x4332FC93. Equal operands select the first argument.
float maximum_number(float first, float second) noexcept {
    if (is_nan(first))
        return second;
    if (is_nan(second))
        return first;
    if (negative(first) != negative(second))
        return negative(first) ? second : first;
    return second > first ? second : first;
}

// Complete 19-iteration copier at 0x433E1179..0x433E11B0.
void copy_hitbox_matrices(const HitboxMatrices& source, HitboxMatrices& destination) noexcept {
    for (std::size_t matrix = 0; matrix != source.size(); ++matrix) {
        for (std::size_t row = 0; row != 3; ++row) {
            std::array<std::byte, 16> temporary;
            std::memcpy(temporary.data(), source[matrix].rows[row], temporary.size());
            std::memcpy(destination[matrix].rows[row], temporary.data(), temporary.size());
        }
    }
}
} // namespace skeet
