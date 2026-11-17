#pragma once
#include "skeet/script/renderer_api.hpp"
#include <immintrin.h>
#include <cstdint>

namespace skeet::script::renderer_detail {
inline std::int32_t integer(const LuaApi& api, void* state, int index, bool checked) {
    const auto value = checked ? api.checknumber(state, index) : api.tonumber(state, index);
    return _mm_cvttsd_si32(_mm_set_sd(value));
}
inline std::uint32_t color(const LuaApi& api, void* state, int first, bool checked) {
    const auto r = std::uint32_t(integer(api, state, first, checked));
    const auto g = std::uint32_t(integer(api, state, first + 1, checked));
    const auto b = std::uint32_t(integer(api, state, first + 2, checked));
    const auto a = std::uint32_t(integer(api, state, first + 3, checked));
    return (r & 255) | ((g & 255) << 8) | ((b & 255) << 16) | (a << 24);
}
inline float fraction(float value) noexcept {
    return _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_setzero_ps()), _mm_set_ss(1.0f)));
}
std::uint32_t text_flags(const LuaApi&, void*, int);
} // namespace skeet::script::renderer_detail
