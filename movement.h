#pragma once

// Movement Subsystem Declarations
#include "skeet/client/commands.hpp"
#include "skeet/rage/ballistics.hpp"
#include "skeet/config.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/esp/projection.hpp"
#include <vector>
#include "skeet/engine/startup_convars.hpp"
#include <variant>
#include "skeet/engine/trace.hpp"
#include "skeet/client/command_orchestrator.hpp"
#include "skeet/prediction/history.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/client/player_state.hpp"
#include <optional>
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/hud/movement_indicators.hpp"
#include "skeet/movement_config.hpp"
#include <array>
#include "skeet/core.hpp"
#include "skeet/movement/types.hpp"
#include <bit>
#include <cmath>


// Declarations from acceleration.hpp

namespace skeet {
struct GroundAcceleration {
    float amount;
    float wish_speed;
    float acceleration;
};

// Complete arithmetic helpers 0x4339E713 and 0x433AC9C0. Direction is supplied
// by their caller; these functions do not normalize it implicitly.
float air_acceleration_delta(MovementVector velocity, MovementVector direction, float wish_speed,
                             float acceleration, float interval_per_tick) noexcept;
GroundAcceleration ground_acceleration(const MovementPhysics& physics, const UserCommand& command,
                                       MovementVector velocity, MovementVector direction,
                                       float wish_speed) noexcept;

struct GroundAccelerationCapacity {
    float amount;
    bool ducking;
};
// Complete helper 0x433EE5FC, including prediction of the duck transition.
GroundAccelerationCapacity ground_acceleration_capacity(const MovementPhysics& physics,
                                                        const UserCommand& command,
                                                        MovementVector velocity,
                                                        MovementVector direction) noexcept;
} // namespace skeet

// Declarations from air_feedback.hpp

namespace skeet {

// Raw pre-speed guard path4337C573..4337C636, before ordinary final steering.
// The caller uses smoothing100 for this path. Only yaw can change here.
void apply_guarded_pre_speed_heading(float speed, float velocity_yaw, float previous_heading,
                                     float maximum_angle, MovementAngleView) noexcept;

// Complete feedback overshoot decision4337C3D0..4337C549 after the selected
// pre-speed heading has been committed. True goes directly to the ground-speed
// tail, retaining side_flip and the raw heading; false uses ordinary steering
// with smoothing0. The feedback heading alone is normalized on the true edge.
bool resolve_pre_speed_feedback(float previous_heading, float ideal_step, float turn_limit,
                                float side_magnitude, MovementFeedback&, UserCommand&,
                                ConstMovementAngleView) noexcept;

} // namespace skeet

// Declarations from air_input.hpp

namespace skeet {

// Complete solver input-heading stages 0x43373CBC..0x43373F63 and
// 0x43379B3B..0x43379C59. The boolean selects their proven basis behavior.
// False continues at the ground-speed tail; command writes before that edge
// are preserved. The caller has already selected the airborne steering path.
bool prepare_air_strafe_heading(const AirStrafeSelection& selection, MovementVector velocity,
                                bool normalized_basis, UserCommand& command,
                                MovementAngleView movement_angles) noexcept;

} // namespace skeet

// Declarations from air_steering.hpp

namespace skeet {

struct AirSteeringInput {
    float velocity_yaw;
    float ideal_step;
    float speed;
    float side_magnitude;
    std::int32_t smoothing;
    bool large_turn;
};

struct AirSteeringState {
    bool side_flip; // command feedback + 0
    float feedback_heading; // command feedback + 8
};

// Complete final air-steering chain 0x4337C54D..0x4337C883. Simulation and
// collision stages supply the inputs; all command/state mutations stay here.
void write_air_strafe_step(const AirSteeringInput& input, AirSteeringState& state,
                           UserCommand& command, MovementAngleView movement_angles) noexcept;

} // namespace skeet

// Declarations from air_strafe.hpp

namespace skeet {

struct AirStrafeSettings {
    std::int32_t direction;
    bool pre_speed;
    HotkeyBinding pre_speed_hotkey;
};

struct AirStrafeSelection {
    bool continue_solver;
    bool view_angles;
    // The native intermediate retains the upper 24 bits of direction >> 1.
    // Its low byte is the movement-keys selection and can be cleared alone.
    std::uint32_t movement_key_mode;
    bool pre_speed_requested;
    bool pre_speed_effective;
};

// Complete mode-selection block 0x433732C6..0x4337338B. The false continuation
// goes to the ground-speed tail at 0x4337C899. pre_speed_guard_enabled is the
// decoded guard-list comparison at 0x43373317, separate from the hotkey.
AirStrafeSelection select_air_strafe_modes(const AirStrafeSettings& settings, InputBitmap input,
                                           bool pre_speed_guard_enabled,
                                           const UserCommand& command);

} // namespace skeet

// Declarations from air_velocity.hpp

namespace skeet {

struct AirVelocityPreparation {
    MovementVector velocity;
    float stamina;
    bool continue_solver;
};

// Complete normalized-basis solver stage43373984..43373CBC. The native
// airborne path applies its first half-gravity step here; a grounded jump
// also recovers stamina, applies the speed cap and duck/stamina jump scaling.
// Grounded no-jump and low-speed paths continue at the ground-speed tail.
AirVelocityPreparation prepare_air_strafe_velocity(MovementVector velocity, float stamina,
                                                   const MovementPhysics&,
                                                   const MovementSimulationRules&,
                                                   bool movement_keys, bool bunny_hop_enabled,
                                                   bool jump_speed_cap_enabled,
                                                   float side_magnitude, UserCommand&) noexcept;

} // namespace skeet

// Declarations from air_wall_steering.hpp

namespace skeet {

struct AirWallSteeringInput {
    bool enabled; // movement owner +0x158
    std::int32_t prediction_milliseconds; // movement owner +0x16C
    float angle_scale, maximum_angle, side_magnitude; // +0x1C0/+0x1C8/+0x1C4
    std::uint32_t player;
    MovementVector position, velocity;
    std::uint32_t player_flags;
    std::int32_t weapon_handle;
};

struct AirWallSteeringBindings {
    const MovementSimulationPlayer& snapshot;
    const MovementSimulationRules& rules;
    const MovementSimulationEnvironment& environment;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* entity_list; // actual entity-list virtual +0x10
    void* surface_properties; // actual surface-properties virtual +0x14
};

struct AirWallSteeringLiveTuning {
    const std::int32_t& prediction_milliseconds; // owner+16C, after first hull getters
    const std::int32_t& smoothing; // owner+68, each normalized trial
    const float& angle_scale; // owner+1C0, after constructor
    const float& maximum_angle; // owner+1C8
    const float& side_magnitude; // owner+1C4, each prediction tick
    const float& acceleration_limit; // owner+1D0, after speed getter
    const float& angle_step; // owner+1D4
};

struct AirWallSteeringLiveBindings {
    void* player;
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& capture;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* const& surface_properties; // reload current43469268 after the first trace
    const MovementVector&
        origin; // actual earlier GetAbsOrigin result / secondary simulation position
    const MovementVector& velocity; // raw retained velocity pointer / normalized prepared local
    AirWallSteeringLiveTuning tuning;
};

// Complete raw-basis wall-steering block 0x43379C59..0x4337A829. The caller
// has selected the airborne raw-basis path and projected its input heading.
// False is the 0x4337A829 bypass; true reaches the completed 0x4337A816 edge.
// Only movement_angles.yaw is written. Prediction uses the concrete movement
// simulation; EngineTrace, surface data and entity access remain real boundaries.
bool steer_air_wall(const AirWallSteeringInput&, const MovementFeedback&, const client::Command&,
                    MovementAngleView movement_angles, const AirWallSteeringBindings&);

// Actual command path. The native pre-constructor hull getters, constructor
// captures, per-tick reads, weapon lookup and final mask getter execute live.
// Input configuration values, player/position/player_flags/weapon_handle belong
// only to the typed API above. The live path uses input.enabled and the cached
// velocity seed; constructor velocity and late tuning come from live bindings.
bool steer_live_air_wall(const AirWallSteeringInput&, const MovementFeedback&,
                         const client::Command&, MovementAngleView movement_angles,
                         const AirWallSteeringLiveBindings&);

} // namespace skeet

// Declarations from blockbot.hpp

namespace skeet {
struct BlockbotConfig {
    BooleanConfig enabled; // native phase owner +0C, value +1C
    HotkeyConfig hotkey; // +20, registered binding +30
};
struct BlockbotState {
    MovementVector locked_right{}; // native owner +48
    std::uint8_t activated = 0; // +54; reset only by the entry gates
    std::int32_t target_handle = -1; // +58
    std::uint8_t flags = 0; // +5C; bit0 marks successful prediction
};
// Native constructor43480261 registers the boolean, then an Escape/hold
// hotkey. The preexisting boolean value is preserved.
void initialize_blockbot_config(ConfigRegistries&, BlockbotConfig&) noexcept;
// Exact sparse runtime writes434802BC..434802CC: only the low byte of
// locked_right.x is cleared; its other bytes/y/z survive construction.
void initialize_blockbot_state(BlockbotState&) noexcept;
struct BlockbotOffsets {
    std::ptrdiff_t move_type; // 7501CB91, byte
    std::ptrdiff_t ground_flags; // E2FBE695, byte
    std::ptrdiff_t team; // 8B5586F6, int32
    std::ptrdiff_t view_offset; // C246315E, vector
    std::ptrdiff_t position_history; // 3CDE8B30, engine interpolation history
    std::ptrdiff_t network_origin; // A9EE6062, vector
    std::ptrdiff_t maximum_speed; // 3EB71843, float
    std::ptrdiff_t rejected_9d920e35, rejected_cde290d2, rejected_d070684c; // bytes
    std::ptrdiff_t rule_d720aea6; // byte
};
struct BlockbotBindings {
    void* engine_client; // 4346A514, actual GetViewAngles slot18
    void* entity_list; // 4346A890, actual index/handle slots3/4
    void* const* game_rules; // 4346A3A4, both levels optional
    const std::int32_t* max_clients; // 43469300
    const lagcomp::TickTimingSamples* timing; // canonical43478C70..78
    const float* interval; // 4346A338, read on each predicted tick
    MovementCollisionEngine* collision; // actual EngineTrace::TraceRay
    std::uint8_t* trace_flag; // optional native trace guard target
    BlockbotOffsets offsets;
};
// Complete valid-player433B92DC. Config value/hotkey are native owner+1C/+30.
// Includes concrete teammate scoring433EA084, center433FB98A and three-sample
// finite differences433B171A. Requires a nonnull player once enabled and hotkey
// active, matching the original deliberate invalid-instruction null paths.
// Retains a usable target, projects its velocity through hull traces, and
// adjusts only side_move and movement_angles; command view angles stay intact.
void process_blockbot(BlockbotState&, bool enabled, const HotkeyBinding&, InputBitmap, void* player,
                      client::Command&, MovementAngleView, const BlockbotBindings&);
} // namespace skeet

// Declarations from blockbot_owner.hpp

