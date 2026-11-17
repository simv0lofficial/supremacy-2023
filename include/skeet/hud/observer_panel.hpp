#pragma once
#include "skeet/esp/player_runtime.hpp"
#include <array>

namespace skeet::hud {

struct ObserverPlayerInfo {
    std::array<char, 128> name{};
    bool fake_player = false;
    bool hltv = false;
};
struct ObserverEngine {
    const esp::PlayerRuntimeEngine& players;
    void* context = nullptr;
    // IVEngineClient::GetPlayerInfo, engine virtual slot 8.
    bool (*player_info)(void*, std::int32_t, ObserverPlayerInfo&) = nullptr;
};

// Complete 4334E581. The counter is native object +6A8, also exposed as
// PlayerFrameState::transient through the drawing owner's +8 adjustment.
// begin_player_frame resets that counter before the per-player loop.
void draw_observer_panel(ui::RenderQueue& queue, esp::PlayerFrameState& state,
                         const ObserverEngine& engine, const esp::RuntimePlayer* initial_view,
                         const esp::RuntimePlayer* observer, std::int32_t entity_index,
                         std::int32_t screen_width);

// The observer_panel handoff emitted by prepare_player (4340996B..43409974).
void draw_prepared_observer(ui::RenderQueue& queue, esp::PlayerFrameState& state,
                            const ObserverEngine& engine, const esp::RuntimePlayer* initial_view,
                            const esp::PreparedPlayer& prepared, std::int32_t screen_width);

} // namespace skeet::hud
