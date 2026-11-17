#pragma once
#include <algorithm>
#include <bit>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <immintrin.h>
#include <string>

namespace skeet::esp::detail {
using i32 = std::int32_t;
inline i32 add(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) + std::uint32_t(b));
}
inline i32 sub(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) - std::uint32_t(b));
}
inline i32 mul(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) * std::uint32_t(b));
}
inline i32 truncate(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
inline float sse_max(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
inline float sse_min(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
inline std::u16string decimal(i32 value) {
    char buffer[12];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    return std::u16string(buffer, result.ptr);
}
} // namespace skeet::esp::detail

namespace skeet::esp {
    using detail::i32;
    using detail::add;
    using detail::sub;
    using detail::mul;
    using detail::truncate;
    using detail::sse_max;
    using detail::sse_min;
    using detail::decimal;
}
