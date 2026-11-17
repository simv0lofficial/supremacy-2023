#pragma once

// Lag Compensation Subsystem Declarations
#include "skeet/anti_aim/body_yaw.hpp"
#include <cstring>
#include "skeet/engine/interface_call.hpp"
#include <span>
#include "skeet/client/command_frame.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/resolver_runtime/build.hpp"
#include <optional>
#include <memory>
#include <vector>
#include <cstddef>
#include <cstdint>
#include "skeet/animation/entity_update.hpp"
#include "skeet/animation/state_copy.hpp"
#include "skeet/movement/collision.hpp"
#include <array>
#include "skeet/core.hpp"
#include "skeet/resolver_runtime/environment_probe.hpp"
#include "skeet/engine/jobs.hpp"
#include "skeet/config.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/rage/shot_feedback.hpp"
#include "skeet/client/entity_observer_receiver.hpp"
#include "skeet/resolver_runtime/environment_entry.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/players/state.hpp"
#include "skeet/lagcomp/records.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/legit/historical_bones.hpp"
#include "skeet/animation/bone_source_reference.hpp"
#include "skeet/lagcomp/history.hpp"
#include <algorithm>
#include <cmath>
#include "skeet/resolver_runtime/attempts.hpp"


// Declarations from angle_publication.hpp

namespace skeet::lagcomp {
// Complete433F19CC. Entries have a native stride of16; capacity and first
// are the original unsigned16-bit fields. Only one subtraction is performed.
std::uint32_t history_angle_entry_index(std::uint16_t capacity, std::uint16_t first,
                                        std::int32_t ordinal) noexcept;
// Complete43416836..1698F: normalize/clamp pitch to[-90,90], then publish
// changed pitch/yaw to the native interpolation ring and optional live target.
// state_fac21eaf is the actual decoded entity-property offset. Its pointers
// are native pointers on the x86 target, not copied 32-bit addresses on a host.
void publish_history_final_angles(void* player, const Vector& previous_angles,
                                  const EntityStateRuntime&, std::uint32_t state_fac21eaf);
} // namespace skeet::lagcomp

// Declarations from animation_capture.hpp

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
struct HistoryAnimationSeed {
    float previous_update_time; // saved animation6C
    Vector velocity; // saved live velocity before resolver animation changes
    bool active_air_layer; // native low byte of stack1E4
};
// Complete434118B6..43411A37. animation is the nonnull state pointer retained
// by the caller's CachedHistoryFrame. Writes owner180 through433CAFE0 and
// initializes the native ground bytes only on a first discontinuous capture.
HistoryAnimationSeed prepare_history_animation(client::PlayerState&, void* player,
                                               void* animation_state, std::int32_t simulation_ticks,
                                               std::uint32_t update_flags,
                                               const EntityStateRuntime&,
                                               std::uint32_t player_flags_e2fbe695);
} // namespace skeet::lagcomp

// Declarations from animation_math.hpp

namespace skeet::lagcomp {
struct AnimationYawMotion {
    anti_aim::AnimationYawInput bounds;
    float horizontal_speed_ec;
    float vertical_speed_f0;
};

// Complete 433CAFE0. The three scalar arguments are native XMM0/1/2;
// current_yaw is the incoming value at EDX. The result is its final write.
float advance_animation_yaw(const AnimationYawMotion&, float target_yaw, float stationary_yaw,
                            float elapsed, float current_yaw) noexcept;
} // namespace skeet::lagcomp

// Declarations from capture_speed.hpp

namespace skeet::lagcomp {
struct HistoryCaptureSpeedBindings {
    const EntityStateRuntime& entity;
    std::uint32_t flags_e2fbe695;
    std::uint32_t maximum_speed_3eb71843;
    std::uint32_t mode_7090d8dd;
    void* convar_4b52a9bf;
};
// Complete434115E3..117E6. XMM2 is zero on both incoming native paths
// (11452/115DD). The landing branch checks the actual ConVar integer; the
// already-grounded branch calls the player's maximum-speed virtual276.
// Only horizontal velocity changes; Z retains its original bits.
bool apply_history_capture_speed_limit(const RecordRing&, void* player,
                                       const HistoryCaptureSpeedBindings&);
} // namespace skeet::lagcomp

// Declarations from entity_preparation.hpp

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::lagcomp {
struct EntityPreparationLiveGlobals {
    void* const& engine_globals; //43467B04, first word is comparison realtime
    float* const& simulation_time; //43467EB8, raw bits copied to entity stamp
    float* const& current_time; //4346A334
    const TickTimingSamples& latency; //43478C70
};
struct EntityPreparationContext {
    const EntityStateRuntime& entity;
    std::uint32_t global_frame; // *0x43467EB8
    float comparison_time; // *0x43467B04
    float current_time; // *0x4346A334
    TickTimingSamples latency; // 0x43478C70/74/78
    // Native optional current timing state. A present state supplies D4+18C;
    // otherwise mean latency plus corrected command tick supplies the clock.
    const HistoryTickTiming* current_timing = nullptr;
    const EntityPreparationLiveGlobals* live_globals = nullptr;
    std::uint32_t frame_value() const noexcept {
        if (!live_globals)
            return global_frame;
        std::uint32_t value;
        std::memcpy(&value, live_globals->simulation_time, sizeof value);
        return value;
    }
    float comparison_value() const noexcept {
        if (!live_globals)
            return comparison_time;
        float value;
        std::memcpy(&value, live_globals->engine_globals, sizeof value);
        return value;
    }
    float current_value() const noexcept {
        return live_globals ? *live_globals->current_time : current_time;
    }
    const TickTimingSamples& latency_values() const noexcept {
        return live_globals ? live_globals->latency : latency;
    }
};
// Complete4341DDA0. The target has a valid nonzero networkable index, and its
// shared cache has the native at-most-six packet samples. Writes the two
// collision fields and eye angles, and may stamp cache+F4. Return codes1..7
// are retained verbatim for callers that publish the native preparation mode.
std::uint32_t prepare_entity_angles(client::PlayerStateCache&, void* target,
                                    const LagRecord& source, bool copy_yaw,
                                    const EntityPreparationContext&);
} // namespace skeet::lagcomp

// Declarations from entity_state.hpp

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::lagcomp {
struct EntityNetvars {
    std::uint32_t eye_angles;
    std::uint32_t simulation_time;
    std::uint32_t old_simulation_time;
    std::uint32_t field_a2a8247f;
    std::uint32_t origin;
    std::uint32_t player_flags;
    std::uint32_t velocity;
    std::uint32_t field_fb7e3404;
    std::uint32_t duck_amount;
    std::uint32_t weapon_handle;
    std::uint32_t weapon_field_6777c9ce;
    std::uint32_t animation_state;
    std::uint32_t animation_layers;
    std::uint32_t pose_parameters;
    std::uint32_t bone_cache;
    std::uint32_t bone_accessor;
    std::uint32_t recent_bone_counter;
    std::uint32_t last_bone_setup_time;
    std::uint32_t field_0471e13c;
    std::uint32_t effects_bb78dc43;
    std::uint32_t field_432b1f5b;
    std::uint32_t field_0d560fac;
    std::uint32_t transform_guard_49d93e62;
};
struct PlayerCaptureState {
    std::uint32_t field_180;
    std::uint32_t field_1ac;
    std::uint8_t resolver_index;
    std::array<std::uint8_t, 256> resolver_modes;
    std::uint8_t resolver_state;
};
struct EntitySnapshotHeader {
    std::int32_t capture_frame;
    std::uint32_t handle;
    float simulation_time;
    std::uint32_t player_flags;
    float collision_max_z;
    Vector origin;
    Vector absolute_origin;
    float absolute_yaw;
    std::uint32_t readable_bones;
    std::uint32_t writable_bones;
    std::uint32_t recent_bone_counter;
    Vector velocity;
    AnimationState animation;
    std::int32_t bone_count;
    std::uint32_t padding_21c;
};
static_assert(sizeof(EntitySnapshotHeader) == 0x220);
struct EntitySnapshot {
    EntitySnapshotHeader state;
    std::span<Matrix3x4> bones; // caller owns storage for the live bone count
};
struct EntityHitboxModelOffsets {
    std::ptrdiff_t studio_header, hitbox_set;
};
using SetAbsoluteVector = engine::MemberFunction<void, const Vector&>;
using SetCollisionBounds = engine::MemberFunction<void, const Vector&, const Vector&>;
// 43407F19..43407F5B after the actual studio model/hitbox-set lookup.
// A missing set supplies an empty index span. Native-valid bone indices must
// address the caller's cache; source matrices and cache storage are distinct.
void scatter_record_hitbox_matrices(const HitboxMatrices&,
                                    std::span<const std::uint32_t> hitbox_bones,
                                    Matrix3x4* bone_cache) noexcept;
struct EntityStateRuntime {
    EntityNetvars offsets;
    void* engine;
    void* entity_list;
    // Borrow the live engine words. Individual native transactions retain
    // their own local captures at the read sites in the recovered bodies.
    const float& interval;
    const float& receive_interval;
    const std::int32_t& frame;
    const std::int32_t& command_tick;
    const std::int32_t* const& previous_command;
    const std::int32_t* const& current_command;
    // These three are resolved engine entry points, native globals
    // 434688C8,43467E44,4346A7F0. Missing optional entries are native no-ops.
    SetAbsoluteVector set_absolute_angles;
    SetAbsoluteVector set_absolute_origin;
    SetCollisionBounds set_collision_bounds;
    std::uint8_t* transform_override; // native optional 434683D0

    void capture_record(LagRecord&, void* player, const PlayerCaptureState&) const;
    // Complete live433FD690: nonzero index, actual cache lookup, and owner
    // field reads on their respective sides of the engine77/14 boundaries.
    void capture_record(LagRecord&, void* player, client::PlayerStateCache&) const;
    void capture_snapshot(EntitySnapshot&, void* player) const;
    // 4331E453 after snapshot capture and record copy, including43407EA3's
    // studio hitbox-to-bone scatter and native cache invalidation paths.
    void apply_record(void* player, const LagRecord&, std::span<const std::uint32_t> hitbox_bones,
                      const std::uint32_t* current_bone_counter) const;
    // Live43407EA3 producer. Studio/bone reads occur at the actual scatter
    // site, after the captured entity has received its record state.
    void apply_record_live_model(void* player, const LagRecord&, const EntityHitboxModelOffsets&,
                                 const std::uint32_t* current_bone_counter) const;
    // Valid recorded handle/entity contract. Native invalid-handle path enters
    // its deliberate invalid-instruction branch, rather than a recovery path.
    void restore_snapshot(const EntitySnapshot&) const;

private:
    void apply_record_inputs(void* player, const LagRecord&, std::span<const std::uint32_t>,
                             const std::uint32_t*, const EntityHitboxModelOffsets*) const;
};
// Complete 4339F326/433E41CA/4331E453/4336630D transaction. Snapshot capture
// occurs once per frame; repeated applications share it, and restore invalidates
// its frame marker. The copied record owns its full0x10A0 bytes.
class EntityRecordScope {
public:
    EntityRecordScope(const EntityStateRuntime&, void* player,
                      std::span<Matrix3x4> saved_bones) noexcept;
    ~EntityRecordScope() {
        restore();
    }
    EntityRecordScope(const EntityRecordScope&) = delete;
    EntityRecordScope& operator=(const EntityRecordScope&) = delete;
    void apply(const LagRecord&, std::span<const std::uint32_t> hitbox_bones,
               const std::uint32_t* current_bone_counter);
    void apply_live_model(const LagRecord&, const EntityHitboxModelOffsets&,
                          const std::uint32_t* current_bone_counter);
    void restore();
    void* player;
    std::uint32_t flags;
    LagRecord record;
    EntitySnapshot snapshot;

private:
    const EntityStateRuntime& runtime;
};
float animation_speed_factor(const void* animation_state) noexcept; // 43383F8B
} // namespace skeet::lagcomp