namespace skeet {
class BlockbotOwner;
std::size_t blockbot_owner_size() noexcept;
// Whole43480261: primary, phase2, HUD registration; all table assignments;
// canonical descriptors; sparse state initialization. The incoming storage is
// a native43387448 allocation and primary slot0 frees that header exactly.
// settings must be unregistered; state/configuration/bindings remain shared.
BlockbotOwner& construct_blockbot_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                                        client::CommandPhaseRegistry& commands,
                                        engine::OwnerRegistryBinding hud, ConfigRegistries&,
                                        BlockbotConfig&, BlockbotState&, const BlockbotBindings&,
                                        const esp::Projection&,
                                        const std::uint32_t& life_state_offset);
} //namespace skeet

// Declarations from bunny_hop.hpp

namespace skeet {
struct BunnyHopGuardNode {
    const BunnyHopGuardNode* next;
    const BunnyHopGuardNode* release_marker;
};
// A standalone semantic chain and a real encoded ConVar are distinct layouts.
// Production rereads the encoded pointer, parent+1C and integer+30 only after
// the airborne/enabled gates. No synthetic node snapshot represents a ConVar.
class BunnyHopGuard {
public:
    BunnyHopGuard(const BunnyHopGuardNode& node) noexcept : source_(&node) {}
    BunnyHopGuard(engine::NativeConVarReference variable) noexcept : source_(variable) {}
    bool releases_jump() const noexcept;

private:
    std::variant<const BunnyHopGuardNode*, engine::NativeConVarReference> source_;
};
// Native node chain must terminate in a self-linked node.
void suppress_airborne_jump(bool enabled, std::uint32_t player_flags,
                            const BunnyHopGuardNode& guard, UserCommand& command) noexcept;
void suppress_airborne_jump(bool enabled, std::uint32_t player_flags, const BunnyHopGuard&,
                            UserCommand&) noexcept;

struct BunnyHopTraceResult {
    float fraction;
    bool flag_a;
    bool flag_b;
};
// These are the two actual recovery sites, called only when their respective
// surrounding movement-solver branches are reached, not a replacement solver.
void recover_bunny_hop_low_speed(bool enabled, const BunnyHopTraceResult& trace,
                                 MovementVector velocity, bool required_state, bool blocked,
                                 UserCommand& command, float forced_move = 450.0f) noexcept;
void recover_bunny_hop_trace(bool enabled, const BunnyHopTraceResult& trace, bool blocked,
                             UserCommand& command, float forced_move = 450.0f) noexcept;
} // namespace skeet

// Declarations from collision.hpp

namespace skeet {

struct MovementCollisionRay {
    MovementVector start, delta, start_offset, extents;
    bool is_ray, swept;
};

struct MovementCollisionTrace {
    MovementVector start, end, plane_normal;
    float plane_distance;
    std::uint8_t plane_type, plane_signs;
    std::uint16_t plane_padding;
    float fraction;
    std::uint32_t contents;
    std::uint16_t displacement_flags;
    std::uint8_t all_solid, start_solid;
    float fraction_left_solid;
    std::uint32_t surface_name;
    std::int16_t surface_property;
    std::uint16_t surface_flags;
    std::int32_t hitgroup;
    std::int16_t physics_bone;
    std::uint16_t physics_padding;
    std::uint32_t entity;
    std::int32_t hitbox;
};
static_assert(sizeof(MovementCollisionTrace) == engine::trace_result_size);
static_assert(offsetof(MovementCollisionTrace, fraction) == engine::trace_fraction_offset);
static_assert(offsetof(MovementCollisionTrace, entity) == engine::trace_entity_offset);

enum class MovementCollisionFilterKind {
    // 0x434727C0: ShouldHitEntity always false, GetTraceType returns zero.
    reject_entities,
    skip_player,
    // 0x434701D0: ShouldHitEntity always false, GetTraceType returns one.
    // Kept distinct from reject_entities, whose native trace type is zero.
    world_only,
};

struct MovementCollisionFilter {
    MovementCollisionFilterKind kind;
    std::uint32_t player;
    std::int32_t collision_group;
    std::uint32_t extra;
};

class MovementCollisionEngine {
public:
    virtual ~MovementCollisionEngine() = default;
    // Actual EngineTrace::TraceRay boundary. Collision decisions stay in source.
    virtual void trace_ray(const MovementCollisionRay&, std::uint32_t mask, MovementCollisionFilter,
                           MovementCollisionTrace&) = 0;
    // Actual surface-properties interface +0x10, friction output only.
    virtual float surface_friction(std::int16_t surface_property) = 0;
};

// Native 0x433AC5BD / 0x4341873D. The engine adapter supplies the optional
// byte pointed to by the native trace flag; nested guards restore in order.
class MovementTraceFlagGuard {
    std::uint8_t* flag_;
    std::uint8_t saved_;

public:
    explicit MovementTraceFlagGuard(std::uint8_t* flag) noexcept;
    ~MovementTraceFlagGuard();
    MovementTraceFlagGuard(const MovementTraceFlagGuard&) = delete;
    MovementTraceFlagGuard& operator=(const MovementTraceFlagGuard&) = delete;
    void restore() noexcept;
};

// Complete wrapper 0x433CC0F2..0x433CC130, including flag restoration.
void trace_movement_world(MovementCollisionEngine& engine, const MovementCollisionRay& ray,
                          std::uint32_t mask, MovementCollisionTrace& trace,
                          std::uint8_t* engine_trace_flag);

MovementCollisionRay make_movement_hull_ray(MovementVector start, MovementVector end,
                                            MovementVector minimum,
                                            MovementVector maximum) noexcept;

} // namespace skeet

// Declarations from command_air.hpp

namespace skeet {

// Main4336F912 air branches after the player gate, ending before sharedC899.
// This owns selection/scout/velocity ordering and calls only concrete live
// wall, pre-speed and normalized-final routines.
void process_live_movement_air(MovementCommandOwner&, void* player, void* weapon, client::Command&,
                               MovementAngleView, const MovementFeedbackHistory&, MovementFeedback&,
                               const MovementCommandBindings&);

} // namespace skeet

// Declarations from command_bindings.hpp

namespace skeet {

struct MovementCommandFields {
    std::uint32_t velocity_36375c23;
    std::uint32_t movement_type_7501cb91;
    std::uint32_t player_state_ac7f2913;
    std::uint32_t stamina_2f33c681;
    std::uint32_t duck_time_04b798c6;
    std::uint32_t weapon_simulation_time_0176a381;
    std::uint32_t player_next_attack_de530a8b;
    std::uint32_t weapon_next_attack_f71ce331;
    std::uint32_t attack_guard_8815d240;
    std::uint32_t weapon_blocked_b8969256;
    std::uint32_t weapon_definition_5c9f0fbb;
    std::uint32_t simulation_mode_3d10b94c;
    std::uint32_t origin_a9ee6062;
    std::uint32_t shot_counter_8cfc633a;
};

// Bind the actual globals/engine objects. None of the stage producers is an
// internal callback; the scheduler calls each one at its recovered read site.
struct MovementCommandBindings {
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& simulation;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* const& engine_client;
    void* const& surface_properties;
    InputBitmap input;
    MovementCommandFields fields;
    const client::CommandBudgetBindings& budget;
    const HotkeyBinding& duck_peek_hotkey;
    const std::uint8_t& infinite_duck;
    const HotkeyBinding& slow_motion_hotkey;
    const HotkeyBinding& command_mark_hotkey; //43467F44/48
    BunnyHopGuard bunny_hop_guard;
    engine::ConVarBinding duck_delay; //43467D7C xor4A285142
    engine::NativeConVarReference pre_speed_guard; //43468EA4 xorFCD2DA7A
    const float& current_time; //*43467EB8
    const float& engine_time; //*4346A334
    const std::int32_t& last_acknowledged; //*43475B2C
    const std::int32_t& history_command; //4346DB1C
    const std::int32_t& history_available; //4346DB2C
    const std::int32_t& weapon_state; //43468488
    const std::uint32_t& weapon_policy; //43467E40
    const std::uint8_t& periodic_gate_4346a740;
    const std::int32_t& periodic_gate_43467f60;
    const std::int32_t& periodic_gate_434685b8;
    client::EntityCommandOwner* const& history_owner; //actual43475898 owner
    std::span<std::uint32_t, 150> marked_commands;
    std::uint32_t& scheduler_flags;
};

std::uint32_t movement_command_player_flags(void*, const MovementCommandBindings&) noexcept;
std::uint8_t movement_command_player_type(void*, const MovementCommandBindings&) noexcept;
MovementVector movement_command_velocity(void*, const MovementCommandBindings&) noexcept;
MovementVector movement_command_position(void*);
const MovementVector& movement_command_position_ref(void*);
std::uint32_t movement_command_trace_mask(void*);
PlayerAirGateBindings capture_player_air_gate_bindings(const MovementCommandBindings&) noexcept;
bool movement_command_weapon_blocked(void*, const MovementCommandBindings&) noexcept;
float movement_command_player_attack(void*, const MovementCommandBindings&) noexcept;
float movement_command_weapon_attack(void*, const MovementCommandBindings&) noexcept;
float movement_command_attack_guard(void*, const MovementCommandBindings&) noexcept;
std::uint16_t movement_command_weapon_definition(void*, const MovementCommandBindings&) noexcept;
float movement_command_convar_float(const void*) noexcept;

struct MovementCommandFriction {
    float stop_speed, friction, interval;
};
// Native friction loops read stop, friction and interval anew each iteration.
MovementCommandFriction capture_movement_command_friction(const MovementCommandBindings&) noexcept;

// Duck prefix reads are gated before the weapon virtual6C4 call. The native
// button write happens before that call, and its cvar read happens afterwards.
void process_live_duck_peek(MovementCommandOwner&, void* player, void* weapon, UserCommand&,
                            std::uint8_t& send_packet, const MovementCommandBindings&);

struct MovementCommandHull {
    MovementVector minimum, maximum;
    std::uint32_t mask;
};
// Origin probe: player260 -> player00C/collideable008 -> player00C/004.
// Maximum/minimum pointer aliases are copied after both collideable calls.
MovementCommandHull capture_quick_peek_origin_hull(void* player);
// Distance clamp: player130 -> player134; copy both vectors before player260.
MovementCommandHull capture_quick_peek_limit_hull(void* player);
QuickPeekHistory capture_quick_peek_history(void*, std::int32_t command,
                                            const MovementCommandBindings&);
bool movement_command_quick_peek_shot(const QuickPeekState&, void* player,
                                      const MovementCommandBindings&) noexcept;

// Only player450 plus its following3EB71843 read. Friction/weapon values are
// separate stage reads; this must be called again after a simulation getter.
MovementPhysics capture_movement_command_speed(void*, const MovementCommandBindings&);
GroundAcceleration movement_command_ground_acceleration(void* player, const UserCommand&,
                                                        MovementVector velocity,
                                                        MovementVector direction, float wish_speed,
                                                        const MovementCommandBindings&);
bool write_live_movement_counter(void* player, UserCommand&, MovementVector velocity,
                                 MovementAngleView, const MovementCommandBindings&);

} // namespace skeet

// Declarations from command_runtime.hpp

