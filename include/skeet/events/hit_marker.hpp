#pragma once
#include "game_events.hpp"
#include "skeet/rage/target_relationships.hpp"
#include "skeet/render_queue.hpp"
#include "skeet/esp/config.hpp"
#include "skeet/misc/native_text.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include <span>

namespace skeet::events {
struct HitMarkerView {
    const void* player = nullptr;
    std::ptrdiff_t rejection_byte = 0; // decoder 43468E78 ^434684E4 ^9D920E35
    float frame_time = 0; // globals +14
    std::int32_t width = 0, height = 0; // renderer context +F4 / +F0
    float font_scale = 1; // renderer context +104
};
struct HitMarkerBindings {
    void* const& engine;
    void* const& entity_list;
    void* const& surface; // live434689C0, distinct from constructor's captured+C
    rage::TargetRelationshipBindings enemy;
    const std::uint8_t& sound_blocked; // live43468E58
    // Live4346D558 pointer; a null value is the native no-list case.
    std::int32_t* const& sound_players;
};
struct HitMarker {
    HitMarker(const esp::HitMarkerConfig& settings, float& remaining,
              const misc::NativeText& sound) noexcept
        : settings(settings), remaining(remaining), sound(sound) {}
    const esp::HitMarkerConfig& settings; // canonical9C345DFA /69C9D9F2
    float& remaining; // one native owner+38 cell
    const misc::NativeText& sound; // one native owner+3C string
    void player_hurt(GameEventReader&, HitMarkerBindings&, void* local_player); // 4337F176
    void draw(ui::RenderQueue&, const HitMarkerView&); // complete 433D066C
    // Production entry: clock, rejection offset and supplied queue dimensions
    // are read inside their actual native gates, never at owner construction.
    void draw_live(ui::RenderQueue&, void* player, void* const& global_variables,
                   const engine::DecodedOffset& rejection_byte);
    void reset() noexcept {
        remaining = 0;
    } // 4336897C
};
} // namespace skeet::events