// Declarations from extrapolated_record.hpp

namespace skeet {
struct MovementExtrapolationBindings;
struct MovementExtrapolationLiveBindings;
}
namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet::client {
struct CommandBudgetBindings;
}
namespace skeet::lagcomp {

struct ExtrapolatedRecordBindings {
    client::PlayerStateCache& players;
    const EntityPreparationContext& preparation;
    const RecordAttachmentContext& attachments;
    animation::BoneSourceReference<resolver_runtime::HypothesisBuildContext> bones;
    const MovementExtrapolationBindings& movement;
    std::optional<std::int32_t> prediction_limit; // decoded fixture result; absent in live entry
};

// Complete4339C8E2. Unlike the hypothesis builder's80300 mask, this entry
// requests300, takes the origin/yaw from the destination record and publishes
// flag4/clears20 whenever the engine/model build succeeds, even with no set.
void build_extrapolated_record_matrices(LagRecord&, resolver_runtime::HypothesisBuildContext&,
                                        std::span<Matrix3x4, 256> scratch);
// Live4339C8E2 entry. The model/set is read from this player after the build.
void build_extrapolated_record_matrices(void* player, LagRecord&, const animation::BoneRuntime&,
                                        std::span<Matrix3x4, 256> scratch);

// Concrete input projection for433FAC08. The original valid-player contract
// requires its animation state for the ensuing hypothesis computation.
resolver_runtime::HypothesisInput
capture_record_hypothesis_input(void* player, const client::PlayerState&, const LagRecord&,
                                const EntityStateRuntime&) noexcept;

// Complete43387BCE transaction. The networkable index/cache lookup is repeated
// here as in native. A stale/empty history captures live state; the other path
// copies all10A0 bytes. A negative movement result exits before flags/bones/
// attachments. Destination.field_00 is copied or left by capture unchanged.
bool capture_extrapolated_record(client::CommandFrame&, void* player, LagRecord& destination,
                                 std::uint32_t& output_flags, const ExtrapolatedRecordBindings&);
// Full live timeline/movement/bone entry. Budget reads occur inside433F94AA,
// after source capture and at its three concrete clamp sites.
bool capture_extrapolated_record(client::CommandFrame&, void* player, LagRecord& destination,
                                 std::uint32_t& output_flags, const ExtrapolatedRecordBindings&,
                                 const animation::BoneRuntime&,
                                 const MovementExtrapolationLiveBindings&,
                                 const client::CommandBudgetBindings&,
                                 std::int32_t* predicted_ticks = nullptr);

} // namespace skeet::lagcomp

// Declarations from history.hpp

namespace skeet::lagcomp {
struct RecordRing {
    std::uint32_t first = 0;
    std::uint32_t count = 0;
    std::array<LagRecord*, 32> records{};
    LagRecord& at(std::uint32_t age) const noexcept {
        return *records[(first + age) & 31u];
    }
};
// The native global free list is 434797B8. Removed records are returned LIFO;
// their first dword temporarily stores the next native 32-bit record address.
// The host ownership vector keeps the same records alive across ring reuse.
class RecordPool {
public:
    LagRecord& acquire();
    void release(LagRecord&);
    std::size_t available() const noexcept {
        return free.size();
    }
    LagRecord* free_head() const noexcept {
        return free.empty() ? nullptr : free.back();
    }
    //4335BB22 consumes the free list after all64 canonical rings are drained.
    // Frees each returned source allocation once, in native LIFO order.
    void finalize_free_records();

private:
    std::vector<std::unique_ptr<LagRecord>> allocated;
    std::vector<LagRecord*> free;
};
LagRecord& push_history_record(RecordRing&, RecordPool&); // complete 4334C657
void pop_history_record(RecordRing&, RecordPool&); // complete 433B04F3
void clear_history_records(RecordRing&, RecordPool&); // 4341680F..4341682F
// 4340CEC3..4340CF87. Absolute wrapped clock differences prune records and
// packet samples once they reach one second, rounded to native tick units.
void prune_history_records(RecordRing&, RecordPool&, std::int32_t clock, float interval);
void prune_packet_samples(LagTimeline&, std::int32_t clock, float interval) noexcept;

struct ExtrapolationPlan {
    std::int32_t ticks = 0;
    bool inspected_third = false;
};
// Complete 4336C692..4336C8AE after the copied record has been selected.
// The layer values are read at their proven native offsets FC/104/108.
ExtrapolationPlan plan_history_extrapolation(const RecordRing&, std::uint32_t selected_ordinal,
                                             std::uint32_t options, float interval,
                                             std::int32_t prediction_limit) noexcept;

struct HistoryTickLiveGlobals {
    float* const& current_time; //43467EB8
    const float& interpolation; //43476928
    float* const& interval; //4346A338
    const std::int32_t& maximum_unlag_ms; //43468230
};
struct HistoryTickTiming {
    float current_time;
    float latency_198;
    float interpolation;
    float interval;
    std::int32_t maximum_unlag_ms;
    std::int32_t tick_d4;
    std::int32_t tick_18c;
    const HistoryTickLiveGlobals* live_globals = nullptr;
    const std::uint32_t* live_tick_d4 = nullptr;
    const std::uint32_t* live_tick_18c = nullptr;
};
HistoryTickTiming resolve_history_tick_timing(const HistoryTickTiming&) noexcept;
struct HistoryTickWindow {
    std::int32_t first, last, corrected;
};
// Complete 4339438E,4337EA43,4337F00E. The last helper deliberately truncates
// seconds to an integer before converting back to tick units.
HistoryTickWindow history_tick_window(const HistoryTickTiming&) noexcept;
struct TickTimingSamples {
    std::uint32_t first;
    std::int32_t count;
    std::array<float, 16> values;
};
// Complete43385D57. Empty/nonpositive count returns1. Conversion uses the
// current SSE rounding mode, including ties, rather than truncating.
std::int32_t mean_history_timing_ticks(const TickTimingSamples&) noexcept;

// Native repeated publication block 4341651A..434165CF (also434160D6..1618B).
void publish_record_flags(LagRecord&, const LagRecord* previous, bool compare_layer,
                          std::uint32_t update_flags, bool flag_43468d7c) noexcept;
struct HistoryRecordingState {
    RecordRing ring;
    LagRecord fallback{}; // native owner+0x660; receive tick aliases owner+0x668
    std::uint32_t flags = 0;
    std::uint8_t resolver_index = 0;
};
enum class HistoryCaptureKind {
    none,
    inserted,
    fallback
};
struct HistoryCaptureSlot {
    LagRecord* record;
    HistoryCaptureKind kind;
};
// Recording slot lifecycle43415FF9..43416064 /43416497..4341649E and clear
// 4341680F..43416836. The following actual entity capture and bone builds use
// EntityStateRuntime and build_deferred_history_record, respectively.
HistoryCaptureSlot prepare_history_capture(HistoryRecordingState&, RecordPool&, bool enabled,
                                           bool suppressed, std::int32_t simulation_tick,
                                           bool refresh_fallback);
// Complete fallback publication43416749..43416836. A newly captured fallback
// marks the owner for another update; an older eligible one replaces the full
// newest record and receives the native -1 receive-tick marker.
bool publish_history_fallback(HistoryRecordingState&, bool captured_fallback_now, void* engine,
                              float interval, float receive_interval,
                              std::int32_t maximum_unlag_ms);
} // namespace skeet::lagcomp

// Declarations from history_activation.hpp

namespace skeet::legit {
class LifecycleOwner;
}
namespace skeet::lagcomp {
struct HistoryActivationBindings {
    const std::uint8_t* prediction_suppressed; // address of global434693EC
    // Address of global4346A3A4, whose optional value points to the live local
    // player cell. Both levels are read by433C0714 after its flag gate.
    void* const* const* local_player;
    std::ptrdiff_t player_prediction_disabled; // decodedBF567619 byte offset
    legit::LifecycleOwner* const* configuration; // canonical owner slot4347588C
    const std::uint8_t* recording_enabled; // address of global43467224
};

// Complete433C0714 boolean result. The caller publishes it to context+34
// before asking the engine's82/93 gates about recording availability.
bool history_prediction_available(const HistoryActivationBindings&) noexcept;

// Complete43401219. Resolve the optional actual LEGIT owner, then read its
// canonical nine enabled/trigger-enabled/trigger-reaction/accuracy entries.
// Source owners are never interpreted with the native allocation's offsets.
bool history_configuration_active(const HistoryActivationBindings&) noexcept;

} // namespace skeet::lagcomp

// Declarations from history_alternative.hpp