namespace skeet {

// Portable owner of the proven434759B0 movement state. The source projection
// keeps full+53C in duck_peek.previous_buttons, shared by both command phases.
struct MovementCommandOwner {
    MovementConfig config;
    QuickPeekConfig quick_peek_config;
    MovementFeedback feedback;
    DuckPeekState duck_peek;
    QuickPeekState quick_peek;
    std::uint32_t previous_buttons_1e0 = 0;
    std::uint32_t suppressed_buttons_1e4 = 0;
    const void* air_accelerate = nullptr; //native+14
    const void* jump_speed_cap = nullptr; //native+18
    PreSpeedLimits limits{845.5f, 90.0f};
    float side_magnitude = 450.0f;
    float brake_reduction_limit = 15.0f;
    float brake_threshold = 30.0f;
    float angle_step = 60.0f;
    float field_1d8 = 286.0f;
    std::uint32_t callback_flags_1dc = 0;
    MovementVector previous_packet_origin; //full+56C, captured by4334659B
};

// Concrete live engine/global bindings are defined with the enclosing native
// scheduler implementation. Wrappers retain this typed reference while the
// individual source stages own their actual engine-boundary reads.
struct MovementCommandBindings;

// Complete secondary-interface phase1 callback4333F746. Infinite Duck is the
// current global43467CEC value. This also publishes the shared full+53C word.
void process_movement_phase_one(MovementCommandOwner&, client::Command&,
                                bool infinite_duck) noexcept;

// Fullowner entry4336F912, with the native unused/interface framing removed.
// feedback is explicit because the native main accepts a separate20-byte
// feedback record. Standard command wrappers pass owner.feedback.
void process_movement_command(MovementCommandOwner&, void* player, void* weapon, client::Command&,
                              MovementAngles& movement_angles, std::uint8_t& send_packet,
                              MovementFeedback&, const MovementCommandBindings&);
void process_movement_command(MovementCommandOwner&, void* player, void* weapon, client::Command&,
                              std::array<float, 3>& movement_angles, std::uint8_t& send_packet,
                              MovementFeedback&, const MovementCommandBindings&);

} // namespace skeet

// Declarations from command_state.hpp

namespace skeet {

// Command feedback at full movement owner +0x1AC. The secondary callback
// interface receives owner+8 and passes this as its +0x1A4 subobject.
struct MovementFeedback {
    std::uint8_t side_flip;
    std::uint8_t was_grounded;
    std::uint8_t prediction_blocked;
    std::uint8_t reserved;
    std::uint32_t original_buttons;
    float primary_heading;
    float turn_direction;
    float secondary_heading;
};
static_assert(sizeof(MovementFeedback) == 20);

struct MovementFeedbackHistory {
    float primary_heading;
    float turn_direction;
    float secondary_heading;
};

// Complete command entry writes 0x4336F936..0x4336F9AA. Flags and the feedback
// record are reset after preserving the three values consumed by the solver.
MovementFeedbackHistory begin_movement_command(MovementFeedback& feedback,
                                               const UserCommand& command,
                                               std::uint32_t& original_buttons,
                                               std::uint32_t& scheduler_flags) noexcept;

} // namespace skeet

// Declarations from command_wrapper.hpp

namespace skeet {

// The normal command registry and generated commands borrow the same movement
// owner. The command count is the live pointer cell at434757DC.
struct MovementCommandPhaseBindings {
    const MovementCommandBindings& command;
    std::int32_t* const& choked_commands;
    const HotkeyBinding& jump_at_edge_key; //4346760C
    std::uint32_t movement_flags_38ca5228;
};
struct MovementCommandPhaseOwner {
    MovementCommandOwner& state;
    const MovementCommandPhaseBindings& bindings;
};

void movement_owner_phase1(MovementCommandPhaseOwner&, client::Command&);
// Complete4334659B on the native valid-player contract. The enclosing master
// handles rejected/null players before invoking this phase.
void movement_owner_phase2(MovementCommandPhaseOwner&, void* player, void* weapon, client::Command&,
                           std::array<float, 3>& incoming_angles, std::uint8_t& send_packet);
// Complete433D1F99, using the same settings and callback flags as phase2.
void movement_owner_phase4(MovementCommandPhaseOwner&, client::CommandFrame&, client::Command&);

} //namespace skeet

// Declarations from duck_peek.hpp

namespace skeet {

struct DuckPeekSettings {
    HotkeyBinding hotkey; // 43467C9C / 43467CA0
    bool crouch_cycle; // 43467CEC
};

// Full movement owner fields 51C..548. The master scheduler captures the
// original command buttons and updates choked/previous_buttons/last_key.
struct DuckPeekState {
    std::uint8_t phase = 0; // +51C, states 0..3
    std::int32_t delay = 0; // +520
    std::int32_t release_commands = 0; // +524
    std::int32_t crouch_commands = 0; // +528
    float target = 0; // +52C
    std::int32_t field_530 = -1; // +530, written by4347DCB2
    std::int32_t choked = 0; // +534
    std::uint32_t original_buttons = 0; // +538
    std::uint32_t previous_buttons = 0; // +53C
    std::int32_t last_key = 27; // +540
    std::uint8_t crouch_active = 0; // +544, shared with the HUD byte view
    std::uint8_t release_latch = 0; // +545
    const std::int32_t* configured_limit = nullptr; // +548
};

struct DuckPeekFrame {
    MovementPhysics physics;
    MovementVector velocity;
    float current_time;
    float player_duck_time; // decoded player field 04B798C6
    float duck_delay; // decoded terminal cvar at 43467D7C
    float weapon_simulation_time; // decoded weapon field 0176A381
    float weapon_duck_delay; // actual weapon virtual +6C4 result
    std::int32_t command_budget; // actual network_command_budget result
    bool has_weapon;
};

// Native scheduler prefix 4336F9AA..43370191, on the valid-local-player
// contract. The caller has saved original_buttons and reset scheduler_flags.
// configured_limit is the native live configuration pointer and must be valid
// when the crouch-cycle limit is queried. send_packet is only cleared here.
void process_duck_peek(const DuckPeekSettings&, InputBitmap, DuckPeekState&, const DuckPeekFrame&,
                       UserCommand&, std::uint32_t& scheduler_flags, std::uint8_t& send_packet);

} // namespace skeet

// Declarations from easy_strafe.hpp

namespace skeet {
struct EasyStrafeState {
    std::uint32_t previous_buttons = 0; // owner + 0x1E0
    std::uint32_t suppressed_buttons = 0; // owner + 0x1E4
    std::uint32_t saved_buttons = 0; // owner + 0x53C
};

// Complete secondary-interface callback 0x4333F746. The settings are the
// owner+0x130 byte, global 0x43467CEC byte, and owner+0x1C4 magnitude.
void process_easy_strafe(EasyStrafeState& state, UserCommand& command, bool enabled,
                         bool force_bullrush, float default_move = 450.0f) noexcept;
} // namespace skeet

// Declarations from engine_collision.hpp

namespace skeet {

struct MovementEngineCollisionBindings {
    void* const& engine_trace; //43468304; actual EngineTrace slot5
    void* const& surface_properties; //43469268; actual surface physics slot4
    void* const* const& skip_filter_vtable; //live genuine engine table434758B4
};

// Production engine boundary for the recovered movement collision routines.
// The adapter only packs Ray_t/filter ABI and invokes the genuine interfaces;
// all stepping, clipping, surface scaling and collision decisions stay there.
class EngineMovementCollision final : public MovementCollisionEngine {
    MovementEngineCollisionBindings bindings_;

public:
    explicit EngineMovementCollision(const MovementEngineCollisionBindings& bindings)
        : bindings_(bindings) {}
    void trace_ray(const MovementCollisionRay&, std::uint32_t mask, MovementCollisionFilter,
                   MovementCollisionTrace&) override;
    float surface_friction(std::int16_t surface_property) override;
};

} //namespace skeet

// Declarations from extrapolation.hpp

namespace skeet {

struct MovementExtrapolationBindings {
    lagcomp::EntityNetvars offsets;
    std::uint32_t velocity_7a2d60c1;
    std::uint32_t origin_4e66bb34;
    std::uint32_t origin_2a671b42;
    std::uint32_t field_7090d8dd;
    std::uint32_t field_52899df3;
    std::uint32_t field_d3d4230d;
    // Decoded fixture inputs are absent in the mandatory live entry. Its
    // actual target is captured at43385AFD and each simulation construction.
    std::optional<std::uint32_t> player_handle;
    client::PlayerStateCache* players; //actual433B8A2B lookup in43385AFD
    std::optional<MovementSimulationPlayer> snapshot;
    std::optional<MovementSimulationRules> rules;
    std::optional<MovementSimulationEnvironment> environment;
    MovementCollisionEngine* collision;
    animation::AnimationUpdateEngine animation_engine;
    std::int32_t* animation_frame; //434678F8
    float* current_time; //43467EB8
    float* interval; //4346A338
    std::uint8_t* animation_reset; //optional43467B24
    lagcomp::SetAbsoluteVector set_absolute_angles; //optional434688C8
    void* allocator; //actual43467204 interface, Free virtual slot5
};
struct MovementExtrapolationLiveBindings {
    const MovementCaptureInput& capture;
    const MovementEnvironmentBindings& environment;
};
// Complete43322B09 valid-player path. The unused native ECX argument is
// omitted. More than32 ticks or a null player animation state returns0.
// The animation state copy, entity-field overrides, clock restoration and
// animation-vector cleanup are owned here; simulation calls the full tick.
std::int32_t extrapolate_lag_record(void* player, client::PlayerState&, lagcomp::LagRecord&,
                                    std::int32_t ticks, const MovementExtrapolationBindings&);
// Raw43322B09 entry. Constructor and every tick read this actual target at
// their native sites. Frozen semantic snapshots above are not consumed.
std::int32_t extrapolate_lag_record(void* player, client::PlayerState&, lagcomp::LagRecord&,
                                    std::int32_t ticks, const MovementExtrapolationBindings&,
                                    const MovementExtrapolationLiveBindings&);

} // namespace skeet

// Declarations from extrapolation_command.hpp

namespace skeet {
namespace client {
class PlayerStateCache;
}
struct MovementEnvironmentBindings;

struct MovementExtrapolationCommandPlayer {
    MovementVector velocity;
    float duck_amount;
    std::uint32_t player_flags;
    std::uint8_t field_7090d8dd;
    std::int32_t field_52899df3;
    float field_d3d4230d;
};
struct MovementExtrapolationCommandOffsets {
    std::uint32_t velocity, duck_amount, player_flags;
    std::uint32_t field_7090d8dd, field_52899df3, field_d3d4230d;
};

// Complete 4334D3FF, including the distinct player-state acceleration helper
// 433F3DC7. The command is already zeroed by the enclosing 43385AFD entry.
bool infer_history_movement_command(client::Command&, const lagcomp::RecordRing&,
                                    float history_time_110,
                                    const MovementExtrapolationCommandPlayer&,
                                    const MovementPhysics&) noexcept;

// Complete 43385AFD after its actual entity-index/cache lookup. The original
// valid-player path requires a nonzero index. Both history and live-player
// branches zero the entire 0x64-byte command before reconstructing its inputs.
bool reconstruct_extrapolation_command(client::Command&, const lagcomp::RecordRing&,
                                       float history_time_110, std::int32_t history_tick_168,
                                       const MovementExtrapolationCommandPlayer&,
                                       const MovementPhysics&) noexcept;
// Raw43385AFD/4334D3FF/433F3DC7. Index/cache, player fields, ConVars and
// guarded weapon virtuals are read where the original branches consume them.
bool reconstruct_live_extrapolation_command(client::Command&, void* player,
                                            client::PlayerStateCache&,
                                            const MovementExtrapolationCommandOffsets&,
                                            const MovementEnvironmentBindings&);

} // namespace skeet

