#pragma once
#include "grenade_damage.hpp"

namespace skeet::esp {
struct GrenadeWarningRules {
    bool friendly_fire, teammates_are_enemies;
    float self_he_damage, teammate_he_damage, teammate_fire_damage;
    // Concrete leaves 433DF22C / 433A104B, decoded game-rules bytes.
    bool game_player_override, teammate_damage_disabled;
    std::int32_t game_type;
};
struct GrenadeWarningSnapshot {
    std::uint32_t identity = 0, reference_handle = 0xffffffffu, native_class = 0;
    std::int32_t index = 0;
    bool player = false, dormant = false;
    std::int32_t team = 0, survival_team = 0, cached_team = 0;
    bool cached_team_valid = false;
    Vector3 origin{}, virtual_origin{}, velocity{}, initial_velocity{};
    std::uint32_t thrower_handle = 0xffffffffu, owner_handle = 0xffffffffu;
    std::uint32_t flags = 0; // 38CA5228
    std::int32_t explosion_effect_tick = 0, inferno_start_tick = 0;
    std::int32_t collision_group = 13; // E4E2D1A7
    std::uint16_t projectile_flags = 0; // 7418868F
    float damage = 0, simulation_time = 0;
    std::span<const Vector3> fire_cells; // decoded active world-space inferno cells
};
struct GrenadeWarningClasses {
    std::uint32_t base_projectile, molotov_projectile, inferno, smoke_projectile;
};
struct GrenadeWarningPacketUpdate {
    std::uint32_t index, state;
};
struct GrenadeWarningPartitionEntity {
    // Actual IHandleEntity +08 -> entity-list +10 -> entity +1C results.
    bool resolved, base_entity_present;
    std::uint32_t native_class; // networkable +08
    Vector3 origin; // base entity +28
    bool smoke_active; // 58AE2423
};
class GrenadeWarningEngine : public GrenadeDamageEngine {
public:
    // Actual entity-list +10/+0C and decoded engine fields/virtual results.
    virtual bool snapshot_for_handle(std::uint32_t, GrenadeWarningSnapshot&) = 0;
    virtual bool snapshot_for_index(std::uint32_t, GrenadeWarningSnapshot&) = 0;
    // Actual spatial-partition +34 candidate order. Class filtering and the
    // native collector's sixteen accepted entities stay in source below.
    virtual std::span<const GrenadeWarningPartitionEntity>
    partition_entities(Vector3 center, float radius, std::uint32_t mask, bool coarse) = 0;
};
struct GrenadeWarningUpdateFrame {
    const GrenadeWarningSnapshot* local; // param2, selected damage target
    const GrenadeDamagePlayer* local_damage;
    const GrenadeWarningSnapshot* actual_local; // global43475808, damage policy only
    std::int32_t tick_count;
    float current_time; // *43467EB8
    float engine_time; // actual EngineClient virtual +38
    float engine_clock_interval; // actual client-state +20
    bool grenade_entities_enabled; // global43467CD4
    std::uint32_t command_buttons; // global434759C0
    GrenadeWarningRules rules;
    GrenadeWarningClasses classes;
    std::span<const GrenadeWarningPacketUpdate> packet_updates;
};
struct GrenadeWarningOwner {
    std::vector<GrenadeWarningRecord> records;
    // Native +B0 scratch vector is reset for each eligible existing projectile.
    std::vector<Vector3> projectile_points;
    std::uint32_t command_buttons = 0; // updated global434759C0 value
};
std::int32_t grenade_warning_team(const GrenadeWarningSnapshot&, std::int32_t game_type) noexcept;
bool grenade_warning_expired(const GrenadeWarningRecord&, bool entity_present,
                             std::int32_t explosion_effect_tick, std::int32_t tick_count,
                             float tick_interval) noexcept;
bool grenade_warning_admitted(std::uint32_t type, const GrenadeWarningSnapshot* thrower,
                              const GrenadeWarningSnapshot& local,
                              const GrenadeWarningRules&) noexcept;
// Post-exposure policy 4333B6DC..4333B8B0 deliberately reads ordinary team,
// unlike the admission gate's dormancy/survival-aware 43341683.
float grenade_warning_damage_scale(std::uint32_t type, const GrenadeWarningSnapshot* thrower,
                                   const GrenadeWarningSnapshot& local,
                                   const GrenadeWarningRules&) noexcept;
float grenade_projectile_elapsed(float initial_z, float current_z, float gravity,
                                 float tick_interval) noexcept;
bool grenade_warning_smoke_overlap(Vector3 fire, Vector3 smoke) noexcept;
// Shared43337BC0 tail / command-owner4338AD80 assessment. The caller owns
// config/local-state gates and invalid-handle mutation; prior damage is kept
// until the HE protection comparison. Type2 reads active inferno cells.
void assess_grenade_warning_record(GrenadeWarningRecord&, const GrenadeWarningUpdateFrame&,
                                   GrenadeWarningEngine&, GrenadeTraceProvider,
                                   const GrenadeDamageMath&, std::uint32_t& command_buttons);
// Complete remote record owner 43337BC0: stable purge, existing endpoint
// refresh, packet admission, concrete simulation, HE exposure and fire policy.
void update_grenade_warning_records(GrenadeWarningOwner&, const GrenadeEspConfig&,
                                    const GrenadeWarningUpdateFrame&, const GrenadeEnvironment&,
                                    GrenadeWarningEngine&, GrenadeTraceProvider,
                                    const GrenadeDamageMath&);
// Live entity-list resolution followed by the recovered433BF4A8 warning owner.
void draw_grenade_warning_records(ui::RenderQueue&, const GrenadeEspConfig&, const Projection&,
                                  GrenadeWarningOwner&, GrenadeWarningEngine&,
                                  const GrenadeWarningFrame&,
                                  GrenadeWarningTexture explosive_texture,
                                  GrenadeWarningTexture fire_texture);
} // namespace skeet::esp