namespace skeet::lagcomp {
struct HistoryAlternativeFields {
    std::uint32_t velocity_36375c23;
    std::uint32_t origin_4e66bb34, origin_2a671b42;
    std::uint32_t tick_3d10b94c, player_flags_e2fbe695, angles_e23a9be1;
    std::uint32_t maximum_speed_3eb71843;
    std::uint32_t weapon_time_0176a381;
};
struct HistoryAlternativePredictionLimit {
    void* const* local_player; // dereferenced global4346A3A4, nullable
    std::uint32_t field_bf567619, field_dda99ab5, field_b7ea8fda;
    const std::int32_t* configured; //43469274
};
struct HistoryAlternativeVariables {
    // Decoded engine ConVars: follow parent+1C, XOR float+2C/int+30 with
    // the terminal object's native address. Only recovery may be absent.
    const void* stamina_recovery; //43467E4C ^FC23F1E7
    const void* gravity; //43467A54 ^B882FC15
    const void* bunnyhop; //43467FA0 ^4B52A9BF
    const void* jump_impulse; //43467990 ^AF1F2C93
    const void* stamina_maximum; //4346A88C ^D9D937D8
    const void* stamina_jump; //43468FB4 ^C9B16EBB
    const void* stamina_land; //43468F4C ^B0CF25E8
    const void* air_acceleration; //434674E4 ^DCEAEAC8
};
struct HistoryAlternativeBindings {
    EntityNetvars offsets;
    HistoryAlternativeFields fields;
    HistoryAlternativePredictionLimit prediction;
    HistoryAlternativeVariables variables;
    void* engine; // actual engine-client virtual12/14
    void* entity_list; // actual entity-list virtual4
    float* interval;
    const float* receive_interval;
    float* current_time;
    const std::int32_t* animation_frame;
    std::uint8_t* animation_reset;
    std::uint8_t& alternate_strafe; //4347CA58, shared across players/calls
    MovementCollisionEngine& collision;
    std::uint8_t* engine_trace_flag;
    animation::AnimationUpdateEngine animation_engine;
    RageTrigonometry trigonometry;
    double (*atan2_import)(double y, double x); // actual CRT77509EE0
    SetAbsoluteVector set_absolute_angles; // optional434688C8
    void* allocator; //43467204
    // Native local348-byte workspace. Its initial bytes are retained when
    // the player's animation pointer is null, except constructor+13C..14C.
    // Each invocation requires independent storage; it must not alias player.
    std::span<std::byte, animation::copied_animation_state_size> animation_scratch;
};

// Native4333D6D9: context, nonnull player, canonical per-player cache,
// destination record and requested extrapolation ticks, in that order.
std::int32_t model_alternative_history_record(client::CommandFrame& context, void* player,
                                              client::PlayerState& state, LagRecord& destination,
                                              std::int32_t ticks,
                                              const HistoryAlternativeBindings& bindings);
} // namespace skeet::lagcomp

// Declarations from history_animation_publish.hpp

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
struct HistoryAnimationPublishFields {
    std::uint32_t animation_enabled; //8481EA04, byte
    std::uint32_t poses; //1CAAF0A8; pose vector and its interpolation watchers
    std::uint32_t absolute_velocity; //36375C23
    std::uint32_t studio_header; //32E4DAF1
    std::uint32_t player_origin; //A9EE6062
    std::uint32_t render_origin; //C865F10E
    std::uint32_t player_flags; //E2FBE695
    std::uint32_t old_simulation_time; //D3D4230D
    std::uint32_t local_time; //9F8E7F71, local player
    std::uint32_t invalidation_counter; //2B554AD0
};
struct HistoryAnimationPublishSaved {
    float* current_time; //43467EB8 retained before the animation transaction
    float saved_current_time;
    float* frame_time; //43467B04+14
    float saved_frame_time;
    Vector absolute_origin; //original player slot10, saved before animation
};
struct HistoryAnimationMovementEngine {
    void* context;
    // Genuine43468DC0: ECX=studio wrapper,EDX=sequence,stack(poses,out).
    // The engine entry writes the displacement; its length is recovered code.
    void (*displacement)(void* context, void* studio, std::int32_t sequence, const float* poses,
                         Vector& output);
};
struct HistoryAnimationPublishBindings {
    const EntityStateRuntime& entity;
    HistoryAnimationPublishFields fields;
    void* clock; //4346A514, actual engine virtual14
    void* const* const& local_time_entity; //live outer pointer cell4346A3A4; nullable
    const float* now; //4346A334
    const float& receive_interval; //live43467B04+20
    std::uint8_t* reset_byte; //optional43467B24
    const HistoryRecordingConfiguration& configuration;
    const std::uint8_t* player_table; //4347BE08, stride20
    std::uint32_t player_table_offset; //retained networkable index*20
    const std::int32_t& allow_zero_sequence; //4347C9EC
    const float& movement_exponent; //43475AC0+5C
    const std::uint8_t* candidate_attempts; //434745E0, three native bytes
    HistoryAnimationMovementEngine movement;
    // Actual imports7750BE50,775090B0,77509EE0. They are the only numeric
    // boundaries here; candidate generation and selection remain concrete.
    float (*power)(float, float);
    float (*arccosine)(float);
    double (*atan2)(double, double);
};
//43414F3C..43415F6F, including the acceptance cold tail434161F4..43416370.
// animation_state is the same nonnull state retained by CachedHistoryFrame.
// layer_weight and speed_fraction are the resolver worker's outgoing carries.
void publish_history_animation(client::PlayerState&, void* player, void* animation_state,
                               std::int32_t simulation_tick, std::int32_t simulation_ticks,
                               float speed_fraction, float layer_weight,
                               std::uint32_t& update_flags, const HistoryAnimationPublishSaved&,
                               const HistoryAnimationPublishBindings&);
//43415F6F..43415FF9, also reached by the native missing-animation-state arm.
void publish_history_animation_origin(client::PlayerState&, void* player,
                                      const Vector& saved_absolute_origin,
                                      const HistoryAnimationPublishBindings&);
} //namespace skeet::lagcomp

// Declarations from history_build.hpp

namespace skeet::lagcomp {
struct HistoryHypothesisPolicy {
    bool enabled;
    bool force;
    std::int32_t game_type;
    std::uint32_t local_handle;
    void* entity_list;
    const std::uint8_t* player_table; // native 4347BE08, stride0x20; value+0x14
    HotkeyBinding hotkey;
    InputBitmap keys;
    std::uint32_t settings_mask;
    std::uint32_t active_mode_mask;
    bool physical_override;
    std::uint32_t input_flags;
};
// Shared complete policy4341639C..43416476 /4336A638..4336A6ED. Entity-list[4]
// and networkable(+8)[10] remain actual engine calls; the hotkey uses the
// existing concrete key bitmap evaluator.
bool should_build_history_hypotheses(const HistoryHypothesisPolicy&, void* player);
// Complete valid-player deferred worker4336A5B3. Its history lookup is provided
// as the recovered HypothesisInput projection, while both bone and hypothesis
// builders execute concrete recovered functions, not internal callback stubs.
void build_deferred_history_record(LagRecord&, void* player, float speed_fraction,
                                   const HistoryHypothesisPolicy&,
                                   const resolver_runtime::HypothesisInput&,
                                   resolver_runtime::HypothesisBuildContext&);
} // namespace skeet::lagcomp

// Declarations from history_configuration.hpp

namespace skeet::lagcomp {
// One source-owned configuration for the complete4347E5DE owner. Offsets
// document its native +8 receiver; portable receiver/backend fields do not
// alias that native storage. The constructor and lifecycle bind these cells.
struct HistoryRecordingConfiguration {
    const std::int32_t* target_hitboxes; // receiver+28, full owner+30
    const std::uint8_t* recording_enabled; // receiver+2C, global43467224
    const std::uint8_t* resolver_enabled; // receiver+30, global4346754C
    std::uint8_t prediction_available; // receiver+34, full owner+3C
    void* engine; // receiver+38, full owner+40
    const std::uint8_t* rework = nullptr;
    bool reworked() const noexcept {
        return rework && *rework;
    }
};
} //namespace skeet::lagcomp

// Declarations from history_environment.hpp

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
#if defined(_M_IX86) || defined(__i386__)
using HistoryEnvironmentAtan2 = double(__vectorcall*)(double, double);
#else
using HistoryEnvironmentAtan2 = double (*)(double, double);
#endif
struct HistoryEnvironmentInput {
    RageVector local_origin; // native cached origin43475BD4, before the +64 eye offset
    std::int32_t simulation_ticks;
    // Raw x86 address of the canonical native owner+5D4 ring. It is read as
    // float bits on the original both-blocked path, not dereferenced there.
    std::uint32_t native_history_address;
    // Read only if the final yaw-change gate succeeds, after every trace.
    const float& current_time; // *4346A334
};
struct HistoryEnvironmentScratch {
    // Native caller stack+2EC..2F4, retained between players in4340C971.
    // The native enclosing function owns these automatic words and does not
    // initialize them before first use; the opaque load retains actual bytes.
    RageVector direction;
};
struct HistoryEnvironmentEngine {
    const resolver_runtime::EnvironmentEngine& probe;
    std::uint32_t origin_offset; // decoded A9EE6062 target-origin field
    // Native77509EE0 __libm_sse2_atan2: y in XMM0, x in XMM1, result in XMM0.
    HistoryEnvironmentAtan2 atan2;
};
struct HistoryEnvironmentUpdate {
    bool selected = false;
    bool timestamp_updated = false;
};

// Complete4340D89F..4340E58B; stops before E58B's resolver-index store.
// Caller has entered the preceding resolver-detection branch. Reads the live
// target origin, executes the concrete world/hull and
// penetration probe, publishes owner+F0, and conditionally stamps owner+F4.
// The shared PlayerState owns the only history ring and all published fields.
HistoryEnvironmentUpdate update_history_environment(client::PlayerState&, const void* target,
                                                    HistoryEnvironmentScratch&,
                                                    const HistoryEnvironmentInput&,
                                                    const HistoryEnvironmentEngine&);
} // namespace skeet::lagcomp

// Declarations from history_jobs.hpp

