#pragma once

// HvH / Anti-Aim Subsystem Declarations
#include "skeet/anti_aim/config.hpp"
#include "skeet/anti_aim/state.hpp"
#include "skeet/anti_aim/scheduler.hpp"
#include "skeet/anti_aim/pitch.hpp"
#include "skeet/anti_aim/body_yaw.hpp"
#include "skeet/anti_aim/edge.hpp"
#include <limits>
#include "skeet/anti_aim/freestanding.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/anti_aim/jitter.hpp"
#include "skeet/anti_aim/body_freestanding.hpp"
#include "skeet/anti_aim/runtime.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/anti_aim/angles.hpp"
#include "skeet/anti_aim/eligibility.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/config.hpp"
#include "skeet/movement/collision.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/anti_aim/proximity.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/anti_aim/yaw.hpp"
#include "skeet/rage/ballistics.hpp"
#include <array>
#include <cstdint>
#include "skeet/combobox.hpp"
#include <span>
#include "skeet/movement/simulation_capture.hpp"
#include "skeet/movement/simulation_tick.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/hud/feature_indicators.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include <vector>
#include "skeet/anti_aim/movement_capture.hpp"
#include "skeet/anti_aim/command_phase.hpp"
#include "skeet/anti_aim/precalculation.hpp"
#include "skeet/anti_aim/finisher.hpp"
#include "skeet/anti_aim/command_owner.hpp"
#include <memory>
#include "skeet/rage_hitboxes.hpp"
#include "skeet/rage_geometry.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/movement/prediction.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/esp/player_runtime.hpp"
#include "skeet/anti_aim/target_geometry.hpp"


// Declarations from angles.hpp

namespace skeet::anti_aim {
struct AngleParameters {
    float value_134 = 60.0f, value_138 = 180.0f, value_13c = 118.0f, value_140 = 58.0f;
    float value_144 = 179.0f, value_148 = 90.0f, value_14c = 20.0f, value_150 = 89.0f,
          value_154 = 0.75f;
    std::array<float, 12> pattern_15c{-1, 1, 0, -1, 1, -1, 0, 1, -1, 0, 1, 0};
};
struct AngleStageState : State {
    float cached_yaw_18c = std::numeric_limits<float>::max();
    float saved_yaw_198 = 0.0f, reference_yaw_258 = 0.0f;
};
struct AngleFrame {
    ActionGateFrame gates;
    float scalar_c0, scalar_38, height_164, angle_9c, angle_b8, angle_a0, interpolation_158;
    BallisticVector origin_10c, origin_168, origin_dc;
    std::int32_t command_ac;
};
struct AnglePlayer {
    ActionGatePlayer gates;
    float edge_scalar_3eb71843;
    std::uint8_t state_7501cb91;
    float weapon_pitch_340;
    AnimationYawInput animation;
    float feet_yaw_80;
};
template <class Body> struct AngleStageBindingsFor {
    ProximityEngine& proximity;
    EdgeEngine& edge;
    BallisticEngine& ballistics;
    const FreestandingContext& freestanding;
    const Body& body;
    const PitchInterfaces& pitch;
    const JitterRandom& random;
    std::uint8_t* trace_flag;
    std::int32_t last_body_command, shot_command, last_sent_command;
    bool shot_correction;
};
using AngleStageBindings = AngleStageBindingsFor<BodyYawFreestandingBindings>;
using LiveAngleStageBindings = AngleStageBindingsFor<LiveBodyYawFreestandingBindings>;
// Complete main angle stage4332D3D2..4332EF25, after targetrefresh. It calls
// concrete action/pitch/edge/freestanding/yaw/jitter/body solvers in nativeorder.
// The separate eligibility/cycle prefix runs before this stage.
void process_angle_stage(const GlobalConfig&, const Config&, const AngleParameters&,
                         AngleStageState&, AngleFrame&, const AnglePlayer&, ActionGateGlobals&,
                         client::Command&, std::uint8_t& send_packet, const AngleStageBindings&);
void process_angle_stage(const GlobalConfig&, const Config&, const AngleParameters&,
                         AngleStageState&, AngleFrame&, const AnglePlayer&, ActionGateGlobals&,
                         client::Command&, std::uint8_t& send_packet,
                         const LiveAngleStageBindings&);
} // namespace skeet::anti_aim

// Declarations from body_freestanding.hpp

namespace skeet::anti_aim {
struct BodyFreestandingContext {
    std::int32_t target_handle;
    RageVector target_origin;
    RageVector shot_position;
    EntityLookup entities;
};
struct BodyFreestandingMath {
    RageTrigonometry trigonometry;
    double (*atan2)(double, double);
};
struct BodyFreestandingBones {
    void* player;
    animation::BoneEntityState* state;
    const animation::BoneModel* model;
    const animation::BoneEngine* engine;
    float* eye_pitch; // decoded C1B66EE8 +4
    float* eye_yaw; // decoded C1B66EE8 +8
    float* pose_parameter_11; // decoded 1CAAF0A8 +2C
    bool hitbox_set_present;
    std::int32_t hitbox_count;
    std::int32_t first_hitbox_bone;
    RageVector first_hitbox_minimum, first_hitbox_maximum;
    std::span<const RageLocalHitbox> hitboxes;
};
// Per-call production projection. Hierarchy, selected set and descriptors are
// acquired from the current entity; the bone transaction uses the source live
// BoneRuntime rather than a detached BoneEntityState snapshot.
struct LiveBodyFreestandingBones {
    void* player;
    const animation::BoneRuntime& runtime;
    float* eye_pitch;
    float* eye_yaw;
    float* pose_parameter_11;
    bool hitbox_set_present;
    std::int32_t hitbox_count;
    std::int32_t first_hitbox_bone;
    RageVector first_hitbox_minimum, first_hitbox_maximum;
    std::span<const RageLocalHitbox> hitboxes;
    std::span<const std::uint32_t> hitbox_bones;
};
struct LiveBodyFreestandingSource {
    void* player;
    const animation::BoneRuntime& runtime;
    std::uint32_t eye_angles, pose_parameters;
};
struct LiveBodyFreestandingModel {
    bool hitbox_set_present;
    std::int32_t hitbox_count;
    std::array<RageLocalHitbox, 19> hitboxes;
    std::array<std::uint32_t, 19> hitbox_bones;
};
LiveBodyFreestandingModel capture_body_freestanding_model(const animation::BoneRuntime&,
                                                          const void* player);
LiveBodyFreestandingBones bind_body_freestanding_bones(const animation::BoneRuntime&, void* player,
                                                       const LiveBodyFreestandingModel&,
                                                       std::uint32_t eye_angles,
                                                       std::uint32_t pose_parameters) noexcept;
struct BodyFreestandingWorkspace {
    // Failed native bone builds leave their corresponding scratch center
    // untouched; preserve that caller-owned storage through all hypotheses.
    std::array<RageVector, 3> centers;
    std::array<Matrix3x4, animation::maximum_bones> matrices;
    FreestandingWorkspace freestanding;
};
// Complete native 433B7773. The command argument is unused by the native body.
bool solve_animation_body_freestanding(const RageVector& frame_origin_dc,
                                       const BodyFreestandingContext&, BodyFreestandingBones&,
                                       float maximum_yaw, float& result,
                                       const BodyFreestandingMath&, BodyFreestandingWorkspace&);
bool solve_animation_body_freestanding(const RageVector& frame_origin_dc,
                                       const BodyFreestandingContext&, LiveBodyFreestandingSource&,
                                       float maximum_yaw, float& result,
                                       const BodyFreestandingMath&, BodyFreestandingWorkspace&);
struct BodyFreestandingSettings {
    std::int32_t yaw_mode_2c;
    float maximum_angle_148;
};
// Complete 433375AC: the head must intersect, then a torso hitgroup 2/3
// must intersect at a strictly earlier fraction. Every clip is engine 43468C3C.
bool body_occludes_head(const RageRay&, const BodyFreestandingBones&, std::span<const Matrix3x4>,
                        const RageBoxClipper&);
bool body_occludes_head(const RageRay&, const LiveBodyFreestandingBones&,
                        std::span<const Matrix3x4>, const RageBoxClipper&);
// Complete 433EFAAA: three bone hypotheses, five support rays each, then
// strict maximum coverage selection. Unequal coverage is the success result.
bool solve_covered_body_freestanding(const BodyFreestandingSettings&,
                                     const RageVector& frame_origin_dc,
                                     const BodyFreestandingContext&, BodyFreestandingBones&,
                                     float command_pitch, float base_yaw, float maximum_yaw,
                                     float& result, const BodyFreestandingMath&,
                                     const RageBoxClipper&, BodyFreestandingWorkspace&);
bool solve_covered_body_freestanding(const BodyFreestandingSettings&,
                                     const RageVector& frame_origin_dc,
                                     const BodyFreestandingContext&, LiveBodyFreestandingSource&,
                                     float command_pitch, float base_yaw, float maximum_yaw,
                                     float& result, const BodyFreestandingMath&,
                                     const RageBoxClipper&, BodyFreestandingWorkspace&);
} // namespace skeet::anti_aim

