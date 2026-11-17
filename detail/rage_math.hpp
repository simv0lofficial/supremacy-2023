#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <xmmintrin.h>
#include "util/address.h"

namespace skeet::rage_detail {
inline constexpr std::array<float, 16> hitgroup_multipliers{1, 4, 1, 1.25f, 1, 1, .75f, .75f,
                                                            1, 1, 1, 1,     1, 1, 1,    1};
inline std::int32_t signed_word(std::uint32_t value) noexcept {
    return std::bit_cast<std::int32_t>(value);
}
using ::skeet::add;
inline std::int32_t shots_to_kill(std::int32_t health, std::int32_t damage) noexcept {
    if (damage <= 0)
        return std::numeric_limits<std::int32_t>::max();
    return std::max(1, add(add(health, -1), damage) / damage);
}
inline std::uint32_t damage_priority(std::int32_t health, std::int32_t damage) noexcept {
    auto capped_health = std::min(std::uint32_t(health) + 5U, 127U);
    return std::min(std::uint32_t(damage), capped_health) & 127U;
}
inline std::uint32_t chance_priority(float chance) noexcept {
    const auto quantized = _mm_cvtss_si32(_mm_set_ss(chance * 100.0f));
    return std::uint32_t(std::clamp(quantized, 0, 127));
}
} // namespace skeet::rage_detail
