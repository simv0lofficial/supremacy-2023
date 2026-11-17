#pragma once
#include "skeet/misc/config.hpp"

namespace skeet::misc {
struct ZoomSensitivityBindings {
    void* const& override_context; //43468994; target float is context+8
    void* const& local_player; //43475808, read on each update
    const std::uint32_t& encoded_mouse_sensitivity_cvar; //43467A48 ^1EEEF408
    std::uint32_t fov_offset; //decoded43468248/43469218
};
// This owner adjusts zoom sensitivity only. The primary Override FOV setting
// belongs to a separate view hook and is not inferred from the adjacent slider.
class ZoomSensitivityRuntime {
public:
    ZoomSensitivityRuntime(const FovConfig& settings, const ZoomSensitivityBindings& bindings)
        : settings(settings), bindings(bindings) {}
    const FovConfig& settings;
    const ZoomSensitivityBindings& bindings;
    void update(); // complete 0x433281C1, native receiver is FOV owner +8
    void reset(); // complete 0x43416A53
};
} // namespace skeet::misc