// Declarations from body_yaw.hpp

namespace skeet::anti_aim {
struct AnimationYawInput {
    float duck_amount_a4, running_fraction_f8, ducking_fraction_fc;
    float speed_fraction_11c, minimum_yaw_334, maximum_yaw_338;
};
// Complete433EAEEB. Native writes maximum before reading/writing minimum.
void animation_yaw_limits(const AnimationYawInput&, float& minimum, float& maximum) noexcept;
struct BodyYawSettings {
    std::int32_t mode, jitter_mode, jitter_seed_98, static_offset_9c;
    float opposite_angle_134, jitter_maximum_144;
    std::array<float, 12> skitter;
};
struct BodyYawState {
    std::uint32_t phase_190;
    float cached_opposite_1a0, reference_yaw_258;
};
// Regular branch of43365675 with freestanding_body_yaw disabled. Valid modes
// are1..3; the native caller excludes0. Special freestanding solves are separate.
float regular_body_yaw(const BodyYawSettings&, BodyYawState&, std::int32_t frame_command,
                       std::int32_t last_command, float base_yaw, const JitterRandom&);
struct BodyBlendSettings {
    float minimum_magnitude_134, maximum_magnitude_138, base_angle_13c, expanded_angle_144,
        scale_154;
};
struct BodyBlendInput {
    float offset, minimum_yaw, maximum_yaw, base_yaw, feet_yaw;
    std::int32_t choked_commands, choke_limit;
};
// Native numeric/packet blend4332E973..4332EE37, exitingbeforecommand-yaw
// normalization. The integer truncation gate intentionally treats(-1,1)as0.
void blend_body_yaw(const BodyBlendSettings&, const BodyBlendInput&, float& command_yaw,
                    std::uint8_t& send_packet) noexcept;
struct BodyYawRequest {
    bool freestanding_body_yaw;
    std::int32_t frame_command, last_command, shot_command;
    RageVector frame_origin_dc;
    float command_pitch, base_yaw, maximum_yaw;
};
template <class Bones> struct BodyYawFreestandingBindingsFor {
    BodyFreestandingSettings settings;
    const BodyFreestandingContext& context;
    Bones& bones;
    const BodyFreestandingMath& math;
    const RageBoxClipper& clipper;
    BodyFreestandingWorkspace& workspace;
};
using BodyYawFreestandingBindings = BodyYawFreestandingBindingsFor<BodyFreestandingBones>;
using LiveBodyYawFreestandingBindings = BodyYawFreestandingBindingsFor<LiveBodyFreestandingSource>;
// Complete43365675 on native valid body modes1..3. Special bone/coverage
// solves execute before the regular branch; only actual engine calls are bound.
float solve_body_yaw(const BodyYawSettings&, BodyYawState&, const BodyYawRequest&,
                     const BodyYawFreestandingBindings&, const JitterRandom&);
float solve_body_yaw(const BodyYawSettings&, BodyYawState&, const BodyYawRequest&,
                     const LiveBodyYawFreestandingBindings&, const JitterRandom&);
} // namespace skeet::anti_aim

// Declarations from command_owner.hpp

namespace skeet::client {
class TargetRefreshRuntimeOwner;
}
namespace skeet::anti_aim {
struct CommandOwnerPhase2Bindings {
    MovementCollisionEngine& collision;
    MovementSimulation& workspace;
    const client::ScriptCommandEnvironment& roll_guard;
};
struct CommandOwnerPitchRandomBindings {
    void* context;
    void (*seed)(void*, std::int32_t);
    std::int32_t (*integer)(void*, std::int32_t, std::int32_t);
};
struct CommandOwnerEligibilityBindings {
    void* const& input_system;
    void* const& engine_client;
    void* const& hud_context;
    engine::MemberFunction<void*, const char*> const& find_hud;
};
struct CommandOwnerAngleBindings {
    ProximityEngine& proximity;
    BallisticEngine& ballistics;
    const BodyFreestandingMath& body_math;
    const RageBoxClipper& clipper;
    BodyFreestandingWorkspace& workspace;
    const CommandOwnerPitchRandomBindings& pitch;
    const JitterRandom& random;
};
struct CommandOwnerPhase4Bindings {
    const CommandOwnerEligibilityBindings& eligibility;
    const client::TargetRefreshRuntimeOwner& target_refresh;
    const CommandOwnerAngleBindings& angles;
};
struct CommandOwnerPhase6Bindings {
    const float& simulation_time;
    const RageVector& final_angles;
    const lagcomp::EntityNetvars& entity_offsets;
    std::uint8_t* const& reset_byte;
    void* const& animation_update;
    lagcomp::SetAbsoluteVector const& set_absolute_angles;
    const animation::BoneRuntime& bones;
};
struct CommandOwnerOffsets {
    std::uint32_t player_flags_e2fbe695, velocity_36375c23, duck_3309c0cf, rate_fb7e3404;
    std::uint32_t ducking_6b1a59b5, movement_type_7501cb91, movement_type_8b5586f6;
    std::uint32_t cached_a15c3105, tick_base_3d10b94c, field_7fcb7381, edge_3eb71843;
    std::uint32_t guard_d720aea6, clock_9f8e7f71;
    std::uint32_t rule_bf567619, rule_dda99ab5, rule_b7ea8fda;
    std::uint32_t weapon_state_e1df7e9b, weapon_time_3b93667b;
    std::uint32_t flags_38ca5228, field_043fa575, field_334b460c, field_86a5f8b3;
};
struct CommandOwnerGlobals {
    std::int32_t* const& mode; //43475B2C, pointed value read at dispatch
    std::int32_t* const& choked; //434757DC
    float* const& interval; //4346A338
    float* const& current_time; //43467EB8
    void** const& shared_player; //4346A3A4
    std::int32_t* const& sequence; //434678F8
    std::int32_t* const& maximum_clients; //43469300
    void* const& engine_globals; //43467B04
    const std::int32_t& prediction_budget; //43469274
    std::uint32_t& scheduler_flags; //434759C0
    TargetState& target;
    const std::int32_t& last_body_command; //434759C8
    const std::int32_t& shot_command; //434759CC
    const std::int32_t& last_sent_command; //434759F4
    const std::uint8_t& shot_correction; //43468410
    const RageVector& shot_position; //434759D0
    const HotkeyBinding& height_hotkey; //43467C8C
    const InputBitmap& input;
    std::uint8_t* const& trace_flag; //434683D0
    const std::uint8_t& pending_angles_active; //43468D7C
    const std::array<animation::AnimationAngleSample, 2>& pending_angles; //434759DC,E8
};
// Concrete owner data consumed by the CreateMove master's native registration
// list. The phase bindings retain actual engine boundaries and live model/state
// views; player/frame/command/global reads below are refreshed at each dispatch.
struct CommandOwner {
    const GlobalConfig& global;
    Config& config;
    AngleParameters& parameters;
    AngleStageState& state;
    PrecalculationState& precalculation;
    FinisherState& finisher;
    const CommandOwnerPhase2Bindings& phase2;
    const CommandOwnerPhase4Bindings& phase4;
    const CommandOwnerPhase6Bindings& phase6;
    const MovementCaptureInput& movement;
    const MovementEnvironmentBindings& environment;
    CommandOwnerOffsets offsets;
    CommandOwnerGlobals globals;
    void* const& engine_client;
    void* const& engine_trace;
    void* const& entity_list;
};
AngleFrame capture_angle_frame(const client::CommandFrame&) noexcept;
void command_owner_phase2(CommandOwner&, void* player, void* weapon, client::Command&,
                          const std::array<float, 3>& incoming_angles, std::uint8_t& send_packet);
void command_owner_phase4(CommandOwner&, client::CommandFrame&, void* player, client::Command&,
                          std::uint8_t& send_packet);
void command_owner_phase6(CommandOwner&, client::CommandFrame&, void* player, client::Command&,
                          std::uint8_t& send_packet);
} // namespace skeet::anti_aim

