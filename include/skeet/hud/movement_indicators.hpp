#pragma once
#include "skeet/esp/player_runtime.hpp"
#include "skeet/movement/quick_peek.hpp"
#include "skeet/rage/settings.hpp"

namespace skeet::hud {
// References bind the same movement owner fields changed by gameplay. The
// crouch-cycle writer has not yet assigned semantic names to every byte.
struct MovementIndicatorState {
    QuickPeekState& quick_peek;
    std::uint8_t& byte_51c;
    float& duck_target_52c;
    std::int32_t& value_540;
    std::uint8_t& crouch_active_544;
};
// Complete valid-player43384B72. DUCK is the four-byte label constructed by
// 4347DCB2. The activation chain is433305FB ->4341D036 ->actual hotkey;
// command_flags is global434759C0 and duck_enabled is byte43467CEC.
void draw_movement_indicators(ui::RenderQueue& queue, script::Bridge& scripts,
                              const IntegerConfig& feature_indicators, std::uint8_t duck_enabled,
                              const rage::GlobalSettings& rage, std::uint32_t command_flags,
                              const QuickPeekConfig& quick_peek, MovementIndicatorState state,
                              const esp::RuntimePlayer& local_player, InputBitmap input,
                              std::int32_t& cursor, const esp::IndicatorLayout& layout,
                              const esp::Projection& projection);
} // namespace skeet::hud
