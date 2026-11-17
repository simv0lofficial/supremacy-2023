#pragma once
#include "skeet/combobox.hpp"
#include <span>

namespace skeet::anti_aim {
struct MenuDependencies {
    ui::Slider& custom_pitch;
    ui::Slider& yaw_offset;
    ui::ComboBox& yaw_jitter;
    ui::Slider& yaw_jitter_offset;
    ui::Slider& body_yaw_offset;
    ui::Checkbox& freestanding_body_yaw;
    std::span<std::int32_t, 6> yaw_offsets;
    std::span<std::int32_t, 4> body_yaw_offsets;
};
// Complete native dependency methods, including out-of-range early returns.
// The nonzero yaw/body selector indices must address their native arrays.
void pitch_changed(MenuDependencies&, const ui::ComboBox&); // 0x433B5760
void yaw_changed(MenuDependencies&, const ui::ComboBox&); // 0x433B2007
void jitter_changed(MenuDependencies&, const ui::ComboBox&, const ui::ComboBox& yaw); // 0x4332AE6E
void body_yaw_changed(MenuDependencies&, const ui::ComboBox&); // 0x433CE908
} // namespace skeet::anti_aim
