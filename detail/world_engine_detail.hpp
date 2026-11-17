#pragma once
#include "skeet/esp/world_engine.hpp"
#include "esp_detail.hpp"
#include "util/address.h"
#include <cmath>
#include <cstring>

namespace skeet::esp::world_detail {
using skeet::field;
/* convar_integer moved/defined in visuals.cpp */
inline float squared(Vector3 a, Vector3 b) {
    const auto x = a.x - b.x, y = a.y - b.y, z = a.z - b.z;
    return (y * y + x * x) + z * z;
}
inline void distance(ui::RenderQueue& queue, Point at, float squared, std::uint32_t color) {
    auto text = detail::decimal(
        detail::mul(detail::sub(2, detail::truncate(std::sqrt(squared) / -12.0f)) / 5, 5));
    text += u" FT";
    queue.text(at.x, at.y, color, 0x14, 0, text);
}
} // namespace skeet::esp::world_detail