// Declarations from command_phase.hpp

namespace skeet::anti_aim {
struct CommandPhaseInput {
    std::uint32_t secondary_frame_flags;
    bool player_a15c3105;
    EligibilityGuard guard;
};
struct CommandPhaseBindings {
    const EligibilityBindings& eligibility;
    const TargetRefreshContext& target_refresh;
    const AngleStageBindings& angles;
};
// Complete phase4 owner4332D1C6. The UI yaw enum has native range0..5.
// TargetState is the shared4346DB14 publication also consumed by RAGE/packets.
void process_command_angles(const GlobalConfig&, const Config&, const AngleParameters&,
                            AngleStageState&, AngleFrame&, const AnglePlayer&, ActionGateGlobals&,
                            client::Command&, std::uint8_t& send_packet, TargetState&,
                            const CommandPhaseInput&, const CommandPhaseBindings&);
} // namespace skeet::anti_aim

// Declarations from config.hpp

namespace skeet::anti_aim {
using BodyYawOffsetsConfig = ConfigValue<std::array<std::int32_t, 4>>;
using YawOffsetsConfig = ConfigValue<std::array<std::int32_t, 6>>;

struct GlobalConfig {
    BooleanConfig enabled{{{nullptr, nullptr, 0x96C0EF29u}, 1, ConfigType::boolean, 0},
                          0}; // 0x4346A730
    IntegerConfig pitch{{{nullptr, nullptr, 0x6D5F1172u}, 4, ConfigType::integer, 0},
                        0}; // 0x43467F50
    IntegerConfig yaw{{{nullptr, nullptr, 0xF2D408F3u}, 4, ConfigType::integer, 0},
                      0}; // 0x434682EC
    IntegerConfig body_yaw{{{nullptr, nullptr, 0x4D6D88C6u}, 4, ConfigType::integer, 0},
                           0}; // 0x434685A8
};

// Offsets are relative to the owner constructed at 0x43490435.
struct Config {
    IntegerConfig yaw_base; // 0x01C
    IntegerConfig yaw_jitter; // 0x030
    IntegerConfig yaw_jitter_offset; // 0x044
    BooleanConfig edge_yaw; // 0x058
    BooleanConfig freestanding; // 0x06C
    BodyYawOffsetsConfig body_yaw_offsets; // 0x080
    YawOffsetsConfig yaw_offsets; // 0x0A0
    HotkeyConfig freestanding_hotkey; // 0x0C8
    BooleanConfig freestanding_body_yaw; // 0x0F0
    IntegerConfig roll; // 0x104
    IntegerConfig custom_pitch; // 0x118
};

// Static descriptor registration preserves their stored values. The owner
// constructor registers booleans without overwriting their stored bytes and
// clears integer/array values at 0x43490482..0x43490599, then sets the final
// yaw defaults (-12 for 180, 80 for Spin) at 0x434906DF..0x434906F3.
void register_global_config(ConfigRegistries&, GlobalConfig&) noexcept;
//43490482..43490599. The late constructor yaw defaults are separate so the
// whole owner retains the intervening string/animation initialization order.
void initialize_owner_config(ConfigRegistries&, Config&) noexcept;
void finish_owner_config(Config&) noexcept;
void initialize_config(ConfigRegistries&, Config&) noexcept;
struct FakeLagSettings {
    BooleanConfig enabled{{{nullptr, nullptr, 0xF9BBEC7BU}, 1, ConfigType::boolean, 0},
                          0}; //43468D54
    HotkeyConfig hotkey{{{nullptr, nullptr, 0x27D21BD4U}, 8, ConfigType::hotkey, 0}, {}}; //43467E00
    IntegerConfig amount, variance, limit; //owner434901DA +0C/+20/+34
};
struct OtherGlobalSettings {
    HotkeyConfig slow_motion_key{{{nullptr, nullptr, 0x7293DE2AU}, 8, ConfigType::hotkey, 0},
                                 {}}; //4346849C
    IntegerConfig leg_movement{{{nullptr, nullptr, 0x8F8A4366U}, 4, ConfigType::integer, 0},
                               0}; //434685E8
};
// Independent static constructors and the complete configuration portion of
//434901DA. Runtime fields andRDTSCseeding belong to network::FakeLagState.
void initialize_fake_lag_settings(ConfigRegistries&, FakeLagSettings&) noexcept;
void initialize_fake_lag_enabled(ConfigRegistries&, FakeLagSettings&) noexcept;
void initialize_fake_lag_hotkey(ConfigRegistries&, FakeLagSettings&) noexcept;
void initialize_other_global_settings(ConfigRegistries&, OtherGlobalSettings&) noexcept;
} // namespace skeet::anti_aim

// Declarations from edge.hpp