namespace skeet::lagcomp {
// Native queue payload434161DC..434161EC /43416614..43416624. These are
// native x86 addresses; no worker state is appended to the three dwords.
struct HistoryJobItem {
    std::uint32_t player;
    float speed_fraction;
    std::uint32_t record;
};
static_assert(sizeof(HistoryJobItem) == 12);
static_assert(offsetof(HistoryJobItem, speed_fraction) == 4);
static_assert(offsetof(HistoryJobItem, record) == 8);

struct HistoryJobQueue {
    HistoryJobItem* begin;
    HistoryJobItem* end;
    HistoryJobItem* limit;
    std::uint32_t unknown_0c;
    std::array<HistoryJobItem, 64> items;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(HistoryJobQueue, items) == 0x10);
static_assert(sizeof(HistoryJobQueue) == 0x310);
#endif

// Complete433986B9. Initialization leaves the unknown dword and items alone.
void initialize_history_job_queue(HistoryJobQueue&) noexcept;
std::uint32_t history_job_count(const HistoryJobQueue&) noexcept;
// The two native append gates4341618B..434161EF /434165CF..43416627.
// A rejected append leaves the queue untouched; its caller executes the
// corresponding immediate recording branch, including its different setup.
bool append_history_job(HistoryJobQueue&, bool asynchronous, void* player, float speed_fraction,
                        LagRecord&) noexcept;

// Concrete source worker4336A5B3, defined by the canonical recording runtime.
// It reads the live process policy/cache at execution time, as native does.
#if defined(_M_IX86) || defined(__i386__)
void __fastcall execute_history_record_job(void* item);
#else
void execute_history_record_job(void* item);
#endif

// The dispatcher and its vtables must outlive every submitted task/handle.
// This is the native12-byte specialization, not one of the8/36-byte queues.
class HistoryJobDispatcher {
public:
    HistoryJobDispatcher(void* thread_pool, engine::JobMemory);
    HistoryJobDispatcher(const HistoryJobDispatcher&) = delete;
    HistoryJobDispatcher& operator=(const HistoryJobDispatcher&) = delete;
    engine::JobTask* create(); // 43387448 + complete4341F142
    void dispatch(engine::JobTask&, HistoryJobItem*, std::uint32_t count); // complete433943E6
private:
    struct TaskTable {
        std::array<void*, 4> methods;
        HistoryJobDispatcher* owner;
    } table_;
    std::array<void*, 4> destroyed_;
    void* pool_;
    engine::JobMemory memory_;
    static engine::JobTask* destroy(engine::JobTask&, std::uint8_t flags);
#if defined(_M_IX86) || defined(__i386__)
    static engine::JobTask* __thiscall destruction_entry(void*, std::uint8_t flags);
#else
    static engine::JobTask* destruction_entry(void*, std::uint8_t flags);
#endif
};
// Complete433DF5F6 /4339E700. Every claimed item uses the task's concrete
// worker, increments completed after return, then releases its task reference.
void consume_history_jobs(engine::JobTask&);
// Nonempty local-queue drain434169AD..43416A01. The valid native path requires
// a nonnull task allocation; no failure substitute is introduced. Queue
// pointers retain their native final values.
void dispatch_history_jobs(HistoryJobQueue&, HistoryJobDispatcher&);
} // namespace skeet::lagcomp

// Declarations from history_owner.hpp

namespace skeet::client {
struct EntityObserverRegistry;
}
namespace skeet::lagcomp {
class HistoryOwnerRuntime;
struct HistoryOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::EntityObserverRegistry& entities; //43475BA8
    engine::OwnerRegistryBinding packets; //4347584C, registrar43400957
    engine::OwnerRegistryBinding frame_update; //43475AD8, registrar433FCB12
    ConfigRegistries& configuration;
    events::EventRegistry& events;
};
struct HistoryOwnerFields {
    std::uint32_t packet_player_index; //B6F85C08; zero-based signed integer
    std::uint32_t packet_timing; //F62F1430; two raw dwords
    std::uint32_t local_life_state; //9D920E35, byte
};
struct HistoryOwnerBindings {
    HistoryOwnerRegistries registries;
    // Process bindings are borrowed. The owner makes its lifetime-stable
    // recording/publication bindings with configuration/exponent rebound to
    // its own single storage, retaining the shared cache/pool/job runtime.
    const HistoryRecordingBindings& recording;
    HistoryRecordingScratch* scratch; //explicit fixture input; production is automatic
    BooleanConfig& field_ed048f3d; //fresh canonical descriptor; value survives
    void* const& local_player; //actual43475808, read at each native gate
    engine::NativeConVarReference environment_convar; //4346A850 xor478720C2
    const ShotFeedbackState& feedback; //canonical pending history vector
    const std::int32_t& command_tick; //actual input to4340C3FA
    HistoryOwnerFields fields;
};

// Full4347E5DE owner. Primary4346EFA4 and its three interfaces share this
// one recording worker, configuration and player cache. Scratch is per call.
class HistoryOwnerRuntime {
public:
#if defined(_M_IX86) || defined(__i386__)
    using PrimaryEntry = void(__thiscall*)(void*);
    using PacketEntry = void(__fastcall*)(void*, const void*);
#else
    using PrimaryEntry = void (*)(void*);
    using PacketEntry = void (*)(void*, const void*);
#endif
    const PrimaryEntry* vtable = nullptr;
    HistoryOwnerRuntime(HistoryOwnerRuntime*& active, HistoryOwnerBindings);
    ~HistoryOwnerRuntime(); //4336B1E9 for the owned_crouch string only
    HistoryOwnerRuntime(const HistoryOwnerRuntime&) = delete;
    HistoryOwnerRuntime& operator=(const HistoryOwnerRuntime&) = delete;
    HistoryRecordingRuntime& recording() noexcept {
        return runtime_;
    }
    HistoryRecordingConfiguration& configuration() noexcept {
        return configuration_;
    }
    const float& movement_exponent() const noexcept {
        return movement_exponent_;
    }
    const char* crouch_suffix() const noexcept {
        return crouch_.begin;
    }
    std::int32_t first_activity() const noexcept {
        return first_activity_;
    }
    std::int32_t second_activity() const noexcept {
        return second_activity_;
    }
    void bind_configuration(); //primary1,4347E56C: target-hitbox descriptor
    void observe_packet(const void* packet); //433C00AB
    void update_environment(); //4334832C, frame-update byte is unused
    void on_event(events::GameEventReader&); //433E738A ->433EBBC2
private:
    friend void destroy_history_owner(void*, HistoryOwnerRuntime*&);
    struct PacketEndpoint {
        const PacketEntry* vtable = nullptr;
        HistoryOwnerRuntime* owner;
    };
    struct FrameEndpoint : engine::FrameUpdateEndpoint {
        HistoryOwnerRuntime* owner;
    };
    struct OwnedText {
        char* begin;
        char* end;
        char* limit;
    };
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_HISTORY_MEMBER __thiscall
#define SKEET_HISTORY_FAST __fastcall
#else
#define SKEET_HISTORY_MEMBER
#define SKEET_HISTORY_FAST
#endif
    static void SKEET_HISTORY_MEMBER destroy(void*);
    static void SKEET_HISTORY_MEMBER initialize(void*);
    static void SKEET_HISTORY_MEMBER unchanged(void*);
    static void SKEET_HISTORY_FAST packet(void*, const void*);
    static void SKEET_HISTORY_FAST frame(void*, std::uint8_t);
    static void event(void*, events::GameEventReader&, events::EventPlayerIndices&);
#undef SKEET_HISTORY_MEMBER
#undef SKEET_HISTORY_FAST
    inline static constexpr PrimaryEntry primary_[9]{destroy,   initialize, unchanged,
                                                     unchanged, unchanged,  unchanged,
                                                     unchanged, unchanged,  unchanged};
    inline static constexpr PacketEntry packet_[1]{packet};
    inline static constexpr engine::FrameUpdateEntry frame_[1]{frame};
    HistoryOwnerBindings bindings_;
    HistoryOwnerRuntime** active_;
    HistoryRecordingConfiguration configuration_;
    float movement_exponent_; //native+5C; bitpattern3EDC8172, not event hash
    HistoryVelocityReconstructionBindings physics_;
    HistoryVelocityCaptureBindings velocity_;
    HistoryAnimationPublishBindings publication_;
    HistoryRecordingBindings recording_;
    HistoryRecordingRuntime runtime_;
    std::optional<HistoryRecordingReceiver> receiver_;
    PacketEndpoint packet_endpoint_{nullptr, this};
    FrameEndpoint frame_endpoint_{{nullptr}, this};
    std::int32_t first_activity_, second_activity_;
    OwnedText crouch_;
};

std::size_t history_owner_size() noexcept;
// Constructor entry only: storage carries the real43387448 allocation header
// and is sized by history_owner_size(). This entry leaves active publication
// to its caller; no allocating wrapper, startup position or allocation-failure
// continuation is reconstructed here.
HistoryOwnerRuntime& construct_history_owner(void* storage, HistoryOwnerRuntime*& active,
                                             HistoryOwnerBindings);
//Complete433B5B46. The global is cleared before the null check; only the
//owned string and native allocation are released. Borrowed registries and
//the event triple retain the native common-shutdown ownership.
void destroy_history_owner(void* storage, HistoryOwnerRuntime*& active);
} //namespace skeet::lagcomp

// Declarations from history_receiver.hpp

namespace skeet::lagcomp {
// The four-entry interface registered by4333BF32, whose original table is
//43471D6C. The receiver lives inside its complete HistoryOwnerRuntime and is
//destroyed with that owner. Production recording owns automatic scratch;
//the second constructor is an explicit differential-fixture input seam.
struct HistoryRecordingReceiver : client::NativeEntityObserver {
    HistoryRecordingRuntime* runtime;
    HistoryRecordingScratch* scratch;
    // The enclosing owner performs registration and later table publication
    // at their distinct native constructor cuts, after binding these views.
    explicit HistoryRecordingReceiver(HistoryRecordingRuntime&) noexcept;
    HistoryRecordingReceiver(HistoryRecordingRuntime&, HistoryRecordingScratch&) noexcept;
    void record_players(void* local_player) const;
};
} //namespace skeet::lagcomp

