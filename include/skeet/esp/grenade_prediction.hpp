#pragma once
#include "other_config.hpp"
#include "projection.hpp"
#include "skeet/render_queue.hpp"
#include <span>
#include <vector>

namespace skeet::effects {
struct GlowEngine;
}
namespace skeet::ui {
class RendererBackend;
}
namespace skeet::rage {
struct TargetBindings;
}
namespace skeet {
struct MovementSimulationPlayer;
struct MovementSimulationRules;
struct MovementSimulationEnvironment;
struct MovementAngles;
class MovementCollisionEngine;
}
namespace skeet::esp {
struct GrenadeEnvironmentBindings;

enum class GrenadeKind : std::int16_t {
    flash = 43,
    explosive = 44,
    smoke = 45,
    molotov = 46,
    decoy = 47,
    incendiary = 48,
    tactical = 82,
    diversion = 83
};

// Engine entity fields used by 0x43331B86/0x4336DA23. Class identifiers are
// runtime ClientClass pointers, compared with the engine's registered classes.
struct GrenadeHitEntity {
    std::uint32_t identity = 0, handle = 0xffffffffu, native_class = 0;
    bool player = false;
    bool incendiary_contact_exempt = false; // entity virtual +0x284
    std::uint8_t movement_type = 0, state = 0, collision_state = 0;
    std::uint32_t flags = 0, solid_flags = 0;
    std::int32_t solid_type = 0;
    std::uint32_t owner_handle = 0xffffffffu;
    float collision_activity[2]{}; // decoders 8DBC9874 and 68140CD8
};
struct GrenadeRay {
    Vector3 start, delta, extents;
    bool swept, is_ray;
};
struct GrenadeTrace {
    Vector3 end, normal;
    float fraction = 1;
    std::uint32_t contents = 0;
    std::uint16_t surface_flags = 0;
    bool all_solid = false, start_solid = false;
    GrenadeHitEntity entity;
};
struct GrenadeTraceFilter {
    std::uint32_t ignored_entity, ignored_handle;
    std::int32_t collision_group;
    std::uint16_t projectile_flags = 0; // current raw bounce word, native filter+16
    GrenadeKind kind = static_cast<GrenadeKind>(0); //native filter+14
};
struct GrenadeTraceProvider {
    void* context;
    // The only callable boundary: EngineTrace::TraceRay. Entity fields above
    // are decoded engine output; prediction, retries and collision stay here.
    GrenadeTrace (*trace)(void*, const GrenadeRay&, std::uint32_t, const GrenadeTraceFilter&);
    std::uint8_t* trace_flag = nullptr; // optional engine byte at native 434683D0
};
struct GrenadeFire {
    Vector3 origin;
    // World positions of active fire cells, from the entity's integer offsets.
    std::span<const Vector3> cells;
};
struct GrenadeEnvironment {
    float tick_interval, frame_interval, gravity;
    float incendiary_duration, incendiary_max_slope;
    std::uint32_t world_entity;
    // Native registry slots 4346A440, 43467478 and 43468CA0 respectively.
    std::uint32_t breakable_class, physics_class, alternate_breakable_class;
    std::span<const GrenadeFire> fires;
};
struct GrenadeState {
    GrenadeKind kind;
    Vector3 position, velocity, last_plane;
    float time = 0, detonate_time = 0;
    std::int32_t next_think_tick = 0, collision_group = 13;
    std::uint32_t owner_entity = 0, ignored_handle = 0xffffffffu;
    std::int16_t bounces = 0;
    bool fire_ground = false, extinguished = false, sky = false;
    bool hit_world = false, finalized = false, collided = false;
};
struct GrenadeScreenPoint {
    float x, y;
};
struct GrenadePath {
    std::vector<GrenadeScreenPoint> points, bounces;
};
struct GrenadeThrowInput {
    Vector3 eye_position, player_velocity, view_angles, aim_punch;
    float strength, throw_velocity, recoil_scale;
    bool compensate_movement;
    std::uint32_t owner_entity;
    bool zero_slow_movement = true; //433BA5DF clears speed<10; command4348675E retains it
};
struct GrenadeLaunch {
    Vector3 position, velocity, angles;
    float strength, speed;
};
enum class GrenadeCompensation {
    exact,
    limited,
    invalid_input
};
// Reworked433E76A9: preserve the stationary launch direction after the game's
// pitch remapping and inherited velocity. Limited solutions minimize angular
// error within legal command angles; they do not promise an unreachable throw.
GrenadeCompensation compensate_grenade_movement(Vector3 velocity, float speed,
                                                Vector3& angles) noexcept;
// Launch chain 433BAB00..433BB16C: strength/speed clamps, pitch adjustment,
// optional recoil/movement compensation, 22-unit hull trace, six-unit setback.
GrenadeLaunch prepare_grenade_throw(const GrenadeThrowInput&, GrenadeTraceProvider);
struct GrenadeTrajectoryOwner {
    Vector3 smoothed_velocity{};
    GrenadeState state{};
    GrenadePath path;
    GrenadeLaunch launch{};
    Vector3 drawing_position{}; //433BB1C9 projects before HE/flash +1Z
};
struct GrenadeTrajectoryFrame {
    // Actual engine virtual +148/+174 and helper433DF22C read results.
    bool engine_reject_148, engine_reject_174, game_player_override;
    std::uint32_t player_entity;
    std::uint8_t movement_type, life_state;
    bool selected_is_local;
    float field_e23a9be1;
    bool weapon_present;
    GrenadeKind kind;
    std::int32_t field_9e2078a1;
    float throw_time;
    // Owner's cached input vector +100, and player byte D070684C.
    Vector3 motion_target;
    bool cached_motion;
    std::int32_t motion_tick, prediction_tick, prediction_shift;
    bool two_tick_preroll;
    GrenadeThrowInput throw_input;
};
struct GrenadeMovementContext {
    Vector3 origin;
    std::uint32_t player_flags;
    const MovementSimulationPlayer* player;
    const MovementSimulationRules* rules;
    const MovementSimulationEnvironment* environment;
    const UserCommand* command;
    const MovementAngles* command_angles;
    MovementCollisionEngine* collision;
};
// 433BA600..433BA845, including unconditional cached-velocity smoothing.
bool begin_grenade_trajectory(GrenadeTrajectoryOwner&, const GrenadeEspConfig&,
                              const GrenadeTrajectoryFrame&) noexcept;
// Actual caller composition, including concrete movement-simulation preroll.
// Movement context is required when the native cached-motion/tick gate passes.
bool predict_grenade_trajectory(GrenadeTrajectoryOwner&, const GrenadeEspConfig&,
                                const GrenadeTrajectoryFrame&, const GrenadeMovementContext&,
                                const GrenadeEnvironment&, GrenadeTraceProvider, const Projection&);
struct GrenadeDamagePlayer;
class GrenadeDamageEngine;
struct GrenadeDamageMath;
struct GrenadeAuxiliaryContext;
struct GrenadeDamageCandidate {
    const GrenadeDamagePlayer* player;
    const void* entity; // actual engine pointer used by the shared enemy predicate
    bool is_player;
    std::uint8_t life_state, field_d070684c, field_cde290d2, field_af649dac;
    bool dormant;
    float history_health;
    Vector3 history_origin;
    std::int32_t health; // actual virtual +1E8 result
};
struct GrenadeDamageSelection {
    float damage = 0;
    bool reached_threshold = false;
    std::int32_t target_index = -1;
};
// Native 433BE1E3..433BE2A8; returns true when the caller stops scanning.
bool consider_grenade_damage(GrenadeDamageSelection&, float damage, std::int32_t armor,
                             std::int32_t health, std::int32_t threshold,
                             std::int32_t target_index) noexcept;
// Candidate storage follows engine client indices, including index zero.
GrenadeDamageSelection select_grenade_damage(Vector3 explosion,
                                             std::span<const GrenadeDamageCandidate>,
                                             std::int32_t maximum_players, std::int32_t threshold,
                                             const rage::TargetBindings&, GrenadeDamageEngine&,
                                             const GrenadeDamageMath&);

// Constructor 4338A122 and simulation 433684B2. Inputs are the engine's
// projectile origin/velocity and absolute projectile/creation times.
// world_path uses derived4331FC5B: append after each tick while time>=0,
// before the time increment. Its owner supplies initial/final endpoints.
void simulate_grenade(GrenadeState& state, GrenadeKind kind, Vector3 origin, Vector3 velocity,
                      float current_time, float creation_time,
                      const GrenadeEnvironment& environment, GrenadeTraceProvider trace,
                      const Projection* projection = nullptr, GrenadePath* path = nullptr,
                      std::int32_t collision_group = 13, std::uint16_t projectile_flags = 0,
                      std::vector<Vector3>* world_path = nullptr);
void simulate_grenade(GrenadeState& state, GrenadeKind kind, Vector3 origin, Vector3 velocity,
                      float current_time, float creation_time, const GrenadeEnvironmentBindings&,
                      GrenadeTraceProvider, const Projection* projection = nullptr,
                      GrenadePath* path = nullptr, std::int32_t collision_group = 13,
                      std::uint16_t projectile_flags = 0,
                      std::vector<Vector3>* world_path = nullptr);
// Full tick chain 433B7469 -> 433F541B -> 43331B86 -> 4336DA23.
void step_grenade(GrenadeState& state, const GrenadeEnvironment& environment,
                  GrenadeTraceProvider trace, const Projection* projection = nullptr,
                  GrenadePath* path = nullptr);
void step_grenade(GrenadeState&, const GrenadeEnvironmentBindings&, GrenadeTraceProvider,
                  const Projection* projection = nullptr, GrenadePath* path = nullptr);
// 43369342; HE final ground placement and incendiary ground search.
void finalize_grenade(GrenadeState& state, GrenadeTraceProvider trace);
// Float projection callbacks 43396CE8 and 4337CF74.
bool project_grenade_point(const Projection& projection, Vector3 position,
                           GrenadeScreenPoint& output) noexcept;
void record_grenade_point(GrenadePath& path, const Projection& projection, Vector3 position,
                          bool bounce);
// 433BF2F2..433BF498: path, collision circles and detonation circle.
void draw_grenade_path(ui::RenderQueue& queue, const GrenadeEspConfig& config,
                       const GrenadePath& path, const Projection& projection,
                       Vector3 detonation_position, std::int32_t damage);
// Native enabled gate and endpoint projection have already occurred. Keeping
// that cached point avoids re-reading the matrix after target engine calls.
void draw_active_grenade_path(ui::RenderQueue&, const GrenadeEspConfig&, const GrenadePath&,
                              const GrenadeScreenPoint* endpoint, std::int32_t magnitude);

struct GrenadeEntityClasses {
    std::int32_t base_projectile, decoy_projectile, molotov_projectile, smoke_projectile, inferno;
};
struct GrenadeEntityLabels {
    std::u16string_view explosive, flash, smoke, decoy, fire, other;
};
struct GrenadeEntityState {
    void* entity;
    std::int32_t class_index, glow_index;
    Vector3 origin;
    float damage, damage_radius;
    std::int32_t explosion_effect_tick;
    bool smoke_effect;
    std::int32_t smoke_start_tick, inferno_start_tick;
};
// Full 4332226A. Runtime class IDs and mutable labels come from the owner
// registry; suspended rendering retains the ordinary opaque label.
void draw_grenade_entity(ui::RenderQueue&, const GrenadeEspConfig&, const Projection&,
                         const GrenadeEntityClasses&, const GrenadeEntityLabels&,
                         const GrenadeEntityState&, std::int32_t current_tick, float tick_interval,
                         bool suspended, effects::GlowEngine* glow);
struct InfernoTeamState {
    bool friendly_fire, owner_present;
    std::int32_t local_team, owner_team;
};
// 43393ECC. The caller supplies a local player, as the native dispatcher does;
// owner fields are the resolved m_hOwnerEntity engine snapshot.
void draw_inferno_entity(ui::RenderQueue&, const GrenadeEspConfig&, const Projection&,
                         const GrenadeEntityClasses&, const GrenadeEntityLabels&,
                         const GrenadeEntityState&, std::int32_t current_tick, float tick_interval,
                         bool suspended, effects::GlowEngine* glow, const InfernoTeamState&);

struct GrenadeWarningRecord {
    std::uint32_t handle;
    Vector3 position;
    float damage;
    std::uint32_t type;
    GrenadeKind kind;
    bool suppressed;
    float creation_time, end_time;
};
struct GrenadeWarningEntity {
    bool present, effects_bit5; //38CA5228; distinct from networkable dormancy
    std::int32_t explosion_effect_tick;
    Vector3 origin;
    std::span<const Vector3> fire_cells;
};
struct GrenadeWarningFrame {
    bool local_present;
    std::uint8_t life_state, movement_type;
    std::int32_t observer_mode, health, width, height, tick_count;
    Vector3 local_position;
    float tick_interval, interpolation;
};
struct GrenadeWarningTexture {
    std::uint32_t texture = 0xffffffffu;
    std::int32_t width = 0, height = 0;
};
struct GrenadeWarningLiveFrame {
    void* player;
    ui::RendererBackend& renderer;
    const std::int32_t& tick_count;
    const float& tick_interval;
    const float& interpolation;
    const GrenadeWarningTexture& explosive_texture;
    const GrenadeWarningTexture& fire_texture;
};
bool project_grenade_warning(const Projection&, Vector3, Point&,
                             bool& in_front) noexcept; // 433AB8BD
// 433BF806..433BFD94: complete proximity-warning render stage.
void draw_grenade_warning(ui::RenderQueue&, const Projection&, const GrenadeWarningRecord&,
                          float distance, const GrenadeWarningFrame&, GrenadeWarningTexture);
void draw_grenade_warning(ui::RenderQueue&, const Projection&, const GrenadeWarningRecord&,
                          float distance, const GrenadeWarningLiveFrame&);
// 433BF4A8..433BF806: live entity/life-state gates and closest fire-cell range.
// Entities correspond to records in engine iteration order. Missing/inactive
// entities invalidate their records exactly as the native owner does.
void draw_grenade_warnings(ui::RenderQueue&, const GrenadeEspConfig&, const Projection&,
                           std::span<GrenadeWarningRecord>, std::span<const GrenadeWarningEntity>,
                           const GrenadeWarningFrame&, GrenadeWarningTexture explosive_texture,
                           GrenadeWarningTexture fire_texture);

struct GrenadePredictionScene {
    const Projection& projection;
    const GrenadeEnvironment& environment;
    GrenadeTraceProvider trace;
    const GrenadeMovementContext& movement;
    const rage::TargetBindings& targeting;
    GrenadeDamageEngine& damage_engine;
    const GrenadeDamageMath& damage_math;
    const GrenadeAuxiliaryContext& auxiliary;
    std::span<const GrenadeDamageCandidate> damage_candidates;
    std::int32_t maximum_players, damage_threshold;
    std::span<GrenadeWarningRecord> warning_records;
    std::span<const GrenadeWarningEntity> warning_entities;
    const GrenadeWarningFrame& viewer;
    GrenadeWarningTexture explosive_texture, fire_texture;
};
struct GrenadePredictionDrawResult {
    bool trajectory = false;
    GrenadeDamageSelection damage;
};
// Concrete owner presentation: predicts the selected throw, performs native HE
// target selection, draws trajectory/damage and updates proximity warnings.
// Engine mode exits precede both overlays, but retain velocity smoothing.
GrenadePredictionDrawResult render_grenade_prediction(ui::RenderQueue&, const GrenadeEspConfig&,
                                                      GrenadeTrajectoryOwner&,
                                                      const GrenadeTrajectoryFrame&,
                                                      const GrenadePredictionScene&);

namespace grenade_detail {
GrenadeTrace move(GrenadeState&, Vector3 displacement, GrenadeTraceProvider);
GrenadeTrace move(GrenadeState&, Vector3 displacement, GrenadeTraceProvider,
                  const GrenadeEnvironmentBindings&);
void collide(GrenadeState&, GrenadeTrace&, Vector3& displacement, const GrenadeEnvironment&,
             GrenadeTraceProvider);
void collide(GrenadeState&, GrenadeTrace&, Vector3& displacement, const GrenadeEnvironmentBindings&,
             GrenadeTraceProvider);
}
} // namespace skeet::esp