namespace skeet::anti_aim {
struct EdgeInput {
    std::uint8_t enabled; // secondary owner +0x60; full owner +0x68
    float frame_scalar_c0;
    float player_scalar; // decoded field 0x3EB71843
    std::uint8_t movement_type; // decoded field 0x7501CB91
};
struct EdgeSettings {
    float maximum_angle; // secondary +0x140, full owner +0x148
    float hull_padding; // secondary +0x144, full owner +0x14C
};
class EdgeEngine : public MovementCollisionEngine {
public:
    // These are the actual player/engine virtual boundaries, in their native
    // call order. Returned hull references may alias engine-owned storage.
    virtual const MovementVector& hull_minimum() = 0; // player +0x130
    virtual const MovementVector& hull_maximum() = 0; // player +0x134
    virtual const MovementVector& origin() = 0; // player +0x28
    virtual MovementAngles view_angles() = 0; // engine +0x48
    virtual std::uint32_t point_contents_world(const MovementVector&, std::uint32_t mask) = 0;
};

// Complete edge cut 0x4332D90A..0x4332E5BC, including its three gates and
// original world-trace wrapper. False continues at E5BC without changing yaw.
// True continues at E621: the caller also sets frame bit0x80000 and full-owner
// cache +0x1A8 to FLT_MAX, exactly as the native success tail does.
bool solve_edge_yaw(const EdgeInput&, const EdgeSettings&, EdgeEngine&,
                    std::uint8_t* engine_trace_flag, float& command_yaw);
} // namespace skeet::anti_aim

// Declarations from eligibility.hpp

namespace skeet::anti_aim {
struct EligibilitySettings {
    bool enabled;
    std::int32_t pitch, yaw, body_yaw;
};
struct EligibilityFrame {
    std::uint32_t flags, secondary_flags;
};
struct EligibilityGuard {
    bool shared_player_available, guard;
    float current_time; //433FC87C: shared player's decoded9F8E7F71 field
    float reference_time; // pointed43467EB8
};
struct EligibilityBindings {
    void* input_system; //4346A89C
    void* engine_client; //4346A514
    void* hud_context; //43468994
    engine::MemberFunction<void*, const char*> find_hud; //4346A374, optional
};
// Complete43384AE8 helper, retaining actual FindHudElement/virtual34 calls.
bool hud_chat_active(const EligibilityBindings&);
// Phase4 entry4332D1C6..4332D28B, including its unconditional flag update.
bool command_eligible(const EligibilitySettings&, const EligibilityFrame&, const EligibilityGuard&,
                      std::int32_t command_handle_1c4, const EligibilityBindings&,
                      std::uint32_t& flags_130);
// Native4332D28B..4332D2FF, before writing owner flag8 and advancing the cycle.
bool command_uses_cached_angles(bool player_a15c3105, std::int32_t movement_type,
                                std::uint32_t frame_flags, std::uint32_t buttons, ProximityEngine&,
                                ProximityEntity local);
} // namespace skeet::anti_aim

// Declarations from finisher.hpp

namespace skeet::anti_aim {
struct FinisherState {
    AngleStageState& common;
    // Native full owner+1D8. Initialized by433F526C, then retained across ticks.
    std::span<std::byte, 0x348> animation_1d8;
    RageVector absolute_angles_1b8;
    std::span<Matrix3x4> matrices_520;
};
struct FinisherPlayer {
    std::uint32_t identity, flags_38ca5228;
    std::uint8_t field_043fa575, field_334b460c;
    float field_86a5f8b3_minus4;
};
struct FinisherBindings {
    void* player;
    void*& animation_state; // actual decoded3D52A1A7 slot
    float& scalar_a2a8247f;
    const float& simulation_time; //43476B40
    const std::uint8_t* const& build_bones_12c; //live owner+12C pointer, read after animationupdate
    RageVector& eye_angles; // decodedC1B66EE8+4
    const RageVector& final_angles; //4347A4F0
    const lagcomp::EntityNetvars& entity_offsets;
    const animation::AnimationAngleHistory& angle_history;
    const animation::AnimationUpdateTiming& timing;
    const animation::AnimationUpdateEngine& update;
    lagcomp::SetAbsoluteVector set_absolute_angles; // optional434688C8
    animation::BoneEntityState& bone_state;
    const animation::BoneModel& model;
    const animation::BoneEngine& bones;
    const RageTrigonometry& trigonometry;
};
struct LiveFinisherBindings {
    void* player;
    void*& animation_state;
    float& scalar_a2a8247f;
    const float& simulation_time;
    const std::uint8_t* const& build_bones_12c;
    RageVector& eye_angles;
    const RageVector& final_angles;
    const lagcomp::EntityNetvars& entity_offsets;
    const animation::AnimationAngleHistory& angle_history;
    const animation::AnimationUpdateTiming& timing;
    const animation::AnimationUpdateEngine& update;
    lagcomp::SetAbsoluteVector set_absolute_angles;
    const animation::BoneRuntime& bones;
};
// Complete phase6 owner433470E4, including saved-state reinitialization,
// temporary player animation/scalar/pose transaction and optional bone cache.
void finish_command_angles(bool enabled, std::int32_t body_yaw, FinisherState&,
                           const FinisherPlayer&, std::int32_t frame_command,
                           const client::Command&, std::uint8_t send_packet,
                           const FinisherBindings&);
void finish_command_angles(bool enabled, std::int32_t body_yaw, FinisherState&,
                           const FinisherPlayer&, std::int32_t frame_command,
                           const client::Command&, std::uint8_t send_packet,
                           const LiveFinisherBindings&);
} // namespace skeet::anti_aim

// Declarations from freestanding.hpp

namespace skeet::anti_aim {
struct FreestandingScore {
    std::int32_t maximum_distance = 0, total_distance = 0, damage = 0;
    BallisticVector direction{};
};
struct FreestandingRay {
    bool enabled = false;
    BallisticVector start{}, end{};
};
struct FreestandingWorkspace {
    BallisticQuery query;
    std::array<FreestandingScore, 3> scores{};
    // 43381817 leaves ray 5's coordinates untouched, including when disabled.
    // The caller retains this scratch slot, matching native preexisting memory.
    std::array<FreestandingRay, 12> rays{};
};
void build_freestanding_rays(std::array<FreestandingRay, 12>&,
                             const std::array<FreestandingScore, 3>&, const BallisticVector& start,
                             const BallisticVector& target,
                             const BallisticVector& side); // 43381817
bool freestanding_difference(const std::array<FreestandingScore, 3>&) noexcept; // 43407226
std::int32_t select_freestanding_side(const std::array<FreestandingScore, 3>&) noexcept; // 433B87F9
std::int32_t score_freestanding_ray(BallisticEngine&, BallisticQuery&, FreestandingScore&,
                                    const BallisticVector& start,
                                    const BallisticVector& target); // 433D094E
// Full native trace/score/selection owner 433F41AF. Only engine access crosses
// the interface; bullet tracing, penetration and ranking stay in recovered code.
bool find_freestanding_direction(BallisticEngine&, const BallisticVector& start,
                                 const BallisticVector& target, BallisticVector& direction,
                                 std::int32_t* total_damage, FreestandingWorkspace&);

struct FreestandingSettings {
    bool enabled;
    HotkeyBinding hotkey;
};
struct FreestandingFrame {
    float scalar_c0, scalar_38, height_164, forward_move;
    BallisticVector origin_10c, origin_168;
};
struct FreestandingContext {
    std::uint32_t flags; // global 434759C0
    HotkeyBinding height_hotkey; // global 43467C8C, mode at43467CA0
    InputBitmap input;
    std::int32_t target_handle;
    BallisticVector target;
    EntityLookup entities;
};
struct FreestandingState {
    std::uint32_t flags_130;
    float yaw_1a8;
    std::uint32_t value_1ac;
};
// Complete native 4338A172, with internal 433305FB/4341D036 activation tests.
bool apply_freestanding(const FreestandingSettings&, const FreestandingFrame&,
                        const FreestandingContext&, FreestandingState&, float& yaw,
                        BallisticEngine&, FreestandingWorkspace&);
} // namespace skeet::anti_aim

