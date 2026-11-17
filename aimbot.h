#pragma once

// Aimbot Subsystem Declarations
#include "skeet/client/command_frame.hpp"
#include "skeet/lagcomp/record_selection_live.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/lagcomp/interpolation.hpp"
#include "skeet/config.hpp"
#include "skeet/animation/bones.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/client/command_empty_tick.hpp"
#include "skeet/client/command_owner_dispatch.hpp"
#include "skeet/engine/owner_registry.hpp"
#include <vector>
#include "skeet/legit/config.hpp"
#include "skeet/rage/targeting.hpp"
#include <memory>
#include "skeet/client/commands.hpp"
#include "skeet/rage/ballistics.hpp"
#include <stdexcept>
#include "skeet/rage/spread.hpp"
#include "skeet/rage/targeting_commit.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/rage/shot_feedback.hpp"
#include "skeet/engine/jobs.hpp"
#include "skeet/lagcomp/extrapolated_record.hpp"
#include "skeet/lagcomp/history_selection.hpp"
#include <optional>
#include "skeet/esp/hitbox_highlight.hpp"
#include "skeet/movement/collision.hpp"
#include "penetration.h"
#include "skeet/weapon/fire_state.hpp"
#include "skeet/animation/entity_update.hpp"
#include <variant>
#include <cstddef>
#include <cstdint>
#include "skeet/animation/bone_source_reference.hpp"
#include "skeet/engine/build_eye_adjustment_bindings.hpp"
#include "skeet/config_storage/native_blob.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include "skeet/menu/rage_clipboard.hpp"
#include <array>
#include "skeet/menu/misc_menu.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/hud/composite_handlers.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/movement/simulation_tick.hpp"
#include "skeet/movement/simulation_capture.hpp"
#include "shots.h"
#include "skeet/lagcomp/entity_state.hpp"
#include <functional>
#include "skeet/client/command_transform.hpp"
#include "skeet/client/verified_command.hpp"
#include "skeet/legit/runtime.hpp"
#include "skeet/movement/simulation_environment.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/rage/target_relationships.hpp"
#include "skeet/lagcomp/entity_preparation.hpp"
#include "skeet/legit/historical_bones.hpp"
#include "skeet/rage_points.hpp"
#include <atomic>
#include "skeet/rage_safety.hpp"
#include "skeet/rage_hitchance.hpp"
#include "skeet/movement_config.hpp"
#include "skeet/lagcomp/record_selection.hpp"
#include "skeet/players/state.hpp"
#include "skeet/rage_geometry.hpp"
#include "skeet/rage_selection.hpp"
#include "skeet/rage_policy.hpp"
#include "skeet/lagcomp/records.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/rage_shot.hpp"
#include "skeet/movement/steering_math.hpp"
#include "includes.h"
#include <limits>
#include <cstring>
#include "skeet/rage/action_state.hpp"
#include "skeet/rage/final_target_context.hpp"
#include "skeet/rage/owner.hpp"
#include "skeet/rage/post_firing.hpp"
#include "skeet/rage/scanning.hpp"
#include "skeet/rage/scanning_geometry.hpp"
#include "skeet/rage/scanning_chance.hpp"
#include "skeet/rage/targeting_pipeline.hpp"


// Declarations from accuracy.hpp

namespace skeet::legit {
// Complete43366128 basis, using the actual scalar CRT import bindings.
RageSpreadBasis spread_basis(const Vector& angles, const RageTrigonometry&);
// Complete43398F25:32 normalized directions, native permutation stride4,
// one pellet, and the exact Revolver spread/inaccuracy override.
std::array<Vector, 32> accuracy_spread_directions(const rage::SpreadParameters&,
                                                  std::uint32_t weapon_flags_7c,
                                                  const RageSpreadBasis&, const rage::SpreadTables&,
                                                  const rage::SpreadRandom&,
                                                  const rage::ShotgunPattern&,
                                                  const RageTrigonometry&);
struct AccuracyEnvironment {
    const rage::TargetBindings& targeting;
    const lagcomp::EntityStateRuntime& entity;
    const lagcomp::RecordSelectionLiveContext& selection;
    std::ptrdiff_t player_punch;
    engine::ConVarBinding recoil_scale;
    const std::uint8_t& suppress_recoil_43467144;
    const std::uint8_t& normalize_angles_434693ec;
    // Actual clocks, reloaded on their respective sides of engine calls.
    const float* time;
    const float* interpolation_43476928;
    const float* interval;
    const std::int32_t* maximum_clients;
    std::span<Matrix3x4, 256> saved_bones;
    AccuracyMatrixScratch& matrix_scratch;
    BallisticEngine& ballistics;
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    engine::ConVarBinding shotgun_patterns_convar; //live43468520 parent chain
    const lagcomp::RotationMath& rotation;
    std::span<const TargetTrack, 64> targets;
    TriggerTraceBindings trace;
    void* const* trace_filter_vtable; // actual external43475B90, slots0..4
    const anti_aim::TargetTraceEntities& trace_entities;
};
// Complete4332FD2B composition. An optional hit byte selects trigger semantics;
// it is only set on success and otherwise retains its incoming value. The
// original entity scope restores even when no history tick was selected.
std::int32_t accuracy_tick(ProfileState&, const client::CommandFrame&, void* local_player,
                           const client::Command&, bool* hit, const AccuracyEnvironment&);
} // namespace skeet::legit

// Declarations from accuracy_history.hpp

namespace skeet::legit {
struct AccuracyMatrixScratch {
    Vector inverse_row1; // automatic433EEDFF frame+80/84/88, opaque first use
    Vector inverse_row2; // automatic frame+90/94/98
};
// Complete433EEDFF. Unlike43367778, a noninterpolated success still returns
// the requested tick. Source/destination do not alias; hitbox is0 or3 here.
std::int32_t select_accuracy_hitbox(const lagcomp::RecordRing&, std::int32_t requested_tick,
                                    std::uint32_t hitbox, Vector& origin, Matrix3x4&,
                                    float interval, AccuracyMatrixScratch&,
                                    const lagcomp::RotationMath&);
struct AccuracyHistoryRay {
    Vector origin, direction;
    float maximum_origin_distance;
};
struct StudioOffsets;
// Complete433983BB after the model/studio-hitbox descriptor lookup. Traverses
// the inclusive tick window backwards and updates both outputs only for a
// strictly better cosine. newest_origin is the newest history origin, or the
// live decodedA9EE6062 origin for an empty history.
void search_accuracy_history(const lagcomp::RecordRing&, const AccuracyHistoryRay&,
                             const Vector& newest_origin, const AcquisitionHitbox&,
                             std::uint32_t hitbox, std::int32_t first_tick, std::int32_t last_tick,
                             std::int32_t& best_tick, float& best_cosine, float interval,
                             AccuracyMatrixScratch&, const lagcomp::RotationMath&);
// Whole433983BB owner, including the nullable wrapper/header/hitbox-set gate,
// native descriptor lookup and newest-record/live-origin reference selection.
void search_target_accuracy_history(const void* target, const StudioOffsets&,
                                    std::ptrdiff_t origin_offset, const lagcomp::RecordRing&,
                                    const AccuracyHistoryRay&, std::uint32_t hitbox,
                                    std::int32_t first_tick, std::int32_t last_tick,
                                    std::int32_t& best_tick, float& best_cosine, float interval,
                                    AccuracyMatrixScratch&, const lagcomp::RotationMath&);
} // namespace skeet::legit

// Declarations from acquisition.hpp

namespace skeet::anti_aim {
class TargetTraceEntities;
}
namespace skeet::legit {
struct StudioOffsets;
struct AcquisitionHitbox {
    std::int32_t bone, hitgroup;
    Vector minimum, maximum;
    float radius;
};
// 4339A43D..4339A95C, after eligibility and live/historical bone preparation.
// The native owner inspects exactly the first seven hitboxes; missing/short
// studio sets take its clear-record path. Matrix order is the selected hitbox
// order produced by complete433899B3, rather than raw bone-array order.
void acquire_visible_target(TargetTrack&, const ProfileState&, std::span<const AcquisitionHitbox>,
                            std::span<const Matrix3x4>, const Vector& eye, const Vector& forward,
                            float interval, BallisticEngine&, const BallisticFilter&,
                            TriggerTraceBindings);
struct AcquisitionTraceContext {
    void* const* engine_filter_vtable; // genuine43475B90 slots0..4
    const anti_aim::TargetTraceEntities& entities;
};
// Full command-owner path. Every hitbox gets a new43470404 filter prefix,
// including the concrete433B4EB6 rejection body and genuine engine delegates.
void acquire_visible_target(TargetTrack&, const ProfileState&, std::span<const AcquisitionHitbox>,
                            std::span<const Matrix3x4>, const Vector& eye, const Vector& forward,
                            float interval, BallisticEngine&, const AcquisitionTraceContext&,
                            TriggerTraceBindings);
// Whole live acquisition: resolve the current studio set after bone setup,
// then read each descriptor at its actual iteration, after preceding smoke/
// TraceRay calls. No per-player descriptor snapshot spans the trace loop.
void acquire_visible_target_live(TargetTrack&, const ProfileState&, const void* target,
                                 const StudioOffsets&, std::span<const Matrix3x4>,
                                 const Vector& eye, const Vector& forward, float interval,
                                 BallisticEngine&, const AcquisitionTraceContext&,
                                 TriggerTraceBindings);
} // namespace skeet::legit

// Declarations from command.hpp

namespace skeet::legit {
struct RuntimeSettings {
    const Settings& legit;
    const BooleanConfig& rage_enabled;
    const HotkeyBinding& rage_key;
};
struct RuntimeNetvars {
    std::ptrdiff_t player_shots_8cfc633a;
    std::ptrdiff_t player_effects_38ca5228;
    std::ptrdiff_t player_byte_043fa575, player_byte_334b460c;
    std::ptrdiff_t player_punch_1bae1168;
    std::ptrdiff_t player_punch_velocity_48442721;
    std::ptrdiff_t player_next_attack_de530a8b;
    std::ptrdiff_t weapon_next_attack_f71ce331;
    std::ptrdiff_t ammunition_ca5ec698, reloading_b8969256;
    std::ptrdiff_t blindness_1bc6d160;
};
struct RuntimeTimes {
    const float* interval; // *4346A338
    const float* command_time; // *4346A334
    const float* world_time; // *43467EB8
    const float* frame_delta; // [43467B04]+20
    const float* setup_bones_time; // [43467B04]+10
    const float* interpolation; // 43476928
    const std::int32_t* clock_tick; // *43468D4C
    const std::int32_t* history_command_tick; // [43475A60]+4, velocity age clock
    const std::int32_t* maximum_clients; //*43469300
};
struct RuntimeBindings {
    RuntimeNetvars offsets;
    RuntimeTimes times;
    AccuracyEnvironment accuracy;
    HistoricalBoneOffsets historical_offsets;
    std::span<Matrix3x4, 19> target_bones;
    std::span<Matrix3x4, 256> bone_scratch;
    RecoilDecayBindings recoil_decay;
    RandomBindings random;
    InputBitmap input;
    const client::CommandAngleBindings& view_angles;
    std::uint32_t& scheduler_flags;
};
// Complete phase0 owner4332CDEA and4339FB19. Runs before the master player's
// rejection gates, obtains engine-client slot18 and advances both view slots.
void capture_legit_view(RuntimeState&, void* engine_client);
// Complete phase4 owner43399A38. All recovered decisions call concrete LEGIT,
// shared history and ballistic implementations; only genuine engine/CRT/RNG
// boundaries are supplied in bindings. Native receiver is owner+8; the source
// RuntimeState expresses the owner's proven fields independently of x86 ABI.
void process_legit_command(RuntimeState&, client::CommandFrame&, void* local_player,
                           client::Command&, const RuntimeSettings&, RuntimeBindings&);
} // namespace skeet::legit

// Declarations from command_source.hpp

namespace skeet::legit {
// Production owns the process dependencies and forms the physical binding
// graph from the current phase's player/frame. No initial fake player or
// retained Command is needed to construct/register the lifecycle owner.
class CommandSource {
public:
    virtual ~CommandSource() = default;
    virtual void clear_allocated_storage() = 0;
    virtual void capture_view(RuntimeState&) = 0;
    virtual void process(RuntimeState&, const RuntimeSettings&, client::CommandFrame&, void*,
                         client::Command&) = 0;
    virtual void reset(RuntimeState&) = 0;
    virtual void pending(RuntimeState&, client::Command&, float) = 0;
};
} //namespace skeet::legit

// Declarations from config.hpp

namespace skeet::legit {
using skeet::BooleanArrayConfig;
using HotkeyConfigs = std::array<HotkeyConfig, 9>;
struct Settings {
    BooleanArrayConfig enabled;
    IntegerArrayConfig maximum_fov, speed, attack_speed, speed_scale_fov, reaction_time;
    BooleanArrayConfig through_smoke, while_blind, quick_stop;
    IntegerArrayConfig recoil_pitch, recoil_yaw, maximum_lock_time;
    BooleanArrayConfig head, chest, stomach;
    IntegerArrayConfig first_shot_delay;
    BooleanArrayConfig trigger_enabled;
    IntegerArrayConfig minimum_hit_chance, minimum_damage;
    BooleanArrayConfig burst_fire;
    IntegerArrayConfig burst_time;
    BooleanArrayConfig config_8a823c30;
    IntegerArrayConfig config_90b2820a;
    BooleanArrayConfig penetration, shoot_smoke, shoot_blind;
    BooleanArrayConfig trigger_head, trigger_chest, trigger_stomach;
    IntegerArrayConfig trigger_reaction, accuracy_boost, accuracy_range;
    BooleanArrayConfig standalone_recoil;
    HotkeyConfigs aim_keys, trigger_keys;
};
constexpr std::uint32_t hotkey_hash(std::uint32_t base, std::uint32_t index) noexcept {
    return ((base ^ 0x92CA5DF1u) + index) ^ 0x92CA5DF1u;
}
// The whole owner places its selective runtime construction between these
// descriptor registrations and the final per-category overrides.
void register_settings(ConfigRegistries& registries, Settings& settings) noexcept;
void finish_settings_initialization(Settings& settings) noexcept;
// Configuration portion of 0x434845CF..0x43484919. Engine state is outside it.
void initialize_settings(ConfigRegistries& registries, Settings& settings) noexcept;
} // namespace skeet::legit

// Declarations from historical_bones.hpp

namespace skeet::legit {
using AttachmentUpdate = engine::MemberFunction<void, Matrix3x4*, std::uint32_t>;
// Complete4338AC35/43407EA3/43341E47/433899B3 transaction. Non-hitbox scratch
// matrices retain caller bytes, as in the native uninitialized stack workspace.
void refresh_hitbox_attachments(void* entity, const animation::BoneModel&,
                                std::span<Matrix3x4, 19> hitboxes,
                                std::span<Matrix3x4, 256> scratch,
                                std::ptrdiff_t bone_accessor_offset, AttachmentUpdate);

struct HistoricalBoneSnapshot {
    std::uint32_t field_12c;
    float field_130;
    std::uint32_t field_134;
    float field_144;
    std::uint32_t field_148;
    std::array<std::uint32_t, 2> field_1a8;
};
struct HistoricalBoneOffsets {
    lagcomp::EntityNetvars common;
    std::ptrdiff_t field_e23a9be1;
    std::ptrdiff_t field_6e9852d6;
    std::ptrdiff_t field_2a671b42;
    std::ptrdiff_t field_4e66bb34;
    std::ptrdiff_t field_83a6cf2b;
};
struct HistoricalBoneBindings {
    HistoricalBoneOffsets offsets;
    HistoricalBoneSnapshot snapshot;
    animation::BoneModel model;
    AttachmentUpdate update_attachments;
    float interval, time, frame_time;
    std::int32_t current_tick;
};
// Complete43367778, including record traversal, translation-only interpolation,
// the temporary entity/collision/netvar transaction and attachment rebuild.
std::int32_t historical_target_bones(const lagcomp::RecordRing&, void* entity,
                                     std::int32_t requested_tick, std::span<Matrix3x4, 19>,
                                     std::span<Matrix3x4, 256>, const HistoricalBoneBindings&);
struct ProfileState;
// Complete4336EC3B. A failed historical selection enters the genuine renderable
// SetupBones call; successful historical ages only replace interval in(0,.3].
bool prepare_target_bones(const ProfileState&, const lagcomp::RecordRing&, void* entity,
                          std::span<Matrix3x4, 19>, std::span<Matrix3x4, 256>, float& interval,
                          const HistoricalBoneBindings&);
} // namespace skeet::legit

// Declarations from historical_bones_live.hpp

namespace skeet::client {
struct NativePlayerStateLayout;
}
namespace skeet::legit {
struct LiveHistoricalBoneBindings {
    const HistoricalBoneOffsets& offsets;
    const client::NativePlayerStateLayout& cached;
    const animation::BoneRuntime& bones;
    const float* interval;
    const float* time;
    const float* frame_time;
    const std::int32_t* current_tick;
};
// Whole43367778 with the actual cache and BoneRuntime. Cached fields are read
// at their native transaction sites; model/set lookup occurs inside each raw
// scatter/select, including after the genuine attachment engine call.
std::int32_t historical_target_bones_live(const lagcomp::RecordRing&, void* target,
                                          std::int32_t requested_tick, std::span<Matrix3x4, 19>,
                                          std::span<Matrix3x4, 256>,
                                          const LiveHistoricalBoneBindings&);
// Whole4336EC3B live entry. SetupBones is followed by a fresh model selection.
bool prepare_target_bones_live(const ProfileState&, const lagcomp::RecordRing&, void* target,
                               std::span<Matrix3x4, 19>, std::span<Matrix3x4, 256>, float& interval,
                               const LiveHistoricalBoneBindings&);
} // namespace skeet::legit

// Declarations from manual_motion.hpp

namespace skeet::legit {
// 43399F05..4339A2A2: complete manual-motion projection/filter/flag update.
// time_scale is the owner's native frame-time*64 value established on entry.
void update_manual_motion(RuntimeState&, const client::Command&, const Vector& frame_angles,
                          float current_time, const lagcomp::RotationMath&);
} // namespace skeet::legit

// Declarations from owner.hpp

namespace skeet::legit {
class LifecycleOwner;
struct OwnerBindings {
    engine::OwnerRegistryBinding lifecycle;
    client::CommandPhaseRegistry& command_owners;
    client::CommandEmptyTickRegistry& empty_tick_owners;
    ConfigRegistries& configuration;
    Settings& settings;
    client::LegitCommandOwner& commands;
    LifecycleOwner*& active; // canonical4347588C, cleared by433689DD
};

// RuntimeState projection of43484884..434848F3. Native+800 already aliases
// commands.settings.rage_enabled.value through RuntimeSettings. Unwritten words, the other
// three bytes of the two view components/maximum-lock-time and track interval
// words survive construction. This is distinct from primary slot7's reset.
void initialize_runtime_state(RuntimeState&) noexcept;
std::size_t legit_owner_size() noexcept;
// Whole434845CF: primary -> command -> empty-tick registration, then tables,
// all51 descriptors, selective state construction and final config overrides.
// Storage carries the43387448 header. Canonical state/settings/bindings and
// borrowed registry ranges outlive the owner. active is published by its caller.
LifecycleOwner& construct_legit_owner(void* storage, OwnerBindings);
// Whole4347FEBC, including the explicit mem_int3.h record for the canonical
// pointer store at4347FED5. The native allocator requests0x1310/flags8; the
// source's borrowed settings/state are zeroed on successful allocation before
// constructor registration. They must be fresh and unregistered. Failure
// publishes null and leaves those borrowed values unchanged.
LifecycleOwner* create_legit_owner(OwnerBindings);
// Whole433689DD: clear active even for a null object, then433837D0 header-free.
void destroy_legit_owner(LifecycleOwner* owner, LifecycleOwner*& active);
void* legit_lifecycle_object(LifecycleOwner&) noexcept;
// Resolve the canonical configuration through the nullable owner slot. The
// source LifecycleOwner is not laid out as the native0x1310-byte allocation.
const Settings* legit_owner_settings(const LifecycleOwner*) noexcept;

// Immutable guest verification executes4347FEBC only through4347FED5: the
// five trapped publication bytes remain unmodified. Their source semantics
// come from the recorded MOV handler, rather than guest execution of that MOV.
} // namespace skeet::legit