// Declarations from global_config.hpp

namespace skeet {
// Separate native global descriptors, outside the4348FA90 movement owner.
// Their CRT entries are independent; callers retain the actual CRT order.
struct MovementGlobalConfig {
    BooleanConfig infinite_duck{{{nullptr, nullptr, 0xD0CA5B87u}, 1, ConfigType::boolean, 0},
                                0}; //43467CDC,value43467CEC
    IntegerConfig air_duck{{{nullptr, nullptr, 0x5B1A5C7Fu}, 4, ConfigType::integer, 0},
                           0}; //4346A570,value4346A580
    HotkeyConfig edge_jump{{{nullptr, nullptr, 0x5BBB224Eu}, 8, ConfigType::hotkey, 0},
                           {}}; //434675FC,value4346760C
};
// Metadata above belongs to the loaded image. These CRT callbacks only link
// the existing descriptors and perform their explicit runtime-value writes.
void initialize_infinite_duck_config(ConfigRegistries&, BooleanConfig&) noexcept; //434933A6
void initialize_air_duck_config(ConfigRegistries&, IntegerConfig&) noexcept; //43490B39
void initialize_edge_jump_hotkey(ConfigRegistries&, HotkeyConfig&) noexcept; //4349340B
} //namespace skeet

// Declarations from ground_contact.hpp

namespace skeet {

// Shared native ground-classification chain: full hull, then the four hull
// quadrants when required. The original fraction/end survive quadrant retries.
// This uses the actual engine-backed skip-player filter (4346EDC0 forwarding
// through434758B4), collision trace and surface-friction lookup boundaries.
bool categorize_movement_ground(MovementSimulation& state, MovementCollisionEngine& engine,
                                bool extend_grounded_drop, bool mark_hard_landing = false);

// Initial 0x433D7868..0x433D8565 pass. MOVETYPE_WALK retains its native shortcut.
void prepare_movement_simulation_ground(MovementSimulation& state, MovementCollisionEngine& engine);

} // namespace skeet

// Declarations from ground_phase.hpp

namespace skeet {

struct GroundPhaseSettings {
    QuickStopSettings quick_stop;
    SlowMotionSettings slow_motion;
};

enum class GroundPhaseContinuation {
    player_movement, // 0x43372C76
    weapon_speed_features, // 0x4337206A
    complete, // 0x4337CAA4
};

// Ground/jump gate at 0x43371D4C and its quick-stop/slow-motion chain.
// crouch_cycle_active is owner + 0x544, reset only by the first gate.
GroundPhaseContinuation
process_ground_phase(const GroundPhaseSettings& settings, InputBitmap input,
                     const MovementPhysics& physics, bool has_weapon, std::int32_t weapon_state,
                     bool weapon_blocked, bool& crouch_cycle_active, std::uint32_t& scheduler_flags,
                     std::int32_t command_number, UserCommand& command, MovementVector velocity,
                     MovementAngles command_angles, MovementAngleView movement_angles);

} // namespace skeet

// Declarations from ground_phase_live.hpp

namespace skeet {

GroundPhaseContinuation process_live_ground_phase(MovementCommandOwner&, void* player, void* weapon,
                                                  client::Command&, MovementAngleView,
                                                  const MovementCommandBindings&);

// Actual4337C899 tail. Each regulator obtains its own live reads; the second
// gate observes command mutations made by the first call.
void process_live_ground_speed_tail(const MovementCommandOwner&, void* player, client::Command&,
                                    MovementAngleView, const MovementCommandBindings&);

} // namespace skeet

// Declarations from ground_speed.hpp

namespace skeet {

struct GroundSpeedTailSettings {
    bool fast_walk;
    // Three independent native gates: 0x4346A740, 0x43467F60 and 0x434685B8.
    // Their association with a menu option has not been established.
    bool periodic_speed_enabled;
    bool periodic_context_a;
    bool periodic_context_b;
};

// Complete final movement-scheduler block 0x4337C899..0x4337CAA4.
// player_state is the decoded byte consumed by 0x433BA215 (state >= 2
// prevents this block). The caller supplies the command's signed sequence
// number and the engine's two distinct command/movement angle vectors.
void process_ground_speed_tail(const GroundSpeedTailSettings& settings,
                               const MovementPhysics& physics, std::uint8_t player_state,
                               std::int32_t command_number, UserCommand& command,
                               MovementVector velocity, MovementAngles command_angles,
                               MovementAngleView movement_angles) noexcept;

} // namespace skeet

// Declarations from initialization.hpp

namespace skeet {

//4348FA90 through the scalar portion of4347DCB2. Both configuration groups
// must still be unregistered. ConVar pointers are the decoded actual globals.
void initialize_movement_state(MovementCommandOwner&, ConfigRegistries&, const void* air_accelerate,
                               const void* jump_speed_cap);
//4348FD15..4348FD3F, after the owner's two native strings are constructed.
void finish_movement_state_initialization(MovementCommandOwner&) noexcept;
void reset_movement_state(MovementCommandOwner&) noexcept; //433F4938
void bind_movement_settings(MovementCommandOwner&, ConfigRegistries&); //43490711

} //namespace skeet

// Declarations from normalized_air_final.hpp

namespace skeet {

struct NormalizedAirFinalSettings {
    const std::int32_t& smoothing; // movement owner +0x68
    const float& maximum_angle; // +0x1C8
    const float& forward_magnitude; // +0x1C4
    const float& acceleration_limit; // +0x1D0
    const float& angle_step; // +0x1D4
};

// Settings retain the owner's cells across speed getters, constructors and
// both forecasts. A getter can change later knob reads without changing the
// earlier saved smoothing/difference values in each individual trial.

struct NormalizedAirFinalInput {
    const MovementVector& position; // live GetAbsOrigin pointer / simulation position
    MovementVector velocity; // prepared velocity at ESP+0x114
    float stamina; // prepared ESP+0x14, separate from snapshot
    float velocity_yaw; // original heading at ESP+0x10C
    float speed; // prepared horizontal speed at ESP+0x13C
    AirStrafeSelection selection;
};

struct NormalizedAirFinalBindings {
    void* player;
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& capture;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* const& surface_properties;
    const void* const& air_acceleration; // actual movement owner +0x14 cvar cell
};

// Complete normalized-basis final solver 43376EFE..43378DF9. The pre-speed
// stage has already selected movement_angles.yaw. This stage records feedback,
// compares both full movement forecasts when required, and writes forward move.
// It does not execute the shared command tail at 4337C899.
void finish_normalized_air_strafe(const NormalizedAirFinalSettings&, const NormalizedAirFinalInput&,
                                  const MovementFeedbackHistory&, MovementFeedback&,
                                  client::Command&, MovementAngleView,
                                  const NormalizedAirFinalBindings&);

} // namespace skeet

// Declarations from normalized_air_solver.hpp

namespace skeet {

struct NormalizedAirStrafeSettings {
    RawAirStrafeSettings steering;
    float angle_step; //fullowner+1D4
};
struct NormalizedAirStrafeFrame {
    NormalizedAirWallInput collision; //velocity is the prepared73984 seed
    MovementVector forecast_position; //live A9EE6062
    std::uint32_t forecast_player_flags; //live E2FBE695
    float stamina;
    float engine_time;
};
struct NormalizedAirStrafeBindings {
    const AirWallSteeringBindings& wall;
    const NormalizedAirFinalBindings& final;
};

// Complete normalized solver43373CBC..43378DF9 after scout/velocity gates.
// Unlike the raw branch, the normalized guarded pre-speed and Z-Hop paths
// retain the configured smoothing for their distinct dynamic final solver.
bool process_normalized_air_strafe(const NormalizedAirStrafeSettings&, const AirStrafeSelection&,
                                   InputBitmap, const NormalizedAirStrafeFrame&,
                                   const MovementFeedbackHistory&, MovementFeedback&,
                                   client::Command&, MovementAngleView,
                                   const NormalizedAirStrafeBindings&);

} // namespace skeet

// Declarations from normalized_air_wall.hpp

namespace skeet {

struct NormalizedAirWallInput {
    bool enabled; // movement owner +0x158
    std::int32_t prediction_milliseconds; // +0x16C
    std::int32_t smoothing; // +0x68
    float maximum_angle, side_magnitude; // +0x1C8 / +0x1C4
    float acceleration_limit, angle_step; // +0x1D0 / +0x1D4
    float air_acceleration; // decoded cvar through owner +0x14
    bool pre_speed_effective;
    std::uint32_t player;
    MovementVector position, velocity;
    std::uint32_t player_flags;
    std::int32_t weapon_handle;
};

// Dynamic prediction heading used inside the normalized wall forecast. The
// simulated stamina/friction and live player duck state are distinct inputs.
float normalized_air_prediction_yaw(const NormalizedAirWallInput&, const MovementSimulation&,
                                    const UserCommand&, float previous_yaw,
                                    const MovementPhysics&) noexcept;

// Complete normalized-basis wall block 0x43373F63..0x433750E3, ending before
// the pre-speed dispatch. Returns the original velocity heading at ESP+0x10C;
// only movement_angles.yaw can change. The simulation constructor reads its
// stamina from the player snapshot, separately from the caller's ESP+0x14.
float steer_normalized_air_wall(const NormalizedAirWallInput&, const MovementFeedback&,
                                const client::Command&, MovementAngleView movement_angles,
                                const AirWallSteeringBindings&);

// Actual command path: constructor, each prediction tick and both trace-hull
// reads use the live engine bindings at their native call sites. Input carries
// only the cached velocity/selection/enable state in this path; owner tuning
// is read through bindings. The cvar cell is the actual owner+0x14 and is
// decoded separately for both angle terms.
float steer_live_normalized_air_wall(const NormalizedAirWallInput&, const MovementFeedback&,
                                     const client::Command&, MovementAngleView movement_angles,
                                     const AirWallSteeringLiveBindings&,
                                     const void* const& air_acceleration);

} // namespace skeet

// Declarations from normalized_scout_gate.hpp

namespace skeet {

struct NormalizedScoutGateFrame {
    void* player; // actual player; collideable virtual +0x0C
    void* engine_client; // actual GetViewAngles virtual +0x48
    MovementVector original_velocity;
    MovementVector origin; // player virtual +0x28, acquired before this block
    bool has_weapon;
    std::uint16_t weapon_definition;
};

// Complete 4337338B..43373984. A nearly stationary scout with no movement
// keys probes 64 units along the normalized horizontal engine view, then
// repeats the same hull sweep 64 units higher when the first sweep is blocked.
// False selects the shared command tail at 4337C895; true reaches 43373984.
// Collideable and view getters execute only after the native scalar gates.
bool pass_normalized_scout_gate(const NormalizedScoutGateFrame&, const client::Command&,
                                MovementCollisionEngine&, std::uint8_t* engine_trace_flag);

} // namespace skeet

// Declarations from owner_lifecycle.hpp