// Declarations from history_resolver_update.hpp

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
struct HistoryResolverInput {
    std::uint8_t previous_resolver_index; // captured before owner+A1 is cleared
    float maximum_yaw; // earlier433EAEEB result, default58 when state is absent
    float initial_speed_fraction; // earlier animation+11C sample
    std::uint32_t player_table_offset; // earlier networkable index *20
};
struct HistoryResolverScratch {
    // Enclosing4340C971 stack+0x328..0x330 (decompiler stack10B8). The original
    // both-point-contents-blocked arm consumes the incoming three words.
    RageVector probe_direction;
};
struct HistoryResolverFields {
    std::uint32_t recoil_pitch_66146802;
    std::uint32_t absolute_velocity_4e66bb34;
    std::uint32_t animation_override_00a5bd68;
    std::uint32_t pose_parameters_1caaf0a8;
    std::uint32_t studio_wrapper;
    std::uint32_t hitbox_set_c2b6fb10;
    std::uint32_t local_time_9f8e7f71;
};
struct HistoryResolverUpdateBindings {
    const EntityStateRuntime& entity;
    // Shared source-owned receiver configuration. The recovered decisions
    // read its live resolver cell and prediction-availability byte at use.
    const HistoryRecordingConfiguration& configuration;
    const std::uint8_t* player_table; // global4347BE08, stride20
    const std::int32_t& selected_entity; // global4346DB1C
    void* const* const& local_time_entity; // live outer pointer cell4346A3A4; nullable
    const float& current_time; // pointed4346A334
    float& animation_time; // pointed43467EB8; stepping intentionally changes it
    const std::int32_t& animation_frame; // pointed434678F8
    const std::uint8_t& shot_angles_active; // global43468D7C
    std::uint8_t* animation_reset_byte; // optional43467B24
    RageVector local_origin; // cached43475BD4..5BDC before the +64 offset
    // The source producer can grow this queue after the receiver is composed.
    // Each native traversal captures its own range at the actual gate below.
    const std::vector<HistoryEvaluation>& pending_shots;
    HistoryResolverFields fields;
    engine::NativeConVarReference recoil_scale_convar; // actual4346794C xorC429E1BF
    const HistoryVelocityReconstructionBindings& velocity; // actual cvars/classification
    const animation::AnimationUpdateEngine& animation;
    const resolver_runtime::EnvironmentEngine& environment;
};
struct HistoryResolverUpdateResult {
    float animation_publish_layer_weight; // native stack13AC, not an address
    float speed_fraction; // native stack1294, updated after each animation step
};
// Complete43411A37..43414F3C. Resolver policy, inline feedback and animation
// stepping execute here; only genuine engine/CRT operations are bound. The
// player and animation state are nonnull. Owner/history are the canonical
// PlayerState storage. Final field restoration precedes the publish worker.
HistoryResolverUpdateResult
update_history_resolver(client::PlayerState&, void* player, void* animation_state,
                        const HistoryAnimationSeed&, SimulationTickSpan, std::int32_t shot_tick,
                        std::uint32_t& update_flags, const HistoryResolverInput&,
                        HistoryResolverScratch&, const HistoryResolverUpdateBindings&);
} // namespace skeet::lagcomp

// Declarations from history_runtime.hpp

namespace skeet::ui {
struct SettingsMenuValues;
}
namespace skeet::lagcomp {
struct HistoryRecordingFields {
    std::uint32_t effects_38ca5228;
    std::uint32_t field_043fa575;
    std::uint32_t field_334b460c;
    std::uint32_t weapon_shot_time_0176a381;
    std::uint32_t animation_enabled_8481ea04;
    std::uint32_t angle_interpolation_fac21eaf;
};
struct HistoryRecordingGlobals {
    const std::int32_t& maximum_players; // *43469300
    const std::int32_t& history_clock; // *43475A60
    const float& current_time; // *4346A334
    float& animation_time; // *43467EB8
    float& frame_time; //43467B04+14
    const std::int32_t& animation_frame; // *434678F8
    const ui::SettingsMenuValues& settings_menu;
    const std::uint8_t& shot_angles_active; //43468D7C
    const std::uint8_t& transition_enabled; //4346754C
    std::uint8_t* animation_reset; //optional43467B24
    const RageVector& local_origin; //43475BD4..5BDC
    players::State& adjustments; //canonical4347BE04 records, settings_base()=BE08
};
struct HistoryHypothesisGlobals {
    engine::NativeConVarReference game_type; //43467A8C xorAC9299CC, integer+30
    const std::int32_t& selected_handle; //4346DB1C
    const HotkeyBinding& hotkey;
    const InputBitmap& keys;
    const std::uint32_t& active_mode_mask; //43467030
    const std::uint8_t& physical_override; //43467D70
    const std::uint32_t& input_flags; //4346A8D0
};
struct HistoryTargetRelationships {
    engine::NativeConVarReference game_type, teammates_are_enemies;
    const rage::TargetOffsets& offsets;
};
struct HistoryRecordingBindings {
    client::PlayerStateCache& players;
    RecordPool& records;
    const EntityStateRuntime& entity;
    HistoryRecordingConfiguration& configuration;
    HistoryActivationBindings activation;
    HistoryTargetRelationships targeting;
    HistoryPreludeOffsets prelude;
    HistoryRecordingFields fields;
    HistoryRecordingGlobals globals;
    HistoryHypothesisGlobals hypotheses;
    HistoryYawSideFmod yaw_side_fmod; //actual external _CIfmod ABI adapter
    const animation::BoneRuntime& bones;
    HistoryJobDispatcher& jobs;
    const HistoryEnvironmentEngine& environment;
    const HistoryVelocityCaptureBindings& velocity;
    const HistoryResolverUpdateBindings& resolver;
    const HistoryAnimationPublishBindings& publication;
    const std::int32_t& maximum_unlag_ms; //43468230
};
struct HistoryRecordingScratch {
    //4340C971 automatic locals. No first-use zero/identity direction exists;
    // both regions are retained between players only within that invocation.
    HistoryEnvironmentScratch environment;
    HistoryResolverScratch resolver;
};

class HistoryRecordingRuntime {
public:
    explicit HistoryRecordingRuntime(const HistoryRecordingBindings& bindings) noexcept
        : bindings_(bindings) {}
    // Production entry owns its current native-style automatic workspace.
    void record_players(void* local_player);
    // Complete4340C971 through its real RET4 at43416A07.
    void record_players(void* local_player, HistoryRecordingScratch&);
    // Concrete4336A5B3 worker and the two synchronous record-build branches.
    void build_record(client::PlayerState&, void* player, const float& speed_fraction, LagRecord&,
                      bool deferred);
    void execute_job(const HistoryJobItem&);

private:
    const HistoryRecordingBindings& bindings_;
    bool hypotheses_enabled(void* player) const;
    void build_live_hypotheses(void* player, client::PlayerState&, float speed_fraction,
                               LagRecord&);
    void publish_record(client::PlayerState&, void* player, const CachedHistoryFrame&,
                        bool recording_active, bool enemy, bool layers_changed,
                        std::uint32_t update_flags, float speed_fraction, HistoryJobQueue&);
};
// One process history owner supplies the same canonical globals to deferred
// workers. Bind once during its initialization; no task-local policy or TLS.
void bind_history_recording_runtime(HistoryRecordingRuntime&) noexcept;
} // namespace skeet::lagcomp

// Declarations from history_selection.hpp

namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet {
struct MovementExtrapolationLiveBindings;
}
namespace skeet::client {
struct CommandBudgetBindings;
}
namespace skeet::lagcomp {
struct HistorySelectionBindings {
    const ExtrapolatedRecordBindings& extrapolation;
    const HistoryAlternativeBindings& alternative;
    const RotationMath& rotation;
    const HistoryMatrixScratch& matrix_scratch;
    float current_time; // actual *43467EB8
    float interpolation; // actual43476928
    std::int32_t maximum_unlag_ms; // actual43468230
};
// Complete4336C53B for a valid nonzero player networkable index. selected_source, if requested,
// mirrors destination.field_00 without truncating the portable source pointer.
bool select_target_history_record(client::CommandFrame&, void* player, std::int32_t requested_tick,
                                  std::uint32_t options, LagRecord& destination,
                                  std::uint32_t& output_flags, const HistorySelectionBindings&,
                                  const LagRecord** selected_source = nullptr);
bool select_target_history_record(client::CommandFrame&, void* player, std::int32_t requested_tick,
                                  std::uint32_t options, LagRecord& destination,
                                  std::uint32_t& output_flags, const HistorySelectionBindings&,
                                  const animation::BoneRuntime&,
                                  const MovementExtrapolationLiveBindings&,
                                  const client::CommandBudgetBindings&,
                                  const LagRecord** selected_source = nullptr,
                                  std::int32_t* predicted_ticks = nullptr);
} // namespace skeet::lagcomp

// Declarations from history_velocity_capture.hpp

namespace skeet::lagcomp {
struct HistoryVelocityCaptureBindings {
    const EntityStateRuntime& entity;
    const HistoryVelocityReconstructionBindings& reconstruction;
    engine::NativeConVarReference maximum_velocity_convar; //43468C2C xor3FE3AE0C
    engine::NativeConVarReference landing_limit_convar; //43467FA0 xor4B52A9BF
};
// Complete velocity owner4340F047..434118B6, excluding the subsequent yaw
// advance. The large physical reconstruction is a concrete source function.
// Both independent engine clock samples, guard restoration and the publish
// versus continue exits retain their native order.
void capture_history_velocity(client::PlayerState&, void* player,
                              const HistoryVelocityCaptureBindings&);
} // namespace skeet::lagcomp

// Declarations from history_velocity_reconstruction.hpp