// Declarations from recoil_state.hpp

namespace skeet::legit {
struct RecoilStateInput {
    Vector player_punch;
    Vector context_velocity; // weapon context+8C, copied before decay
    Vector player_velocity; // decoded48442721, future-shot local copy
    float player_next_attack, weapon_next_attack, time;
    std::uint32_t command_buttons;
};
// 4339A99B..4339ACF5, including future recoil simulation, normalized delta,
// random .2..3 motion cap, and parent-chained weapon_recoil_scale reads.
void update_recoil_state(RuntimeState&, const RecoilStateInput&, const void* recoil_scale,
                         const RecoilDecayBindings&, RandomBindings);
// Complete433D5ACC. Roll is explicitly zero, even when the frame has roll.
Vector recoil_reference_angles(const RuntimeState&, const Vector& frame_angles,
                               const void* recoil_scale);
// Complete433D1EDC, including the optional433E9160 normalization. A pure roll
// punch does not enter the addition, matching the native two-axis zero gate.
void add_player_recoil(Vector& angles, const Vector& punch, const void* recoil_scale,
                       bool normalize);
// Complete433C6416 under the real non-null weapon contract. Slot430 is called
// before testing the attack button, shot count or ammunition.
bool recoil_shot_ready(const ProfileState&, void* weapon, const client::Command&,
                       std::ptrdiff_t ammunition_offset);
} // namespace skeet::legit

// Declarations from runtime.hpp

namespace skeet::legit {
using Vector = std::array<float, 3>;
struct ProfileState {
    std::int32_t profile = 0; // owner + 0x804
    std::int32_t shots = 0; // 0x810
    std::uint32_t previous_buttons = 0; // 0x814
    std::uint8_t enabled = 0; // 0x818
    float speed = 0, speed_scale_fov = 0; // 0x81C, 0x820
    float maximum_lock_time = 0; // 0x824
    bool limit_lock_time = false; // 0x828
    float reaction_time = 0, maximum_fov = 0; // 0x82C, 0x830
    float recoil_pitch = 0, recoil_yaw = 0; // 0x834, 0x838
    std::uint8_t flags = 0, secondary_flags = 0; // 0x83C, 0x83D
    std::uint32_t aim_hitgroups = 0; // 0x840
    float first_shot_range = 0; // 0x844
    std::int32_t minimum_hit_chance = 0; // 0x848
    float trigger_reaction = 0, burst_time = 0; // 0x84C, 0x850
    std::int32_t minimum_damage = 0; // 0x854
    std::uint32_t trigger_hitgroups = 0; // 0x858
    std::int32_t accuracy_boost = 0; // 0x85C
    std::int32_t accuracy_range = 0; // 0x860
    std::array<Vector, 3> recoil_history{}; // 0x898..0x8B8
    float burst_until = 0; // 0x8BC
};
// 43399B2F..43399E4D: complete per-weapon settings projection. Valid engine
// weapon-profile values are nonnegative signed bytes. Values >= 9 read profile
// zero while retaining the previously selected state.profile, as in x86.
bool select_weapon_profile(const Settings&, ProfileState&, std::int32_t weapon_profile,
                           std::uint32_t command_buttons);
// Complete433EA238. A missing LEGIT owner admits recoil compensation. Valid
// engine categories are0..8; the aim hotkey is evaluated only when enabled.
bool recoil_compensation_enabled(const Settings*, std::uint32_t category, InputBitmap);
// Complete native 4340C87E, shared by the disabled-profile and reset paths.
void clear_recoil_history(ProfileState&) noexcept;

struct FirstShotFrame {
    std::uint32_t prediction_buttons;
    Vector eye_position;
    Vector forward;
    std::int32_t maximum_clients;
};
// Complete 433EE27B. Entity enumeration and origin/bounds access use actual
// engine virtuals; candidate rejection reuses the recovered 43358962 chain.
void apply_first_shot_delay(const Settings&, ProfileState&, const FirstShotFrame&,
                            const rage::TargetBindings&, InputBitmap,
                            std::uint32_t& command_buttons);

struct RecoilDecayBindings {
    // Parent-chained native ConVars: parent +0x1C, encoded float +0x2C.
    engine::ConVarBinding linear_decay;
    engine::ConVarBinding angle_decay;
    engine::ConVarBinding velocity_decay;
    float interval;
    // Native msvcrt!__libm_sse2_expf boundary, called twice per update.
    float (*exponential)(float);
};
// Complete 4336B421: exponential and linear angle decay, with the native
// two half-step velocity integrations and in-place velocity update.
void decay_recoil(Vector& velocity, Vector& angle, const RecoilDecayBindings&);
struct AimStepInput {
    float pitch_delta, yaw_delta;
    float target_fov; // native caller stack +0x38
    float target_distance; // native caller stack +0x140
    float time_scale; // owner +0x8D8
    float curve; // owner +0x8EC
    std::int32_t command_number;
};
struct RandomBindings {
    void* context;
    void (*seed)(void*, std::int32_t);
    float (*uniform)(void*, float minimum, float maximum);
};
// 4339BB12..4339BD66, the complete angular step calculation after the
// target/manual-movement gates. Keeps FOV scaling, range scaling, seeded native
// random variation and the final length cap in their original operation order.
Vector calculate_aim_step(const ProfileState&, const AimStepInput&, RandomBindings);
struct TriggerFrame {
    std::uint32_t context_flags; // native weapon context +0
    std::uint32_t weapon_id; // +0xD8
    float time; // native global 43467EB8
    float blind_amount;
};
struct TriggerCommand {
    std::uint32_t buttons;
    std::int32_t tick;
};
struct TriggerHit {
    float damage;
    std::uint32_t hitgroup;
    void* entity;
};
struct TargetTrack;
// Complete 4331FBDB. target_times contains the first float of each native
// 0x28-byte record at owner +0x910, indexed by (networkable index & 63).
bool trigger_hit_allowed(const ProfileState&, const TriggerHit&,
                         std::span<const float, 64> target_times);
// Whole4331FBDB live-owner form: the age is read from the original strided
// target record after the genuine networkable index call has returned.
bool trigger_hit_allowed_live(const ProfileState&, const TriggerHit&, const TargetTrack* targets);
// 4339BFBD..4339C074: activation, pending-burst and blind-player gates before
// tracing. True enters the native ballistic/hitchance path.
bool begin_trigger(const Settings&, ProfileState&, const TriggerFrame&, InputBitmap,
                   TriggerCommand&);
// 4339C408..4339C491: apply the actual trace result/tick, then advance the
// burst timer. RandomFloat is called only on a successful non-burst result.
void finish_trigger(ProfileState&, const TriggerFrame&, bool hit, std::int32_t selected_tick,
                    TriggerCommand&, RandomBindings);
// Whole4339C408..4339C491 continuation. Frame flags are read after tracing,
// and the world clock is read after RandomFloat when random burst is selected.
void finish_trigger_live(ProfileState&, const std::uint32_t* context_flags, const float* time,
                         bool hit, std::int32_t selected_tick, TriggerCommand&, RandomBindings);
} // namespace skeet::legit