namespace skeet {
struct MovementOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
};
struct MovementOwnerHudBindings {
    script::Bridge& scripts;
    const IntegerConfig& feature_indicators; //43468B94
    const std::uint8_t& infinite_duck; //43467CEC
    const rage::GlobalSettings& rage;
    const std::uint32_t& command_flags; //434759C0
    const esp::IndicatorLayout& layout;
    const esp::Projection& projection;
    std::uint32_t life_state_9d920e35;
};
struct MovementOwnerBindings {
    MovementCommandPhaseOwner& command;
    const MovementOwnerHudBindings& hud;
    void* const& engine_client; //4346A514, virtual170 FindCommand
    void* const& input_system; //4346A89C, virtual46
    engine::NativeConVarReference air_accelerate; //434674E4 ^DCEAEAC8
    engine::NativeConVarReference jump_speed_cap; //43467FA0 ^4B52A9BF
    MovementCommandOwner*& active; //canonical434759B0
};

// Native4348FA90 registers primary, normal-command and HUD in this order,
// then publishes their tables before constructing settings/state/two strings.
// Canonical command state and bindings are borrowed and must outlive these
// endpoints. Pass fresh, unregistered configuration descriptors.
std::size_t movement_owner_size() noexcept;
void construct_movement_owner(void* storage, const MovementOwnerRegistries&, ConfigRegistries&,
                              MovementOwnerBindings);
// Allocate through the actual process-heap boundary with43387448's alignment
// header. The canonical active slot is published after construction.
void* create_movement_owner(const MovementOwnerRegistries&, ConfigRegistries&,
                            MovementOwnerBindings);
// Complete43328BC8, including clearing the global before the null test,
// reverse string destruction433DF641 and original allocation-header free.
void destroy_movement_owner(void* storage, MovementCommandOwner*& active);

// Whole primary slot7 (4337EBEF): reset through slot5, resolve "+duck" by
// engine170, reject0/-1, then input46 publishes the same last_key+540.
void refresh_movement_owner_key(void* storage);
// HUD43470424 slot0 consumes this frame's raw entity and reads its live life
// byte before invoking the existing complete43384B72 renderer.
void draw_movement_owner_hud(MovementCommandPhaseOwner&, const MovementOwnerHudBindings&,
                             ui::RenderQueue&, void* local_player, std::int32_t& cursor);
} // namespace skeet

// Declarations from player_air_gate.hpp

namespace skeet {

struct PlayerAirGateSettings {
    bool air_strafe_enabled; // owner +2C, latched before the water gate
    bool bunny_hop_enabled; // owner +194
    float maximum_move; // owner +1C4
    float brake_reduction_limit; // owner +1CC
    float brake_threshold; // owner +1D0
    const void* air_accelerate; // owner +14, live ConVar parent chain
};

struct PlayerAirGateFields {
    std::uint32_t movement_type_7501cb91;
    std::uint32_t water_state_ac7f2913;
    std::uint32_t velocity_36375c23;
    std::uint32_t weapon_definition_5c9f0fbb;
    std::uint32_t weapon_blocked_b8969256;
    std::uint32_t player_next_attack_de530a8b;
    std::uint32_t weapon_next_attack_f71ce331;
};

struct PlayerAirGateBindings {
    PlayerAirGateFields fields;
    const MovementEnvironmentBindings& environment;
    const std::uint32_t* speed_policy_flags; // 43467E40
    const float* current_time; // 43467EB8
    BunnyHopGuard bunny_hop_guard;
};

struct PlayerAirGateFrame {
    void* player;
    void* weapon;
    const MovementFeedback* feedback;
};

enum class PlayerAirGateContinuation {
    air_solver, // 4337322A
    ground_speed_tail, // 4337C895/4337C899
    complete, // 4337CAA4
};

struct PlayerAirGateResult {
    PlayerAirGateContinuation continuation;
    bool air_strafe_enabled;
};

// Complete 43372C76..4337322A player gate. The live player450 call and the
// following speed-limit read use the shared environment helper. Subsequent
// acceleration and velocity reads retain their positions after that call.
PlayerAirGateResult process_player_air_gate(const PlayerAirGateSettings&, const PlayerAirGateFrame&,
                                            const PlayerAirGateBindings&,
                                            std::uint32_t& scheduler_flags, UserCommand&,
                                            MovementAngleView movement_angles);

} // namespace skeet

// Declarations from pre_speed.hpp

namespace skeet {

struct PreSpeedLimits {
    float angle_scale; // movement owner + 0x1C0
    float maximum_angle; // movement owner + 0x1C8
};

struct PreSpeedTurn {
    float horizontal_speed;
    float minimum_step;
    float maximum_step;
    float turn_step;
    bool within_speed_limit;
};

// Complete pre-speed initial turn calculation 0x43375124..0x43375207.
PreSpeedTurn prepare_pre_speed_turn(PreSpeedLimits limits, float interval_per_tick,
                                    float player_speed_limit, MovementVector velocity) noexcept;

} // namespace skeet

// Declarations from pre_speed_controller.hpp

namespace skeet {

struct RawPreSpeedInput {
    PreSpeedLimits limits;
    PreSpeedProbeInput forecast;
    // The radial fallback uses the actual player virtual+28 origin, whereas
    // the circular forecast uses the separately decoded A9EE6062 position.
    MovementVector radial_origin;
    float player_speed_limit;
    float previous_heading;
};

// Complete raw-basis pre-speed search4337A8ED..4337C3D0, before feedback
// overshoot handling. The caller has established requested/effective pre-speed.
// This owns both native trace-flag scopes and the positive/negative searches;
// each trial executes the concrete manual forecast rather than the full tick.
float choose_raw_pre_speed_heading(const RawPreSpeedInput&, float movement_heading,
                                   MovementCollisionEngine&, std::uint8_t* engine_trace_flag);

} // namespace skeet

// Declarations from pre_speed_live.hpp

namespace skeet {

struct LivePreSpeedInput {
    PreSpeedLimits limits;
    MovementVector velocity; // cached XY seed used by the initial turn calculation
    const MovementVector& forecast_velocity; // reread in full at the start of each trial
    const MovementVector& radial_origin;
    float previous_heading;
};
struct LivePreSpeedBindings {
    void* player;
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& capture;
    std::uint32_t forecast_position_a9ee6062;
    MovementCollisionEngine& collision;
};

// The same recovered circular search as choose_raw_pre_speed_heading, with
// live native reads at each trial and real mask/maximum/minimum getters at
// every hull trace. The radial origin remains a live reference throughout.
float choose_live_pre_speed_heading(const LivePreSpeedInput&, float movement_heading,
                                    const LivePreSpeedBindings&);

} // namespace skeet

// Declarations from pre_speed_probe.hpp

namespace skeet {

// Decoded snapshot used by the manual circle forecast in 0x4336F912.
// position is decoder A9EE6062; gravity and jump_impulse are the actual cvars.
// collision_mask is the player's virtual +0x260 result, without extra bits.
struct PreSpeedProbeInput {
    MovementVector position, velocity;
    std::uint32_t player_flags;
    float gravity, jump_impulse, interval_per_tick, horizon, turn_step;
    MovementVector hull_minimum, hull_maximum;
    std::uint32_t collision_mask, player;
};

struct PreSpeedProbeResult {
    bool blocked;
    MovementVector position, velocity;
    bool on_ground;
    // A blocked forward trace does not complete the current forecast tick.
    std::int32_t completed_ticks;
};

// 0x4337AED0..0x4337B722 / 0x4337B74B, stopping before the enclosing
// controller changes turn_step. The caller owns its enclosing trace-flag guard.
PreSpeedProbeResult forecast_pre_speed_circle(const PreSpeedProbeInput& input,
                                              MovementCollisionEngine& engine);

} // namespace skeet

// Declarations from prediction.hpp

namespace skeet {

// Complete non-null entity path of 43359415..433598DE. Entity velocity and
// hull bounds are the decoded field and actual virtual130/134 results.
// Speeds outside the open interval (5,340) preserve every origin component.
void predict_movement_origin(MovementVector& origin, MovementVector velocity,
                             MovementVector hull_minimum, MovementVector hull_maximum,
                             std::int32_t ticks, float interval_per_tick,
                             MovementCollisionEngine& collision, std::uint8_t* engine_trace_flag);

// Complete 433376C3..4333781C. The active command's buttons gate prediction;
// the separate copied command supplies movement and retains the native tick
// sentinel comparison. Both ticks execute the concrete simulation directly.
MovementVector advance_movement_origin(std::uint32_t player, MovementVector position,
                                       MovementVector velocity, std::uint32_t player_flags,
                                       const MovementSimulationPlayer& snapshot,
                                       const MovementSimulationRules& rules,
                                       const MovementSimulationEnvironment& environment,
                                       const client::Command& command, std::uint32_t input_buttons,
                                       float yaw, MovementCollisionEngine& collision);

} // namespace skeet

// Declarations from quick_peek.hpp

namespace skeet::ui {
struct RenderQueue;
}
namespace skeet::esp {
struct Projection;
}
namespace skeet {
struct MovementCommandBindings;
namespace client {
struct Command;
}

struct QuickPeekPosition {
    MovementVector value{};
    bool valid = false;
};
struct QuickPeekHistory {
    std::int32_t command_number = 0;
    MovementVector position{};
    bool unavailable = false;
    bool grounded = false;
};
struct QuickPeekState {
    std::uint8_t phase = 0; // native quick-peek subobject +78, states 0..3
    std::uint32_t saved_buttons = 0;
    bool released = false, shot = false;
    QuickPeekPosition origin; // +84 vector, +90 validity
    MovementVector retreat_start{}; // +94
    float start_time = 0, stop_time = 0; // +A0, +A4
    std::uint32_t retreat_buttons = 0; // +A8
    std::int32_t shot_counter = 0; // +AC
    float distance = 0; // +B0
    std::array<QuickPeekHistory, 32> history{}; // +B4, native stride14
};

// These are decoded records returned by the native command-history lookup
// 433952F0, whose 150 slots are keyed by command number.
struct QuickPeekShotRecord {
    std::int32_t command_number;
    std::uint32_t flags;
    float shot_time;
    bool fired;
};
struct QuickPeekFrame {
    MovementPhysics physics;
    MovementVector position, velocity, hull_minimum, hull_maximum;
    float current_time, player_next_attack, weapon_next_attack;
    std::int32_t command_number, last_acknowledged_command, shot_counter;
    std::uint32_t original_buttons, trace_mask;
    std::uint32_t player_entity;
    bool history_unavailable;
    bool mark_command;
    std::span<const QuickPeekShotRecord> shot_records;
};

enum class QuickPeekContinuation {
    movement_features,
    complete
};

// Constructor writes at 434029C6..434029E0 and reset at 433F496E..433F4988.
void initialize_quick_peek_state(QuickPeekState& state) noexcept;
void reset_quick_peek_state(QuickPeekState& state) noexcept;
// Prediction wrappers 4334659B and 43328EE7 commit this counter after the
// command stage, so the next command can observe a newly fired shot.
void update_quick_peek_shot_counter(QuickPeekState& state, std::int32_t counter) noexcept;

// Native scheduler feature 4337019B..43371D4A, including its 32-command
// origin history, three retreat triggers, collision steering and stop window.
// The caller has already established a local player and active weapon.
// marked_commands is native 43476090's shared 150-slot command-number table.
QuickPeekContinuation process_quick_peek(const QuickPeekConfig& config, InputBitmap input,
                                         QuickPeekState& state, const QuickPeekFrame& frame,
                                         UserCommand& command, MovementAngleView movement_angles,
                                         std::uint32_t& scheduler_flags,
                                         std::span<std::uint32_t, 150> marked_commands,
                                         MovementCollisionEngine& collision,
                                         std::uint8_t* engine_trace_flag);
QuickPeekContinuation process_live_quick_peek(const QuickPeekConfig&, InputBitmap, QuickPeekState&,
                                              void* player, void* weapon, client::Command&,
                                              std::uint32_t original_buttons, MovementAngleView,
                                              std::uint32_t& scheduler_flags,
                                              const MovementCommandBindings&);

// Render-interface block 43384C27..43384C78; complete disc geometry helpers
// 4337DCA0 and 43397246. Uses the same color descriptor shown by QuickPeekMenu.
void draw_quick_peek(const QuickPeekConfig& config, InputBitmap input, const QuickPeekState& state,
                     const esp::Projection& projection, ui::RenderQueue& queue);

} // namespace skeet

