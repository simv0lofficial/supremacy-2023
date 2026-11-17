#pragma once
#include "skeet/render_queue.hpp"
#include "skeet/core.hpp"

namespace skeet::esp {

struct FlagEntry {
    std::uint32_t mask, color;
    std::u16string_view text;
};
// Detached decoded values for explicit snapshot consumers.
std::array<FlagEntry, 18> default_flag_entries() noexcept;
class FlagTable;
struct AnimationLayer {
    std::int32_t count = 0;
    float weight = 0;
    std::int32_t activity = 0;
    float cycle = 0;
};
float reload_cycle(const AnimationLayer& layer) noexcept;

struct RecordHistory {
    std::uint32_t count = 0, first = 0;
    std::array<std::int32_t, 32> ticks{};
    std::uint32_t correction_count = 0, correction_index = 0;
    struct Correction {
        std::int32_t counter = 0, limit = 0;
    };
    std::array<Correction, 6> corrections{};
};
bool correction_pending(const RecordHistory& history) noexcept;
bool fake_duck(const RecordHistory& history, float duck_amount, float duck_speed) noexcept;

// Netvars, entity handles and engine calls are decoded by the engine adapter;
// these are the actual values read by 0x43417468, not guessed memory offsets.
struct PlayerFlagState {
    std::int32_t game_mode = 0;
    bool helmet = false;
    std::int32_t armor = 0;
    bool dormant = false, scoped = false;
    float flash_alpha = 0;
    bool vip = false;
    std::int32_t team = 0;
    bool defusing = false;
    float duck_amount = 0, duck_speed = 0;
    bool weapon_valid = false;
    std::int32_t ammunition = -1, weapon_kind = 0;
    bool pin_pulled = false;
    AnimationLayer animation;
    std::int32_t entity_index = 1, bomb_owner_index = -1;
    float engine_time = 0, tick_interval = 1;
    std::int32_t last_hit_tick = 0;
    bool on_shot_enabled = false, target_is_player = false;
    bool local_player_present = false, local_player_disallowed = false;
    bool on_shot_extra_enabled = false;
    float simulation_time = 0, simulation_tick_interval = 1;
    RecordHistory history;
};
std::uint32_t active_flags(const PlayerFlagState& player) noexcept;

// Advances the side cursor only for entries whose mutable mask matches.
void draw_flags(ui::RenderQueue& queue, Point& cursor, std::int32_t line_height,
                std::uint32_t active_mask, std::span<const FlagEntry, 18> table);
void draw_flags(ui::RenderQueue& queue, Point& cursor, std::int32_t line_height,
                std::uint32_t active_mask, const FlagTable& table);

} // namespace skeet::esp