// Declarations from runtime_owner.hpp

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
class CrtRuntimeOwner;
class NativeStartupSources;
}
namespace skeet::legit {
// Native phase4 automatic workspace. The explicit-input constructors retain
// a differential-fixture seam; production needs no persistent seed arrays.
struct CommandFrameInputs {
    lagcomp::RecordInterpolationScratch record_interpolation;
    AccuracyMatrixScratch accuracy_interpolation;
    std::array<Matrix3x4, 256> attachment_matrices;
    std::array<Matrix3x4, 256> saved_matrices;
};
// Genuine RNG/math/engine boundary services. The explicit constructor supports
// authored standalone fixtures using the same complete recovered source body.
struct RuntimeServices {
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    lagcomp::RotationMath rotation;
    float (*exponential)(float);
    TriggerTraceBindings trace;
};

// Owns the single source settings/runtime state and the real4347FEBC factory
// context. No initial player/Command/target is retained. All phase4 physical
// contexts are formed from the actual arguments and canonical engine cells.
// Construction registers nothing and calls no engine/game function.
class RuntimeOwner final {
public:
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&, CommandFrameInputs&);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, CommandFrameInputs&, RuntimeServices);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, RuntimeServices);
    ~RuntimeOwner();
    RuntimeOwner(const RuntimeOwner&) = delete;
    RuntimeOwner& operator=(const RuntimeOwner&) = delete;
    RuntimeOwner(RuntimeOwner&&) = delete;
    RuntimeOwner& operator=(RuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    Settings& settings() noexcept;
    RuntimeState& state() noexcept;
    client::LegitCommandOwner& commands() noexcept;
    OwnerBindings& factory_bindings() noexcept;
    // Binding selects the existing real allocation/constructor body only.
    // NativeStartupSourceGraph may instead consume factory_bindings() in its
    // eight-owner bundle; do not bind the same initializer through both paths.
    void bind_initializer(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::legit

// Declarations from state.hpp

namespace skeet::legit {
struct RuntimeState {
    ProfileState profile;
    std::int32_t last_command = 0; // owner808
    std::uint8_t new_shot = 0; // 80C
    Vector pending_step{}; // 864
    float pending_fraction = 0; // 870
    Vector remainder{}; // 874
    float block_until = 0; // 880
    std::int32_t target = 0, previous_target = 0; //884,888
    Vector recoil_velocity{}; //88C
    Vector filtered_motion{}; //8C0
    Vector raw_motion{}; //8CC
    float time_scale = 0; //8D8
    float manual_time = 0; //8DC
    Vector target_offset{}; //8E0
    float curve = 0; //8EC
    Vector current_view{}; //8F0
    std::uint8_t current_view_valid = 0; //8FC
    Vector previous_view{}; //900
    std::uint8_t previous_view_valid = 0; //90C
    std::array<TargetTrack, 64> targets{}; //910
};
// Complete433B2251: reset an active lock and reseed the native curvature.
void release_target(RuntimeState&, float interval, RandomBindings);
// Complete433C8F71, including record ages/masks/points and view-history validity.
void reset_runtime(RuntimeState&, float interval, RandomBindings);
// The full owner reloads the interval after target-release RNG callbacks.
void reset_runtime_live(RuntimeState&, const float* interval, RandomBindings);
// Complete433CC76E, the owner's secondary-interface fractional angle consumer.
void apply_pending_aim(RuntimeState&, client::Command&, float delta_time, float interval);
} // namespace skeet::legit

// Declarations from studio.hpp

namespace skeet::legit {
struct StudioOffsets {
    std::ptrdiff_t wrapper, hitbox_set;
};
struct StudioHitboxSet {
    const std::byte* descriptors;
    std::int32_t count;
    bool studio;
};
// Full nullable433FD30D wrapper/header gate and native set lookup. Descriptor
// indexing remains a caller operation; the native accuracy search uses0/3.
StudioHitboxSet load_target_hitbox_set(const void* entity, const StudioOffsets&);
struct TargetStudio {
    bool ready = false;
    std::int32_t count = 0;
    std::array<AcquisitionHitbox, 19> hitboxes;
    std::array<std::uint32_t, 19> bones;
    std::span<const AcquisitionHitbox> descriptors() const noexcept {
        return {hitboxes.data(), std::size_t(count)};
    }
    animation::BoneModel bone_model() const noexcept {
        return {true, 0, ready, false, false, {}, {}, {bones.data(), std::size_t(count)}};
    }
};
// Concrete433FD30D plus the shared studio/header/set/bone descriptor reads.
// The native hitbox-to-bone helpers cap the model's descriptor count at19.
TargetStudio load_target_studio(const void* entity, const StudioOffsets&);
} // namespace skeet::legit

// Declarations from target_selection.hpp

namespace skeet::legit {
struct TargetSelectionInput {
    Vector eye_position;
    Vector reference_angles; // complete433D5ACC result, roll ignored
    std::int32_t maximum_clients;
    float time;
    const float* live_time = nullptr;
};
struct AimTarget {
    std::int32_t index = 0;
    Vector angles{};
    float field_of_view = 0;
    float distance = 0;
};
struct TargetSelectionBindings {
    const rage::TargetBindings& entities;
    const TargetTrackingBindings& tracking;
    // Native __libm_sse2_atan2, with its original double ABI.
    double (*arc_tangent2)(double, double);
    RandomBindings random;
};
// 4339AF3F..4339B70E: nearest angular candidate, concrete target prediction,
// previous-target eligibility/retention, and target-change random state. The
// caller has already admitted the aim hotkey/lock-time gates. A failed previous
// target sets the native .4-second block; every failed selection releases it.
AimTarget select_aim_target(RuntimeState&, const TargetSelectionInput&,
                            const TargetSelectionBindings&);
} // namespace skeet::legit

// Declarations from trace_filter.hpp

namespace skeet::legit {
// Complete4346DCEC ABI prefix. Its five native entries only tail-forward to
// the actual engine table43475B90; the prefix owns source/last-hit state.
struct BulletTraceFilter {
    void* const* vtable;
    std::array<std::uint32_t, 5> fields_04{};
    std::uint32_t mask = 0x4600400b, exit_mask = 0x4600400b;
    float minimum_damage = 1.0f;
    explicit BulletTraceFilter(void* const* table) : vtable(table) {
        if (!table)
            throw std::logic_error("LEGIT requires the actual engine trace-filter table");
    }
    BallisticFilter binding() noexcept {
        return {this, fields_04[4], mask, exit_mask, minimum_damage};
    }
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(BulletTraceFilter) == 0x24);
static_assert(offsetof(BulletTraceFilter, mask) == 0x18);
static_assert(offsetof(BulletTraceFilter, minimum_damage) == 0x20);
#endif
// Both software entity clipping and genuine TraceRay use the same native ABI
// filter object. No should-hit decision is delegated to a generic substitute.
class FilterBallistics final : public BallisticEngine {
    BallisticEngine& inner;

public:
    explicit FilterBallistics(BallisticEngine& engine) : inner(engine) {}
    std::int32_t filter_type(const BallisticFilter& f) override {
        return engine::invoke<std::int32_t>(f.object, 1);
    }
    void bind_source(const BallisticFilter& f, BallisticEntity e) override {
        engine::invoke<void>(f.object, 2, e);
    }
    void bind_last_hit(const BallisticFilter& f, BallisticEntity e) override {
        engine::invoke<void>(f.object, 4, e);
    }
    bool accepts(const BallisticFilter& f, BallisticEntity e, std::uint32_t m) override {
        return engine::invoke<bool>(f.object, 0, e, m);
    }
    std::uint32_t point_contents(const BallisticVector& p, std::uint32_t m, bool w) override {
        return inner.point_contents(p, m, w);
    }
    void trace_ray(const BallisticTraceRequest& r, BallisticTrace& t) override {
        inner.trace_ray(r, t);
    }
    void clip_ray_to_entity(const BallisticRay& r, std::uint32_t m, BallisticEntity e,
                            BallisticTrace& t) override {
        inner.clip_ray_to_entity(r, m, e, t);
    }
    PenetrationSurface surface(std::int16_t p) override {
        return inner.surface(p);
    }
    BallisticEntity entity(std::int32_t i) override {
        return inner.entity(i);
    }
    std::int32_t maximum_clients() override {
        return inner.maximum_clients();
    }
    BallisticEntityInfo entity_info(BallisticEntity e) override {
        return inner.entity_info(e);
    }
    BallisticEntityInfo clip_entity_info(BallisticEntity e) override {
        return inner.clip_entity_info(e);
    }
    BallisticBreakableInfo breakable_info(BallisticEntity e) override {
        return inner.breakable_info(e);
    }
    BallisticModelInfo model_info(std::int32_t i) override {
        return inner.model_info(i);
    }
    std::uint8_t damage_mode(BallisticEntity e) override {
        return inner.damage_mode(e);
    }
    void set_damage_mode(BallisticEntity e, std::uint8_t m) override {
        inner.set_damage_mode(e, m);
    }
    bool has_engine_breakable_callback() override {
        return inner.has_engine_breakable_callback();
    }
    bool engine_breakable(BallisticEntity e) override {
        return inner.engine_breakable(e);
    }
    BallisticRules rules() override {
        return inner.rules();
    }
};
} // namespace skeet::legit

// Declarations from tracking.hpp

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::legit {
struct TargetTrack {
    float elapsed = 0;
    std::uint32_t visible_hitgroups = 0;
    Vector point{};
    Vector velocity{};
    std::uint8_t velocity_valid = 0;
    std::array<std::uint8_t, 3> padding{};
    float interval = 0;
};
static_assert(sizeof(TargetTrack) == 0x28);
static_assert(offsetof(TargetTrack, velocity) == 0x14);
struct TargetTrackingBindings {
    std::span<const lagcomp::RecordRing, 64> histories;
    std::span<TargetTrack, 64> tracks;
    std::ptrdiff_t velocity_offset;
    std::int32_t clock_tick;
    float interval;
    // Full command owner binds the actual433B8A2B cache, including allocation
    // side effects. A supplied ring span remains the standalone verified API.
    client::PlayerStateCache* players = nullptr;
    const std::int32_t* live_clock_tick = nullptr; // [43475A60]+4, not43468D4C
    const float* live_interval = nullptr;
};
// Complete native4340C148, with cached history owners already present.
// Uses the entity networkable index, command-tick window and actual velocity
// netvar when no eligible historical velocities were accumulated.
Vector estimate_target_velocity(const ProfileState&, const TargetTrackingBindings&, void* entity);
// Complete4340B419, including its second networkable-index lookup and persistent
// velocity clamp, then the bounded 0.1..320 speed translation of the aim point.
void predict_target_point(const ProfileState&, const TargetTrackingBindings&, void* entity,
                          Vector& point);
} // namespace skeet::legit

// Declarations from trigger_trace.hpp

namespace skeet::legit {
struct TriggerTraceInput {
    BallisticEntity source;
    Vector origin;
    RageSpreadBasis basis;
    rage::SpreadParameters spread;
    std::uint32_t weapon_flags_7c;
    std::span<const float, 64> target_times;
    const TargetTrack* live_targets = nullptr; // full owner uses actual0x28-byte records
};
struct TriggerTraceBindings {
    void* context;
    // Actual entity networkable virtual slot 10. Invoked only when a hit with
    // nonzero reaction time reaches the predicate at native 4331FBDB.
    std::int32_t (*entity_index)(void*, BallisticEntity);
    // The optional engine smoke function reached through native 433E7C6B.
    bool (*through_smoke)(void*, const Vector&, const Vector&);
};
// 4339C260..4339C404: full 128-seed trigger hitchance traversal, using the
// native seed permutation, actual spread generator, direction trace, damage,
// hitgroup and reaction-time predicate, including both early stop rules.
bool evaluate_trigger_samples(BallisticEngine&, const ProfileState&, const TriggerTraceInput&,
                              const BallisticFilter&, BallisticQuery&, const rage::SpreadTables&,
                              const rage::SpreadRandom&, const rage::ShotgunPattern&,
                              const RageTrigonometry&, TriggerTraceBindings);
// 4339C194..4339C404: initial center trace/smoke check and the exact zero-hit-
// chance fast path, followed by the complete sample traversal above.
bool trace_trigger(BallisticEngine&, const ProfileState&, const TriggerTraceInput&,
                   const BallisticFilter&, BallisticQuery&, const rage::SpreadTables&,
                   const rage::SpreadRandom&, const rage::ShotgunPattern&, const RageTrigonometry&,
                   TriggerTraceBindings);
} // namespace skeet::legit

// Declarations from acquisition.hpp

namespace skeet {
struct MovementExtrapolationLiveBindings;
}
namespace skeet::client {
struct CommandBudgetBindings;
}
namespace skeet::ui {
struct SettingsMenuValues;
}
namespace skeet::rage {
class FeedbackOwner;
// The ten native dwords copied from the preamble's SP178 to evaluation+BC.
// Unnamed option bytes are preserved until their actual consumers read them.
struct AcquisitionTargetSettings {
    std::array<std::uint32_t, 10> words{};
    std::uint8_t option(std::size_t native_offset) const noexcept;
    void option(std::size_t native_offset, std::uint8_t value) noexcept;
    ScanPolicy scan_policy() const noexcept;
    void scan_policy(const ScanPolicy&) noexcept;
};
struct AcquisitionEvaluation {
    // One physical native0x1190-byte owner. Its unwritten prefix/EC padding
    // remains incoming storage; record is an alias of native.record.
    lagcomp::EvaluationState native;
    lagcomp::LagRecord& record;
    AcquisitionEvaluation() noexcept : record(native.record) {}
    AcquisitionEvaluation(const AcquisitionEvaluation&) = delete;
    AcquisitionEvaluation& operator=(const AcquisitionEvaluation&) = delete;
    const lagcomp::LagRecord* source_record = nullptr;
    AcquisitionTargetSettings settings;
    std::int32_t requested_tick = -1; // native candidate+B4
    std::uint32_t body_disabler_reasons = 0; // native candidate+B8
    std::uint32_t ballistic_evaluations = 0; // native candidate+E8
    bool extrapolated = false;
    std::int32_t extrapolation_ticks = 0;
    std::uint32_t target_handle = UINT32_MAX;
};
// Publishes only proven prefix fields into the retained native owner. This
// includes every point word and preserves the independent0xEC padding dword.
const lagcomp::EvaluationState& synchronize_evaluation_state(const TargetCandidate&,
                                                             AcquisitionEvaluation&) noexcept;
// Projects an existing/restored complete owner without synthesizing its holes.
void restore_evaluation_state(const lagcomp::EvaluationState&, TargetCandidate&,
                              AcquisitionEvaluation&) noexcept;
struct AcquisitionWorkItem {
    std::array<std::byte, 36> bytes;
};
static_assert(sizeof(AcquisitionWorkItem) == 36);
struct AcquisitionQueue {
    std::array<AcquisitionWorkItem, 80> items;
    std::uint32_t count = 0;
    // Native4340C789 accepts appends only while begin aliases the in-object
    // 80-item buffer. Preserve this distinct state after an external rebind.
    bool inline_storage = true;
    void clear() noexcept {
        count = 0;
    }
};
// The constructor434803BC allocates its80-record block separately from the
// primary owner;43387719 frees that block and clears its pointer. Candidates,
// requests and the embedded work queue keep their existing canonical owners.
class AcquisitionEvaluations {
public:
    AcquisitionEvaluations() noexcept = default;
    ~AcquisitionEvaluations();
    AcquisitionEvaluations(const AcquisitionEvaluations&) = delete;
    AcquisitionEvaluations& operator=(const AcquisitionEvaluations&) = delete;
    void allocate(engine::JobMemory);
    void release() noexcept;
    AcquisitionEvaluation& operator[](std::size_t index) noexcept {
        return data_[index];
    }
    const AcquisitionEvaluation& operator[](std::size_t index) const noexcept {
        return data_[index];
    }
    AcquisitionEvaluation* data() noexcept {
        return data_;
    }
    const AcquisitionEvaluation* data() const noexcept {
        return data_;
    }
    std::size_t size() const noexcept {
        return data_ ? 80u : 0u;
    }
    AcquisitionEvaluation* begin() noexcept {
        return data_;
    }
    AcquisitionEvaluation* end() noexcept {
        return data_ ? data_ + 80 : nullptr;
    }
    const AcquisitionEvaluation* begin() const noexcept {
        return data_;
    }
    const AcquisitionEvaluation* end() const noexcept {
        return data_ ? data_ + 80 : nullptr;
    }

private:
    AcquisitionEvaluation* data_ = nullptr;
    engine::JobMemory memory_{};
};
struct AcquisitionStorage {
    // The canonical contiguous80-candidate projection is consumed directly by
    // rank_completed_target_candidates; its matching record slot has the same index.
    std::array<TargetCandidate, 80> candidates;
    AcquisitionEvaluations evaluations;
    std::array<std::optional<ScanRequest>, 80> requests;
    AcquisitionQueue queued;
};
struct PreambleState;
struct PreambleResult;
struct AcquisitionRecordBindings {
    client::PlayerStateCache& players;
    const void* lag_compensation;
    float interval;
    std::int32_t remembered_target_handle;
    const std::int32_t& body_disablers; //live IntegerConfig.value4346A8D0; low byte read per record
    FeedbackOwner* const& feedback_owner; //canonical43475A7C slot; resolved after health getter
    const lagcomp::HistorySelectionBindings& history;
    const lagcomp::ExtrapolatedRecordBindings& extrapolation;
    ScanContext& scanner;
    const ui::SettingsMenuValues& settings_menu; // live Multithreading434683FC
    const animation::BoneRuntime& bone_runtime;
    const MovementExtrapolationLiveBindings& live_movement;
    const client::CommandBudgetBindings& live_budget; // live433A13ED global references
    // Buffered-shot433C155F clears the safe-neck bit after record policy.
    bool clear_safe_neck = false;
};
// Shared5182A..51C32 /5430E..5473E. Every record load, capture/extrapolation,
// body/safe policy and scanner invocation is a concrete recovered source call.
void prepare_and_submit_target_records(PreambleState&, PreambleResult&, AcquisitionStorage&,
                                       void* target, client::PlayerState&,
                                       std::span<const std::int32_t> ticks,
                                       const AcquisitionTargetSettings&,
                                       std::uint32_t descriptor_flags,
                                       const AcquisitionRecordBindings&);
// Concrete native enqueue/direct branch43354663..43354703 and its duplicated
// single-target block. Queued worker43406EE0 observes the canonical previous
// target mask before deciding whether to execute the full scanner.
void submit_acquisition_scan(AcquisitionStorage&, PreambleState&, ScanContext&, void* target_player,
                             std::uint32_t slot, std::uint32_t descriptor_flags,
                             const ui::SettingsMenuValues&);
// Complete nonempty-job drain4335473E..43354793, using the recovered36-byte
// dispatcher and this module's actual source worker. The live checkbox is
// read again here; a disabled drain preserves the queued logical end.
void dispatch_acquisition_scans(AcquisitionStorage&, engine::JobDispatcher&,
                                const ui::SettingsMenuValues&);
// Final per-player write43354793..433547C9 after the job drain.
void finish_target_acquisition(PreambleState&, AcquisitionStorage&, engine::JobDispatcher&,
                               client::PlayerStateCache&, std::int32_t maximum_clients,
                               const ui::SettingsMenuValues&);
struct PendingTargetDamage {
    std::int32_t entity_index;
    std::int32_t damage;
};
// Complete4341F16E. False leaves both output fields untouched.
bool pending_target_damage(const ShotFeedbackState*, PendingTargetDamage&) noexcept;
// 4335435C..433543BF, after the actual entity health getter at slot122.
std::int32_t scan_target_health(std::int32_t health, std::uint32_t entity_index,
                                const PendingTargetDamage*) noexcept;
// Complete433318B5. reference_damage is deliberately retained.
void reset_scan_point(TargetPoint&) noexcept;
// 43383EAE..43383F11 after the canonical player cache lookup and ConVar gate.
bool target_history_is_usable(const client::NativePlayerStateLayout&, const lagcomp::RecordRing&,
                              const client::CommandFrame&, std::uint32_t entity_index,
                              float interval) noexcept;
} // namespace skeet::rage

// Declarations from acquisition_frame.hpp

namespace skeet::rage {
// Refreshed timing/reference owners for one acquisition branch. Production
// record paths use records.bone_runtime for fresh per-target model/pose reads;
// existing semantic helper bindings retain their separate fixture contracts.
struct AcquisitionBranchFrameBindings {
    TargetBindings targeting;
    lagcomp::HistoryTickTiming prediction_timing, scanner_timing, extrapolation_timing,
        history_timing;
    lagcomp::EntityPreparationContext prediction_preparation, scanner_preparation,
        extrapolation_preparation, history_preparation;
    lagcomp::RecordSelectionContext prediction_records;
    lagcomp::ExtrapolatedRecordBindings extrapolation, history_extrapolation;
    lagcomp::HistorySelectionBindings history;
    TargetPreparationBindings preparation;
    RecordCandidateBindings record_candidates;
    ScanContext scanner;
    AcquisitionRecordBindings records;
    BodyAimProbeContext body_probe;
    TargetPredictionBindings prediction;
    AcquisitionBranchFrameBindings(PreambleState&, const client::CommandFrame&,
                                   float prediction_time, std::int32_t remembered_target_handle,
                                   std::uint32_t scheduler_flags,
                                   const std::array<RageSpreadSample, 32>&,
                                   const TargetPreparationBindings&, const BodyAimProbeContext&,
                                   const TargetPredictionBindings&, const RecordCandidateBindings&,
                                   const AcquisitionRecordBindings&,
                                   const players::State& adjustments,
                                   const ui::SettingsMenuValues&);
    AcquisitionBranchFrameBindings(const AcquisitionBranchFrameBindings&) = delete;
    AcquisitionBranchFrameBindings& operator=(const AcquisitionBranchFrameBindings&) = delete;
};
class AcquisitionFrameBindings {
public:
    // Construct immediately after prepare_rage_command. All owned reference
    // targets outlive .single/.multiple and every queued scan until its drain.
    AcquisitionFrameBindings(PreambleState&, const PreambleResult&, float live_prediction_time,
                             std::int32_t live_yaw_target_handle,
                             std::uint32_t live_scheduler_flags, const SingleTargetBindings&,
                             const MultipleTargetBindings&, const ui::SettingsMenuValues&);
    AcquisitionFrameBindings(const AcquisitionFrameBindings&) = delete;
    AcquisitionFrameBindings& operator=(const AcquisitionFrameBindings&) = delete;
    const std::array<RageSpreadSample, 32> samples;
    AcquisitionBranchFrameBindings single_frame, multiple_frame;
    SingleTargetBindings single;
    MultipleTargetBindings multiple;
};
// The post-firing stop projection changes the selected frame and clock again.
// Owned reference targets remain alive through its prediction and scan calls.
class TargetFrameBindings {
public:
    TargetFrameBindings(const TargetPredictionBindings&, ScanContext&, const client::CommandFrame&,
                        float live_prediction_time, std::uint32_t live_scheduler_flags);
    TargetFrameBindings(const TargetFrameBindings&) = delete;
    TargetFrameBindings& operator=(const TargetFrameBindings&) = delete;
    lagcomp::HistoryTickTiming prediction_timing, scanner_timing;
    lagcomp::EntityPreparationContext prediction_preparation, scanner_preparation;
    lagcomp::RecordSelectionContext prediction_records;
    BodyAimProbeContext body_probe;
    TargetPredictionBindings prediction;
    ScanContext scanner;
};
} // namespace skeet::rage

// Declarations from action_state.hpp

namespace skeet::rage {
class FeedbackOwner;
struct ActionStateBindings {
    bool active; // actual global byte4346A65C
    const esp::AimbotVisualizationConfig& visualization;
    PreambleState& state;
    const PreambleBindings& preamble;
    const TargetPreparationBindings& target_preparation;
    const players::State& player_adjustments;
    FeedbackOwner* const& feedback_owner; //canonical43475A7C slot; resolved after health getter
    const lagcomp::HistorySelectionBindings& history;
    ScanContext& scan;
    MovementCollisionEngine& collision;
    std::uint8_t* trace_flag;
    // Production history/attachments always read the action's actual target.
    const animation::BoneRuntime& bone_runtime;
    const MovementExtrapolationLiveBindings& live_movement;
    const client::CommandBudgetBindings& live_budget;
};
// Native433320D4, called at4335773F with the selected frame/player/command.
// Produces the same four hitbox flags and target handle consumed by433D12F7.
// The configuration owner is represented by its existing typed settings and
// shared RAGE state in ActionStateBindings, rather than another native layout.
void update_action_state(esp::HitboxHighlightState&, client::CommandFrame&, void* local_player,
                         client::Command&, const ActionStateBindings&);
} // namespace skeet::rage

// Declarations from body_aim_probe.hpp

namespace skeet::anti_aim {
class TargetTraceEntities;
}
namespace skeet::rage {

struct BodyAimProbeOffsets {
    std::ptrdiff_t network_origin; // A9EE6062
    std::ptrdiff_t velocity; // 36375C23
    std::ptrdiff_t studio_header; // 433FD30D decoded pointer field
    std::ptrdiff_t hitbox_set; // C2B6FB10
    std::ptrdiff_t cached_bones; // 17978090
    std::ptrdiff_t weapon_item;
};
struct BodyAimProbeContext {
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    BallisticEntity source; // actual local player43475808
    void* weapon; // frame+0C, resolved by the enclosing scheduler
    BodyAimProbeOffsets offsets;
    std::uint32_t context_flags; // acquisition owner+170
    std::uint32_t weapon_flags; // acquisition owner+174
    const std::uint8_t& allow_penetration; //live BooleanConfig.value4346A3E0
    std::uint8_t* trace_flag; // optional434683D0
    void* const* trace_filter_vtable; // actual external table43475B90
    const anti_aim::TargetTraceEntities& trace_entities;
};

// Exact433533CB..433537A1, including the repeated normalizations and the
// ordered endpoint comparisons. The capsule is already transformed by its
// current live bone; hitbox-angle rotation is not applied by this branch.
std::array<RageVector, 2> body_probe_surface_points(const RageCapsule&,
                                                    const RageVector& origin) noexcept;

// Complete optional acquisition branch433528CC..433538BF. Valid live studio
// hitbox6 and its cached bone are an enclosing acquisition contract, as in the
// native code. On admission, only masks C0/C4 and frame flag01000000 change.
bool probe_body_aim(client::CommandFrame&, AcquisitionTargetSettings&, void* target_player,
                    const BodyAimProbeContext&);

} // namespace skeet::rage

// Declarations from command_owner.hpp

namespace skeet::rage {
struct FinalTargetContextBindings;
struct PostFiringBindings;
// One owner composes the recovered secondary-vtable phases. All referenced
// bindings describe the current engine snapshot and share the same canonical
// state, acquisition storage, player cache and runtime-owned job dispatcher.
// ScanContext.configuration_flags aliases state.flags; its target_mask aliases
// state.previous_targets (+1A4), distinct from grouping's state.target_mask.
// readiness.prediction_time and preamble.prediction_time reference one clock.
struct CommandOwner {
    PreambleState& state;
    AcquisitionStorage& acquisition;
    const PreambleBindings& preamble;
    const SingleTargetBindings& single_target;
    const MultipleTargetBindings& multiple_targets;
    const FinalTargetContextBindings& final_target;
    FiringSharedState& firing_shared;
    const FiringReadinessBindings& readiness;
    const FiringPostAngleBindings& post_angles;
    const PostFiringBindings& post_firing;
    const PreambleTailBindings& tail;
    const CommandPhase2Bindings& phase2;
    const CommandPhase6Bindings& phase6;
    const ui::SettingsMenuValues& settings_menu; // canonical MISC Settings byte owners
};
// Native43358716, called before CommandFrame construction.
void process_rage_command_phase2(CommandOwner&, void* player, void* weapon, client::Command&,
                                 std::array<float, 3>& incoming_angles, std::uint8_t& send_packet);
// Complete phase4 owner4334F55F. The selected historical frame/command flow
// through acquisition, lazy final evaluation, firing, continuation and tail.
// send_packet is the live byte represented by the frame's native+8 pointer.
void process_rage_command(CommandOwner&, client::CommandFrame&, void* player, client::Command&,
                          std::uint8_t& send_packet);
// Native4335CBFB and433B76DE use the same live global angle pair/frame ring.
void process_rage_phase6(CommandOwner&, const client::CommandFrame&, const client::Command&);
void capture_rage_command_frame(CommandOwner&, const client::CommandFrame&) noexcept;
} // namespace skeet::rage

// Declarations from command_phase2.hpp

namespace skeet::rage {
struct PreambleState;
struct CommandPhase2Offsets {
    // These are the same live decoded fields used by evaluate_fire_state.
    const weapon::FireStateOffsets& fire_state;
    std::ptrdiff_t tick_base; //434693D8 /3D10B94C, int32
    std::ptrdiff_t reserve_ammunition; //4346749C /49C770CA, int32
};
struct CommandPhase2Bindings {
    const GlobalSettings& settings;
    InputBitmap input;
    const std::int32_t& field_4347c9ec;
    const void* accuracy_convar; //resolved43468EA4 XOR FCD2DA7A
    void* const* game_rules; //4346A3A4; either level may be absent
    float interval; //*4346A338
    CommandPhase2Offsets offsets;
};
inline constexpr std::uint32_t rage_command_phase2_body = 0x43358716u;
// Complete43358716..43358951. This runs before CommandFrame construction.
// The +174 word belongs to the same PreambleState consumed by acquisition
// and firing; it is cleared even when RAGE is disabled. Native engine input
// supplies a nonnull player. Both trailing phase arguments are unused here.
void process_rage_command_phase2(PreambleState&, void* player, void* weapon, client::Command&,
                                 std::array<float, 3>& incoming_angles, std::uint8_t& send_packet,
                                 const CommandPhase2Bindings&);
} //namespace skeet::rage

// Declarations from command_phase6.hpp

namespace skeet::rage {
struct CommandPhase6Bindings {
    const void* const* game_rules; // native4346A3A4; both levels may be null
    std::ptrdiff_t rule_guard; // decoded43467910 / D720AEA6, byte
    std::ptrdiff_t weapon_identifier; // decoded4346908C / 5C9F0FBB, uint16
    // The actual global pair434759DC/E8, also read by animation43384C9E
    // and anti-aim433470E4. The RAGE gameplay owner has no private copy.
    std::array<animation::AnimationAngleSample, 2>& pending_angles;
};

// Complete433DF660. A different command shifts the current three raw words
// into the previous sample before publishing the new command/pitch/yaw.
// Repeating the same command only replaces its two angle words.
void publish_command_animation_angles(std::array<animation::AnimationAngleSample, 2>&,
                                      std::int32_t command_number,
                                      const std::array<float, 3>& angles) noexcept;

// Complete RAGE vtable phase6 owner4335CBFB. Its owner/player arguments are
// unused natively. The two weapon pointers, shared rule byte, exact firearm
// classification and firing-button branches precede the shared publication.
void process_rage_phase6(const client::CommandFrame&, const client::Command&,
                         const CommandPhase6Bindings&) noexcept;
} // namespace skeet::rage

// Declarations from command_phase8.hpp

namespace skeet::rage {
struct PreambleState;
// Complete secondary-vtable phase8 entry433B76DE. The same sixteen-frame
// history is consumed by prepare_rage_command; newest frame is frame_first.
void capture_rage_command_frame(PreambleState&, const client::CommandFrame&) noexcept;
} // namespace skeet::rage

// Declarations from command_source.hpp

namespace skeet::rage {
class PredictionMaskOwner;
// Production endpoints form their physical contexts only at invocation. The
// registered endpoint retains source ownership, never a player or frame view.
class CommandSource {
public:
    virtual ~CommandSource() = default;
    virtual PreambleState& state() noexcept = 0;
    virtual AcquisitionStorage& acquisition() noexcept = 0;
    virtual std::int32_t& shot_index() noexcept = 0;
    virtual esp::HitboxHighlightState& action_state() noexcept = 0;
    virtual std::array<animation::AnimationAngleSample, 2>& pending_angles() noexcept = 0;
    virtual PredictionMaskOwner* const& prediction_masks() const noexcept = 0;
    virtual void require_factory(const AimbotSettings&, const GlobalSettings&,
                                 const esp::AimbotVisualizationConfig&,
                                 const esp::HitboxHighlightState&) const = 0;
    virtual void phase2(void*, void*, client::Command&, std::array<float, 3>&, std::uint8_t&) = 0;
    virtual void process(client::CommandFrame&, void*, client::Command&, std::uint8_t&) = 0;
    virtual void phase6(const client::CommandFrame&, const client::Command&) = 0;
    virtual void capture(const client::CommandFrame&) noexcept = 0;
};
class CommandEndpoint {
    std::variant<CommandOwner*, CommandSource*> source_;

public:
    CommandEndpoint(CommandOwner& value) noexcept : source_(&value) {}
    CommandEndpoint(CommandSource& value) noexcept : source_(&value) {}
    PreambleState& state() const noexcept;
    AcquisitionStorage& acquisition() const noexcept;
    std::int32_t& shot_index() const noexcept;
    esp::HitboxHighlightState& action_state() const noexcept;
    std::array<animation::AnimationAngleSample, 2>& pending_angles() const noexcept;
    PredictionMaskOwner* const& prediction_masks() const noexcept;
    void require_factory(const AimbotSettings&, const GlobalSettings&,
                         const esp::AimbotVisualizationConfig&,
                         const esp::HitboxHighlightState&) const;
    void phase2(void*, void*, client::Command&, std::array<float, 3>&, std::uint8_t&);
    void process(client::CommandFrame&, void*, client::Command&, std::uint8_t&);
    void phase6(const client::CommandFrame&, const client::Command&);
    void capture(const client::CommandFrame&) noexcept;
};
} //namespace skeet::rage

// Declarations from entity_update.hpp

namespace skeet::client {
struct EntityPredictionEntities;
}
namespace skeet::rage {
struct EntityUpdateOffsets {
    std::ptrdiff_t movement_type; //7501CB91, byte
    std::ptrdiff_t owner_handle; //8D1BD8D0, int32
    std::ptrdiff_t model_index; //47877DEB, int16
    std::ptrdiff_t health; //9BB80163, int32
};
struct EntityUpdateBindings {
    void* const& engine_client; //4346A514
    void* const& entity_list; //4346A890
    void* const& model_info; //4346A614
    std::int32_t* const& maximum_clients; //43469300
    void* const& required_class; //43467478, actual ClientClass pointer
    EntityUpdateOffsets offsets;
};
// Complete43325DEC, entity-observer43472480 slot1. The incoming count is
// reread each iteration, and the two collideable getters remain distinct.
void update_rage_entities(const EntityUpdateBindings&, void* local_player,
                          const client::EntityPredictionEntities&);
}

// Declarations from final_target_context.hpp

namespace skeet::ui {
struct SettingsMenuValues;
}
namespace skeet::client {
struct CommandFrame;
}
namespace skeet::rage {
struct PreambleState;
struct AcquisitionEvaluation;
struct FinalTargetContextBindings {
    ScanContext& scan;
    const std::uint32_t* current_bone_counter;
    const ui::SettingsMenuValues& settings_menu; // live Multithreading434683FC
    engine::JobDispatcher* jobs;
};
// Complete engine-input owner4341B4C2. Call lazily for each attempted point,
// including promotions: bypass precedes entity lookup, then the bullet/basis,
// teammate policy/rays, studio reads, record transaction and sample workers.
// Failure updates the same player cache and context_flags used by acquisition.
bool evaluate_final_target(client::CommandFrame&, PreambleState&, const TargetCandidate&,
                           const AcquisitionEvaluation&, std::uint32_t& context_flags,
                           const FinalTargetContextBindings&, float& chance);
} // namespace skeet::rage

// Declarations from firing.hpp

namespace skeet::animation {
struct BoneRuntime;
}

namespace skeet::rage {
// Typed projection of the scheduler's weapon owner. Offset names deliberately
// retain fields whose meaning is not established by their consumers.
struct FiringState {
    std::uint32_t flags;
    std::int32_t weapon_id, marker, previous_tick, current_tick;
    std::int32_t field_fc, field_15c, field_1d4;
    float field_198;
    MovementAngles reference_angles; // +20
    RageVector recoil, origin, bone_origin; // +9C,+DC,+10C
    float field_84, field_a8, field_164; // +E4 is origin[2], not another field
    float special_threshold, angle_step; // +1E4,+1E8
    std::uint8_t* send_packet;
};
struct FiringTargetData {
    const TargetCandidate& candidate;
    std::int32_t field_b4, field_13c;
};
struct FiringBranchState {
    bool field_22, alternate_path, field_33, field_83, timing_ready;
    bool field_5b = false;
};
struct FiringSharedState {
private:
    std::int32_t local_marker_;
    RageVector local_point_;
    std::int32_t local_delay_;

public:
    struct Live {};
    std::int32_t& selected_marker;
    RageVector& selected_point;
    std::int32_t& last_delay_marker;
    FiringSharedState(std::int32_t marker = 0, RageVector point = {},
                      std::int32_t delay = 0) noexcept
        : local_marker_(marker), local_point_(point), local_delay_(delay),
          selected_marker(local_marker_), selected_point(local_point_),
          last_delay_marker(local_delay_) {}
    FiringSharedState(Live, std::int32_t& marker, RageVector& point, std::int32_t& delay) noexcept
        : selected_marker(marker), selected_point(point), last_delay_marker(delay) {}
    FiringSharedState(const FiringSharedState&) = delete;
    FiringSharedState& operator=(const FiringSharedState&) = delete;
};
struct FiringReadinessBindings {
    client::PlayerStateCache& players;
    void* engine_client;
    const void* timing_convar;
    float tick_interval, current_time;
    float& prediction_time;
    std::int32_t latency_milliseconds;
    float correction_time;
    float receive_interval;
    bool enabled, mode_enabled, send_mode;
    std::int32_t delay_option;
    std::uint32_t scheduler_flags;
    std::int32_t committed_marker, command_marker;
    float revolver_gate;
};
enum class FiringReadiness {
    angles,
    retained_target,
    stopped
};
// Complete43354C61..433551BA. Cache lookup is the recovered433B8A2B;
// engine-client slot14 supplies time on the history-age branch.
FiringReadiness evaluate_firing_readiness(FiringState&, const FiringTargetData&, FiringBranchState&,
                                          FiringSharedState&, client::Command&,
                                          const FiringReadinessBindings&);
std::int32_t firing_mode(bool mode_enabled, bool send_mode) noexcept; //4331FF20

// Both copies43355497..43355615 /43355C4D..43355DC8. Returns true when
// the angular distance was at least the configured limit and angles changed.
bool limit_firing_angle_step(MovementAngles&, const MovementAngles& reference,
                             float maximum_step) noexcept;
struct FiringPlayerOffsets {
    std::ptrdiff_t flags, first_byte, second_byte, animation_state;
    std::ptrdiff_t studio_header, hitbox_set, pose_parameters, animation_guard;
};
class FiringPoseParameter {
    using Decoded = float (*)(const void*, std::uint32_t, float, float&);
    std::variant<Decoded, std::reference_wrapper<const engine::EyePoseParameterRuntime>> source_;

public:
    FiringPoseParameter(Decoded entry) noexcept : source_(entry) {}
    FiringPoseParameter(const engine::EyePoseParameterRuntime& source) noexcept
        : source_(std::cref(source)) {}
    float operator()(const void* wrapper, std::uint32_t index, float angle,
                     float& normalized) const {
        if (auto* decoded = std::get_if<Decoded>(&source_))
            return (*decoded)(wrapper, index, angle, normalized);
        return std::get<std::reference_wrapper<const engine::EyePoseParameterRuntime>>(source_)
            .get()
            .convert(wrapper, index, angle, normalized);
    }
};
struct FiringPostAngleBindings {
    void* player;
    FiringPlayerOffsets offsets;
    void* input; // actual CInput; GetUserCmd slot8(slot=0,sequence=number-1)
    animation::BoneSourceReference<animation::BoneEntityState> bone_state;
    animation::BoneSourceReference<const animation::BoneModel> bone_model;
    animation::BoneSourceReference<const animation::BoneEngine> bone_engine;
    RageTrigonometry trigonometry;
    TargetAngleMath angle_math;
    // Genuine resolved engine target43467CF0. The output reference receives
    // the normalized pose value; the returned adjusted angle is unused here.
    FiringPoseParameter convert_pose_parameter;
    bool compensate_recoil, enabled, send_mode;
    bool recoil_option; // native43467144, also copied into stack branch33
    std::uint32_t scheduler_flags;
    float prediction_time;
};
// Complete433553BB..43355DEF after target_aim_angles. Reuses the concrete
// bone transaction, corrects eye height from the selected studio bone, then
// rebuilds angles and repeats the limiter. Only actual engine getters/poses
// are dependencies. False takes native43356146 before accepted-shot commit.
bool evaluate_firing_post_angles(FiringState&, const TargetCandidate&, FiringBranchState&,
                                 client::Command&, const FiringPostAngleBindings&);
// Production43388720 entry. Every build reads this player's current engine
// model/pose/entity fields through the mandatory raw runtime. The semantic
// bone_state/model/engine fields above belong only to the separate fixture API.
bool evaluate_firing_post_angles(FiringState&, const TargetCandidate&, FiringBranchState&,
                                 client::Command&, const FiringPostAngleBindings&,
                                 const animation::BoneRuntime&);
// Complete433D6CD1 and433DF6A6 substeps used by that transaction.
bool firing_animation_ready(const void* animation_state, std::uint8_t player_guard) noexcept;
float correct_firing_eye_height(const Matrix3x4&, float height) noexcept;
} // namespace skeet::rage

// Declarations from group_storage.hpp

namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;
}
namespace skeet::rage {
// One canonical producer for the tables43472B98 (tag12 descriptors) and
//4346EFE0 (enabled descriptors). Each blob header and pointer triple share
// their actual adjacent canonical cells; semantic blobs borrow that storage.
// Construction binds storage only. Individual CRT callbacks perform native
// registration/writes in the live scheduler order and append actual finalizers.
class GroupStorage {
public:
    GroupStorage(engine::PreservedInitialization&, engine::JobMemory);
    GroupStorage(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory);
    GroupStorage(const GroupStorage&) = delete;
    GroupStorage& operator=(const GroupStorage&) = delete;
    std::span<BooleanConfig*, 12> enabled_descriptors() noexcept {
        return enabled_pointers_;
    }
    std::span<ConfigBlob*, 12> blob_descriptors() noexcept {
        return blob_pointers_;
    }
    ui::RageWeaponEnabledSettings& enabled_settings() noexcept {
        return enabled_;
    }
    void initialize_blob(std::size_t, ConfigLink&);
    void initialize_enabled(std::size_t, ConfigLink&) noexcept;
    void finalize_blob(std::size_t);
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    ui::RageWeaponEnabledSettings enabled_;
    std::array<NativeBlobBytes, 12> native_;
    std::array<ConfigBlob, 12> blobs_;
    std::array<BooleanConfig*, 12> enabled_pointers_;
    std::array<ConfigBlob*, 12> blob_pointers_;
};

// Retain this context and GroupStorage through the same ExitCallbacks::run()
// used by startup/shutdown. No grouped initialization runs when this is bound.
struct GroupInitializerBindings {
    ConfigRegistries& configuration;
    client::ExitCallbacks& callbacks;
    GroupStorage& groups;
    void (*source_finalizer(std::uint32_t native_address))();
};
void bind_native_group_initializers(client::NativeStartupSources&, GroupInitializerBindings&);
}

// Declarations from initialization.hpp

namespace skeet::rage {
//4348055C..4348062D, after the owner's configuration registrations. The
// canonical AcquisitionStorage already supplies the80 allocated evaluations.
// Unlike lifecycle reset, construction initializes highlight hitbox IDs and
// each frame's sparse constructor, and preserves the seven target-mask words.
void initialize_rage_state(const RageResetBindings&) noexcept;
// Production constructor path: prefix stores precede the actual heap call;
// queue/frame initialization still runs when the evaluation allocation fails.
void initialize_rage_state(const RageResetBindings&, engine::JobMemory);
}

// Declarations from multiple_targets.hpp

namespace skeet::rage {
struct MultipleTargetBindings {
    const TargetPreparationBindings& preparation;
    const BodyAimProbeContext& body_probe;
    const TargetPredictionBindings& prediction;
    const RecordCandidateBindings& record_candidates;
    const AcquisitionRecordBindings& records;
    const players::State& adjustments;
    std::int32_t maximum_clients;
    engine::JobDispatcher& jobs;
};
// Complete multi-target acquisition43351C37..433547C9. The canonical
// preamble owns flags/masks/completion and storage supplies the same candidates
// consumed by the subsequent rank_completed_target_candidates call.
void acquire_multiple_targets(PreambleState&, PreambleResult&, AcquisitionStorage&,
                              const MultipleTargetBindings&);
} // namespace skeet::rage

// Declarations from owner.hpp

namespace skeet::client {
class CommandPhaseRegistry;
}
namespace skeet::rage {
class LifecycleOwner;
struct OwnerBindings {
    engine::OwnerRegistryBinding lifecycle;
    client::CommandPhaseRegistry& command_owners;
    engine::OwnerRegistryBinding hud_owners;
    client::EntityObserverRegistry& entity_owners;
    ConfigRegistries& configuration;
    AimbotSettings& aimbot;
    esp::AimbotVisualizationConfig& visualization;
    CommandEndpoint commands;
    hud::LiveRageHudBinding hud;
    EntityUpdateBindings entities;
};
// Complete434803BC owner construction. Aimbot/visualization descriptors must
// be unregistered, and commands.acquisition.evaluations must be empty. Their
// canonical storage and every borrowed binding outlive the returned owner.
// Registration is lifecycle -> command -> HUD -> entity, before descriptors.
LifecycleOwner* construct_rage_owner(OwnerBindings, LifecycleOwner*& active);
//433F19F5: clear the canonical43475810 slot before evaluation/primary frees.
void destroy_rage_owner(LifecycleOwner*& active);
// Exposes the native primary object for the common lifecycle dispatcher.
void* rage_lifecycle_object(LifecycleOwner&) noexcept;
}

// Declarations from post_firing.hpp

namespace skeet {
struct RageStopBindings;
}
namespace skeet::animation {
struct BoneRuntime;
}

namespace skeet::rage {
struct PostFiringBindings;
enum class PreambleTailEntry;
// Full main continuation contract shared with the command owner. The binding
// owns views of native globals and concrete helper contexts, not phase calls.
PreambleTailEntry continue_after_target_firing(PreambleState&, PreambleResult&, AcquisitionStorage&,
                                               TargetBatch&, const TargetFiringResult&,
                                               void* local_player, const PostFiringBindings&);
class PredictionMaskOwner;
//433C8BD6 loads the canonical singleton at use time. RAGE is constructed
// before the mask owner, so spans captured during startup would be premature.
struct PostFiringPredictionMasks {
    PredictionMaskOwner* const& owner; //43475A58; initialized before frame dispatch
};
bool post_firing_prediction_allowed(const PostFiringPredictionMasks&) noexcept; //433C8BD6
// First prefer the remembered low16-bit entity, then all prepared descriptors.
// Greater E8 ballistic count wins; ties keep the earlier descriptor.
std::size_t select_post_firing_candidate(const PreambleState&, const AcquisitionStorage&,
                                         std::int32_t remembered_handle) noexcept;
struct PostFiringOffsets {
    std::ptrdiff_t network_origin; //A9EE6062
    std::ptrdiff_t player_next_attack; //DE530A8B
    std::ptrdiff_t scope_guard_cacaeecc;
    std::ptrdiff_t scope_guard_c07d1150;
};
struct PostFiringMovementBindings {
    const GlobalSettings& settings;
    InputBitmap input;
    std::uint32_t& context_flags;
    std::uint32_t& scheduler_flags;
    std::uint32_t weapon_flags;
    std::span<const std::uint32_t, 2> target_mask; // canonicalPreambleState.target_mask
    const TargetBindings& targeting;
    PostFiringOffsets offsets;
    BallisticEngine& ballistics;
    BallisticEntity source;
    std::ptrdiff_t weapon_item;
    void* const* trace_filter_vtable;
    const anti_aim::TargetTraceEntities& trace_entities;
    PostFiringPredictionMasks prediction;
    const float& current_time;
    const std::int32_t& remembered_handle;
};
// Complete43356EED..433576F0: auto-scope, both native quick-stop publications,
// target damage probes, between-shot gate and final frame-bit400 restoration.
// scope_ground_gate is native AL at56EED; restore_firing_bit is saved SP+D8.
void finish_post_firing_movement(client::CommandFrame&, client::Command&,
                                 const TargetCandidate* leading,
                                 const AcquisitionEvaluation* leading_evaluation,
                                 bool scope_ground_gate, bool restore_firing_bit,
                                 const PostFiringMovementBindings&);
struct FinalTargetContextBindings;
struct ShotDiagnosticBindings;
struct ShotModelPublicationBindings;
struct PendingShotPublicationBindings;
struct PostFiringBindings {
    const PreambleBindings& preamble;
    const PostFiringMovementBindings& movement;
    client::PlayerStateCache& players;
    const client::CommandAngleBindings& command_angles;
    const FinalTargetContextBindings& final_target;
    script::Bridge& scripts;
    const ShotDiagnosticBindings& diagnostics;
    const PendingShotPublicationBindings& pending_shots;
    const ShotModelPublicationBindings& shot_models;
    std::int32_t& shot_index; //native gameplay+168
    const std::int32_t& entity_limit; //actual pointed43469300, exclusive upper bound
    const float& tick_correction; //43469254
    const TargetCandidate* prior_leading = nullptr; //SP+E0, initialized0 at4334FB52
    const RageStopBindings& stop;
    const MovementCaptureInput& simulation_capture;
    MovementCollisionEngine& collision;
    const TargetPredictionBindings& prediction;
    ScanContext& scanner;
    const animation::BoneRuntime& bone_runtime;
};
} // namespace skeet::rage

// Declarations from preamble.hpp

namespace skeet::client {
class TargetRefreshRuntimeOwner;
}

namespace skeet::rage {
struct AcquisitionStorage;
// The exact ten copied dwords at stack+178..19F / evaluation+BC..E3:
// mode, regular/safe masks, scale, multipoint, damage, chance, FOV cosine,
// then DC safe, DD body, DE delay, DF body-on-peek, E0 damage override,
// E1 additional hypotheses, E2/E3 zero.
using PreambleScanTemplate = std::array<std::uint32_t, 10>;

struct PreambleAnimationSnapshot {
    std::uint32_t sequence;
    std::array<std::byte, 0x10> fields_04;
    lagcomp::AnimationState animation;
    float absolute_yaw;
    std::array<std::byte, 0x354> fields_1e8;
};
static_assert(sizeof(PreambleAnimationSnapshot) == 0x53C);
static_assert(offsetof(PreambleAnimationSnapshot, animation) == 0x14);
static_assert(offsetof(PreambleAnimationSnapshot, absolute_yaw) == 0x1E4);
class PreambleAnimationHistory {
    const std::byte* records_;

public:
    PreambleAnimationHistory(const std::array<PreambleAnimationSnapshot, 150>* records) noexcept
        : records_(reinterpret_cast<const std::byte*>(records)) {}
    explicit PreambleAnimationHistory(const std::byte* records) noexcept : records_(records) {}
    const std::byte* at(std::uint32_t sequence) const noexcept {
        return records_ ? records_ + (sequence % 150u) * sizeof(PreambleAnimationSnapshot)
                        : nullptr;
    }
};

// Persistent fields of the RAGE gameplay subobject, whose address is the
// configuration owner+8. Acquisition and firing use these same references.
struct PreambleState {
    std::uint32_t flags = 0; // +170
    std::uint32_t weapon_flags = 0; // +174; separate retained owner
    std::array<std::uint32_t, 3> completed{}; // +1AC
    std::uint32_t evaluation_count = 0; // +1B8; prepared candidates
    std::array<std::uint32_t, 2> target_mask{}; // +19C
    std::array<std::uint32_t, 2> previous_targets{}; // +1A4
    SpreadCache spread; // primary+D14, compensated+1114
    std::uint32_t frame_first = 0; // +1514
    std::int32_t frame_count = 0; // +1518
    std::array<client::CommandFrame, 16> frames; // +151C
};
struct PreambleOffsets {
    std::ptrdiff_t weapon_reload_guard; // decoder B8969256
    std::ptrdiff_t ammunition; // CA5EC698
    std::ptrdiff_t reserve_ammunition; // 49C770CA
    std::ptrdiff_t weapon_identifier; // 5C9F0FBB
    std::ptrdiff_t weapon_next_attack; // F71CE331
    std::ptrdiff_t player_next_attack; // DE530A8B
    std::ptrdiff_t taser_timing; // D2563274
    std::ptrdiff_t local_guard; // D720AEA6
    std::ptrdiff_t animation_layers; // B538BBA5
    std::ptrdiff_t pose_parameters; // 1CAAF0A8
};
using PreambleTargetRefresh =
    std::variant<std::reference_wrapper<const anti_aim::TargetRefreshContext>,
                 std::reference_wrapper<const client::TargetRefreshRuntimeOwner>>;
struct PreambleBindings {
    const GlobalSettings& settings;
    const AimbotSettings& weapon_settings;
    InputBitmap input;
    std::uint32_t& scheduler_flags; // native434759C0
    const void* restricted_convar; // resolved43469028
    const void* const* game_rules; // 4346A3A4
    PreambleOffsets offsets;
    std::span<client::Command, 150> command_ring; // actual CInput+F0
    PreambleAnimationHistory animation_history;
    float& prediction_time; // pointed43467EB8
    float interval;
    bool firing_mode_enabled, send_mode;
    bool command_sequence_gate; // native43468D7C
    std::int32_t current_sequence; // *43475B2C
    std::int32_t& selected_marker; // native434759C4
    std::int32_t
        double_tap_chance; // retained ABI snapshot; reads use settings.double_tap_hit_chance
    bool shotgun_patterns; // terminal43468520 ConVar
    const SpreadTables& spread_tables;
    const SpreadRandom& random;
    const ShotgunPattern& pattern;
    RageTrigonometry trigonometry;
    anti_aim::TargetState& yaw_target;
    PreambleTargetRefresh target_refresh;
    // Genuine optional engine434688C8, identical to the animation runtime.
    lagcomp::SetAbsoluteVector set_absolute_angles;
};
enum class PreambleContinuation {
    no_weapon, //43358507
    inactive, //43357733
    ammunition, //433576F0
    single_target, //4335034F
    multiple_targets, //43351C37
};
struct PreambleResult {
    PreambleContinuation continuation = PreambleContinuation::no_weapon;
    client::CommandFrame* frame = nullptr;
    client::Command* command = nullptr;
    PreambleScanTemplate scan_template{};
    FiringBranchState firing{};
    bool original_firing_bit = false; // stack+D8 bit0, frame+0 mask400
    std::int32_t tick_base = 0; // stack+E4, frame+C8
    std::uint32_t command_hash = 0; // stack+11C
    float previous_prediction_time = 0; // observed stack+124, not restored at exit
    bool replaced_frame = false;
    std::uint32_t prediction_clock_marker = 0x7F7FFFFFU; // stack+120
    bool point_scale_is_automatic = false; // owner+C4==24
    float maximum_point_scale = 0.9f; // stack+22C
    std::uint32_t first_entity = 1;
};
// Complete true shared setup4334F55F..43350349. The selected-frame command,
// shared yaw-target refresh, animation copy and spread preparation are concrete
// source operations. No internal prelude phase is supplied as a callback.
// Native433E0A59 resets the acquisition owner's queued-work logical end without
// releasing its allocation or deleting evaluations.
PreambleResult prepare_rage_command(PreambleState&, client::CommandFrame& original,
                                    client::Command&, void* local_player, AcquisitionStorage&,
                                    const PreambleBindings&);
// Shared identical settings block also in action owner43332281..43332630.
PreambleScanTemplate prepare_rage_scan_template(const PreambleState&, const client::CommandFrame&,
                                                const PreambleBindings&);
} // namespace skeet::rage

// Declarations from preamble_tail.hpp

namespace skeet::rage {
struct PreambleTailBindings {
    const PreambleBindings& shared;
    esp::HitboxHighlightState& action_state;
    const ActionStateBindings& action;
    const legit::Settings* legit_settings; // nullable native4347588C
    TargetAngleMath angle_math; // genuine msvcrt atan2
    float (*arctangent)(float); // __libm_sse2_atanf
    void* input; // actual CInput virtual8
    FiringPlayerOffsets player_offsets;
    const client::CommandButtonPolicy& button_policy;
    client::CommandTrigonometry command_trigonometry;
    client::ScriptCommandEnvironment command_environment;
    std::span<client::VerifiedCommand, 150> verified_commands; // CInput+F4
    client::CommandChecksum checksum; // genuine optional43467C48
    void* prediction; // engine owner43469264
};
enum class PreambleTailEntry {
    after_firing, //433576F0
    actions, //43357733
    reconcile_command, //433583F2
    prediction_clock, //433584E5
    return_only, //43358507
};
struct PreambleTailResult {
    bool shot_marker_updated = false;
    bool historical_command_published = false;
};
// Complete43389A9A. This keeps the two discarded RNG draws per pellet and
// averages only the engine shotgun pattern component, without inaccuracy.
RageSpreadSample preamble_pattern_correction(const SpreadParameters&, std::uint32_t seed,
                                             const SpreadRandom&, const ShotgunPattern&,
                                             const RageTrigonometry&);
// Complete inline exits433576F0..4335850C. Action visualization calls the
// concrete433320D4 owner; simulation, angle limits and command finalizers reuse
// their actual recovered modules. The explicit entry identifies the native
// caller jump, so return-only paths do not run action/reconciliation work.
PreambleTailResult finish_rage_command(PreambleState&, PreambleResult&,
                                       client::CommandFrame& original, void* local_player,
                                       const PreambleTailBindings&, PreambleTailEntry);
} // namespace skeet::rage

// Declarations from prediction_mask_owner.hpp

namespace skeet::client {
class NativeStartupSources;
}
namespace skeet::rage {
class PredictionMaskOwner;
struct PredictionMaskFactoryBindings {
    engine::OwnerRegistryBinding lifecycle;
    engine::OwnerRegistryBinding frames;
    PredictionMaskRuntimeBinding runtime;
    PredictionMaskOwner*& active; //43475A58
};

// Complete4347EA07 factory and433BFE13 destruction. Source-only context
// follows the original0x1C prefix; the masks keep their native+0C/+14 offsets.
PredictionMaskOwner* create_prediction_mask_owner(PredictionMaskFactoryBindings&);
void destroy_prediction_mask_owner(PredictionMaskOwner*, PredictionMaskOwner*&, engine::JobMemory);
PredictionMaskState& prediction_mask_state(PredictionMaskOwner&) noexcept;
engine::FrameUpdateEndpoint& prediction_mask_endpoint(PredictionMaskOwner&) noexcept;
void bind_prediction_mask_factory(client::NativeStartupSources&, PredictionMaskFactoryBindings&);
}

// Declarations from prediction_mask_runtime_owner.hpp

namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::rage {
//4347EA07 precedes the actual ESP factory. This source context borrows that
// nullable singleton cell and resolves its EnginePlayers at the native weapon
// metadata use site. No synthetic player or second ESP adapter is created.
class PredictionMaskRuntimeOwner final {
public:
    PredictionMaskRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                               client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                               engine::SceneQueryRuntimeOwner&, esp::EspRuntimeOwner&);
    ~PredictionMaskRuntimeOwner();
    PredictionMaskRuntimeOwner(const PredictionMaskRuntimeOwner&) = delete;
    PredictionMaskRuntimeOwner& operator=(const PredictionMaskRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    PredictionMaskFactoryBindings& factory_bindings() noexcept;
    void bind_initializer(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::rage

// Declarations from prediction_masks.hpp

namespace skeet::esp {
class EnginePlayers;
class EspOwnerRuntime;
}
namespace skeet::anti_aim {
class TargetTraceEntities;
}
namespace skeet::rage {

// Canonical native owner+0C/+14, also read by433C8BD6 after target firing.
struct PredictionMaskState {
    std::array<std::uint32_t, 2> allowed;
    std::array<std::uint32_t, 2> blocked;
    // Native index64 uses index>>5 without masking. allowed[2] aliases
    // blocked[0]; blocked[2] reaches the zeroed heap tail at full-owner+1C.
    // Retain that actual word separately from source-only callback metadata.
    std::uint32_t trailing_word = 0;
};
static_assert(sizeof(PredictionMaskState) == 0x14);
class PredictionMaskPlayers {
    std::variant<std::reference_wrapper<esp::EnginePlayers>,
                 std::reference_wrapper<esp::EspOwnerRuntime* const>>
        source_;

public:
    PredictionMaskPlayers(esp::EnginePlayers& fixture) noexcept : source_(std::ref(fixture)) {}
    PredictionMaskPlayers(esp::EspOwnerRuntime* const& live) noexcept : source_(std::cref(live)) {}
    esp::EnginePlayers& get() const;
};
struct PredictionMaskBindings {
    void* const& engine_client; //4346A514
    const TargetBindings& targeting;
    const std::uint8_t& enabled; //434671E0
    const std::uint32_t& context_flags; //43467E40
    const std::int32_t& entity_limit; //pointed43469300, inclusive in this callback
    PredictionMaskPlayers players;
    BallisticEngine& ballistics;
    void* const* trace_filter_vtable; //actual engine filter interface43475B90
    const anti_aim::TargetTraceEntities& trace_entities;
};
struct LivePredictionMaskBindings {
    void* const& engine_client;
    void* const& local_player;
    const LiveTargetRelationships& targeting;
    const std::uint8_t& enabled;
    const std::uint32_t& context_flags;
    std::int32_t* const& entity_limit;
    esp::EspOwnerRuntime* const& players;
    BallisticEngine& ballistics;
    void* const* const& trace_filter_vtable;
    const anti_aim::TargetTraceEntities& trace_entities;
};
class PredictionMaskRuntimeBinding {
    std::variant<std::reference_wrapper<const PredictionMaskBindings>,
                 std::reference_wrapper<const LivePredictionMaskBindings>>
        source_;

public:
    PredictionMaskRuntimeBinding(const PredictionMaskBindings& decoded) noexcept
        : source_(std::cref(decoded)) {}
    PredictionMaskRuntimeBinding(const LivePredictionMaskBindings& live) noexcept
        : source_(std::cref(live)) {}
    void update(PredictionMaskState&, std::uint8_t) const;
};

void reset_prediction_masks(PredictionMaskState&) noexcept; //4332AD41
// Complete433EDCA9. Inactive frames and airborne eligible targets preserve
// prior bits. Index64 preserves its original overlapping/tail publications.
void update_prediction_masks(PredictionMaskState&, std::uint8_t update,
                             const PredictionMaskBindings&);
void update_prediction_masks(PredictionMaskState&, std::uint8_t, const LivePredictionMaskBindings&);
}

// Declarations from record_candidates.hpp

namespace skeet::client {
struct CommandBudgetBindings;
}

namespace skeet::rage {
struct RecordCandidateBindings {
    const void* lag_compensation; // decoded ConVar43467A78 XOR3AE06513
    void* engine_client; // actual engine slots12(max clients),14(time)
    float current_time; // pointed global43467EB8
    float interval, receive_interval;
    float interpolation; // global43476928
    std::int32_t maximum_unlag_ms; // global43468230
    std::optional<std::int32_t> prediction_limit; // explicit decoded fixture input only
    const client::CommandBudgetBindings* live_budget = nullptr;
};

// Complete433AEBA8. Appends simulation ticks to the existing output, retaining
// its prefix and the native priority/cap rules. The canonical player cache is
// supplied by the acquisition owner; the network index still comes from the
// actual target virtual. Its nonzero index resolves the supplied PlayerState.
// Rings contain at most32 nonnull records.
void append_target_record_ticks(const client::CommandFrame&, void* target, client::PlayerState&,
                                std::uint32_t maximum, const RecordCandidateBindings&,
                                std::vector<std::int32_t>& output);
std::vector<std::int32_t> collect_target_record_ticks(const client::CommandFrame&, void* target,
                                                      client::PlayerState&, std::uint32_t maximum,
                                                      const RecordCandidateBindings&);
} // namespace skeet::rage

// Declarations from reset.hpp

namespace skeet::esp {
struct HitboxHighlightState;
}
namespace skeet::rage {
struct PreambleState;
struct AcquisitionStorage;
struct CommandOwner;

//4340BCDA receives the complete configuration owner, eight bytes before its
// gameplay interface. References retain the already shared source owners.
struct RageResetBindings {
    PreambleState& state;
    AcquisitionStorage& acquisition;
    std::int32_t& shot_index; //complete+170, gameplay+168
    std::int32_t& field_174; //complete+174; constructor/reset sentinel-1
    esp::HitboxHighlightState& highlight; //target_handle complete+180
    std::array<animation::AnimationAngleSample, 2>& pending_angles; //434759DC/E8
};
// Complete4340BCDA, also reached through primary slot7's433CF2F5->slot5.
// Logical ends/counters are reset; frame, spread, evaluation and work buffers
// retain their incoming bytes. The current command flags remain untouched.
void reset_rage_state(const RageResetBindings&) noexcept;
void reset_rage_command_owner(CommandOwner&, std::int32_t& field_174) noexcept;
}

// Declarations from runtime_owner.hpp

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::effects {
class EffectsRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
class CrtRuntimeOwner;
class MovementHudRuntimeOwner;
class TargetRefreshRuntimeOwner;
class VisualRuntimeOwner;
class NativeStartupSources;
}
namespace skeet::rage {
struct ShotDiagnosticEntry;
struct BufferedShotRepairScratch;
struct RuntimeImports {
    void* thread_pool;
};
// Layout for native per-invocation automatic storage. The explicit-input
// constructor is a differential-fixture seam, not a bootstrap dependency.
struct CommandFrameInputs {
    lagcomp::RecordInterpolationScratch record_interpolation;
    lagcomp::HistoryMatrixScratch history_interpolation;
    std::array<std::byte, animation::copied_animation_state_size> alternative_animation;
    std::array<Matrix3x4, 256> attachment_matrices;
};
// All four views borrow invocation-local bindings. They are valid only while
// the synchronous with_frame consumer is executing and cannot be retained.
struct RuntimeFrameBindings {
    const TargetBindings& targeting;
    const RecordCandidateBindings& candidates;
    const lagcomp::RecordSelectionContext& records;
    const lagcomp::ExtrapolatedRecordBindings& extrapolation;
};
// The complete4348DA25 source factory and four RAGE command phases. Settings,
// acquisition and firing publications share their existing canonical owners.
// Construction registers nothing, calls no game method and retains no frame,
// command, target or local-player snapshot.
class RuntimeOwner final {
public:
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&, effects::EffectsRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&, CommandFrameInputs&);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&, effects::EffectsRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&);
    // The same source contexts with an explicit external thread-pool import.
    // Production still owns each phase's automatic workspace at invocation.
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&, effects::EffectsRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&, RuntimeImports);
    ~RuntimeOwner();
    RuntimeOwner(const RuntimeOwner&) = delete;
    RuntimeOwner& operator=(const RuntimeOwner&) = delete;
    RuntimeOwner(RuntimeOwner&&) = delete;
    RuntimeOwner& operator=(RuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    PreambleState& state() noexcept;
    AcquisitionStorage& acquisition() noexcept;
    CommandEndpoint& commands() noexcept;
    OwnerBindings& factory_bindings() noexcept;
    void with_frame(client::CommandFrame&, void* current_player, client::Command&,
                    const std::function<void(const RuntimeFrameBindings&)>&);
    const lagcomp::EvaluationState* repair_buffered(client::CommandFrame&, void* current_player,
                                                    client::Command&, const ShotDiagnosticEntry&,
                                                    BufferedShotRepairScratch&);
    void bind_initializer(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::rage

// Declarations from scan_hypotheses.hpp

namespace skeet::rage {
// Native groups have an untouched 16-byte prefix, followed by these three
// hitbox sets. The semantic view excludes those unconsumed prefix bytes.
using ScanHypothesisMatrices = std::array<std::array<HitboxMatrices, 3>, 3>;
struct ScanHypothesisOffsets {
    std::ptrdiff_t field_6e9852d6;
    std::ptrdiff_t field_2a671b42;
    std::ptrdiff_t field_4e66bb34;
    std::ptrdiff_t field_83a6cf2b;
};
struct ScanHypothesisContext {
    client::PlayerStateCache& players;
    const lagcomp::EntityPreparationContext& preparation;
    ScanHypothesisOffsets offsets;
    const animation::BoneModel& model;
    legit::AttachmentUpdate update_attachments;
};

// Complete433DFFC7/4336B8FE. Packet yaw is the raw float at each sample+1C;
// the six-entry packet ring lives in the same physical cached player owner.
float mean_scan_history_yaw_change(const client::NativePlayerStateLayout&) noexcept;

// Complete433B4728 transaction. All three sets are refreshed by the concrete
// 4338AC35 helper, with the original entity/netvar restoration order. Scratch
// is private to this invocation and retains caller bytes between all nine
// attachment calls; non-hitbox bone slots are intentionally not initialized.
// Target has the original caller's live, nonzero networkable index.
void refresh_scan_hypothesis_sets(const ScanHypothesisContext&, void* target,
                                  const lagcomp::LagRecord& selected, std::span<HitboxMatrices, 3>,
                                  std::span<Matrix3x4, 256> attachment_scratch);

// Complete4340313B. The selected record's first native dword is a source-record
// address; the typed source argument supplies that already-resolved original.
// Its three alternatives seed every group, rather than the selected copy's.
void build_scan_hypotheses(const ScanHypothesisContext&, void* target,
                           const lagcomp::LagRecord& selected, const lagcomp::LagRecord& source,
                           ScanHypothesisMatrices&, std::span<Matrix3x4, 256> attachment_scratch);

// Scanner4335D61A..4335D68B gates, followed by actual cache/history/build code.
// entity_index is evaluation+4, already available before this optional path.
bool build_optional_scan_hypotheses(const ScanHypothesisContext&, void* target,
                                    std::uint32_t entity_index, const lagcomp::LagRecord& selected,
                                    const lagcomp::LagRecord* source, bool enabled,
                                    std::uint32_t safe_hitboxes, ScanHypothesisMatrices&,
                                    std::span<Matrix3x4, 256> attachment_scratch);
} // namespace skeet::rage

// Declarations from scanning.hpp

namespace skeet::anti_aim {
class TargetTraceEntities;
}

namespace skeet::rage {

// Per-record settings copied by433543BF into evaluation+BC..E3. Fields unused
// by the scanner stay with the scheduler's canonical evaluation owner.
struct ScanPolicy {
    RageHitboxMasks hitboxes; // C0/C4
    float point_scale; // C8
    std::uint32_t multipoint_mask; // CC
    std::int32_t minimum_damage; // D0
    float minimum_chance; // D4
    std::uint32_t body_disabler_reasons; // B8
    bool prefer_safe, prefer_body; // DC/DD
    bool additional_hypotheses; // E1
};

// Views into the scheduler's existing target/record owners, not another copy
// of the native evaluation allocation. E8 counts concrete ballistic evaluations.
struct ScanEvaluation {
    TargetCandidate& candidate;
    lagcomp::LagRecord& record;
    const lagcomp::LagRecord* source_record;
    ScanPolicy policy;
    std::uint32_t& ballistic_evaluations;
};
struct ScanModelOffsets {
    std::ptrdiff_t studio_header; //32E4DAF1
    std::ptrdiff_t hitbox_set; //C2B6FB10
    std::ptrdiff_t bone_cache; //17978090
    std::ptrdiff_t network_origin; //A9EE6062
};
struct ScanContext {
    BallisticEngine& ballistics;
    const TargetBindings& targeting;
    BallisticEntity source; //actual native43475808 entity address
    void* weapon;
    std::ptrdiff_t weapon_item_offset;
    const std::uint8_t& allow_penetration; //live BooleanConfig.value4346A3E0, read per query
    std::int32_t weapon_id; //aim owner+D8, distinct from weapon's netvar
    RageVector origin; //aim owner+DC
    float spread; //aim owner+160
    std::int32_t record_count; //aim owner+19C
    std::int32_t team; //aim owner+1D0
    std::int32_t accuracy_percent; //configuration+2C
    std::uint32_t& configuration_flags; //configuration+178
    std::span<std::uint32_t, 2> target_mask; //configuration+1AC
    const std::array<RageSpreadSample, 32>& samples;
    ScanModelOffsets model_offsets;
    RageTrigonometry trigonometry;
    RageBoxClipper box_clipper;
    client::PlayerStateCache& players;
    const lagcomp::EntityPreparationContext& preparation;
    ScanHypothesisOffsets hypothesis_offsets;
    legit::AttachmentUpdate update_attachments;
    FinalTeammateConvars teammate_convars;
    // Genuine external43475B90 table and actual entity virtual/netvar reads.
    // The43470404 rejection predicate is recovered source, per-call storage.
    void* const* trace_filter_vtable;
    const anti_aim::TargetTraceEntities& trace_entities;
};
struct ScanRequest {
    void* target_player;
    BallisticEntity target_entity; //actual native target address, not its index
    ScanEvaluation evaluation;
    std::uint32_t descriptor_flags;
    std::span<std::uint32_t, 3> completed;
    std::uint32_t ordinal;
};

// Native4335CD37..43364782. The context owns concrete engine/model bindings;
// generation, geometry, ballistics, refinement and admission remain source.
void scan_candidate(ScanContext&, ScanRequest&);

} // namespace skeet::rage

// Declarations from scanning_chance.hpp

namespace skeet::rage {
struct ScanSafeHitchanceInput {
    RageVector origin;
    RageSpreadBasis basis;
    float range;
    std::int32_t hitgroup;
    float minimum_hitgroup_multiplier;
    RageResolverHitboxes hypotheses;
};
// Native4335F8FA..4335FEB9 safe branch. Each sample has one normalization;
// its ray then spans the weapon range and must qualify in all three records.
RageHitchance evaluate_scan_safe_hitchance(const ScanSafeHitchanceInput&,
                                           std::span<const RageSpreadSample, 32>,
                                           const RageBoxClipper&);
}

// Declarations from scanning_geometry.hpp

namespace skeet::rage {
struct ScanIntersection {
    std::uint32_t hitbox = 0;
    float fraction = 1.0f;
    bool found = false;
};
// Exact scanner4335F16C..4335F269 nearest-hitbox traversal. Head candidates
// keep searching rank<=2; the first closer non-head hit terminates traversal.
ScanIntersection intersect_scan_hitboxes(const RageRay&, std::span<const RageScanHitbox>,
                                         const RageBoxClipper&);
}

// Declarations from settings.hpp

namespace skeet::rage {
// Static descriptors retain their stored boolean values. The native startup
// routines write only the integer/key defaults identified below.
struct GlobalSettings {
    BooleanConfig enabled{{{nullptr, nullptr, 0x23060EC5u}, 1, ConfigType::boolean, 0},
                          0}; //0x43467214
    HotkeyConfig activation{{{nullptr, nullptr, 0x861E7F9Au}, 8, ConfigType::hotkey, 0},
                            {}}; //0x43467F24;6,hold
    IntegerConfig target_hitboxes{{{nullptr, nullptr, 0x5B900854u}, 4, ConfigType::integer, 0},
                                  0}; //0x434691BC;2
    HotkeyConfig multipoint_key{{{nullptr, nullptr, 0x29E2C8EDu}, 8, ConfigType::hotkey, 0},
                                {}}; //0x43469400;Escape,always
    BooleanConfig prefer_safe_point{{{nullptr, nullptr, 0xF5A00B3Bu}, 1, ConfigType::boolean, 0},
                                    0}; //0x43468F68
    HotkeyConfig force_safe_point{{{nullptr, nullptr, 0x333C1DBEu}, 8, ConfigType::hotkey, 0},
                                  {}}; //0x434678D0;Escape,hold
    IntegerConfig avoid_unsafe_hitboxes{
        {{nullptr, nullptr, 0xECBBD134u}, 4, ConfigType::integer, 0}, 0}; //0x43467020;0
    BooleanConfig prefer_body_aim{{{nullptr, nullptr, 0x2B54548Du}, 1, ConfigType::boolean, 0},
                                  0}; //0x43467D60
    IntegerConfig body_aim_disablers{{{nullptr, nullptr, 0x16C91A85u}, 4, ConfigType::integer, 0},
                                     0}; //0x4346A8C0;preserved
    HotkeyConfig force_body_aim{{{nullptr, nullptr, 0x02F59674u}, 8, ConfigType::hotkey, 0},
                                {}}; //0x4346A2F0;Escape,hold
    BooleanConfig quick_stop{{{nullptr, nullptr, 0xC06FD8CCu}, 1, ConfigType::boolean, 0},
                             0}; //0x434671D0
    HotkeyConfig quick_stop_key{{{nullptr, nullptr, 0x1AC79342u}, 8, ConfigType::hotkey, 0},
                                {}}; //0x43469308;Escape,always
    IntegerConfig quick_stop_options{{{nullptr, nullptr, 0x11195AD9u}, 4, ConfigType::integer, 0},
                                     0}; //0x43467E30;0
    IntegerConfig double_tap_hit_chance{
        {{nullptr, nullptr, 0x3F69C17Cu}, 4, ConfigType::integer, 0}, 0}; //0x4346A4A0;50
    IntegerConfig double_tap_quick_stop{
        {{nullptr, nullptr, 0x9FBDD0E6u}, 4, ConfigType::integer, 0},
        0}; // 0x4346A664; stored value survives
    BooleanConfig automatic_scope{{{nullptr, nullptr, 0x9E2B2E76u}, 1, ConfigType::boolean, 0},
                                  0}; //0x434693C4
    BooleanConfig anti_aim_correction{{{nullptr, nullptr, 0xA4D31376u}, 1, ConfigType::boolean, 0},
                                      0}; // 0x4346753C
    BooleanConfig automatic_fire{{{nullptr, nullptr, 0x59BD212Au}, 1, ConfigType::boolean, 0},
                                 0}; //0x43468400
    BooleanConfig automatic_penetration{
        {{nullptr, nullptr, 0x64715C40u}, 1, ConfigType::boolean, 0}, 0}; //0x4346A3D0
    BooleanConfig silent_aim{{{nullptr, nullptr, 0x86BFE317u}, 1, ConfigType::boolean, 0},
                             0}; //0x434684D0
    BooleanConfig remove_recoil{{{nullptr, nullptr, 0x63660060u}, 1, ConfigType::boolean, 0},
                                0}; //0x43467134
    HotkeyConfig duck_peek_assist{{{nullptr, nullptr, 0xD23ED2A7u}, 8, ConfigType::hotkey, 0},
                                  {}}; //0x43467C7C;Escape,hold
    BooleanConfig reduce_aim_step{{{nullptr, nullptr, 0x4034958Cu}, 1, ConfigType::boolean, 0},
                                  0}; // 0x434670B0
    IntegerConfig aim_step_limit{{{nullptr, nullptr, 0x20E889DEu}, 4, ConfigType::integer, 0},
                                 0}; // 0x4346708C; CRT default 15
    BooleanConfig pattern_recoil_correction{
        {{nullptr, nullptr, 0xA434F7C7u}, 1, ConfigType::boolean, 0}, 0}; //0x43467CA4
};

// Configuration subset of owner 0x434803BC. Its visualization descriptors
// at +10C..+147 already belong to esp::AimbotVisualizationConfig.
struct AimbotSettings {
    IntegerConfig minimum_hit_chance; // +01C; 50
    IntegerConfig maximum_fov; // +030; 180
    IntegerConfig target_selection; // +044; 0
    IntegerConfig minimum_damage; // +058; 10
    BooleanConfig damage_override; // +06C
    HotkeyConfig damage_override_key; // +080; Escape, hold
    IntegerConfig override_damage; // +0A8; 10
    IntegerConfig multipoint_scale; // +0BC; 90
    IntegerConfig multipoint; // +0D0; 0
    IntegerConfig accuracy_boost; // +0E4; 1
    BooleanConfig delay_shot; // +0F8
    BooleanConfig force_body_aim_on_peek; // +15C
};

// One native owner (0x4347DEEC) supplies both RAGE exploits and the AA
// on-shot/fake-peek controls. These shared descriptors must be registered once.
struct ExploitSettings {
    BooleanConfig on_shot_anti_aim; // +030
    HotkeyConfig on_shot_key; // +044; Escape, always
    IntegerConfig double_tap_fake_lag_limit; // +06C; 2
    BooleanConfig fake_peek; // +080
    HotkeyConfig fake_peek_key; // +094; X, hold
    BooleanConfig double_tap; // +0BC
    HotkeyConfig double_tap_key; // +0D0; Escape, hold
    IntegerConfig double_tap_mode; // +0F8; 0
};
struct ShotLogSettings {
    BooleanConfig spread_misses; // owner 0x4347F708, +038; true
};
struct PerformanceSettings {
    IntegerConfig mitigations; // owner 0x4347E6F2, +014; stored value survives
    BooleanConfig warning; // +028; shared MISC / Settings descriptor 0x740C32AE
};

void initialize_global_settings(ConfigRegistries&, GlobalSettings&) noexcept;
void initialize_aimbot_settings(ConfigRegistries&, AimbotSettings&) noexcept;
void initialize_exploit_settings(ConfigRegistries&, ExploitSettings&) noexcept;
void initialize_shot_log_settings(ConfigRegistries&, ShotLogSettings&) noexcept;
void initialize_performance_settings(ConfigRegistries&, PerformanceSettings&) noexcept;

// The aggregate uses an explicit owner order; it does not reproduce process
// startup. Owner constructors preserve their local registration order/writes.
// GlobalSettings gathers independent startup routines in declaration order.
// Quick peek reuses the recovered movement-owner initializer.
struct Settings {
    GlobalSettings global;
    AimbotSettings aimbot;
    ExploitSettings exploits;
    ShotLogSettings shot_log;
    PerformanceSettings performance;
    QuickPeekConfig quick_peek;
};
void initialize_settings(ConfigRegistries&, Settings&) noexcept;
} // namespace skeet::rage

// Declarations from single_target.hpp

namespace skeet::rage {
struct SingleTargetSelection {
    const TargetBindings& targeting;
    std::int32_t maximum_clients;
    RageVector origin, forward;
    float minimum_cosine;
};
// Complete433EC805. Eligible engine entities are scored at collision-max-Z-8;
// equality retains the earlier entity, and the initial cosine is exclusive.
void* select_single_target(const SingleTargetSelection&);

struct TargetPreparationOffsets {
    std::ptrdiff_t pose_parameters; //1CAAF0A8
    std::ptrdiff_t animation_state; //3D52A1A7
    std::ptrdiff_t studio_header; //433FD30D's decoded wrapper field
    std::ptrdiff_t hitbox_set; //C2B6FB10
    std::ptrdiff_t bone_cache; //17978090, pointer to0x30-byte matrices
    std::ptrdiff_t velocity; //36375C23
    std::ptrdiff_t animation_layers; //B538BBA5
};
struct TargetPreparationBindings {
    const TargetBindings& targeting;
    const GlobalSettings& global;
    const AimbotSettings& weapon_settings;
    InputBitmap input;
    PreambleState& state;
    std::int32_t remembered_target_handle; //shared AA target4346DB1C
    const void* accuracy_convar; //decoded43468EA4
    TargetPreparationOffsets offsets;
};
// Shared native multipoint-scale calculation. The action consumer uses this
// stage directly after its own settings/weapon getter sequence.
float acquisition_point_scale(const client::CommandFrame&, void* target,
                              const AcquisitionTargetSettings&, const TargetPreparationBindings&);
// Complete433E63FD, including newest-three traversal, the0x200 stop marker,
// skipped bit8 records, wrapped yaw difference and strict60-degree threshold.
bool target_history_yaw_changed(void* target, const client::PlayerState&,
                                std::ptrdiff_t animation_state_offset) noexcept;
// Shared503EE..50E12 /51E2D..528CC. Mutates the actual ten-word settings and
// player counter, using engine getters for weapon factors, health and origin.
void prepare_acquisition_target_settings(const client::CommandFrame&, void* target,
                                         client::PlayerState&, std::uint32_t entity_index,
                                         const players::Adjustments&, AcquisitionTargetSettings&,
                                         const TargetPreparationBindings&);
struct AcquisitionTargetTicks {
    std::vector<std::int32_t> values;
    std::uint32_t descriptor_flags = 0;
};
// Shared5154B..51824 /54011..5430E. A nonempty priority list receives flag4
// and descending native qsort order. Fallbacks retain flag0 and may set DD.
// Accuracy mode is the native valid0..4 configuration index.
AcquisitionTargetTicks acquisition_target_ticks(const client::CommandFrame&, void* target,
                                                client::PlayerState&, AcquisitionTargetSettings&,
                                                const TargetPreparationBindings&,
                                                const RecordCandidateBindings&);
struct TargetPredictionBindings {
    const BodyAimProbeContext& trace;
    const lagcomp::RecordSelectionContext& record_selection;
    void* engine_trace; //actual43468304, TraceRay slot5
    std::ptrdiff_t local_maximum_speed; //3EB71843
    std::uint32_t scheduler_flags; //434759C0
    // Post-firing56672 branch reads the active command. The earlier shared
    // acquisition copies retain their separate frame+24/+28 input source.
    const client::Command* command = nullptr;
};
// Shared50E1E..51547 /538BF..54011. Copies the newest complete record,
// refreshes its actual attachments, traces present/future damage and raises D0
// only when the projected shot-count improves under the original exploit gates.
void refine_acquisition_minimum_damage(const client::CommandFrame&, void* target,
                                       client::PlayerState&, AcquisitionTargetSettings&,
                                       const TargetPredictionBindings&);
// Production overload:43327659 refreshes live studio/attachment state through
// the required raw runtime. The five-argument fixture projection stays separate.
void refine_acquisition_minimum_damage(const client::CommandFrame&, void* target,
                                       client::PlayerState&, AcquisitionTargetSettings&,
                                       const TargetPredictionBindings&,
                                       const animation::BoneRuntime&);
struct SingleTargetBindings {
    const TargetPreparationBindings& preparation;
    const TargetPredictionBindings& prediction;
    const RecordCandidateBindings& record_candidates;
    const AcquisitionRecordBindings& records;
    const players::State& adjustments;
    std::int32_t maximum_clients;
    engine::JobDispatcher& jobs;
};
// Complete single-target acquisition5034F..51C32 and shared5473E..547C9.
// False is the native empty-selection jump54908, which bypasses the job drain
// and per-player cleanup. Successful selection uses canonical scanner storage.
bool acquire_single_target(PreambleState&, PreambleResult&, AcquisitionStorage&,
                           const SingleTargetBindings&);
} // namespace skeet::rage

// Declarations from target_relationships.hpp

namespace skeet::rage {
// Process relationships are sampled at the actual enemy-helper invocation,
// after engine/event callbacks have had the opportunity to replace the cells.
struct LiveTargetRelationships {
    void* const& entity_list;
    engine::NativeConVarReference game_type;
    engine::NativeConVarReference teammates_are_enemies;
    const players::State& adjustments;
    const TargetOffsets& offsets;
    TargetBindings capture(void* local_player) const noexcept {
        return {local_player, entity_list, game_type, teammates_are_enemies, adjustments, offsets};
    }
};

// The explicit standalone view keeps existing pure-function fixtures useful;
// production borrows LiveTargetRelationships over canonical process cells.
class TargetRelationshipBindings {
public:
    TargetRelationshipBindings(const TargetBindings& standalone) noexcept
        : standalone_(&standalone) {}
    TargetRelationshipBindings(const LiveTargetRelationships& live) noexcept : live_(&live) {}
    TargetRelationshipBindings(TargetBindings&&) = delete;
    TargetRelationshipBindings(LiveTargetRelationships&&) = delete;
    TargetBindings capture(void* local_player) const noexcept {
        if (live_)
            return live_->capture(local_player);
        auto result = *standalone_;
        result.local_player = local_player;
        return result;
    }

private:
    const TargetBindings* standalone_ = nullptr;
    const LiveTargetRelationships* live_ = nullptr;
};
} //namespace skeet::rage

// Declarations from targeting.hpp

namespace skeet::rage {
// The explicit decoded object profile is retained for standalone semantic
// inputs. Production supplies a live canonical reference, including its XOR
// representation; copying a TargetBindings keeps that same reference live.
using TargetConVarBinding = engine::ConVarBinding;

struct TargetOffsets {
    // Decoded values of 43468E78, 4346A5DC, 43468BF8 and 43467DD0.
    // Their byte tests are recovered; no unproved netvar names are assigned.
    std::array<std::ptrdiff_t, 4> rejection_bytes;
    std::ptrdiff_t team;
    std::ptrdiff_t survival_team;
    std::ptrdiff_t movement_type;
    std::ptrdiff_t movement_flags;
    std::ptrdiff_t origin;
    std::ptrdiff_t duck_amount;
    std::ptrdiff_t active_weapon;
    std::ptrdiff_t weapon_identifier;
};
struct TargetBindings {
    void* local_player;
    void* entity_list;
    // Native ConVar objects. Parent pointer +0x1C, encoded integer +0x30.
    TargetConVarBinding game_type;
    TargetConVarBinding teammates_are_enemies;
    // Original 0x20-byte per-player table, shared with PLAYERS/plist.
    const players::State& player_adjustments;
    TargetOffsets offsets;
};

// 4339EF8C: complete team/survival-team predicate, including ConVar parents.
bool target_is_enemy(const TargetBindings&, const void* target) noexcept;
// 43358962: calls entity slots 158/122, networkable slots 9/10, then the
// recovered enemy predicate. Only actual engine virtuals remain dependencies.
bool target_is_eligible(const TargetBindings&, void* target);
// 433849B5: complete smoothstep duck interpolation of native eye height.
struct TargetEyePositionOffsets {
    std::ptrdiff_t origin;
    std::ptrdiff_t duck_amount;
};
RageVector target_eye_position(TargetEyePositionOffsets, const void* target) noexcept;
RageVector target_eye_position(const TargetBindings&, const void* target) noexcept;
struct TargetView {
    RageVector origin;
    RageVector forward;
    std::int32_t field_of_view;
    float minimum_cosine;
};
// 43351D0E..43351E2D, after the eligibility predicate succeeds. The native
// field-of-view value 180 skips the eye-position and cosine calculation.
bool target_passes_scan_filter(const TargetBindings&, const void* target,
                               const TargetView&) noexcept;

struct TargetPoint {
    RageVector position;
    RageVector reference_position;
    std::int32_t reference_damage;
    std::int32_t damage;
    std::int32_t hitbox; // native point+20 / candidate+2C, studio hitbox index
    std::int32_t hitgroup;
    std::uint32_t flags;
    float chance;
    float descriptor_weight;
    std::uint32_t priority;
};
static_assert(sizeof(TargetPoint) == 0x38);
struct TargetCandidate {
    std::uint32_t flags;
    std::uint32_t entity_index;
    std::int32_t health;
    TargetPoint primary;
    TargetPoint alternate_first;
    TargetPoint alternate_second;
    std::uint32_t priority;
    std::uint32_t target_state_flags;
    bool prefer_body;
    bool prefer_safe;
    bool weapon_priority_enabled;
};
struct TargetBatch {
    std::array<TargetCandidate*, 16> candidates{};
    std::size_t count = 0;
};
// 4336070E..43360869, copying all 14 dwords of each selected point into the
// candidate. Unassigned primary/alternate slots retain their previous bytes.
void select_target_points(TargetCandidate&, std::span<const TargetPoint>,
                          std::uint32_t priority_mask);
struct TargetRecordPolicy {
    RageBodyAimPolicy body;
    std::uint32_t evaluation_flags;
    bool delay_shot;
};
// 43351A28..43351B6D. False stops the remaining record traversal on an
// alternative-record result after ordinal zero, before policy mutation.
bool prepare_target_record_policy(TargetRecordPolicy&, std::uint32_t record_flags,
                                  std::uint32_t ordinal, std::uint8_t body_disablers) noexcept;
// 433547C9..43354908. Traverses exactly 80 completion bits, retains the
// highest scan priority per (entity_index & 63), and admits at most 16 groups.
// Equal priority preserves the earlier candidate. Existing groups can still
// be replaced after all 16 slots have been occupied.
TargetBatch collect_target_candidates(std::span<TargetCandidate, 80>,
                                      const std::array<std::uint32_t, 3>& completed,
                                      std::uint32_t context_flags,
                                      std::array<std::uint32_t, 2>& target_mask) noexcept;
// 43354925..43354AA0. Recalculates priorities after grouping. Weapon lookup
// is performed through entity-list slots 3 and 4 only when its gate is set.
void recalculate_target_priorities(TargetBatch&, const TargetBindings&);
// Native 433FF5C7 comparator passed to the host CRT qsort, as in 43354ABE.
void sort_target_candidates(TargetBatch&);
// Concrete post-scan chain 433547C9..43354AD8: group by target, refresh the
// surviving record priorities using engine weapon state, and sort them.
TargetBatch rank_completed_target_candidates(std::span<TargetCandidate, 80>,
                                             const std::array<std::uint32_t, 3>& completed,
                                             std::uint32_t context_flags,
                                             std::array<std::uint32_t, 2>& target_mask,
                                             const TargetBindings&);

struct TargetRecordHistory {
    RageVector origin;
    std::uint32_t first;
    std::uint32_t count;
    std::array<const lagcomp::LagRecordHeader*, 32> records;
};
struct TargetRecordChoice {
    const lagcomp::LagRecordHeader* record = nullptr;
    std::uint32_t ordinal = 0;
};
// 4336C59D..4336C64E, the opening traversal of history_select_lag_record.
// Compares consecutive origins, rejecting differences above 4096 squared,
// then chooses the first simulation tick no newer than the requested tick.
// A request older than the ring selects its last record. Record interpolation,
// resolver mutation, extrapolation and finalization follow this stage.
TargetRecordChoice choose_target_record(const TargetRecordHistory&, std::int32_t requested_tick,
                                        float interval) noexcept;
struct TargetLagRecordHistory {
    RageVector origin;
    std::uint32_t first;
    std::uint32_t count;
    std::array<lagcomp::LagRecord*, 32> records;
};
struct TargetRecordCopy {
    lagcomp::LagRecord* source = nullptr;
    std::uint32_t ordinal = 0;
};
// 4336C572..4336C692: complete opening selection/copy stage. Clears the
// output flags and destination source dword before traversal, advances a
// pending resolver attempt on the chosen source, then copies all 0x10A0 bytes.
// The destination's first dword becomes the native 32-bit source address.
TargetRecordCopy prepare_target_record_copy(TargetLagRecordHistory&, std::int32_t requested_tick,
                                            float interval, std::uint32_t options,
                                            lagcomp::LagRecord& destination,
                                            std::uint32_t& output_flags) noexcept;

} // namespace skeet::rage

// Declarations from targeting_acquired.hpp

namespace skeet::rage {
struct FinalTargetContextBindings;
// The sorted pointer list refers to the canonical contiguous candidate array.
// Record/settings metadata retains that original slot, rather than sort order.
const AcquisitionEvaluation& acquisition_evaluation_for(const AcquisitionStorage&,
                                                        const TargetCandidate&) noexcept;
// Native43354AD8..43354C61 using the complete engine-input consumer on each
// attempted point. Reads for unattempted candidates are never performed.
TargetShotChoice select_acquired_target_for_shot(TargetBatch&, AcquisitionStorage&,
                                                 client::CommandFrame&, PreambleState&,
                                                 const FinalTargetContextBindings&);
} // namespace skeet::rage

// Declarations from targeting_commit.hpp
namespace skeet::engine {
class JobDispatcher;
}
namespace skeet::ui {
struct SettingsMenuValues;
}

namespace skeet::rage {
struct FinalTargetContextBindings;
namespace final_target_detail {
struct Producer;
}
// Complete direction-trace owner43364CEC. Unlike the point-to-point owner,
// an unobstructed trace is a miss. The supplied direction is not normalized.
std::int32_t trace_target_direction(BallisticEngine&, BallisticEntity source,
                                    const RageVector& origin, const RageVector& direction,
                                    const BallisticFilter&, BallisticQuery&);
// 433EB28B/433C8C4D: genuine weapon slots461(data),434(armor ratio),431(damage)
// and the decoded item-definition-index netvar. Query+24 carries pellet count.
BallisticQuery prepare_target_bullet(void* weapon, std::ptrdiff_t item_offset,
                                     std::uint8_t allow_penetration);
struct FinalHitchancePlan {
    std::uint32_t samples;
    std::uint32_t sample_stride;
    std::int32_t failure_budget;
};
RageSpreadBasis final_target_spread_basis(const RageVector& point,
                                          const RageVector& origin) noexcept;
// 4341BBC3..4341BC46; the required-vote conversion uses current MXCSR rounding.
FinalHitchancePlan
plan_final_hitchance(bool safe, float minimum_chance, std::uint32_t context_flags,
                     std::uint32_t entity_index,
                     const std::array<std::uint32_t, 2>& previous_targets) noexcept;
struct FinalHitchanceVotes {
    std::atomic<std::uint32_t> packed{0};
    std::atomic<std::uint32_t> stopped{0};
};
struct FinalHitchanceSample {
    void* weapon;
    std::ptrdiff_t weapon_item_offset;
    const std::uint8_t& allow_penetration; //live4346A3E0, including each dispatched sample
    BallisticEntity source;
    BallisticEntity target;
    RageVector origin;
    RageVector point;
    RageVector point_delta;
    RageSpreadBasis basis;
    std::int32_t minimum_damage;
    std::int32_t hitgroup;
    float minimum_hitgroup_multiplier;
    float base_damage;
    bool safe;
    RageResolverHitboxes hypotheses;
    RageBoxClipper box_clipper;
    BallisticFilter direction_filter;
    BallisticFilter projected_filter;
    // The full command owner supplies concrete engine bindings. A worker then
    // constructs its own native filter; the prebound API supplies filter views.
    const FinalTargetContextBindings* live_bindings = nullptr;
};
// Complete433DF8A0 sample worker. Every evaluated sample constructs its weapon
// query through the genuine virtual getters, then runs concrete ballistics.
// Safe samples contribute zero to three votes; regular samples zero or one.
void evaluate_final_hitchance_sample(BallisticEngine&, const FinalHitchanceSample&,
                                     RageSpreadSample, std::int32_t failure_budget,
                                     FinalHitchanceVotes&);
// 4341C7ED..4341C852: validates the shared counters and publishes the fraction.
// The boundary budget comparison is >=, while worker cancellation uses >.
RageHitchance final_hitchance_fraction(const FinalHitchanceVotes&,
                                       std::int32_t failure_budget) noexcept;
RageHitchance evaluate_final_hitchance_inline(BallisticEngine&, const FinalHitchanceSample&,
                                              std::span<const RageSpreadSample, 128>,
                                              const FinalHitchancePlan&);
RageHitchance evaluate_final_hitchance_parallel(BallisticEngine&, const FinalHitchanceSample&,
                                                std::span<const RageSpreadSample, 128>,
                                                const FinalHitchancePlan&, engine::JobDispatcher&);
struct FinalHitchanceFailure {
    std::uint32_t context_flags;
    std::uint32_t weapon_flags;
    void* weapon;
    std::int32_t hitgroup;
    std::array<float, 2> velocity;
    float baseline;
    float current;
    std::int32_t point_damage;
    std::int32_t minimum_damage;
    std::int32_t* target_aim_mode;
};
// Complete failed-consumer tail4341C858..4341C9C3, including the real weapon
// maximum-speed getter at slot442 and conditional target-history mode3 write.
void apply_final_hitchance_failure(FinalHitchanceFailure&);
struct FinalHitchanceContext {
    BallisticEngine& ballistics;
    const lagcomp::EntityStateRuntime& entity_runtime;
    void* target_player;
    BallisticEntity source;
    BallisticEntity target;
    void* weapon;
    std::ptrdiff_t weapon_item_offset;
    const std::uint8_t& allow_penetration; //live4346A3E0; outlives the completed sample jobs
    std::uint32_t weapon_flags;
    std::uint32_t& context_flags;
    std::array<std::uint32_t, 2> previous_targets;
    RageVector origin;
    std::array<float, 2> velocity;
    float baseline;
    float current;
    float spread;
    float inaccuracy;
    bool check_teammates;
    bool teammates_are_enemies;
    std::int32_t team;
    std::int32_t* target_aim_mode;
    std::span<const RageLocalHitbox> local_hitboxes;
    std::span<const std::uint32_t> hitbox_bones;
    const std::uint32_t* current_bone_counter;
    std::span<Matrix3x4> saved_bones;
    RageTrigonometry trigonometry;
    RageBoxClipper box_clipper;
    BallisticFilter direction_filter;
    BallisticFilter projected_filter;
    BallisticFilter teammate_filter;
    std::span<const RageSpreadSample, 128> samples;
    // Read at the native inline/threadpool branch, after model/record work.
    // The menu object aliases the actual byte; contexts never cache its value.
    const ui::SettingsMenuValues& settings_menu;
    // Runtime-owned dispatcher; it must outlive every submitted task.
    engine::JobDispatcher* jobs;
};
// 4341B4C2 consumer after the actual entity/studio-model engine reads have
// populated the typed inputs. Covers teammate rays, hitbox construction, record
// transaction, weighted jobs, publication and failure-state updates. A missing
// target preserves the caller's chance, as in the native entry.
bool evaluate_target_hitchance(FinalHitchanceContext&, const TargetCandidate&,
                               const lagcomp::LagRecord&, std::int32_t minimum_damage,
                               float minimum_chance, float& chance);
// Concrete4341B4C2 stage overload. The producer performs studio and player-cache
// reads at their native consumer sites, after the earlier engine trace stages.
bool evaluate_target_hitchance(FinalHitchanceContext&, const TargetCandidate&,
                               const lagcomp::LagRecord&, std::int32_t minimum_damage,
                               float minimum_chance, float& chance, final_target_detail::Producer&);
struct FinalTargetEvaluation {
    FinalHitchanceContext* context;
    const lagcomp::LagRecord* record;
    std::int32_t minimum_damage;
    float minimum_chance;
};
struct TargetShotChoice {
    TargetCandidate* candidate = nullptr;
    std::size_t ordinal = no_rage_point;
};
// Complete43354AD8..43354C61 selection loop: real final-hitchance calls,
// original alternate-point promotions, Revolver bypass and final fraction gate.
// evaluations correspond to the already sorted batch, preserving its ordering.
TargetShotChoice select_target_for_shot(TargetBatch&, std::span<const FinalTargetEvaluation>,
                                        std::uint32_t context_flags, std::uint32_t weapon_flags,
                                        std::int32_t weapon_id, float chance_floor,
                                        float point_penalty);
struct TargetAngleMath {
    double (*atan2)(double y, double x);
};
struct FinalTeammateConvars {
    const void* mode_43467c3c;
    const void* coefficient_43467084;
    const void* coefficient_43468c90;
    const void* teammates_are_enemies;
};
// Complete4341B7A5..4341B8B1 policy and terminal-parent ConVar reads.
bool final_teammate_precheck_required(std::uint32_t context_flags,
                                      const FinalTeammateConvars&) noexcept;
// 433551BA..433553BB: angle construction, conditional recoil subtraction and
// complete native angle normalization. compensate_recoil is the concrete
// 433EA238/category-policy +43467144 gate consumed by this block.
MovementAngles target_aim_angles(const RageVector& point, const RageVector& origin,
                                 const RageVector& recoil, bool compensate_recoil,
                                 std::uint32_t& weapon_flags, const TargetAngleMath&);
// 43355DEF..43355EA9 after the enclosing scheduler's firing gates admit the
// selected candidate. Preserves the native attack-button-before-weapon64 order.
RageShotCommit commit_target_choice(const TargetShotChoice&, RageShotState&, RageShotCommitState&,
                                    UserCommand&, float special_gate_value, bool alternate_path,
                                    bool send_mode) noexcept;
}

// Declarations from targeting_pipeline.hpp

namespace skeet::rage {
struct TargetFiringTiming {
    std::int32_t field_b4;
    std::int32_t field_13c;
};
enum class TargetFiringStatus {
    empty_batch, //4335619C; no target bookkeeping
    rejected_batch, //4335617E; set context100 and retain sorted[0]
    retained_target, //4335615E; selected target bookkeeping
    readiness_stopped, //4335615A; selected target bookkeeping
    post_angles_stopped, //43356146; player-cache cleanup then bookkeeping
    attack_button_only, //43356146; Revolver commit gate
    recorded //43355EA9; publish accepted shot
};
struct TargetFiringResult {
    TargetFiringStatus status;
    TargetShotChoice choice;
};
// 43354C61..43355EA9, entered with an admitted selection. Keeping this
// concrete tail separate lets acquisition invoke the full final-consumer
// engine reads lazily for each attempted primary or alternate point.
TargetFiringResult fire_target_choice(TargetShotChoice, const TargetFiringTiming&, FiringState&,
                                      FiringBranchState&, FiringSharedState&, client::Command&,
                                      RageShotState&, RageShotCommitState&,
                                      const FiringReadinessBindings&,
                                      const FiringPostAngleBindings&);
// Post-scan43354AD8..43355EA9 composition. All bindings refer to the same
// current command/weapon snapshot; evaluation context_flags references the
// supplied commit.context_flags. Timing entries follow the sorted batch.
// This is the recovered selection/firing chain, not the enclosing scheduler's
// target scan, command entry, quickstop or later shot-feedback owner.
TargetFiringResult select_and_fire_target(TargetBatch&, std::span<const FinalTargetEvaluation>,
                                          std::span<const TargetFiringTiming>, FiringState&,
                                          FiringBranchState&, FiringSharedState&, client::Command&,
                                          RageShotState&, RageShotCommitState&,
                                          const FiringReadinessBindings&,
                                          const FiringPostAngleBindings&, float chance_floor,
                                          float point_penalty);
}

// Declarations from rage_config.hpp

namespace skeet {
// A live view of the descriptor storage used by the recovered RAGE policy.
// It owns no values; the registry and its descriptors must outlive the view.
class RageRuntimeConfigView {
public:
    explicit RageRuntimeConfigView(ConfigRegistries& registries);
    RageThresholdSettings threshold_settings() const noexcept;
    RageThresholds thresholds(std::uint16_t weapon_id, std::uint32_t configuration_flags,
                              bool override_hotkey_active) const noexcept;
    RageHitboxMasks hitboxes(std::uint32_t configuration_flags,
                             bool force_safe_active) const noexcept;
    void apply_body_disablers(RageBodyAimPolicy& policy) const noexcept;

private:
    const IntegerConfig *minimum_chance_, *minimum_damage_, *override_damage_, *double_tap_chance_;
    const BooleanConfig *override_enabled_, *delay_shot_;
    const IntegerConfig *selected_hitboxes_, *avoid_unsafe_, *body_disablers_;
    const BooleanConfig *prefer_safe_, *prefer_body_;
};
} // namespace skeet

// Declarations from rage_damage.hpp

namespace skeet {

bool hitgroup_armored(std::int32_t armor, std::uint8_t helmet, std::int32_t hitgroup) noexcept;
float hitgroup_damage_scale(std::int32_t armor, std::uint8_t helmet, std::int32_t hitgroup,
                            std::uint16_t weapon_id, float armor_ratio) noexcept;
// 0x433E9ECD: truncate each pellet's scaled damage, then multiply with x86 wrap.
std::int32_t final_bullet_damage(float ballistic_damage, float armor_ratio, std::int32_t pellets,
                                 std::uint16_t weapon_id, std::int32_t hitgroup,
                                 std::int32_t target_armor, std::uint8_t target_helmet) noexcept;

struct PenetrationSurface {
    std::uint16_t material = 0;
    float penetration_modifier = 0;
};
struct SurfacePenetration {
    std::array<float, 3> entry;
    std::array<float, 3> exit;
    PenetrationSurface entry_surface;
    PenetrationSurface exit_surface;
    bool contents_bit_3 = false;
    bool surface_flag_bit_7 = false;
    float weapon_penetration = 0;
};
struct BulletPenetrationState {
    std::array<float, 3> position;
    float damage = 0;
    std::int32_t remaining_penetrations = 0;
};
// Standard post-exit-trace path, 0x4339E320..0x4339E4C9. Trace-to-exit,
// entity access and the earlier trace-distance budget are outside this function.
// False clears damage; only success moves position and consumes penetration.
bool apply_surface_penetration(BulletPenetrationState&, const SurfacePenetration&) noexcept;

// Primary rage normalization 0x433C1D85..0x433C1FBC, also present in the
// Visualize aimbot rescan at 0x43332A2B..0x43332C3B. source_factor/source_limit
// are results of two native virtual getters; their engine meanings are unresolved.
std::int32_t rage_minimum_damage(std::int32_t raw, std::int32_t target_health,
                                 std::uint32_t configuration_flags, float source_factor,
                                 std::int32_t source_limit) noexcept;

} // namespace skeet

// Declarations from rage_geometry.hpp

namespace skeet {
using RageVector = std::array<float, 3>;

struct RageRay {
    RageVector start;
    RageVector delta;
};
struct RageCapsule {
    RageVector first;
    RageVector second;
    float radius;
};
struct CapsuleIntersection {
    float fraction = 1.0f;
    RageVector point{};
    RageVector normal{};
    bool start_solid = false;
};
// 0x43329A78 initialization followed by complete 0x43418772 geometry.
// Retains the native near-stationary capsule arithmetic and rounding order.
CapsuleIntersection intersect_rage_capsule(const RageRay&, const RageCapsule&) noexcept;

struct RageTrace {
    float fraction = 1.0f;
    RageVector point{};
    RageVector normal{};
    bool start_solid = false;
};
// Complete native 0x433F8E09: scales the delta by the current trace fraction,
// applies capsule intersection, and updates only when its fraction is below 1.
int clip_rage_ray_to_capsule(const RageRay&, const RageCapsule&, RageTrace&) noexcept;
} // namespace skeet

// Declarations from rage_hitboxes.hpp

namespace skeet {
struct RageLocalHitbox {
    std::int32_t hitgroup;
    RageVector minimum;
    RageVector maximum;
    RageVector rotation;
    float radius;
    const void* engine_hitbox;
};
struct RageTrigonometry {
    // Imported __libm_sse2_sinf / __libm_sse2_cosf semantics. The bindings are
    // required: choosing a different CRT may change the final matrix by one ULP.
    float (*sine)(float);
    float (*cosine)(float);
};
// Native preparation loop 0x4335D23C..0x4335D5F9, also used for resolver
// hypotheses. Matrices are the per-hitbox matrices produced by the record's
// bone lookup. The loop caps the model's count at 19 and preserves the original
// descriptor pointer for box clipping while constructing world-space capsules.
std::vector<RageScanHitbox> prepare_rage_scan_hitboxes(std::span<const RageLocalHitbox>,
                                                       const HitboxMatrices&,
                                                       const RageTrigonometry&);
} // namespace skeet

// Declarations from rage_hitchance.hpp

namespace skeet {
struct RageSpreadSample {
    float first;
    float second;
};
struct RageSpreadBasis {
    RageVector center;
    RageVector first;
    RageVector second;
};
struct RageScanHitbox {
    std::int32_t hitgroup;
    RageCapsule capsule;
    // Engine box-clipping inputs from native record +4 and +0x10.
    const void* engine_hitbox;
    Matrix3x4 transform;
};
struct RageBoxClipper {
    void* context;
    // Actual engine dependency at 0x43468C3C through 0x4339FA8E.
    // Required when a hitbox's capsule radius is not positive.
    int (*clip)(void*, const RageRay&, const RageScanHitbox&, RageTrace&);
};
struct RageHitchanceInput {
    RageVector origin;
    RageSpreadBasis basis;
    float distance;
    std::span<const RageScanHitbox> hitboxes;
    std::int32_t record_count;
    std::int32_t preferred_hitgroup;
    float minimum_hitgroup_multiplier;
};
struct RageHitchance {
    std::uint32_t hits;
    float fraction;
};
// Complete 32-sample loop 0x433600FE..0x433604AC. Input samples are the two
// consumed floats of each original 0x20-byte record; no RNG model is assumed.
// Capsule clipping is native recovered code. Box clipping stays an engine call.
RageHitchance evaluate_rage_hitchance(const RageHitchanceInput&,
                                      const std::array<RageSpreadSample, 32>&,
                                      const RageBoxClipper&);
} // namespace skeet

// Declarations from rage_points.hpp

namespace skeet {
struct RageAimPoint {
    RageVector position;
    float scale;
    std::int32_t hitgroup;
    std::uint32_t flags;
};
struct RagePointGeneration {
    RageVector origin;
    std::uint32_t regular_mask;
    std::uint32_t multipoint_mask;
    float point_scale;
    float spread;
    std::int32_t accuracy_percent;
    bool fixed_scale;
};
// Regular points from 0x4335D68B..0x4335E91A and the native hitbox traversal.
// Both spans contain the 19 model records used by this scanner. Hitboxes must
// already be transformed by prepare_rage_scan_hitboxes. Resolver-intersection
// candidates are generated separately by 0x433FD9B7 and are not part of this API.
std::vector<RageAimPoint> generate_rage_regular_points(const RagePointGeneration&,
                                                       std::span<const RageLocalHitbox, 19>,
                                                       std::span<const RageScanHitbox, 19>);
// Same native body for one index of the traversal. The scanner uses this to
// insert that hitbox's resolver-intersection candidates before the next index.
// An index outside the actual model count contributes no regular points.
std::vector<RageAimPoint> generate_rage_regular_hitbox_points(const RagePointGeneration&,
                                                              std::uint32_t hitbox_index,
                                                              std::span<const RageLocalHitbox>,
                                                              std::span<const RageScanHitbox>);
} // namespace skeet

// Declarations from rage_policy.hpp

namespace skeet {

struct RageHitboxMasks {
    std::uint32_t regular = 0;
    std::uint32_t safe = 0;
};
// Settings preparation at 0x433C15E8..0x433C16CD. Force-safe's hotkey
// evaluator is a caller input; the remaining mask transformations are native.
RageHitboxMasks prepare_rage_hitboxes(std::uint32_t selected, bool prefer_safe, bool force_safe,
                                      std::uint32_t avoid_unsafe, std::uint32_t configuration_flags,
                                      bool prefer_body, std::uint8_t body_disablers) noexcept;

struct RageBodyAimPolicy {
    RageHitboxMasks hitboxes;
    std::uint32_t target_state_flags = 0;
    std::uint32_t disabler_reasons = 0;
    bool context_ready = false;
};
// 0x43351A85..0x43351B51, including the final target-state safe-mask gate.
// Configuration bit 1 (Low inaccuracy) has no effect in this consumer.
void apply_rage_body_disablers(RageBodyAimPolicy&, std::uint8_t options) noexcept;

struct RageThresholdSettings {
    std::int32_t minimum_chance = 50; // config hash 0x3F5E5553, +0x2C
    std::int32_t minimum_damage = 10; // 0x18C0E6BE, +0x68
    bool override_enabled = false; // 0xB80303B4, +0x7C
    std::int32_t override_damage = 10; // 0x7DC413FE, +0xB8
    bool delay_shot = false; // 0xA09C0D52, +0x108
};
struct RageThresholds {
    std::int32_t raw_minimum_damage;
    float minimum_chance;
    bool override_active;
    bool delay_shot;
};
// Native consumers in 0x433C175D..0x433C1977. Config flag 0x20 selects
// the integer at global 0x4346A4B0; its value is an input, not assumed zero.
// Override activation is the result of hotkey 0x7EB3A142 evaluation.
RageThresholds prepare_rage_thresholds(const RageThresholdSettings&, std::uint16_t weapon_id,
                                       std::uint32_t configuration_flags,
                                       std::int32_t forced_chance,
                                       bool override_hotkey_active) noexcept;

} // namespace skeet

// Declarations from rage_safe_points.hpp

namespace skeet {
// Complete non-head branch of 0x433FD9B7 and caller 0x4335E9D5..0x4335EED9.
// hitbox_index is 1..18. Matrices are either the three record hypotheses or
// nine additional hypotheses, in native traversal order; they transform the
// original local endpoints directly. center is the prepared hitbox midpoint.
// target_origin is candidate +0x120, used when averaging the retained bounds.
// The caller applies the safe-hitbox mask before invoking this function.
std::vector<RageAimPoint>
generate_rage_body_safe_points(const RagePointGeneration&, const RageVector& center,
                               const RageVector& target_origin, std::uint32_t hitbox_index,
                               const RageLocalHitbox&, std::span<const Matrix3x4> hypotheses);

struct RageHeadOcclusion {
    // Model hitbox 6 resolved through the entity's model/hitbox-set interface at
    // 0x433FE9E0..0x433FEA3D, followed by the same three record matrices.
    RageLocalHitbox hitbox;
    std::array<Matrix3x4, 3> matrices;
};
// Head branch of 0x433FD9B7, including native clipping and the 15-angle search.
// A null occlusion pointer specifically denotes the native additional-records
// pointer being present: 0x433FE9C0 returns the angular bounds in that case.
// Otherwise model hitbox 6 and its matrices are required engine/model inputs.
std::vector<RageAimPoint>
generate_rage_head_safe_points(const RagePointGeneration&, const RageVector& center,
                               const RageVector& target_origin, const RageLocalHitbox& head,
                               std::span<const Matrix3x4, 3> hypotheses, const RageHeadOcclusion*,
                               const RageTrigonometry&);
} // namespace skeet

// Declarations from rage_safety.hpp

namespace skeet {
using RageResolverHitboxes = std::array<std::span<const RageScanHitbox>, 3>;
// Scanner's three-hypothesis safe-point gate, 0x4335F44A..0x4335F6FB.
// Head checks use the engine hitbox clipper and reject closer body intersections.
// Other hitgroups require a qualifying capsule intersection in every hypothesis.
// The head path requires a populated first hitbox in all three input sets.
bool rage_point_safe(const RageRay&, std::int32_t requested_hitgroup,
                     float minimum_hitgroup_multiplier, const RageResolverHitboxes&,
                     const RageBoxClipper&);
} // namespace skeet

// Declarations from rage_segments.hpp

namespace skeet {
struct RageSegment {
    RageVector first;
    RageVector second;
};
struct RageSegmentClosest {
    float first_fraction;
    float second_fraction;
    RageVector first_point;
};
// Complete 0x433B5355; returns the point and writes its clamped fraction.
RageVector project_rage_point_to_segment(const RageSegment&, const RageVector&,
                                         float& fraction) noexcept;
// Complete 0x433F4DAD with 0x433B55A8 and 0x433B5355 dependencies.
RageSegmentClosest closest_rage_segments(const RageSegment&, const RageSegment&) noexcept;
// Complete 0x4339CBE2; used by non-head safe-point validation.
float rage_segment_distance(const RageSegment&, const RageSegment&) noexcept;
// Complete 0x434012F7. The first parameter is a ray with the native normalized
// direction; its parameter is clamped at zero but has no upper bound.
RageSegmentClosest closest_rage_ray_segment(const RageRay&, const RageSegment&) noexcept;
} // namespace skeet

// Declarations from rage_selection.hpp

namespace skeet {

struct RagePointScore {
    std::int32_t damage = 0;
    std::int32_t hitgroup = 0;
    std::uint32_t flags = 0; // bit 0 safe; bit 1 carries the native 0x80 priority gate.
    float chance = 0;
    std::uint32_t priority = 0;
};
struct RagePointRequirements {
    std::int32_t target_health = 0;
    std::int32_t minimum_damage = 0;
    float minimum_chance = 0;
    bool prefer_body = false;
};
struct RagePointReference {
    std::array<float, 3> position;
    std::int32_t damage;
};
struct RagePointAdmissionState {
    RagePointReference reference;
    std::uint32_t priority_mask;
    std::uint32_t evaluation_flags;
    std::uint32_t body_disabler_reasons;
};
// 0x4336060A..0x433606E2. A descriptor without bit 2 becomes the reference
// even when rejected. Accepted records inherit this reference's position/damage.
// Failure may set point flag 0x80 or request the evaluation flag 0x100.
bool admit_rage_point(RagePointScore&, const RagePointRequirements&, RagePointAdmissionState&,
                      const std::array<float, 3>& descriptor_position,
                      std::uint32_t descriptor_flags) noexcept;
// 0x433604AC..0x433605ED. chance*100 uses the current x86 rounding mode.
std::uint32_t rage_point_priority(const RagePointScore&, const RagePointRequirements&) noexcept;

inline constexpr std::size_t no_rage_point = std::numeric_limits<std::size_t>::max();
struct RagePointSelection {
    std::size_t primary = no_rage_point;
    std::size_t alternate_first = no_rage_point;
    std::size_t alternate_second = no_rage_point;
    std::uint32_t candidate_flags = 0;
};
// Complete selection loop 0x4336070E..0x43360869. Returned indices identify
// the original evaluated point records to copy; unassigned slots stay untouched.
// Equal priorities retain the first point. Masking applies only to primary choice.
RagePointSelection select_rage_points(std::span<const RagePointScore> points,
                                      std::uint32_t priority_mask, bool prefer_body,
                                      bool prefer_safe, std::uint32_t candidate_flags) noexcept;

struct RageCandidatePriority {
    std::int32_t target_health = 0;
    RagePointScore primary;
    bool prefer_body = false;
    bool prefer_safe = false;
    bool target_weapon_priority_enabled = false; // candidate +0x10C bit 0
    // Result of the entity-list/weapon-handle lookup at 0x4336466A..0x433646D0.
    bool target_weapon_is_taser = false;
    bool target_priority = false; // target table +3
    std::uint32_t target_state_flags = 0;
};
std::uint32_t rage_candidate_priority(const RageCandidatePriority&) noexcept;
// Native qsort comparator 0x433FF5C7 preserves wrapping 32-bit subtraction.
std::int32_t compare_rage_candidate_priorities(std::uint32_t left, std::uint32_t right) noexcept;

} // namespace skeet

// Declarations from action_detail.hpp

namespace skeet::rage::action_detail {
template <class T> T read(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> void write(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
// 43332999..4333337E. The shared point-scale body is called at its native
// place, between minimum damage and the per-player cache/adjustment updates.
void prepare_settings(const client::CommandFrame&, void* target, std::uint32_t entity_index,
                      AcquisitionTargetSettings&, const ActionStateBindings&);
std::array<RageVector, 2> origins(const client::CommandFrame&, void* target,
                                  const ActionStateBindings&);
struct Point {
    RageVector position;
    std::uint32_t flags, highlight;
};
std::vector<Point> points(const client::CommandFrame&, const AcquisitionTargetSettings&,
                          const lagcomp::LagRecord&, const scan_detail::Model&,
                          const scan_detail::Geometry&, std::uint32_t hitbox, bool safe,
                          bool regular, const ActionStateBindings&);
} // namespace skeet::rage::action_detail

// Declarations from final_target_context_detail.hpp

namespace skeet::rage::final_target_detail {
struct Producer {
    const FinalTargetContextBindings& bindings;
    client::CommandFrame& frame;
    PreambleState& state;
    std::array<Matrix3x4, 256> saved_bones;
    const void* selected_descriptor = nullptr;
};
void bind_teammate_policy(FinalHitchanceContext&, const Producer&);
bool teammate_hit(const Producer&, BallisticEntity);
bool select_studio_hitbox(FinalHitchanceContext&, Producer&, const TargetCandidate&,
                          const lagcomp::LagRecord&);
std::vector<RageScanHitbox> hitbox_geometry(const FinalHitchanceContext&, const Producer&,
                                            const HitboxMatrices&);
void apply_record(const FinalHitchanceContext&, const Producer&, lagcomp::EntityRecordScope&,
                  const lagcomp::LagRecord&);
void publish_failure(FinalHitchanceFailure&, const Producer&, std::uint32_t entity_index);
std::int32_t trace_direction(const FinalTargetContextBindings&, BallisticEngine&,
                             const FinalHitchanceSample&, const RageVector& direction,
                             BallisticQuery&);
std::int32_t trace_projected(const FinalTargetContextBindings&, BallisticEngine&,
                             const FinalHitchanceSample&, const RageVector& point, BallisticQuery&);
std::int32_t trace_teammate(const Producer&, FinalHitchanceContext&, const RageVector& direction,
                            BallisticQuery&);
// Shared failure predicate is also used by the prebound lower-level API.
bool requests_body_aim(const FinalHitchanceFailure&);
bool requests_body_aim(const FinalHitchanceFailure&, const client::CommandFrame*);
} // namespace skeet::rage::final_target_detail

// Declarations from owner_detail.hpp
namespace skeet::rage::owner_detail {
void initialize_settings(ConfigRegistries&, AimbotSettings&,
                         esp::AimbotVisualizationConfig&) noexcept;
}

// Declarations from post_firing_detail.hpp
namespace skeet::rage::post_firing_detail {
void predict_stop(PreambleState&, PreambleResult&, AcquisitionStorage&, void* local_player,
                  const PostFiringBindings&);
}

// Declarations from scanning_detail.hpp

namespace skeet::rage::scan_detail {
struct Model {
    const void* header = nullptr;
    std::vector<RageLocalHitbox> hitboxes;
    std::vector<std::uint32_t> bones;
};
Model model(void* player, const ScanModelOffsets&);
bool teammate_blocks(const ScanContext&, const RageVector& point);
struct Geometry {
    std::vector<RageScanHitbox> primary;
    std::array<std::vector<RageScanHitbox>, 3> alternatives;
    RageResolverHitboxes views() const noexcept {
        return {alternatives[0], alternatives[1], alternatives[2]};
    }
};
struct ProjectedPoint {
    RageVector position{};
    RageVector direction{};
    std::int32_t hitbox = -1, hitgroup = -1;
    bool found = false;
};
ProjectedPoint project(const ScanContext&, const Geometry&, const RageVector&, float range);
float minimum_multiplier(const ScanPolicy&, std::int32_t health, std::int32_t damage,
                         std::int32_t hitgroup, std::int32_t weapon_id) noexcept;
RageSpreadBasis spread_basis(const RageVector&) noexcept;
RageHitchance normal_chance(const ScanContext&, void* player, const Geometry&,
                            const RageSpreadBasis&, float range, std::int32_t hitgroup,
                            float minimum_multiplier);
void refine(const ScanContext&, const ScanPolicy&, std::int32_t health, void* player,
            BallisticEntity target, const Geometry&, TargetPoint&);
}

// Declarations from targeting_pipeline_detail.hpp

namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet::rage::targeting_detail {
// Entered at433551BA after the concrete readiness result admits angles.
TargetFiringResult aim_and_commit_target(TargetShotChoice, FiringState&, FiringBranchState&,
                                         client::Command&, RageShotState&, RageShotCommitState&,
                                         const FiringReadinessBindings&,
                                         const FiringPostAngleBindings&);
TargetFiringResult aim_and_commit_target(TargetShotChoice, FiringState&, FiringBranchState&,
                                         client::Command&, RageShotState&, RageShotCommitState&,
                                         const FiringReadinessBindings&,
                                         const FiringPostAngleBindings&,
                                         const animation::BoneRuntime&);
}
