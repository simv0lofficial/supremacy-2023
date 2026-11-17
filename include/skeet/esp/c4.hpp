#pragma once
#include "other_config.hpp"
#include "projection.hpp"
#include "skeet/render_queue.hpp"

namespace skeet::esp {

struct DroppedC4State {
    bool local_player_present;
    // Byte read through decoder 0x4346921C (XOR 0x82F5AD08).
    std::uint8_t local_state;
    // Dword read through decoder 0x43468F40 (XOR 0x71A5AC08).
    std::int32_t entity_state;
    Vector3 origin, view_origin;
};
// Native CC4 class handler 0x433AE05A; entity presence is a caller contract.
// The native gate is the dropped-weapons mask, while its color comes from
// the Bomb descriptor. The mutable label is held in owner storage +0xF70.
void draw_dropped_c4(ui::RenderQueue& queue, const WorldEspConfig& config,
                     const Projection& projection, const DroppedC4State& state,
                     std::u16string_view label);

struct ActiveBomb {
    const void* entity;
    bool ticking;
    Vector3 origin;
};
struct BombDamageTarget {
    Vector3 origin, view_offset;
    std::int32_t armor;
};
struct BombDamageRay {
    Vector3 start, delta;
    bool swept;
};
struct BombGroundResult {
    float fraction;
    Vector3 end_position, plane_normal;
};
struct BombTraceProvider {
    void* context;
    // EngineTrace::TraceRay, mask 0x200400B. The filter ignores bomb_entity.
    BombGroundResult (*trace)(void*, const BombDamageRay&, std::uint32_t mask,
                              const void* bomb_entity);
};
// 0x433CD3A7. A null bomb represents an empty/missing engine bomb registry.
float bomb_damage(const ActiveBomb* bomb, const BombDamageTarget& target, float peak_damage,
                  BombTraceProvider trace);

} // namespace skeet::esp
