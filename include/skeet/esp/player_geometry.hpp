#pragma once
#include "projection.hpp"

namespace skeet::esp {
// Native per-player segment at record +0x1E8. Used by the shared player cache,
// preparation/fade owner and projected player overlay.
struct PlayerGeometry {
    std::int32_t last_live_tick = 0;
    float alpha = 0;
    Vector3 lower{}, center{}, origin{}, upper{};
    float pose_adjustment = 0, eye_height = 0;
};
static_assert(sizeof(PlayerGeometry) == 64);
} // namespace skeet::esp