// Declarations from jitter.hpp

namespace skeet::anti_aim {
struct JitterSettings {
    std::int32_t mode; // owner + 0x40
    std::int32_t offset; // owner + 0x54
    float minimum_skitter; // owner + 0x14C; default 20
    float maximum_skitter; // owner + 0x148; default 90
    std::array<float, 12> skitter = {-1, 1, 0, -1, 1, -1, 0, 1, -1, 0, 1, 0}; // +0x15C
};
struct JitterRandom {
    void* context;
    void (*seed)(void*, std::int32_t); // vstdlib!RandomSeed
    float (*range)(void*, float, float); // vstdlib!RandomFloat
};

// Native scheduler block 0x4332E63D..0x4332E837, including the yaw-off gate,
// all five jitter modes and the command-number seed. Randomness stays at its
// original external vstdlib boundary; no random generator is substituted.
void apply_yaw_jitter(float& yaw, std::int32_t yaw_mode, const JitterSettings&, std::uint32_t phase,
                      std::int32_t command_number, const JitterRandom&);
} // namespace skeet::anti_aim

// Declarations from menu_dependencies.hpp

namespace skeet::anti_aim {
struct MenuDependencies {
    ui::Slider& custom_pitch;
    ui::Slider& yaw_offset;
    ui::ComboBox& yaw_jitter;
    ui::Slider& yaw_jitter_offset;
    ui::Slider& body_yaw_offset;
    ui::Checkbox& freestanding_body_yaw;
    std::span<std::int32_t, 6> yaw_offsets;
    std::span<std::int32_t, 4> body_yaw_offsets;
};
// Complete native dependency methods, including out-of-range early returns.
// The nonzero yaw/body selector indices must address their native arrays.
void pitch_changed(MenuDependencies&, const ui::ComboBox&); // 0x433B5760
void yaw_changed(MenuDependencies&, const ui::ComboBox&); // 0x433B2007
void jitter_changed(MenuDependencies&, const ui::ComboBox&, const ui::ComboBox& yaw); // 0x4332AE6E
void body_yaw_changed(MenuDependencies&, const ui::ComboBox&); // 0x433CE908
} // namespace skeet::anti_aim

// Declarations from movement_capture.hpp

namespace skeet::anti_aim {
using ::skeet::MovementCaptureOffsets;
using ::skeet::MovementCaptureConvars;
using ::skeet::MovementCaptureInput;
using ::skeet::capture_movement_player;
using ::skeet::capture_movement_rules;
using ::skeet::initialize_live_movement_simulation;
// Complete433376C3 with the live constructor reads retained behind its gate.
MovementVector advance_live_movement_origin(void* player, MovementVector position,
                                            MovementVector velocity, std::uint32_t player_flags,
                                            const MovementCaptureInput&,
                                            const MovementEnvironmentBindings&,
                                            const client::Command&, std::uint32_t input_buttons,
                                            float yaw, MovementCollisionEngine&);
} // namespace skeet::anti_aim

// Declarations from owner_lifecycle.hpp

namespace skeet::anti_aim {
struct OwnerRegistries {
    engine::OwnerRegistryBinding primary; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
    engine::OwnerRegistryBinding frames; //43475AD8
};
struct OwnerHudBindings {
    script::Bridge& scripts;
    const IntegerConfig& indicators;
    const esp::IndicatorLayout& layout;
    std::uint32_t life_state_9d920e35;
};
struct OwnerBindings {
    CommandOwner& command;
    const OwnerHudBindings& hud;
    void* const& allocator; //43467204, animation-vector Free virtual5
    CommandOwner*& active; //canonical43475940, also used by generatedcommands
};
std::size_t owner_size() noexcept;
// Complete43490435, on fresh canonical owner configuration. Four actual
// registrars retain their local order; independent global AA descriptors use
// their own CRT initializer. All command/animation/HUD state is borrowed once.
void construct_owner(void* storage, const OwnerRegistries&, ConfigRegistries&, OwnerBindings);
//43387448 allocation contract. Canonical projected values are zeroed as by
// HEAP_ZERO_MEMORY before construction. Storage allocation must succeed.
// The protected startup caller owns publication of the returned owner into
//43475940; the constructor itself has no such native store.
void* create_owner(const OwnerRegistries&, ConfigRegistries&, OwnerBindings);
// Complete433D6575 ->433E06EE ->433CF41B/4336B1E9 ->433837D0.
// Clears active before the null test, then releases animation, text and owner.
void destroy_owner(void* storage, CommandOwner*& active);
// Whole primary slots1/7. Slot1 borrows the actual96DB2E1C payload; slot7
// calls the current slot5 before resolving the owned "+spray_menu" command.
void bind_owner_settings(void* storage); //4348E637
void refresh_owner_command(void* storage); //433DF0C2
void draw_owner_hud(CommandOwner&, const OwnerHudBindings&, ui::RenderQueue&, void* local_player,
                    std::int32_t& cursor); //433E8FD3
} //namespace skeet::anti_aim

// Declarations from pitch.hpp

namespace skeet::anti_aim {
struct PitchSettings {
    std::int32_t mode;
    std::int32_t custom_pitch; // owner + 0x128
    float special_angle; // owner + 0x138, default 180
    float maximum_pitch; // owner + 0x150, default 89
};
struct PitchFrame {
    std::uint32_t flags;
    std::uint32_t buttons;
    std::int32_t movement_type; // decoded player field
    std::int32_t command_number;
    float angle_9c;
    bool has_weapon;
    float weapon_pitch_340;
};
struct PitchInterfaces {
    void* context;
    void (*random_seed)(void*, std::int32_t); // vstdlib!RandomSeed
    std::int32_t (*random_integer)(void*, std::int32_t, std::int32_t); // vstdlib!RandomInt
    std::int32_t (*player_code)(void*); // external player virtual method +0x1E8
};

// Native scheduler block 0x4332D70D..0x4332D8DE. The caller supplies decoded
// player/weapon fields. Native random choice 4 preserves the incoming pitch.
void apply_pitch(float& pitch, const PitchSettings&, const PitchFrame&, const PitchInterfaces&);
} // namespace skeet::anti_aim

// Declarations from precalculation.hpp