// Declarations from quick_stop.hpp

namespace skeet {

struct QuickStopSettings {
    bool standalone_quick_stop;
    bool quick_peek_enabled;
    HotkeyBinding quick_peek_hotkey;
};

enum class QuickStopContinuation {
    player_movement, // 0x43372C76
    speed_features, // 0x43371E8F
    complete, // 0x4337CAA4
};

// 0x43371D86..0x43371E8F and shared counter-movement return at 0x433702E8.
// The containing scheduler has already passed its initial ground/jump gate.
// Its ground test immediately before the counter-movement call is retained.
QuickStopContinuation process_quick_stop_stage(const QuickStopSettings& settings, InputBitmap input,
                                               const MovementPhysics& physics, bool has_weapon,
                                               std::uint32_t& scheduler_flags, UserCommand& command,
                                               MovementVector velocity,
                                               MovementAngleView movement_angles);

} // namespace skeet

// Declarations from rage_stop.hpp

namespace skeet {

struct RageStopBindings {
    const MovementEnvironmentBindings& environment;
};

//433F0C03: actual weapon virtual6E8. A zero result selects250 before the
// crouch factor and one-unit margin; NaN and signed-zero retain native tests.
float rage_stop_speed(void* weapon);

//4336961E: actual player virtual450 precedes the current decoded3EB71843
// property read. Both values are read anew on every call.
float rage_move_speed(void* player, const RageStopBindings&);

//433CE0FE: the frame receiver is unused. Only simulated on-ground bit1
// invokes the real counter-movement arithmetic with current engine reads.
// The simulation is inspected without changing any of its bytes.
void prepare_rage_stop_command(const client::CommandFrame&, void* player, client::Command&,
                               const MovementSimulation&, const RageStopBindings&);

} // namespace skeet

// Declarations from raw_air_solver.hpp

namespace skeet {

struct RawAirStrafeSettings {
    PreSpeedLimits limits;
    float side_magnitude;
    float feedback_turn_limit; //full movement owner+1D0
    std::int32_t smoothing;
    ZHopSettings z_hop;
};
struct RawAirStrafeFrame {
    AirWallSteeringInput collision;
    MovementVector forecast_position; //decoded A9EE6062
    std::uint32_t forecast_player_flags; //live E2FBE695, distinct from a simulated wall seed
    float previous_heading;
    float engine_time; //4346A334, distinct from simulation clock43467EB8
};

// Complete raw-basis airborne solver43379B3B..4337C899 after its scout/ground
// gates. All wall prediction, pre-speed forecasts, Z-Hop and final feedback are
// concrete source routines. False is an input-heading early edge to the same
// ground-speed tail; true means this solver reached a steering output.
bool process_raw_air_strafe(const RawAirStrafeSettings&, const AirStrafeSelection&, InputBitmap,
                            const RawAirStrafeFrame&, MovementFeedback&, client::Command&,
                            MovementAngleView movement_angles, const AirWallSteeringBindings&);

} // namespace skeet

// Declarations from raw_scout_gate.hpp

namespace skeet {

struct RawScoutGateSettings {
    bool bunny_hop_enabled; // movement owner +0x194
    float forced_forward_move; // movement owner +0x1C4
};

struct RawScoutGateFrame {
    MovementVector velocity;
    std::uint32_t player_flags;
    bool has_weapon;
    std::int32_t weapon_id;
    // The actual player virtual +0x28 has already returned this reference in
    // the enclosing raw-basis preamble. Copy it only after GetViewAngles.
    const MovementVector& position;
};

struct RawScoutGateBindings {
    void* player;
    void* engine_client; // actual GetViewAngles virtual +0x48
    MovementCollisionEngine& collision;
    std::uint8_t* engine_trace_flag;
};

enum class RawScoutGateResult {
    continue_air,
    ground_speed_tail
};

// Complete raw-basis ground/scout gates 0x43378F05..0x43379B3B, after mode
// selection and before the airborne input-heading stage. Hull access uses the
// actual player's collideable virtual +0x0C, then maximum +8 and minimum +4.
// Both traces and the grounded bunny-hop command mutation stay in source.
RawScoutGateResult process_raw_scout_gate(const RawScoutGateSettings&, const AirStrafeSelection&,
                                          const RawScoutGateFrame&, const MovementFeedback&,
                                          UserCommand&, const RawScoutGateBindings&);

} // namespace skeet

// Declarations from release_binding.hpp

namespace skeet {
// The native singleton publishes MovementCommandOwner, not a release adapter
// or a separately allocated feedback pointer. Keep the optional owner gate
// live while borrowing the real source adapter and its embedded feedback.
class MovementReleaseBinding {
public:
    MovementReleaseBinding(MovementCommandOwner* const& owner,
                           MovementReleaseRuntime& runtime) noexcept
        : owner_(&owner), runtime_(&runtime) {}
    // Explicit standalone boundary used by authored command fixtures.
    MovementReleaseBinding(MovementReleaseRuntime* const& runtime) noexcept
        : standalone_(&runtime) {}
    MovementReleaseRuntime* get() const noexcept {
        return owner_ ? (*owner_ ? runtime_ : nullptr) : *standalone_;
    }
    explicit operator bool() const noexcept {
        return get() != nullptr;
    }
    MovementReleaseRuntime& operator*() const noexcept {
        return *get();
    }

private:
    MovementCommandOwner* const* owner_ = nullptr;
    MovementReleaseRuntime* runtime_ = nullptr;
    MovementReleaseRuntime* const* standalone_ = nullptr;
};
class MovementFeedbackBinding {
public:
    explicit MovementFeedbackBinding(MovementCommandOwner* const& owner) noexcept
        : owner_(&owner) {}
    // Explicit standalone boundary used by authored command fixtures.
    MovementFeedbackBinding(MovementFeedback* const& feedback) noexcept : standalone_(&feedback) {}
    MovementFeedback* get() const noexcept {
        return owner_ ? (*owner_ ? &(*owner_)->feedback : nullptr) : *standalone_;
    }
    MovementFeedback& operator*() const noexcept {
        return *get();
    }

private:
    MovementCommandOwner* const* owner_ = nullptr;
    MovementFeedback* const* standalone_ = nullptr;
};
} //namespace skeet

// Declarations from release_command.hpp

namespace skeet {

// References bind the actual movement owner's five runtime scalars. They are
// not copied into a release-preroll snapshot.
struct MovementReleaseTuning {
    const float& angle_scale; // +0x1C0
    const float& side_magnitude; // +0x1C4
    const float& maximum_angle; // +0x1C8
    const float& feedback_turn_limit; // +0x1D0
    const float& normalized_angle_step; // +0x1D4
};
struct MovementReleaseOffsets {
    std::uint32_t player_state_ac7f2913;
    std::uint32_t weapon_definition_5c9f0fbb;
    std::uint32_t forecast_position_a9ee6062;
};

struct MovementReleaseRuntime {
    const MovementConfig& config;
    MovementReleaseTuning tuning;
    InputBitmap input;
    engine::ConVarBinding bunny_hop_guard; //live encoded4346A4E4 parent chain
    engine::ConVarBinding pre_speed_guard; //live encoded43468EA4 parent chain
    const void* const& jump_speed_cap; //owner+18, populated by its actual constructor
    const void* const& air_acceleration; // owner +0x14 parent chain, live cell
    const std::int32_t& force_duck_mode; // 4346A580
    const float& engine_time; // *4346A334
    void* engine_client;
    MovementReleaseOffsets offsets;
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& capture;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* const& surface_properties;
};

// Complete secondary movement-command synthesis at native 433A194E. The
// following simulation tick is owned by the caller, as in the native preroll.
// The caller's null runtime pointer implements the native absent-owner edge.
void synthesize_release_command(MovementReleaseRuntime&, void* player, void* weapon,
                                client::Command&, MovementSimulation&, MovementFeedback&);

} // namespace skeet

// Declarations from simulation.hpp

namespace skeet {
struct MovementEnvironmentBindings;

// Decoded player reads and actual virtual-method results for the native
// 0x4337D3C8 snapshot. Hash-only fields retain their decoder identity.
struct MovementSimulationPlayer {
    float speed_scale;
    float duck_speed;
    float duck_amount;
    float view_offset_z;
    float stamina;
    MovementVector hull_minimum, hull_maximum;
    std::uint8_t ducking;
    float fall_velocity;
    float surface_friction;
    std::uint8_t movement_type;
    std::uint32_t collision_mask;
    std::uint8_t field_f3c10e80_00;
};

// Constructor cvars, in native read order. Known stop/friction/acceleration
// are named; unresolved cvar names retain their encoding identities.
struct MovementSimulationRules {
    float gravity;
    float jump_impulse;
    float step_size;
    float stamina_recovery;
    float stamina_land;
    float stamina_jump;
    float stamina_maximum;
    float stop_speed;
    float friction;
    float accelerate;
    float cvar_dceaeac8;
    float interval_per_tick;
};

// The simulation is local source state. The player value is an opaque engine
// handle supplied by the adapter, not a host pointer to captured memory.
struct MovementSimulation {
    std::uint32_t player;
    MovementVector position, velocity;
    std::uint32_t player_flags, mode;
    float speed_scale, duck_speed, duck_amount, view_offset_z, stamina;
    MovementVector hull_minimum, hull_maximum;
    MovementVector contact_normal;
    float state_5c;
    MovementVector contact_position;
    MovementCollisionTrace trace;
    std::uint8_t ducking;
    std::array<std::byte, 3> padding_c1;
    float fall_velocity;
    float surface_friction;
    std::uint8_t movement_type;
    std::array<std::byte, 3> padding_cd;
    std::uint32_t collision_mask;
    MovementSimulationRules rules;
    float saved_vertical_velocity;
    std::uint8_t simulation_flags;
    std::array<std::byte, 3> padding_109;
    float maximum_speed;
    std::uint32_t buttons;
    MovementVector command_move;
};
static_assert(offsetof(MovementSimulation, trace) == 0x6c);
static_assert(offsetof(MovementSimulation, rules) == 0xd4);
static_assert(offsetof(MovementSimulation, simulation_flags) == 0x108);
static_assert(sizeof(MovementSimulation) == 0x120);

// Complete 0x433B0D5C..0x433B0FCF and 0x4337D3C8..0x4337D643.
// Fields untouched by the native constructors retain their previous bytes.
void initialize_movement_simulation(MovementSimulation& simulation, std::uint32_t player,
                                    MovementVector position, MovementVector velocity,
                                    std::uint32_t player_flags, std::uint32_t mode,
                                    const MovementSimulationPlayer& snapshot,
                                    const MovementSimulationRules& rules) noexcept;

// First complete stage of 0x433D7606: 0x433D7624..0x433D7868. The two speed
// inputs are the actual decoded player field and player virtual-method result.
void prepare_movement_simulation_command(MovementSimulation& simulation, std::uint32_t player,
                                         float player_speed_limit, float player_max_speed,
                                         const UserCommand& command) noexcept;
void prepare_live_movement_simulation_command(MovementSimulation&, void* player, const UserCommand&,
                                              const MovementEnvironmentBindings&);

// Closing state update 0x433DEFAB..0x433DF09F, before the enclosing trace-flag
// guard is restored. The cvar-presence test is independent of its stored value.
void finish_movement_simulation_tick(MovementSimulation& simulation,
                                     bool landing_stamina_cvar_present,
                                     bool advance_vertical = true) noexcept;

} // namespace skeet