namespace skeet::client {
struct PlayerState;
class PlayerStateCache;
}
namespace skeet::lagcomp {

struct HistoryVelocityFields {
    std::uint32_t origin; // A9EE6062
    std::uint32_t duck_amount; // 3309C0CF
    std::uint32_t dormant; // D070684C, byte
    std::uint32_t old_simulation_time; // D3D4230D
    std::uint32_t simulation_time; // D5FD2E2C
    std::uint32_t speed_limit; // 3EB71843
    std::uint32_t player_flags; // E2FBE695
    std::uint32_t walking; // 52899DF3, integer
    std::uint32_t walking_override; // 7090D8DD, byte
    std::uint32_t weapon_handle; // CD812654
};
struct HistoryVelocityCvars {
    // Actual ConVar objects: parent at +1C, encoded float at +2C. The
    // accelerate-using-weapon-speed switch additionally reads encoded +30.
    engine::NativeConVarReference gravity; // global43467A54 xorB882FC15
    engine::NativeConVarReference jump_impulse; // global43467990 xorAF1F2C93
    engine::NativeConVarReference stop_speed; // global43467124 xorC5C9925E
    engine::NativeConVarReference friction; // global4346A820 xorEAAEC541
    engine::NativeConVarReference accelerate; // global43467968 xor1632541C
    engine::NativeConVarReference accelerate_weapon_speed; // global43467E28 xorE80C95F9
};
struct HistoryVelocityReconstructionBindings {
    client::PlayerStateCache& players;
    MovementCollisionEngine& trace;
    void* entity_list;
    const float& interval;
    HistoryVelocityFields fields;
    HistoryVelocityCvars cvars;
    // Actual engine functions4346A364/434678C8 and a truth adapter for the
    // genuine imported strstr774FE670. A nonzero result means substring found.
    // Missing sequence_name is the native empty-string path; classify may
    // itself be absent. The comparison string is global43475AC0+4C.
    engine::MemberFunction<std::int32_t, std::int32_t> classify;
    engine::MemberFunction<const char*, std::int32_t> sequence_name;
    int (*compare_sequence_name)(const char*, const char*);
    const char* jump_sequence_name;
    // Actual77509EE0 double atan2 import. Flight and movement arithmetic,
    // the circle fit and all trace decisions remain concrete source code.
    double (*atan2)(double, double);
    float (*sine)(float); // imported7750C6D0
    float (*cosine)(float); // imported7750A770
    std::uint8_t* trace_flag;
    std::uint32_t& update_flags; // enclosing stack+168, may gain8
};

enum class HistoryVelocityReconstructionExit {
    publish, //4340F1BF ->4340F1CB
    continue_capture //43411342 ->4340F1EC
};

//4340F306..43411342, plus the terminal nonpositive-interval arm at11353..11438.
// The caller owns the enclosing433AC5BD guard and restores it on either exit.
// Inputs are the three newest canonical packet samples and live engine state.
// The result stays in velocity; publish also requests its live-player write.
// As in the enclosing native traversal, player has a valid nonzero networkable
// index and every populated canonical history entry addresses a live record.
HistoryVelocityReconstructionExit
reconstruct_history_velocity(client::PlayerState&, void* player, Vector& velocity,
                             const HistoryVelocityReconstructionBindings&);

} // namespace skeet::lagcomp

// Declarations from history_yaw_side.hpp

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
// Genuine msvcrt!_CIfmod7750834A. The x86 import consumes numerator in ST1
// and denominator in ST0 and returns ST0. This binding adapts that external
// ABI to ordinary double arguments; it does not replace an internal helper.
using HistoryYawSideFmod = double (*)(double numerator, double denominator);

// Complete4340E846..4340EACE. Called after the live absolute-origin update,
// before the animation-state-null gate. enemy is the saved4339EF8C result
// (the inverse of native stack+132). Only owner.flags bit4 can change.
// enabled refers to live global4346754C. fmod is a required CRT binding.
void update_history_yaw_side(client::PlayerState&, bool enemy, std::uint32_t update_flags,
                             const std::uint8_t& enabled, HistoryYawSideFmod fmod);
} // namespace skeet::lagcomp

// Declarations from interpolation.hpp

namespace skeet::lagcomp {
using Quaternion = std::array<float, 4>;
struct RotationMath {
    // The actual SSE CRT imports are genuine dependencies. atan2 uses double
    // inputs/result; the other three imports use scalar float XMM arguments.
    float (*sine)(float);
    float (*cosine)(float);
    float (*arc_cosine)(float);
    double (*arc_tangent2)(double, double);
};
Quaternion history_angle_quaternion(const Vector&, const RotationMath&); //4334DF4D
Quaternion align_history_quaternion(const Quaternion&, const Quaternion&) noexcept; //4334A802
Quaternion slerp_history_quaternion(const Quaternion&, const Quaternion&, float,
                                    const RotationMath&); //4334199D
Vector history_quaternion_angles(const Quaternion&, const RotationMath&); //433945FB

// Complete field interpolation4336C968..4336CB3E, before the matrix branch.
// The following native matrix branch reads uninitialized stackA0/A4/A8/B0/B4/B8;
// it is not assigned replacement values by this field interpolation stage.
float interpolate_history_fields(LagRecord& destination, const LagRecord& newer,
                                 const LagRecord& older, float requested_time, float interval,
                                 const RotationMath&, Vector* interpolated_rotation = nullptr);
// Full interpolation eligibility4336C8AE..4336C968. Returns the older source
// record or nullptr. requested_tick is the original caller's tick.
const LagRecord* history_interpolation_source(const RecordRing&, std::uint32_t selected_ordinal,
                                              const LagRecord& selected,
                                              std::int32_t requested_tick,
                                              std::int32_t planned_ticks,
                                              const HistoryTickWindow&) noexcept;
} // namespace skeet::lagcomp

// Declarations from live_record_attachments.hpp

namespace skeet::lagcomp {
struct LiveRecordAttachmentContext {
    const animation::BoneRuntime& bones;
    std::span<Matrix3x4, 256> scratch;
    std::ptrdiff_t field_6e9852d6;
    std::ptrdiff_t field_2a671b42;
    std::ptrdiff_t field_4e66bb34;
    std::ptrdiff_t field_83a6cf2b;
};
// Live43327659. Each4338AC35 obtains its model before scattering and obtains
// it again after the genuine engine attachment update, under the temporary
// entity origin, collision bounds and cached duck/angle state.
void refresh_record_attachments(client::PlayerStateCache&, void* player, LagRecord&,
                                const EntityPreparationContext&,
                                const LiveRecordAttachmentContext&);
// Explicit adapter for an existing numeric context. Its model and update
// fields are not accessed; all bone/model operations use the required runtime.
void refresh_record_attachments(client::PlayerStateCache&, void* player, LagRecord&,
                                const EntityPreparationContext&, const RecordAttachmentContext&,
                                const animation::BoneRuntime&);
} // namespace skeet::lagcomp

// Declarations from live_timeline.hpp

namespace skeet::client {
struct CommandBudgetBindings;
}
namespace skeet::lagcomp {
// Live433F94AA. Calls concrete433A13ED at each native pattern clamp; the
// arrival budget is captured before engine time14. timing.prediction_limit
// belongs to the separate semantic entry and is not read by this overload.
std::int32_t estimate_missing_ticks(const LagTimeline&, const MissingTickTiming&,
                                    const client::CommandBudgetBindings&);
} // namespace skeet::lagcomp

// Declarations from matrix_interpolation.hpp

namespace skeet::lagcomp {
// Retained native scratch layout. The reconstructed inverse transform now
// derives its translation from the current rotation instead of these incoming
// stack values, which were read before initialization in the captured code.
struct HistoryMatrixScratch {
    Vector inverse_row1; // stackA0/A4/A8
    Vector inverse_row2; // stackB0/B4/B8
};
// Complete4336C968..D739, including primary and all three hypothesis matrices.
// Destination is the record already selected/copied by4336C572..C692.
float interpolate_history_record(LagRecord& destination, const LagRecord& newer,
                                 const LagRecord& older, float requested_time, float interval,
                                 const HistoryMatrixScratch&, std::uint32_t& output_flags,
                                 const RotationMath&);
} // namespace skeet::lagcomp

// Declarations from native_history_boundaries.hpp

namespace skeet::lagcomp {
struct HistoryNumericBindings {
    HistoryYawSideFmod fmod;
    HistoryEnvironmentAtan2 environment_atan2;
    double (*atan2)(double, double);
    float (*power)(float, float);
    float (*arccosine)(float);
    int (*contains_sequence_name)(const char*, const char*);
};
// Adapters for the genuine loaded msvcrt imports. No replacement library or
// numeric implementation is selected. Missing exports are explicit errors.
HistoryNumericBindings native_history_numeric_bindings();
// The actual engine movement entry consumes ECX/EDX plus two caller-cleaned
// stack arguments. The animation entry consumes ECX/EDX, XMM0/1/2 and RET4.
// These functions adapt only those external ABIs; the recovered source bodies
// retain every displacement/animation decision and their optional-entry gates.
HistoryAnimationMovementEngine native_history_movement_engine(void* entry);
animation::AnimationUpdateEngine native_history_animation_engine(void* entry);
} //namespace skeet::lagcomp

// Declarations from packet_capture.hpp

namespace skeet::lagcomp {
LagTimeline history_timeline(const client::PlayerState&) noexcept;
struct HistoryPitchOverride {
    bool enabled; // per-player table+4
    std::int32_t pitch; // per-player table+C
};
//4340E58B..E6A1 for an enemy. The resolver branch requires a nonzero
// simulation span; the final pitch override also runs for zero-span enemies.
// The enclosing non-enemy path skips this entire publication block.
void publish_history_eye_angles(client::PlayerState&, void* player, bool resolver_branch,
                                HistoryPitchOverride, const EntityStateRuntime&);
// Complete4340E6A1..E7BF. A changed set of 91 layer words publishes one of
// six packet samples. The engine time query occurs after the first six words
// of the new sample have been written, before receive tick and reference yaw.
void capture_history_packet(client::PlayerState&, bool layers_changed, void* player,
                            const EntityStateRuntime&, std::uint32_t field_e2fbe695_offset);
} // namespace skeet::lagcomp

// Declarations from packet_direction.hpp

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
struct HistoryPacketDirection {
    Vector direction; // owner16C/170/174
    float time; // owner19C
};
// Complete4340EC1A..F047 once the native enemy/mode branch is selected.
// Uses all six packet slots, and can redirect the newest record's horizontal
// velocity while retaining its speed. Returns whether direction/time changed.
bool update_history_packet_direction(HistoryPacketDirection&, const LagTimeline&, RecordRing&,
                                     std::uint32_t update_flags, std::int32_t last_simulation_tick,
                                     float interval) noexcept;
bool update_history_packet_direction(client::PlayerState&, std::uint32_t update_flags,
                                     std::int32_t last_simulation_tick, float interval) noexcept;
} // namespace skeet::lagcomp

// Declarations from prediction_scope.hpp