namespace skeet::anti_aim {
struct PrecalculationState {
    State& common;
    float feet_yaw_194;
    float micro_move_19c = 1.01f;
    float previous_duck_1a4;
};
struct PrecalculationPlayer {
    std::uint32_t flags_e2fbe695;
    bool has_animation_state;
    float feet_yaw_80;
    bool guard_d720aea6; //433DF22C: shared-player field, false when absent
    std::uint8_t movement_type_7501cb91;
    MovementVector velocity_36375c23;
    float duck_3309c0cf, rate_fb7e3404;
    bool field_6b1a59b5;
};
// Shared native micro-movement block4339F5EF..4339F727, also consumed by
// generated-command owner43320B61. Previous duck publication remains separate.
void apply_micro_movement(PrecalculationState&, const PrecalculationPlayer&, float interval,
                          client::Command&) noexcept;
struct PrecalculationLiveFields {
    std::uint32_t duck_3309c0cf, rate_fb7e3404, field_6b1a59b5, flags_e2fbe695;
    std::int32_t* const* choked;
    float* const* interval;
};
struct PrecalculationBindings {
    void* player; // actual origin getter virtual10
    std::uint32_t player_value;
    const MovementSimulationPlayer& movement_player;
    const MovementSimulationRules& movement_rules;
    const MovementSimulationEnvironment& movement_environment;
    MovementCollisionEngine& collision;
    MovementSimulation& workspace;
    const client::ScriptCommandEnvironment& roll_guard;
};
struct LivePrecalculationBindings {
    void* player;
    const MovementCaptureInput& movement;
    const MovementEnvironmentBindings& environment;
    MovementCollisionEngine& collision;
    MovementSimulation& workspace;
    const client::ScriptCommandEnvironment& roll_guard;
    const PrecalculationLiveFields& fields;
};
// Complete phase2 owner4339F3BF. Choked count is *434757DC, independently
// of the later phase4 frame count. The caller retains the native scratch state.
void precalculate_command_angles(const GlobalConfig&, const Config&, PrecalculationState&,
                                 const PrecalculationPlayer&, std::int32_t choked_commands,
                                 client::Command&, const std::array<float, 3>& incoming_angles,
                                 std::uint8_t& send_packet, const PrecalculationBindings&);
// Raw command owner always supplies live constructor/tick/field bindings.
// The decoded entry above is a separate semantic API, never a runtime fallback.
void precalculate_live_command_angles(const GlobalConfig&, const Config&, PrecalculationState&,
                                      const PrecalculationPlayer&, std::int32_t choked_commands,
                                      client::Command&, const std::array<float, 3>& incoming_angles,
                                      std::uint8_t& send_packet, const LivePrecalculationBindings&);
} // namespace skeet::anti_aim

// Declarations from proximity.hpp

namespace skeet::anti_aim {
using ProximityEntity = std::uintptr_t;
struct ProximityList {
    const ProximityEntity* entities;
    std::int32_t count;
};
class ProximityEngine {
public:
    virtual ~ProximityEngine() = default;
    virtual const ProximityList* list() = 0; // global434688C0
    virtual bool active(ProximityEntity) = 0; // decoded entity field
    virtual BallisticVector origin(ProximityEntity) = 0; // virtual+0x28
};
// Complete native helpers. The first scans every list entry; the second checks
// only slot0. They retain candidate/local origin-call order and live list reload.
bool any_nearby_entity(ProximityEngine&, ProximityEntity local); //4334E49F
bool first_nearby_entity(ProximityEngine&, ProximityEntity local); //4340AEDA
} // namespace skeet::anti_aim

// Declarations from runtime.hpp

namespace skeet::anti_aim {
// Native constructor stages separated by its string allocation.
void initialize_command_state_prefix(AngleStageState&, PrecalculationState&,
                                     AngleParameters&) noexcept;
void initialize_command_animation_state(AngleStageState&, FinisherState&) noexcept;
void initialize_command_pattern(AngleParameters&) noexcept;
// Runtime portion of owner43490435. The two state references must alias the
// supplied AngleStageState. Native-untouched fields retain their input bytes;
// reset_state is the separate native reset method, rather than this constructor.
void initialize_command_state(AngleStageState&, PrecalculationState&, FinisherState&,
                              AngleParameters&) noexcept;
} // namespace skeet::anti_aim