// Declarations from simulation_capture.hpp

namespace skeet {
struct MovementEnvironmentBindings;
struct MovementCaptureOffsets {
    std::uint32_t speed_scale_7f5ed2d0, duck_speed_fb7e3404, duck_amount_3309c0cf;
    std::uint32_t view_offset_c246315e, stamina_2f33c681, ducking_6b1a59b5;
    std::uint32_t fall_velocity_7e9ba7bc, field_f3c10e80, movement_type_7501cb91;
};
struct MovementCaptureConvars {
    engine::ConVarBinding gravity;
    engine::ConVarBinding jump_impulse;
    engine::ConVarBinding step_size;
    engine::ConVarBinding stamina_recovery;
    engine::ConVarBinding stamina_land;
    engine::ConVarBinding stamina_jump;
    engine::ConVarBinding stamina_maximum;
    engine::ConVarBinding stop_speed;
    engine::ConVarBinding friction;
    engine::ConVarBinding accelerate;
    engine::ConVarBinding cvar_dceaeac8;
};
struct MovementCaptureInput {
    const MovementCaptureOffsets& offsets;
    const MovementCaptureConvars& convars;
    const float& interval;
    const MovementEnvironmentBindings* environment = nullptr;
};
// Live4337D3C8 reads, including virtual130/134/260 in native order.
MovementSimulationPlayer capture_movement_player(void*, const MovementCaptureOffsets&);
MovementSimulationRules capture_movement_rules(const MovementCaptureConvars&,
                                               float interval) noexcept;
// Complete live constructor433B0D5C. Caller already owns the copied origin,
// velocity and flags; ConVar reads precede the player/getter snapshot.
void initialize_live_movement_simulation(MovementSimulation&, void* player, MovementVector position,
                                         MovementVector velocity, std::uint32_t player_flags,
                                         std::uint32_t mode, const MovementCaptureInput&);
} // namespace skeet

// Declarations from simulation_collision.hpp

namespace skeet {

// Shared four-bump / five-plane movement solver in 0x433D7606. Ground movement
// can reuse the already completed trace when its destination equals the next
// move; airborne and raised-step movement pass no first destination.
void slide_movement_simulation(MovementSimulation& state, MovementCollisionEngine& engine,
                               const MovementVector* first_destination = nullptr);

} // namespace skeet

// Declarations from simulation_duck.hpp

namespace skeet {
struct MovementEnvironmentBindings;
// Exact 433D8583..433DA2FD. The boolean is the decoded player byte with
// identity A15C3105, read only while a duck transition is in progress.
// Ground contact is recategorized by the shared native trace algorithm.
// The following DA2FD stage owns the amount<=0 flag-four clear.
void update_movement_simulation_duck(MovementSimulation&, MovementCollisionEngine&,
                                     bool field_a15c3105);
void update_live_movement_simulation_duck(MovementSimulation&, MovementCollisionEngine&,
                                          void* player, const MovementEnvironmentBindings&);
} // namespace skeet

// Declarations from simulation_environment.hpp

namespace skeet {
namespace client {
struct Command;
}

// Resolved entity-property offsets. Each native stage reads the current
// binding; engine getters and collision calls may change the live objects.
struct MovementEnvironmentFields {
    std::uint32_t speed_limit_3eb71843;
    std::uint32_t ducking_6b1a59b5;
    std::uint32_t field_a15c3105;
    std::uint32_t weapon_handle_cd812654;
    std::uint32_t player_flags_e2fbe695;
    std::uint32_t duck_amount_3309c0cf;
    std::uint32_t duck_speed_fb7e3404;
};

struct MovementEnvironmentConvars {
    engine::ConVarBinding accelerate_weapon_speed; //43467E28 xorE80C95F9
    engine::ConVarBinding jump_speed_cap; //43467FA0 xor4B52A9BF
    engine::ConVarBinding stamina_land; //43468F4C xorB0CF25E8; presence only
    engine::ConVarBinding stop_speed; //43467124 xorC5C9925E
    engine::ConVarBinding friction; //4346A820 xorEAAEC541
    engine::ConVarBinding accelerate; //43467968 xor1632541C
};

struct MovementEnvironmentBindings {
    MovementEnvironmentFields fields;
    MovementEnvironmentConvars convars;
    void* entity_list; // actual entity-list interface, slot4
    const float* interval; // live4346A338
    std::uint8_t* engine_trace_flag; // live434683D0
};

//433D7624..433D76F2: player450 getter precedes the speed-limit property read.
// Other fields are zero; this result belongs only to the command stage.
MovementPhysics capture_movement_command_physics(void* player, const MovementEnvironmentBindings&);

//433DA894..433DAA31. Call only inside the positive additional-speed branch.
// The raw duck byte follows the command-button short circuit. Weapon lookup
// and virtual6E8/738/6EC calls retain their native guards and call order.
// Duck/walk decisions survive mutations made by those engine methods.
struct MovementGroundWeaponPhysics {
    MovementPhysics physics;
    bool duck;
    bool walk;
};
MovementGroundWeaponPhysics
capture_movement_ground_weapon_physics(void* player, const UserCommand&,
                                       std::uint32_t simulation_player_flags,
                                       const MovementEnvironmentBindings&);

// Direct native reads, called by the corresponding stage after its gates.
bool movement_player_duck_slowdown(void* player, const MovementEnvironmentBindings&) noexcept;
bool movement_jump_speed_cap(const MovementEnvironmentBindings&) noexcept;
bool movement_landing_stamina_present(const MovementEnvironmentBindings&) noexcept;

// Live433D7606. The trace guard encloses command getters, collision, duck,
// weapon getters and finish. Every simulated tick repeats its native reads.
void simulate_live_movement_tick(MovementSimulation&, MovementCollisionEngine&,
                                 const MovementEnvironmentBindings&, void* player,
                                 const UserCommand&, MovementAngles);
// Native command pointer contract: movement is copied before the max-speed
// getter; view angles are read later at the actual ground/air angle-vector call.
void simulate_live_movement_tick(MovementSimulation&, MovementCollisionEngine&,
                                 const MovementEnvironmentBindings&, void* player,
                                 const client::Command&);

} // namespace skeet

// Declarations from simulation_step.hpp

namespace skeet {

// Complete grounded movement chain 0x433DACC9..0x433DD363: initial probe,
// sliding, raised-step comparison, ground retention and completion flag.
void move_movement_simulation_ground(MovementSimulation& state, MovementCollisionEngine& engine);

} // namespace skeet

// Declarations from simulation_tick.hpp

namespace skeet {

struct MovementSimulationEnvironment {
    MovementPhysics physics;
    bool player_field_a15c3105;
    bool jump_speed_cap_enabled;
    bool landing_stamina_cvar_present;
    std::uint8_t* engine_trace_flag;
};

// Native 0x433D7606. All player/cvar/weapon inputs are decoded engine values;
// collision calls use only the actual engine interfaces exposed in collision.hpp.
void simulate_movement_tick(MovementSimulation& state, MovementCollisionEngine& engine,
                            const MovementSimulationEnvironment& environment, std::uint32_t player,
                            const UserCommand& command, MovementAngles command_angles);

} // namespace skeet

// Declarations from simulation_velocity.hpp

namespace skeet {
namespace client {
struct Command;
}
struct MovementEnvironmentBindings;
enum class MovementVelocityPath : std::uint32_t {
    skip_walk,
    ground,
    ground_stopped,
    air
};

// Native433DA2FD..433DACC9 /433DD368..433DD61A. The returned path marks
// the next collision stage; ground_stopped takes native433DD35C directly.
// physics contains actual decoded player/weapon values. The jump cap gate
// means cvar43467FA0^4B52A9BF has a self-valued integer (native false value).
MovementVelocityPath prepare_movement_simulation_velocity(MovementSimulation& state,
                                                          const MovementPhysics& physics,
                                                          const UserCommand& command,
                                                          MovementAngles angles,
                                                          bool jump_speed_cap_enabled) noexcept;

// Same native stage with its actual player/weapon reads performed only after
// their native jump and positive-additional-speed gates.
MovementVelocityPath prepare_live_movement_simulation_velocity(MovementSimulation&,
                                                               const UserCommand&, MovementAngles,
                                                               void* player,
                                                               const MovementEnvironmentBindings&);
MovementVelocityPath prepare_live_movement_simulation_velocity(MovementSimulation&,
                                                               const client::Command&, void* player,
                                                               const MovementEnvironmentBindings&);

// Initial grounded movement probe433DACC9..433DAEF4. The endpoint preserves
// position.z, the trace writes state.trace, and no slide/step response runs.
MovementVector trace_movement_simulation_ground(MovementSimulation& state,
                                                MovementCollisionEngine& engine);
void finish_movement_simulation_ground(MovementSimulation& state) noexcept; //433DD35C..433DD363
} // namespace skeet

// Declarations from slow_motion.hpp

namespace skeet {

struct SlowMotionSettings {
    bool enabled;
    HotkeyBinding hotkey;
};

// 0x43371E8F..0x43372061, including the shared regulator call and return.
// The containing scheduler has already checked ground, jump and weapon presence.
// Returns true when the command is consumed; false continues at 0x4337206A.
// weapon_state is the signed native field at 0x43468488; weapon_blocked is
// the decoded weapon byte tested at 0x43371F12.
bool process_slow_motion_stage(const SlowMotionSettings& settings, InputBitmap input,
                               const MovementPhysics& physics, std::int32_t weapon_state,
                               bool weapon_blocked, std::uint32_t& scheduler_flags,
                               std::int32_t command_number, UserCommand& command,
                               MovementVector velocity, MovementAngles command_angles,
                               MovementAngleView movement_angles);

} // namespace skeet

// Declarations from speed_control.hpp

namespace skeet {
// Complete 0x43340ACB and 0x43388405. Callers retain the feature scheduler's
// original ground/weapon/hotkey gates. Fast walk passes target 134 and true;
// slow-motion callers pass their computed target and acceleration flag.
void regulate_movement_speed(const MovementPhysics& physics, UserCommand& command,
                             MovementVector velocity, MovementAngles command_angles,
                             MovementAngleView movement_angles, float target_speed,
                             bool clear_speed_on_acceleration) noexcept;
bool write_counter_movement(const MovementPhysics& physics, UserCommand& command,
                            MovementVector velocity, MovementAngleView movement_angles) noexcept;
} // namespace skeet