namespace skeet::lagcomp {
struct PredictionTargets {
    std::uint8_t* in_prediction;
    std::uint8_t* first_time_predicted;
    std::uint8_t* command_predicted;
    std::uint32_t* random_seed;
    std::uint32_t* prediction_player;
    std::uint32_t* player_current_command;
    void* convar_aaa77b8a;
    void* convar_de58ce34;
    std::uint32_t* field_a7f8_08;
    Vector* player_field_6610746e;
};
// ConVar parent is at +0x1C. Integer encodings at +0x2C and +0x3C XOR the
// terminal object's address; the two copies must both be restored.
std::uint32_t read_prediction_convar(void* object) noexcept;
void write_prediction_convar(void* object, std::uint32_t bits) noexcept;

// Complete valid-player path of 433697EE and complete 433E2143. The target
// bindings are the engine globals/netvars resolved by their owning runtime.
// restore() is idempotent and clears every restored target, in native order.
class PredictionScope {
public:
    PredictionScope(const PredictionTargets&, std::uint32_t player_address,
                    std::uint32_t command_address, std::uint32_t seed) noexcept;
    ~PredictionScope() {
        restore();
    }
    PredictionScope(const PredictionScope&) = delete;
    PredictionScope& operator=(const PredictionScope&) = delete;
    void restore() noexcept;

private:
    PredictionTargets targets;
    std::array<std::uint8_t, 3> bytes;
    std::array<std::uint32_t, 6> words;
    Vector vector;
};
} // namespace skeet::lagcomp

// Declarations from receive_timing.hpp

namespace skeet::lagcomp {
// Engine14 returns in ST0 on native x86. Keep that precision through the
// post-call interval read, divide and half-tick addition; round once to float
// before CVTTSS2SI. The scalar receive_time_to_tick entry remains separate.
std::int32_t sample_receive_tick(void* engine, const float& receive_interval);
// Receive proxies reload gpGlobals itself after the engine virtual returns.
std::int32_t sample_receive_tick(void* engine, std::byte* const& live_globals);
} //namespace skeet::lagcomp

// Declarations from record_selection.hpp

namespace skeet::lagcomp {
struct RecordChoice {
    const LagRecord* record = nullptr;
    std::uint32_t ordinal = 0;
};
struct RecordInterpolation {
    bool used = false;
    float fraction = 0;
    Vector angles{};
};
// The native frame reads SP+90/94/98 and SP+A0/A4/A8 before initializing
// them. These are its own opaque automatic bytes, not global/caller seed data
// or an identity matrix. Rebuilding primary matrices publishes the current
// inverse rows; alternatives reuse those rows within the invocation.
struct RecordInterpolationScratch {
    std::array<std::uint32_t, 3> inverse_y;
    std::array<std::uint32_t, 3> inverse_z;
};

// 43344C45..43344D13. Consecutive origins are checked against the cached
// owner+13C origin; the oldest record is retained if no tick crosses the time.
RecordChoice choose_history_record(const RecordRing&, const Vector& cached_origin,
                                   std::int32_t requested_tick, float interval) noexcept;
// 43344D31..43345B83. destination initially contains the selected complete
// record. Both the primary and three alternative matrix sets are rebased from
// the older record; state words and other capture fields retain the new copy.
RecordInterpolation interpolate_selected_record(LagRecord& destination, const RecordRing&,
                                                const RecordChoice&, std::int32_t requested_tick,
                                                const HistoryTickWindow&, float interval,
                                                const RotationMath&, RecordInterpolationScratch&);

struct RecordAttachmentContext {
    animation::BoneSourceReference<const animation::BoneModel> model;
    // Same 256-matrix workspace is reused for main and alternative sets.
    std::span<Matrix3x4, 256> scratch;
    legit::AttachmentUpdate update;
    std::ptrdiff_t field_6e9852d6;
    std::ptrdiff_t field_2a671b42;
    std::ptrdiff_t field_4e66bb34;
    std::ptrdiff_t field_83a6cf2b;
};
struct RecordSelectionContext {
    client::PlayerStateCache& players;
    const EntityPreparationContext& preparation;
    const HistoryTickTiming& timing;
    const RotationMath& rotation;
    RecordInterpolationScratch& interpolation_scratch;
    const RecordAttachmentContext& attachments;
    const std::uint32_t* current_bone_counter;
};
// Complete43327659. The cached preparation writes are temporary, except for
// its cache stamps and record.field_50. Engine attachment changes outside the
// explicitly saved entity fields remain observable.
void refresh_record_attachments(client::PlayerStateCache&, void* target, LagRecord&,
                                const EntityPreparationContext&, const RecordAttachmentContext&);
// Complete generic43344C0C entry. The third native options argument is unused.
bool select_and_apply_history_record(EntityRecordScope&, const RecordSelectionContext&,
                                     std::int32_t requested_tick, std::uint32_t unused_options);
} // namespace skeet::lagcomp

// Declarations from record_selection_live.hpp

namespace skeet::lagcomp {
struct RecordSelectionLiveContext {
    client::PlayerStateCache& players;
    const EntityPreparationContext& preparation;
    const HistoryTickTiming& timing;
    const RotationMath& rotation;
    RecordInterpolationScratch& interpolation_scratch;
    LiveRecordAttachmentContext attachments;
    const std::uint32_t* current_bone_counter;
};
// Numeric scratch/timing aliases remain canonical. Only the model-independent
// attachment fields are projected; the legacy context's model is never read.
RecordSelectionLiveContext live_record_selection_context(const RecordSelectionContext&,
                                                         const animation::BoneRuntime&);
// Whole43344C0C with fresh model reads at the initial apply and final scatter,
// and again before/after each4338AC35 reached by raw43327659.
bool select_and_apply_history_record_live(EntityRecordScope&, const RecordSelectionLiveContext&,
                                          std::int32_t requested_tick,
                                          std::uint32_t unused_options);
// Existing callers can retain canonical numeric bindings while requiring the
// actual target's BoneRuntime. This overload always enters the live owner.
bool select_and_apply_history_record(EntityRecordScope&, const RecordSelectionContext&,
                                     std::int32_t requested_tick, std::uint32_t unused_options,
                                     const animation::BoneRuntime&);
} // namespace skeet::lagcomp

// Declarations from record_validity.hpp

namespace skeet::lagcomp {
inline bool record_time_valid(const LagRecord& record, const HistoryTickTiming& input) noexcept {
    const auto timing = resolve_history_tick_timing(input);
    if (!(record.header.flags & 4u) || !(timing.interval > 0.0f) ||
        !std::isfinite(timing.interval) || !std::isfinite(timing.current_time) ||
        !std::isfinite(timing.latency_198) || !std::isfinite(timing.interpolation) ||
        timing.maximum_unlag_ms < 0)
        return false;
    for (float value : record.header.origin)
        if (!std::isfinite(value))
            return false;
    for (float value : record.header.velocity)
        if (!std::isfinite(value))
            return false;
    const double maximum = double(timing.maximum_unlag_ms) * 0.001;
    const double correction =
        std::clamp(double(timing.latency_198) + timing.interpolation, 0.0, maximum);
    const double sample = double(record.header.simulation_tick) * timing.interval;
    if (std::fabs((double(timing.current_time) - sample) - correction) > 0.2)
        return false;
    const auto clock_tick = std::int64_t(timing.tick_d4) + timing.tick_18c;
    const double clock = double(clock_tick) * timing.interval;
    // Keep fractional seconds. Truncating this clock to whole seconds admits
    // up to almost one extra second of history near a second boundary.
    return sample >= clock - maximum && sample <= clock + timing.interval;
}
inline bool record_precedes_discontinuity(const RecordRing& history, std::int32_t tick) noexcept {
    for (std::uint32_t age = 1; age < history.count; ++age) {
        const auto& newer = history.at(age - 1).header;
        const auto& older = history.at(age).header;
        if (tick >= newer.simulation_tick)
            break;
        float squared = 0.0f;
        for (unsigned axis = 0; axis < 3; ++axis) {
            const auto delta = newer.origin[axis] - older.origin[axis];
            squared += delta * delta;
        }
        if (!std::isfinite(squared) || squared > 4096.0f ||
            newer.simulation_tick <= older.simulation_tick)
            return true;
    }
    return false;
}
}

// Declarations from records.hpp

namespace skeet::lagcomp {
using Vector = std::array<float, 3>;

// 4341D38E/4331ED0B transfer seven dwords from each 0x38-byte engine layer,
// starting at engine layer +0x14. The other engine-layer fields are untouched.
struct AnimationLayerState {
    std::array<std::uint32_t, 7> words;
};
struct AnimationState {
    std::int32_t layer_count;
    std::array<AnimationLayerState, 13> layers;
    std::array<float, 24> pose_parameters;
};
static_assert(sizeof(AnimationState) == 0x1D0);

struct LagRecordHeader {
    std::uint32_t field_00; // 433FD690 leaves this dword untouched.
    std::uint32_t player_history_180;
    std::int32_t receive_tick;
    Vector velocity;
    float collision_max_z;
    std::uint32_t player_flags;
    std::uint32_t field_fb7e3404;
    float field_a2a8247f;
    float animation_yaw;
    std::uint32_t player_history_1ac;
    Vector origin;
    float duck_amount;
    float animation_speed_factor;
    float eye_pitch;
    std::int32_t command_tick;
    std::int32_t simulation_tick;
    float field_50; // Not assigned by the capture entry.
    float weapon_field_6777c9ce;
    std::uint8_t resolver_index;
    std::uint8_t resolver_mode;
    std::uint8_t resolver_state;
    std::uint8_t padding_5b;
    std::uint32_t flags;
};
static_assert(sizeof(LagRecordHeader) == 0x60);
static_assert(offsetof(LagRecordHeader, simulation_tick) == 0x4C);
struct LagRecordHypothesis {
    std::array<std::uint32_t, 2> state;
    std::array<std::byte, 8> padding;
    HitboxMatrices matrices;
};
static_assert(sizeof(LagRecordHypothesis) == 0x3A0);
struct LagRecord {
    LagRecordHeader header;
    AnimationState animation;
    HitboxMatrices matrices;
    std::array<LagRecordHypothesis, 3> alternatives;
};
static_assert(sizeof(LagRecord) == 0x10A0);
static_assert(offsetof(LagRecord, matrices) == 0x230);
static_assert(offsetof(LagRecord, alternatives) == 0x5C0);

// Complete assignment entries 43393CBC,4336A43E,433B7333 and copy constructor
// 433D68DA. Assignment preserves the proven padding holes in the destination.
void copy_lag_record(LagRecord& destination, const LagRecord& source) noexcept;
struct EvaluationState {
    std::array<std::uint32_t, 0x3B> words;
    std::uint32_t padding_ec;
    LagRecord record;
};
static_assert(sizeof(EvaluationState) == 0x1190);
struct HistoryEvaluation {
    std::array<std::uint32_t, 0x22> words;
    std::array<std::byte, 8> padding_88;
    EvaluationState state;
};
static_assert(sizeof(HistoryEvaluation) == 0x1220);
void assign_evaluation(HistoryEvaluation&, const HistoryEvaluation&) noexcept;
void copy_construct_evaluation(HistoryEvaluation&, const HistoryEvaluation&) noexcept;

// 4341D38E and 4331ED0B. Engine layer stride is 0x38, pose count is 24.
void capture_animation(AnimationState&, const void* layers, std::int32_t layer_count,
                       const float* pose_parameters) noexcept;
void restore_animation(const AnimationState&, void* layers, std::int32_t layer_count,
                       float* pose_parameters) noexcept;
std::int32_t time_to_tick(float time, float interval) noexcept;
// Engine-time callers retain the x87 quotient until one final float store.
std::int32_t receive_time_to_tick(float time, float interval) noexcept;
std::int32_t corrected_command_tick(std::int32_t tick, const std::int32_t* previous,
                                    const std::int32_t* current) noexcept; // 4340C3FA
} // namespace skeet::lagcomp

