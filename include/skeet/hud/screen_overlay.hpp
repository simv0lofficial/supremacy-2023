#pragma once
#include "skeet/esp/other_config.hpp"
#include "skeet/esp/projection.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/render_queue.hpp"

namespace skeet::hud {
// Shared runtime values published by433361BA. +58 references the Remove
// recoil descriptor; +6C marks the published recoil vector as valid.
struct ScreenOverlayState {
    const std::uint8_t* byte_58 = nullptr;
    float radius_5c = 0, recoil_pitch_60 = 0, recoil_yaw_64 = 0, recoil_roll_68 = 0;
    bool byte_6c = false;
};
// Complete43358A6A, including actual entity virtual171, recoil projection,
// radius-dependent segment count, and native opcode7 transparent-center fan.
void draw_screen_overlay(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                         const ScreenOverlayState& state, void* local_player,
                         std::uint32_t life_state_offset, Point screen_size,
                         const esp::Projection& projection, const RageVector& view_origin,
                         const RageTrigonometry& trigonometry);
} // namespace skeet::hud