// Declarations from speed_control_live.hpp

namespace skeet {
struct MovementCommandBindings;

// Live433EE5FC. Duck-transition reads precede weapon lookups; acceleration
// and final interval are read at their native later stages.
GroundAccelerationCapacity live_movement_acceleration_capacity(void* player, const UserCommand&,
                                                               MovementVector velocity,
                                                               MovementVector direction,
                                                               const MovementCommandBindings&);

// Complete43340ACB with actual command-angle references and stage-local
// engine reads. Both braking and acceleration reach the live capacity path.
void regulate_live_movement_speed(void* player, UserCommand&, MovementVector velocity,
                                  ConstMovementAngleView command_angles,
                                  MovementAngleView movement_angles, float target_speed,
                                  bool clear_speed_on_acceleration, const MovementCommandBindings&);
} // namespace skeet

// Declarations from steering_math.hpp

namespace skeet {

// Complete native scalar helpers used by the air-movement solver.
float round_movement_step(float value) noexcept; // 0x4341AD12..0x4341ADBD
float planar_movement_yaw(MovementVector direction) noexcept; // 0x43397834..0x43397878
// 0x43366128..0x433662C5. Each output is optional. All angles are read before
// the forward, right, then up writes, so outputs may alias each other/input.
void movement_angle_vectors(MovementAngles angles, MovementVector* forward, MovementVector* right,
                            MovementVector* up) noexcept;
// Complete airborne input-heading block 0x43373D4B..0x43373F63. Only yaw is
// replaced; inputs are the moves saved before the solver clears the command.
void project_movement_heading(float forward_move, float side_move,
                              MovementAngleView movement_angles) noexcept;
// Raw-basis counterpart 0x43379BB2..0x43379C55; its basis and combined input
// vector are not normalized before converting the heading.
void project_raw_movement_heading(float forward_move, float side_move,
                                  MovementAngleView movement_angles) noexcept;
float normalize_movement_angle(float value) noexcept;
// Complete 0x433E9160 angle-vector normalization, including NaN handling.
void normalize_movement_angles(MovementAngleView angles) noexcept;

} // namespace skeet

// Declarations from types.hpp

namespace skeet {
struct MovementVector {
    float x, y, z;
};
struct MovementAngles {
    float pitch, yaw, roll;
};

// Native movement angles can belong to either a dedicated vector or the
// command frame's three-float array. Reference views preserve intermediate
// writes across engine calls without aliasing unrelated C++ aggregate types.
struct MovementAngleView {
    float& pitch;
    float& yaw;
    float& roll;
    MovementAngleView(MovementAngles& value) noexcept
        : pitch(value.pitch), yaw(value.yaw), roll(value.roll) {}
    MovementAngleView(std::array<float, 3>& value) noexcept
        : pitch(value[0]), yaw(value[1]), roll(value[2]) {}
    MovementAngleView(const MovementAngleView&) = default;
    operator MovementAngles() const noexcept {
        return {pitch, yaw, roll};
    }
    MovementAngleView& operator=(MovementAngles value) noexcept {
        pitch = value.pitch;
        yaw = value.yaw;
        roll = value.roll;
        return *this;
    }
    MovementAngleView& operator=(const MovementAngleView& value) noexcept {
        return *this = static_cast<MovementAngles>(value);
    }
};
struct ConstMovementAngleView {
    const float& pitch;
    const float& yaw;
    const float& roll;
    ConstMovementAngleView(const MovementAngles& value) noexcept
        : pitch(value.pitch), yaw(value.yaw), roll(value.roll) {}
    ConstMovementAngleView(const std::array<float, 3>& value) noexcept
        : pitch(value[0]), yaw(value[1]), roll(value[2]) {}
    ConstMovementAngleView(MovementAngleView value) noexcept
        : pitch(value.pitch), yaw(value.yaw), roll(value.roll) {}
    operator MovementAngles() const noexcept {
        return {pitch, yaw, roll};
    }
};

inline constexpr std::uint32_t duck_button = 4;
inline constexpr std::uint32_t forward_button = 8;
inline constexpr std::uint32_t back_button = 16;
inline constexpr std::uint32_t move_left_button = 0x200;
inline constexpr std::uint32_t move_right_button = 0x400;
inline constexpr std::uint32_t speed_button = 0x20000;

// Decoded values supplied by the engine adapter. No encoded offsets, binary
// entrypoints, or guessed netvar offsets are required by the algorithms.
struct MovementPhysics {
    float interval_per_tick;
    float stop_speed;
    float friction;
    float accelerate;
    float player_speed_limit; // decoded player field, used by min helper
    float player_max_speed; // external player virtual method
    std::uint32_t player_flags;
    bool ducking;
    float duck_amount;
    float duck_speed;
    bool weapon_speed_enabled; // encoded cvar guard is not self-valued
    bool has_weapon; // external entity lookup succeeded
    float weapon_max_speed; // external weapon virtual method
    int weapon_mode_count; // external weapon virtual method
    int weapon_mode; // external weapon virtual method
};
} // namespace skeet

// Declarations from weapon_speed_phase.hpp

namespace skeet {
struct MovementCommandBindings;

struct WeaponSpeedPhaseSettings {
    std::uint32_t policy_flags; // 43467E40; weapon state replaces bits 1..3
};

struct WeaponSpeedPhasePlayer {
    std::uint32_t entity;
    MovementPhysics physics;
    MovementVector velocity;
};

struct WeaponSpeedPhaseFrame {
    WeaponSpeedPhasePlayer player;
    MovementVector position;
    std::uint32_t simulation_mode;
    MovementSimulationPlayer snapshot;
    MovementSimulationRules rules;
    MovementSimulationEnvironment simulation_environment;
    float current_time;
    float player_next_attack; // decoded DE530A8B field
    float weapon_next_attack; // decoded F71CE331 field
    float attack_guard_8815d240;
    std::int32_t weapon_state; // 43468488; -1 means no per-weapon override
    std::uint16_t weapon_definition;
    bool weapon_blocked; // decoded B8969256 weapon byte
};

enum class WeaponSpeedPhaseContinuation {
    player_movement, // 43372C76
    complete, // 4337CAA4
};

// Native 4337206A..43372C6A, after the preceding ground/weapon gates.
// Simulation and acceleration execute recovered source directly. Collision
// remains the actual engine boundary; frame members are decoded engine reads.
// The full copied command supplies simulation angles and the signed sequence
// number used by the two periodic regulators.
WeaponSpeedPhaseContinuation
process_weapon_speed_phase(const WeaponSpeedPhaseSettings&, const WeaponSpeedPhaseFrame&,
                           client::Command&, MovementAngles command_angles,
                           MovementAngleView movement_angles, std::uint32_t& scheduler_flags,
                           MovementCollisionEngine& collision);

// Live 4337206A..43372C6A. Player/weapon reads, constructor getters, cvars
// and acceleration inputs remain at their original branch sites. Command
// view angles remain a live reference through regulator engine calls.
WeaponSpeedPhaseContinuation process_live_weapon_speed_phase(void* player, void* weapon,
                                                             client::Command&,
                                                             MovementAngleView movement_angles,
                                                             const MovementCommandBindings&);

} // namespace skeet

// Declarations from z_hop.hpp

namespace skeet {

struct ZHopSettings {
    bool enabled;
    HotkeyBinding hotkey;
    std::int32_t amplitude;
    std::int32_t frequency;
};

// Complete 0x43376E82..0x43376EF4 branch, also used at 0x4337C63B.
// The caller chooses the pre-speed/Z-Hop branch and supplies engine time.
bool apply_z_hop_heading(const ZHopSettings& settings, InputBitmap input, float engine_time,
                         MovementAngleView movement_angles);

} // namespace skeet

// Declarations from movement_math.hpp

namespace skeet::movement_detail {
constexpr float epsilon = 0x1p-23f;
constexpr float degrees_to_radians = 0.01745329238474369049072265625f;
constexpr float radians_to_degrees = 57.2957763671875f;
inline float negate(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000u);
}
inline float absolute(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) & 0x7fffffffu);
}
inline float dot(MovementVector a, MovementVector b) noexcept {
    return (a.y * b.y + a.x * b.x) + a.z * b.z;
}
inline float horizontal_length(MovementVector value) noexcept {
    return std::sqrt(value.x * value.x + value.y * value.y);
}
inline float command_length(const UserCommand& command) noexcept {
    return std::sqrt(
        (command.forward_move * command.forward_move + command.side_move * command.side_move) +
        command.up_move * command.up_move);
}
inline MovementVector normalize_horizontal(MovementVector value) noexcept {
    const float reciprocal = 1.0f / (horizontal_length(value) + epsilon);
    return {value.x * reciprocal, value.y * reciprocal, 0.0f};
}
inline float friction_speed(const MovementPhysics& physics, float speed) noexcept {
    const float control = maximum_number(physics.stop_speed, speed);
    const float drop = (control * physics.friction) * physics.interval_per_tick;
    return maximum_number(speed - drop, 0.0f);
}
inline MovementAngles horizontal_angles(MovementVector value, bool negative_pitch_zero) noexcept {
    if (value.x == 0.0f && value.y == 0.0f)
        return {90.0f, 0.0f, 0.0f};
    const float yaw =
        static_cast<float>(std::atan2(static_cast<double>(value.y), static_cast<double>(value.x)));
    const float pitch = static_cast<float>(std::atan2(
        negative_pitch_zero ? -0.0 : 0.0, static_cast<double>(horizontal_length(value))));
    return {pitch * radians_to_degrees, yaw * radians_to_degrees, 0.0f};
}
inline bool weapon_slowdown_exception(const MovementPhysics& physics) noexcept {
    return physics.weapon_mode_count > 0 && physics.weapon_mode > 1 &&
           physics.weapon_max_speed * 0.52f < 110.0f;
}
// SSE MAXSS/MINSS select the second operand on unordered comparisons.
inline float sse_max(float a, float b) noexcept {
    return a > b ? a : b;
}
inline float sse_min(float a, float b) noexcept {
    return a < b ? a : b;
}
inline float walking_acceleration(float acceleration, float speed, float limit) noexcept {
    speed = maximum_number(speed, 0.0f);
    if (speed > limit - 5.0f && speed - limit >= -5.0f) {
        float scale = (limit * 0.2f - (1.0f - speed * -0.2f)) - -1.0f;
        scale = sse_min(sse_max(scale, 0.0f), 1.0f);
        return scale * acceleration;
    }
    return acceleration;
}
inline void command_basis(MovementAngles angles, MovementVector& forward,
                          MovementVector& right) noexcept {
    const float pitch = angles.pitch * degrees_to_radians;
    const float yaw = angles.yaw * degrees_to_radians;
    const float roll = angles.roll * degrees_to_radians;
    const float sp = std::sin(pitch), cp = std::cos(pitch);
    const float sy = std::sin(yaw), cy = std::cos(yaw);
    const float sr = std::sin(roll), cr = std::cos(roll);
    forward = {cy * cp, sy * cp, negate(sp)};
    const float minus_cr = cr * -1.0f;
    const float sr_sp = sr * sp;
    right = {minus_cr * negate(sy) - sr_sp * cy, minus_cr * cy - sr_sp * sy, (sr * -1.0f) * cp};
}
}
