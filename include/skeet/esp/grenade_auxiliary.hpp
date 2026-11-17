#pragma once
#include "grenade_damage.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <string>

namespace skeet::esp {
struct GrenadeFireDistanceContext {
    const rage::TargetBindings& targeting;
    std::int32_t maximum_players;
    const void* smoke_projectile_class;
    std::ptrdiff_t smoke_active;
};
struct GrenadeAuxiliaryContext {
    const rage::TargetBindings& targeting;
    GrenadeDamageEngine& engine;
    RageTrigonometry trigonometry;
    // Actual imported __libm_sse2_atan2 boundary (77509EE0).
    double (*arc_tangent)(double y, double x);
    std::int32_t maximum_players;
    const void* smoke_projectile_class; // runtime ClientClass at43468238
    std::ptrdiff_t smoke_active; // decoded58AE2423
    std::ptrdiff_t view_angles; // decodedE23A9BE1
    std::uint8_t* trace_flag; // actual434683D0 byte
};
struct GrenadeAuxiliaryLiveOffsets {
    explicit GrenadeAuxiliaryLiveOffsets(const engine::NativeOffsetGlobals&);
    std::array<engine::DecodedOffset, 4> rejection_bytes;
    engine::DecodedOffset team, survival_team, origin, duck_amount, smoke_active, view_angles;
};
struct GrenadeAuxiliaryLiveContext {
    const players::State& player_adjustments;
    engine::NativeConVarReference game_type;
    engine::NativeConVarReference teammates_are_enemies;
    void* const& local_player; //43475808, sampled at the enemy-predicate site
    void* const& entity_list; //4346A890, reloaded for each engine visit
    GrenadeDamageEngine& engine;
    RageTrigonometry trigonometry;
    double (*arc_tangent)(double y, double x);
    const std::int32_t& maximum_players; //sampled once at each helper entry
    const void* const& smoke_projectile_class; //43468238, live after SDK calls
    GrenadeAuxiliaryLiveOffsets offsets;
    std::uint8_t* trace_flag;
};
struct GrenadeFlashResult {
    void* target = nullptr;
    float distance_squared = 9000000.0f;
    float exposure = 0.0f;
    std::int32_t percent = 0;
};
struct GrenadeAuxiliaryResult {
    std::int32_t magnitude = 0;
    std::u16string annotation;
    bool highlight = false;
    bool draw_path = true;
};
// 433BE3C4..433BF139. position is the already raised endpoint (+1Z), as in
// the native owner. Eligibility, nearest-player choice, hull height, all seven
// possible trace calls and view-facing attenuation are concrete here.
GrenadeFlashResult evaluate_grenade_flash(Vector3 position, std::uint32_t ignored_entity,
                                          const GrenadeAuxiliaryContext&);
GrenadeFlashResult evaluate_grenade_flash(Vector3 position, std::uint32_t ignored_entity,
                                          const GrenadeAuxiliaryLiveContext&);
// Complete4338385A. A missing nearby target or overlapping live smoke leaves
// both caller-owned outputs untouched; the highlight byte is only set true.
void evaluate_grenade_fire_distance(const GrenadeState&, const GrenadeAuxiliaryContext&,
                                    std::int32_t& distance, bool& highlight);
void evaluate_grenade_fire_distance(const GrenadeState&, const GrenadeFireDistanceContext&,
                                    std::int32_t& distance, bool& highlight);
void evaluate_grenade_fire_distance(const GrenadeState&, const GrenadeAuxiliaryLiveContext&,
                                    std::int32_t& distance, bool& highlight);
// Non-HE presentation433BE36D..433BF2EC; magnitude selects path/circle color.
GrenadeAuxiliaryResult evaluate_grenade_auxiliary(const GrenadeState&,
                                                  const GrenadeAuxiliaryContext&);
GrenadeAuxiliaryResult evaluate_grenade_auxiliary(const GrenadeState&,
                                                  const GrenadeAuxiliaryLiveContext&);
} // namespace skeet::esp
