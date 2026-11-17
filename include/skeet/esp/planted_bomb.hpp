#pragma once
#include "c4.hpp"
#include "indicators.hpp"

namespace skeet::esp {

struct PlantedBombOwner {
    // Owner +0x6AC is cleared at entry, including the disabled path.
    std::int32_t transient = 0;
    float peak_damage = 0.0f; // owner +0x6B0
};
struct PlantedBombBinding {
    // The full frame uses a float timer at constructor +6AC. This handler
    // clears that same four-byte word; it does not own another counter.
    std::byte* transient_word;
    const float& peak_damage;
};
struct PlantedBombState {
    bool ticking;
    float detonation_time, defuse_end;
    std::int32_t defuser_state;
    Vector3 origin;
};
struct BombSites {
    Vector3 first, second;
};
struct PlantedBombPlayer {
    std::uint8_t life_state;
    BombDamageTarget damage_target;
    void* context;
    // Actual entity virtual health query. The owner reads it twice when the
    // predicted damage is nonzero; the values need not remain identical.
    std::int32_t (*health)(void*);
};
struct PlantedBombPlayerView {
    const std::uint8_t& life_state;
    const BombDamageTarget& damage_target;
    void* context;
    std::int32_t (*health)(void*);
    PlantedBombPlayerView(const PlantedBombPlayer& value)
        : life_state(value.life_state), damage_target(value.damage_target), context(value.context),
          health(value.health) {}
    PlantedBombPlayerView(const std::uint8_t& life, const BombDamageTarget& target, void* object,
                          std::int32_t (*query)(void*))
        : life_state(life), damage_target(target), context(object), health(query) {}
};
struct PlantedBombInput {
    const PlantedBombState& bomb;
    PlantedBombPlayerView player;
    // Registry selection used by 0x433CD3A7 is independent of bomb above.
    const ActiveBomb* registered_bomb;
    BombTraceProvider trace;
    const BombSites* sites;
    float current_time;
    std::int32_t label_texture;
    script::Bridge* scripts = nullptr;
};

// 0x43345EF2..0x4334636B, including the defuse bar, nearest-site label and
// damage/FATAL indicators. Bomb/player arguments obey the native caller's
// valid-entity contract. Layout supplies the actual renderer backend values.
void draw_planted_bomb(ui::RenderQueue& queue, const WorldEspConfig& config,
                       PlantedBombOwner& owner, std::int32_t& indicator_cursor,
                       const PlantedBombInput& input, const IndicatorLayout& layout);
void draw_planted_bomb(ui::RenderQueue& queue, const WorldEspConfig& config,
                       PlantedBombBinding owner, std::int32_t& indicator_cursor,
                       const PlantedBombInput& input, const IndicatorLayout& layout);

} // namespace skeet::esp