// Declarations from resolver_detection.hpp

namespace skeet::lagcomp {
struct ResolverDetectionState {
    bool enabled; // per-player table+6
    float yaw_duration_1c0;
    float pitch_duration_178;
    std::uint16_t field_a6;
    resolver_runtime::SelectionState selection;
};
struct ResolverDetectionInput {
    std::int32_t simulation_ticks;
    float interval;
    float previous_yaw_118;
    float observed_yaw_1ac;
    float pitch;
    const LagTimeline& packets;
};
// Complete4334D33C. Only the most recent packet is inspected.
bool newest_packet_timing_discontinuity(const LagTimeline&) noexcept;
// Complete4340D65E..4340D89F. Sustained yaw change>40 or pitch>70 enables
// resolver handling after1.2 seconds. The native packet condition is a separate
// immediate gate; it does not reset timers or publish a new attempt.
bool update_resolver_detection(ResolverDetectionState&, const ResolverDetectionInput&) noexcept;
} // namespace skeet::lagcomp

// Declarations from timeline.hpp

namespace skeet::lagcomp {
struct PacketTimingSample {
    std::int32_t field_00;
    std::array<std::uint32_t, 5> fields_04;
    std::int32_t field_18;
    std::uint32_t field_1c;
};
struct LagTimeline {
    float simulation_time; // player history +0x110
    std::int32_t last_update_tick; // +0x168
    std::uint32_t packet_head; // +0x234
    std::uint32_t packet_count; // +0x238
    std::array<PacketTimingSample, 6> packets;
    std::uint32_t record_head; // +0x5D4
    std::uint32_t record_count; // +0x5D8
    std::array<const LagRecord*, 32> records;
    const LagRecord& at(std::uint32_t age) const noexcept {
        return *records[(record_head + age) & 31u];
    }
};
struct MissingTickTiming {
    std::int32_t tick_d4;
    std::int32_t tick_18c;
    float interval;
    float receive_interval; // globals +0x20, used for engine[14] time conversion
    // Complete 433A13ED result. Nonpositive configured values are retained.
    std::int32_t prediction_limit;
    void* engine; // engine vtable[12] max clients, [14] time
    void* player; // networkable(+8) vtable[10] index
};
// Complete 433FC6C6 and 433F94AA. A ring contains at most 32 nonnull records;
// packet_count is at most 6. The original callers maintain these invariants.
// The forward extrapolation pattern requires a positive prediction_limit.
bool packet_timing_discontinuity(const LagTimeline&) noexcept;
std::int32_t estimate_missing_ticks(const LagTimeline&, const MissingTickTiming&);
// Complete 433A13ED, after resolving its three entity-property offsets.
std::int32_t prediction_tick_limit(bool local_available, bool field_bf567619, bool field_dda99ab5,
                                   std::int32_t field_b7ea8fda, std::int32_t configured) noexcept;
} // namespace skeet::lagcomp

// Declarations from update_capture.hpp

namespace skeet::lagcomp {
struct HistoryPreludeOffsets {
    std::uint32_t life_state_9d920e35;
    std::uint32_t suppressed_cde290d2;
    std::uint32_t previous_time_1599541f;
    std::uint32_t weapon_definition_5c9f0fbb;
};
// Complete433255B6 for a valid player. Updates the live timestamp even when
// the life-state/weapon gates retain the previous cached definition.
void capture_history_display_metadata(client::PlayerState&, void* player, const EntityStateRuntime&,
                                      const HistoryPreludeOffsets&);
// Complete4340D000..4340D06F. The actual collideable virtuals supply height;
// the captured origin precedes those calls, as in the native writer.
void cache_history_entity_fields(client::PlayerState&, void* player, const EntityStateRuntime&);
struct CachedHistoryFrame {
    bool ready = false;
    std::int32_t receive_tick = 0;
    void* animation_state = nullptr;
    float animation_speed_fraction = 0;
    SimulationTickSpan ticks{};
};
// Complete per-player4340D000..4340D24A, including the dead/suppressed clear
// exit4341695A..43416991. Caller already passed player/info/dormancy gates.
// This works with the shared0x1700-byte player cache, its single RecordRing,
// and the existing global record pool rather than another player-state owner.
CachedHistoryFrame prepare_cached_history_frame(client::PlayerState&, RecordPool&, void* player,
                                                bool is_local_player, const EntityStateRuntime&,
                                                const HistoryPreludeOffsets&);
} // namespace skeet::lagcomp

// Declarations from update_timing.hpp

namespace skeet::lagcomp {
struct SimulationTickSpan {
    std::int32_t first, last, count;
};
// Complete4340D1B4..4340D24A. Differences wrap as32-bit integers, clamp0..16,
// and move the beginning forward when a long simulation gap was clamped.
SimulationTickSpan simulation_tick_span(float previous_time, float simulation_time,
                                        float interval) noexcept;

struct SameTickHistoryState {
    std::int32_t simulation_override_receive_tick; // owner+1E4
    std::int32_t simulation_override_tick; // +1E0
    std::int32_t velocity_receive_tick; // +1DC
    Vector velocity; // +1C8
};
// Complete nonlocal correction4340D0DF..4340D16A, before the independently
// recovered player display-cache capture. The history/player simulation fields
// are written together; velocity has its own receive-tick equality gate.
struct SameTickRestoreWrites {
    bool simulation = false, velocity = false;
};
SameTickRestoreWrites restore_same_tick_history_fields(const SameTickHistoryState&,
                                                       std::int32_t receive_tick, float interval,
                                                       float& history_simulation_time,
                                                       float& player_simulation_time,
                                                       Vector& player_velocity) noexcept;

struct UpdateDiscontinuity {
    std::int32_t saved_receive_tick_1a4;
    std::int32_t receive_tick;
    std::uint32_t entity_effects_38ca5228;
    std::uint8_t field_043fa575;
    std::uint8_t field_334b460c;
    std::int32_t velocity_receive_tick_1dc;
    std::int32_t next_receive_tick;
};
// Complete4340D2B2..4340D357; engine[14] receive ticks are independently
// sampled at4340D296 and4340D32B, so the two input ticks remain distinct.
std::uint32_t initial_history_update_flags(const UpdateDiscontinuity&) noexcept;

bool history_weapon_is_firearm(std::uint16_t identifier) noexcept; // complete433D24BB
struct HistoryWeaponShot {
    bool available;
    float time;
    std::uint16_t identifier;
};
// Complete4340D357..4340D44C after the actual entity-list/weapon property
// lookup. Returns the new shot tick or-1. No-update and failed weapon lookup
// retain exactly the native flag bits rather than clearing an unrelated state.
std::int32_t update_history_shot_flags(std::uint32_t& owner_flags, std::uint32_t& update_flags,
                                       SimulationTickSpan, HistoryWeaponShot,
                                       float interval) noexcept;

struct CollisionTransition {
    std::int32_t received_updates_1d4;
    float saved_collision_height_11c;
    float field_12c;
    float field_148;
};
// Numeric write branch4340D494..4340D4F2. Caller invokes player[77] only when
// received_updates>1 and span.count!=0, as in4340D44C..4340D494.
void update_history_collision_transition(CollisionTransition&, SimulationTickSpan,
                                         float current_collision_height, float origin_z,
                                         float previous_time, float interval) noexcept;
// Complete4340D548..4340D577, comparing the13 captured seven-dword layers.
bool history_animation_layers_changed(const std::array<AnimationLayerState, 13>& current,
                                      const std::array<AnimationLayerState, 13>& previous) noexcept;
} // namespace skeet::lagcomp

// Declarations from velocity.hpp

namespace skeet::lagcomp {
// 4341143D..434115E3, before record/ground-dependent speed constraints.
// native starts with260 unless its preceding weapon query supplies maximum_speed.
// blend is owner+5A8; interpolated_factor is the preceding stack+D0 result.
void adjust_capture_velocity(Vector&, float blend, float maximum_speed,
                             float interpolated_factor) noexcept;
} // namespace skeet::lagcomp

// Declarations from velocity_validation.hpp

namespace skeet::lagcomp {
// 433E4F87. NaN components become positive zero. Otherwise clamp against
// maximum, then its sign-bit-flipped lower bound; true means no store occurred.
// Ordered SSE comparisons preserve the native NaN-bound and DAZ behavior.
bool validate_velocity_component(float& value, float maximum) noexcept;

// 4332FB8A's component pass. All three components are visited in x/y/z order,
// even when an earlier component was invalid; true means all were unchanged.
bool validate_velocity(Vector& velocity, float maximum) noexcept;

// Complete 4332FB8A with its ConVar dependency supplied by the caller.
// maximum_convar is the already decoded 43468C2C ^ 3FE3AE0C object. Follow
// parent +1C to a self-parent, then XOR encoded float +2C with its low address.
// Resolve the maximum once before reading or updating any vector component.
bool validate_velocity(Vector& velocity, const void* maximum_convar) noexcept;
} // namespace skeet::lagcomp