// Declarations from runtime_owner.hpp

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class MovementHudRuntimeOwner;
class CommandCaptureRuntimeOwner;
class TargetRefreshRuntimeOwner;
class VisualRuntimeOwner;
class NativeStartupSources;
struct EngineRuntimeBindings;
struct AntiAimRuntimeFactoryBindings;
}
namespace skeet::anti_aim {
struct RuntimeImports {
    using Seed = void(__cdecl*)(std::int32_t);
    using Integer = std::int32_t(__cdecl*)(std::int32_t, std::int32_t);
    using Range = float(__cdecl*)(float, float);
    Seed seed;
    Integer integer;
    Range range;
    const RageTrigonometry& trigonometry;
    double (*atan2)(double, double);
    RageBoxClipper clipper;
};
RuntimeImports native_runtime_imports(client::ClientRuntimeStorage&,
                                      const client::EngineRuntimeBindings&);

// Process-lifetime composition for43485CCC and the three real command phases.
// No player, model, command or frame is sampled by construction. All players
// are supplied at dispatch; live BoneRuntime transactions produce body-yaw
// hypotheses and the matrices consumed by the actual model-effects owner.
// Construction registers no owner and executes no engine or command method.
class RuntimeOwner final {
public:
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                 animation::RuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::HistoryRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                 animation::RuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::HistoryRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&, RuntimeImports);
    ~RuntimeOwner();
    RuntimeOwner(const RuntimeOwner&) = delete;
    RuntimeOwner& operator=(const RuntimeOwner&) = delete;
    RuntimeOwner(RuntimeOwner&&) = delete;
    RuntimeOwner& operator=(RuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    CommandOwner& command() noexcept;
    AngleStageState& state() noexcept;
    PrecalculationState& precalculation() noexcept;
    FinisherState& finisher() noexcept;
    client::AntiAimRuntimeFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::anti_aim

// Declarations from scheduler.hpp

namespace skeet::anti_aim {
// Complete native command-yaw tail 4332EBBC..4332EF25. Native owner offsets
// are absolute (the phase-4 scheduler receives the secondary owner + 8).
struct CommandYawState {
    std::uint32_t flags_130;
    float cached_yaw_18c;
};
void finalize_command_yaw(float& command_yaw, float frame_yaw_24, CommandYawState&) noexcept;
// Cached-only continuation4332EE97..4332EF25: preserve an uncached command's
// current word while computing the normalized value stored in the cache.
void apply_cached_command_yaw(float& command_yaw, CommandYawState&) noexcept;
struct AngleCycleState {
    std::uint32_t flags_130, phase_190;
    float saved_yaw_198;
};
// Native phase-4 prefix4332D1E0..4332D1F4 and cycle block4332D2FF..4332D3A4.
// cached_command is the already recovered player/proximity flag before D2FF.
void update_command_flags(std::uint32_t frame_flags, std::uint32_t& flags_130) noexcept;
void begin_angle_cycle(bool cached_command, std::int32_t choked_commands, std::int32_t spin_offset,
                       AngleCycleState&) noexcept;
struct ActionGateFrame {
    std::uint32_t flags, previous_buttons_44;
    std::int32_t choke_limit_fc, choked_15c, weapon_id_d8;
    std::uint8_t state_104, weapon_kind_124;
    float pitch_20, yaw_24;
};
struct ActionGatePlayer {
    bool weapon_state_e1df7e9b;
    float weapon_time_3b93667b;
    std::int32_t tick_base_3d10b94c, movement_type_8b5586f6;
    bool field_7fcb7381, has_weapon_3d52a1a7;
    ProximityEntity entity;
};
struct ActionGateGlobals {
    std::int32_t mode_43475b2c, choked_434757dc;
    float interval_per_tick;
    std::uint32_t command_flags;
    std::int32_t target_handle;
    EntityLookup entities;
};
struct ActionGateSettings {
    bool enabled;
    std::int32_t pitch_mode, yaw_base;
};
struct ActionGateState {
    std::uint32_t flags_130;
    std::int32_t sequence_158;
};
enum class ActionGatePath : std::uint32_t {
    stop,
    angles,
    cached_yaw
};
// Native main action gates4332D3D2..4332D70D. Returns the actual next native
// continuation; packet/buttons/global state writes occur before the stop path.
ActionGatePath evaluate_action_gates(const ActionGateSettings&, const ActionGateFrame&,
                                     const ActionGatePlayer&, ActionGateGlobals&, ActionGateState&,
                                     client::Command&, std::uint8_t& send_packet, ProximityEngine&);
} // namespace skeet::anti_aim

// Declarations from startup_initializers.hpp

namespace skeet::client {
class NativeStartupSources;
}
namespace skeet::anti_aim {
// Borrow the registered settings used by the menus and command owners. The
// binding object and these referents remain alive while startup can dispatch.
struct StartupInitializerBindings {
    ConfigRegistries& configuration;
    FakeLagSettings& fake_lag;
    OtherGlobalSettings& other;
};

// Bind four ordinary CRT initializer identities. Each callback initializes
// its own descriptor only; static metadata and the enabled byte survive.
// The startup driver's operand order selects when each callback runs.
void bind_native_startup_initializers(client::NativeStartupSources&, StartupInitializerBindings&);
} // namespace skeet::anti_aim

// Declarations from state.hpp

namespace skeet::anti_aim {
// Named by owner offsets until the downstream consumers establish meaning.
struct State {
    std::uint32_t flags_130 = 0;
    std::int32_t value_158 = 0;
    std::uint32_t value_190 = 0; // phase counter, native INC at4332D396
    float value_1a0 = 0;
    float value_1a8 = 0;
    std::uint32_t value_1ac = 0;
    std::uint32_t value_1b0 = 0;
    std::int32_t value_1b4 = 0;
    std::int32_t value_1c4 = 0;
    const std::uint8_t* build_bones_12c = nullptr; //bound4348E637 to96DB2E1C payload
};

// Complete owner virtual method at 0x433C9EE6.
void reset_state(State&) noexcept;
} // namespace skeet::anti_aim

// Declarations from target_geometry.hpp

namespace skeet::anti_aim {
struct TargetPoints {
    std::array<RageVector, 6> points{};
    std::array<std::int32_t, 6> hitgroups{};
};
struct TargetGeometryOffsets {
    std::ptrdiff_t studio_header;
    std::ptrdiff_t hitbox_set;
};
struct TargetStudioHitbox {
    std::int32_t bone, hitgroup;
    RageVector minimum, maximum;
    std::int32_t name_index;
    RageVector rotation;
    float radius;
    std::array<std::int32_t, 4> unused;
};
static_assert(sizeof(TargetStudioHitbox) == 0x44);

// Native433F2CC1 selects hitboxes0/3/6/11/12, independently of the generic
// RAGE point generator. The supplied matrices are indexed by hitbox, not bone.
TargetPoints build_target_points(const RageVector& target_origin, float view_offset_z,
                                 const RageVector& shoot_origin, std::span<const RageLocalHitbox>,
                                 const HitboxMatrices&, const RageTrigonometry&);

// Actual entity slot77 and the decoded studio-header/hitbox-set fields. The
// remaining snapshots correspond to globals4347C9E0 and4347C650 respectively.
TargetPoints target_points_from_entity(void* entity, const TargetGeometryOffsets&,
                                       const RageVector& target_origin,
                                       const RageVector& shoot_origin, const HitboxMatrices&,
                                       const RageTrigonometry&);
} // namespace skeet::anti_aim

// Declarations from targets.hpp

namespace skeet::esp {
class EnginePlayers;
}
namespace skeet::anti_aim {
// Exact ten-dword publication at native 4346DB14..4346DB38. fields_18 are
// flags (4 means positive damage), maximum damage, shots-to-kill, weapon ID.
struct TargetState {
    std::int32_t sequence = -1;
    float timestamp = 0;
    std::int32_t handle = -1;
    RageVector position{};
    std::array<std::uint32_t, 4> fields_18{0, 0, 0x7FFFFFFFu, 0};
};
static_assert(sizeof(TargetState) == 0x28);

// Complete absent-target publication from 433269C4 / 4335BA8E. Native leaves
// the prior position and timestamp intact while resetting the handle/tail.
void clear_target_selection(TargetState&) noexcept;

struct TargetCandidateOrder {
    float score;
    std::uint32_t next = UINT32_MAX;
};
// The candidate insertion block of433269C4 stops only at an ordered greater
// prior score. Equal and unordered comparisons retain enumeration order.
void insert_target_candidate(std::span<TargetCandidateOrder>, std::uint32_t candidate,
                             std::uint32_t& head) noexcept;

struct TargetPlayerHistory {
    lagcomp::LagTimeline timeline;
    esp::PlayerGeometry geometry;
    float field_524 = 0; // native player record +0x524
    std::int32_t last_damage_tick = 0; // native player record +0x100
};
// The decoded fixture and the actual source cache are distinct stores. A
// source view resolves433B8A2B at each selector lookup and borrows the same
// geometry, ring and publications used by history/ESP; it copies no records.
class TargetPlayerHistoryView {
public:
    explicit TargetPlayerHistoryView(TargetPlayerHistory& value) : decoded_(&value) {}
    explicit TargetPlayerHistoryView(client::PlayerState& value) : source_(&value) {}
    std::uint32_t record_count() const noexcept;
    const lagcomp::LagRecord& at(std::uint32_t age) const noexcept;
    const esp::PlayerGeometry& geometry() const noexcept;
    float prediction_field() const noexcept;
    void last_damage_tick(std::int32_t) const noexcept;

private:
    TargetPlayerHistory* decoded_ = nullptr;
    client::PlayerState* source_ = nullptr;
};
class TargetPlayerHistories {
public:
    TargetPlayerHistories(std::span<TargetPlayerHistory, 64> value) : decoded_(value.data()) {}
    TargetPlayerHistories(std::array<TargetPlayerHistory, 64>& value) : decoded_(value.data()) {}
    explicit TargetPlayerHistories(client::PlayerStateCache& value) : source_(&value) {}
    TargetPlayerHistoryView operator[](std::uint32_t index) const;

private:
    TargetPlayerHistory* decoded_ = nullptr;
    client::PlayerStateCache* source_ = nullptr;
};
struct TargetSelectorOffsets {
    std::uint32_t network_origin; // 0xA9EE6062
    std::uint32_t velocity; // 0x36375C23
    std::uint32_t view_offset; // 0xC246315E
    std::uint32_t inventory; // 0xFC3297F7, first two handles
};
struct TargetPredictionLimit {
    bool local_available, field_bf567619, field_dda99ab5;
    std::int32_t field_b7ea8fda, configured;
};
struct TargetSelectorContext {
    const rage::TargetBindings& targeting;
    void* engine_client; // actual engine-client vtable 14 time, 8 player info
    const std::int32_t* max_clients;
    TargetPlayerHistories histories;
    TargetSelectorOffsets offsets;
    const lagcomp::TickTimingSamples& timing;
    TargetPredictionLimit prediction_limit;
    float interval, receive_interval;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    std::uint8_t* trace_flag;
};
// Complete native selector433CC1A8. Existing targets survive a scan with no
// penetrable candidates, exactly as in the native owner; no RAGE ranking is used.
void select_penetrable_yaw_target(TargetState&, const TargetSelectorContext&);

struct TargetScoredOffsets {
    std::uint32_t weapon_ammunition; // 0xCA5EC698
    std::uint32_t field_fb7e3404;
    std::uint32_t local_armor, local_helmet;
    std::uint32_t collision_group; // 0xE4E2D1A7
    std::uint32_t move_parent_handle; // decoder0x49D93E62 minus0x18
};
// Actual engine/netvar reads used by the native43470404 trace filter. The
// player/collision-owner traversal and accept/reject decision stay in source.
class TargetTraceEntities {
public:
    virtual ~TargetTraceEntities() = default;
    virtual BallisticEntity base_entity(BallisticEntity) const = 0; // virtual7
    virtual std::int32_t collision_group(BallisticEntity) const = 0;
    virtual bool is_player(BallisticEntity) const = 0; // virtual158
    virtual std::int32_t parent_handle(BallisticEntity) const = 0;
    virtual BallisticEntity by_handle(std::int32_t) const = 0; // entity-list4
};
class RawTargetTraceEntities final : public TargetTraceEntities {
    void* list_;
    TargetScoredOffsets offsets_;

public:
    RawTargetTraceEntities(void* list, TargetScoredOffsets offsets)
        : list_(list), offsets_(offsets) {}
    RawTargetTraceEntities(void* list, std::uint32_t collision_group,
                           std::uint32_t move_parent_handle)
        : list_(list), offsets_{0, 0, 0, 0, collision_group, move_parent_handle} {}
    BallisticEntity base_entity(BallisticEntity) const override;
    std::int32_t collision_group(BallisticEntity) const override;
    bool is_player(BallisticEntity) const override;
    std::int32_t parent_handle(BallisticEntity) const override;
    BallisticEntity by_handle(std::int32_t) const override;
};
// Actual engine ABI object for native43470404. It must outlive each engine
// trace using binding(). The first0x24 bytes retain the native x86 layout.
// engine_vtable is the genuine external table43475B90; its slots1..4 are
// forwarded directly. Slot0 runs433B4EB6's source predicate, then slot0.
struct TargetTraceFilter {
    void** vtable;
    std::array<std::uint32_t, 5> fields_04{};
    std::uint32_t mask, exit_mask;
    float minimum_damage;
    void* const* engine_vtable;
    const TargetTraceEntities* entities;
    std::array<void*, 5> methods;
    TargetTraceFilter(void* const* engine_vtable, const TargetTraceEntities&,
                      std::uint32_t mask = 0x600400b, std::uint32_t exit_mask = 0x600400b,
                      float minimum_damage = 1.0f);
    TargetTraceFilter(const TargetTraceFilter&) = delete;
    TargetTraceFilter& operator=(const TargetTraceFilter&) = delete;
    BallisticFilter binding() noexcept;
#if defined(_M_IX86) || defined(__i386__)
    static bool __thiscall should_hit(void*, BallisticEntity, std::uint32_t);
#else
    static bool should_hit(void*, BallisticEntity, std::uint32_t);
#endif
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(TargetTraceFilter, mask) == 0x18);
static_assert(offsetof(TargetTraceFilter, minimum_damage) == 0x20);
#endif
struct TargetScoredInputs {
    const TargetSelectorContext& common;
    const client::Command& command;
    std::uint32_t input_buttons; // AA frame+0x44 (argument vector+0x30)
    float yaw; // AA frame+0x11C, incoming XMM0
    const esp::PlayerRuntimeRules& player_rules;
    std::span<const std::int32_t> dormant_health;
    TargetScoredOffsets offsets;
    TargetGeometryOffsets geometry_offsets;
    const RageVector& local_pose_origin; // native4347C9E0
    const HitboxMatrices& local_pose; // native4347C650, per hitbox
    const RageTrigonometry& trigonometry;
    const std::int32_t* sequence; // pointed global434678F8
    const void* const* clock_owner; // native4346A3A4 -> first entity pointer
    std::uint32_t local_time_offset; // 0x9F8E7F71
    void* const* trace_filter_vtable; // actual external table43475B90
    const TargetTraceEntities& trace_entities;
};
struct TargetScoredContext : TargetScoredInputs {
    const MovementCaptureInput& movement;
    const MovementEnvironmentBindings& movement_environment;
};
// Production uses EnginePlayers' live resource/ConVar reads, without forming
// a detached PlayerRuntimeRules array or a retained Command reference.
struct LiveTargetScoredContext {
    const TargetSelectorContext& common;
    const client::Command& command;
    std::uint32_t input_buttons;
    float yaw;
    esp::EnginePlayers& players;
    TargetScoredOffsets offsets;
    TargetGeometryOffsets geometry_offsets;
    const RageVector& local_pose_origin;
    const HitboxMatrices& local_pose;
    const RageTrigonometry& trigonometry;
    const std::int32_t* sequence;
    const void* const* clock_owner;
    std::uint32_t local_time_offset;
    void* const* trace_filter_vtable;
    const TargetTraceEntities& trace_entities;
    const MovementCaptureInput& movement;
    const MovementEnvironmentBindings& movement_environment;
};
// Explicit decoded semantic input, used independently of raw engine owners.
// Production refresh always takes the mandatory live context above.
struct DecodedTargetScoredContext : TargetScoredInputs {
    const MovementSimulationPlayer& movement_player;
    const MovementSimulationRules& movement_rules;
    const MovementSimulationEnvironment& movement_environment;
};
void select_scored_yaw_target(TargetState&, const TargetScoredContext&); // complete433269C4
void select_decoded_scored_yaw_target(TargetState&, const DecodedTargetScoredContext&);
struct TargetRefreshContext {
    const TargetScoredContext& scored;
    const void* selector_convar; // native43468EA4 XOR0xFCD2DA7A
};
// Shared actual owner4335BA8E. AA and packet consumers must share this state;
// the same sequence skips both native selectors and leaves every field intact.
void refresh_yaw_target(TargetState&, const TargetRefreshContext&);
void refresh_live_yaw_target(TargetState&, const LiveTargetScoredContext&,
                             const void* selector_convar);
void refresh_decoded_yaw_target(TargetState&, const DecodedTargetScoredContext&,
                                const void* selector_convar);

} // namespace skeet::anti_aim

// Declarations from yaw.hpp

namespace skeet::anti_aim {
struct YawLimits {
    float backward_angle; // owner + 0x138, constructor default 180
    float maximum_angle; // owner + 0x148, constructor default 90
};
struct YawFrame {
    float angle_b8;
    float origin_x_dc, origin_y_e0;
    float interpolation_158;
};
struct YawTarget {
    std::int32_t handle; // global 0x4346DB1C, -1 means absent
    float x, y; // globals 0x4346DB20/24
};
struct EntityLookup {
    void* object;
    // External entity-list virtual method at byte offset 0x10 in the x86 ABI.
    const void* (*by_handle)(void*, std::int32_t);
};

// Complete native helper 0x433D2118..0x433D231E. The saved-angle argument is
// read only in mode 3; mode 4 resolves the entity only for a non--1 handle.
float transform_yaw(float base, float offset, std::int32_t mode, const float& saved_angle,
                    const YawLimits&, const YawFrame&, const YawTarget&, const EntityLookup&);

// Complete 0x433B76BE..0x433B76D7. The 0x80 flag lowers a nonzero signed
// limit by one with x86 wrapping before the signed comparison.
bool lag_step_pending(std::uint32_t flags, std::int32_t limit_fc,
                      std::int32_t current_15c) noexcept;
// Full4332E6BD..4332E79D branch. Each target/local coordinate is separately
// quantized to0.25 with native SSErounding before the planar angle calculation.
bool apply_target_yaw_base(std::int32_t yaw_base, const YawTarget&, float origin_x, float origin_y,
                           const EntityLookup&, float& yaw);
} // namespace skeet::anti_aim
