#pragma once

// Bone Setup & Animation Fix Subsystem Declarations
#include "skeet/animation/bones.hpp"
#include "skeet/engine/interface_call.hpp"
#include <functional>
#include <variant>
#include <stdexcept>
#include "skeet/rage_hitboxes.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/client/startup_reset.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/client/player_data_update.hpp"
#include "skeet/engine/hooks.hpp"
#include "skeet/network/data_update_observers.hpp"
#include "skeet/network/update_observers.hpp"
#include <array>
#include "skeet/engine/offset_storage.hpp"
#include <memory>
#include "skeet/engine/native_data_image.hpp"
#include <cstdint>
#include "skeet/client/commands.hpp"
#include <cstddef>
#include "skeet/client/command_frame.hpp"
#include "skeet/anti_aim/movement_capture.hpp"
#include "skeet/movement/simulation_environment.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/client/input_commands.hpp"
#include "skeet/core.hpp"
#include "skeet/animation/state_copy.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/animation/entity_update.hpp"
#include <cmath>
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/animation/remote_setup_bones.hpp"
#include "skeet/animation/native_trigonometry.hpp"
#include "skeet/animation/runtime_bindings.hpp"
#include "skeet/animation/remote_animation_preparation.hpp"


// Declarations from bone_runtime.hpp

namespace skeet::animation {
// Decoded entity fields used by43388720. Layer storage is a native vector:
// data at+0, count at+C, stride38; layer12's weight is at+2C0.
struct BoneRuntimeFields {
    std::ptrdiff_t model_index_a25d8271;
    std::ptrdiff_t studio_wrapper_32e4daf1;
    std::ptrdiff_t interpolation_flags_57b2d535;
    std::ptrdiff_t animation_frame_ae5b982e;
    std::ptrdiff_t effects_38ca5228;
    std::ptrdiff_t setup_flags_bb78dc43;
    std::ptrdiff_t owner_handle_cd812654;
    std::ptrdiff_t related_handle_ca2b5ffb;
    std::ptrdiff_t animation_state_3d52a1a7;
    std::ptrdiff_t ik_mask_01d0bacc;
    std::ptrdiff_t animation_layers_b538bba5;
    std::ptrdiff_t angles_3b1f6ecc;
    std::ptrdiff_t angles_f8205eba;
    std::ptrdiff_t origin_2a671b42;
    std::ptrdiff_t origin_4e66bb34;
    std::ptrdiff_t entity_flags_83a6cf2b;
    std::ptrdiff_t bone_origin_a9ee6062;
    std::ptrdiff_t hitbox_set_c2b6fb10;
    std::ptrdiff_t bone_accessor_54783141;
    std::ptrdiff_t pose_parameters;
};

using BoneAttachmentUpdate = engine::MemberFunction<void, Matrix3x4*, std::uint32_t>;

// Owns only the small hitbox-index projection; studio arrays remain live engine
// storage. Call model() after constructing the context so its hitbox span points
// into this object. A negative native hitbox count is retained in hitbox_count.
struct BoneModelContext {
    const void* wrapper = nullptr;
    const void* header = nullptr;
    bool virtual_model_required = false;
    bool virtual_model = false;
    std::span<const std::uint32_t> flags;
    std::span<const std::int32_t> parents;
    std::int32_t hitbox_count = 0;
    std::array<std::uint32_t, 19> hitbox_bones{};
    BoneModel model() const noexcept;
};

struct BoneRuntime {
    BoneRuntimeFields fields;
    void* entity_list;
    // Optional genuine engine entry43468F80, as tested by43341E47.
    BoneAttachmentUpdate update_attachments;
    const RageTrigonometry& trigonometry;

    // Complete433FD30D and studio/hitbox-set descriptor projection. Every call
    // reads the supplied player's current wrapper, header and selected set.
    BoneModelContext load_model(const void* player) const;
    std::span<float, 24> pose_parameters(void* player) const noexcept;

    // Complete live43388720 transaction. Count is zeroed before the renderable
    // model virtual and published after owner/IK resolution, exactly natively.
    // Engine pose accumulation is the actual entity virtual+338 (slot206).
    bool build_bone_matrices(void* player, const BoneRequest&, std::span<Matrix3x4>,
                             std::int32_t* count = nullptr) const;
    // Native callers can supply the player's cache pointer directly. Its
    // output extent is formed from the same fresh hierarchy read used by the
    // build body, after pose accumulation; no early model/count is sampled.
    bool build_bone_matrices(void* player, const BoneRequest&, Matrix3x4* output,
                             std::int32_t* count = nullptr) const;
    // Complete433899B3, with a fresh studio/set read after the preceding build.
    std::int32_t select_hitbox_bone_matrices(const void* player, std::span<const Matrix3x4>,
                                             std::span<Matrix3x4>) const;
    // Complete43407EA3 and43341E47. Scatter leaves all other scratch matrices
    // intact; attachment updates temporarily force only the two accessor masks.
    void scatter_hitbox_bone_matrices(const void* player, std::span<const Matrix3x4> hitboxes,
                                      std::span<Matrix3x4> bones) const;
    void scatter_hitbox_bone_matrices(const void* player, std::span<const Matrix3x4> hitboxes,
                                      Matrix3x4* bones) const;
    void update_bone_attachments(void* player, std::span<Matrix3x4>, std::uint32_t mask) const;
    // Complete4338AC35: scatter, genuine attachment update, then a fresh model
    // selection. The engine may change the selected studio/set during update.
    std::int32_t refresh_hitbox_attachments(void* player, std::span<Matrix3x4, 19>,
                                            std::span<Matrix3x4, 256> scratch) const;
    // Complete433FF63C: fresh A9EE6062 origin, yaw-only angles, zero time,
    // mask80300, all bones, then a fresh433899B3 model selection.
    std::int32_t build_resolver_hitbox_matrices(void* player, float yaw,
                                                std::span<Matrix3x4>) const;
};
} // namespace skeet::animation

// Declarations from bone_source_reference.hpp

namespace skeet::animation {
struct BoneRuntime;
// A production context names its actual raw bone runtime. Decoded fixture
// inputs exist only in the other arm and cannot be accidentally consumed by
// production. Copies preserve the selected source and never fabricate a model.
template <class T> class BoneSourceReference {
    std::variant<std::reference_wrapper<T>, std::reference_wrapper<const BoneRuntime>> source_;

public:
    BoneSourceReference(T& decoded) noexcept : source_(std::ref(decoded)) {}
    BoneSourceReference(const BoneRuntime& live) noexcept : source_(std::cref(live)) {}
    T& decoded() const {
        if (auto* value = std::get_if<std::reference_wrapper<T>>(&source_))
            return value->get();
        throw std::logic_error(
            "A live bone context cannot be consumed through its decoded fixture entry");
    }
    const BoneRuntime* live() const noexcept {
        if (auto* value = std::get_if<std::reference_wrapper<const BoneRuntime>>(&source_))
            return &value->get();
        return nullptr;
    }
};
} //namespace skeet::animation

// Declarations from bones.hpp

namespace skeet::animation {
using BoneQuaternion = std::array<float, 4>;
inline constexpr unsigned maximum_bones = 256;

struct BoneModel {
    // Engine/model readiness checks at 4338875F..433887C5.
    bool renderable_model;
    std::int32_t model_index;
    bool studio_header;
    bool virtual_model_required;
    bool virtual_model;
    std::span<const std::uint32_t> flags;
    std::span<const std::int32_t> parents;
    std::span<const std::uint32_t> hitbox_bones;
};
struct BoneEntityState {
    std::uint8_t interpolation_flags;
    std::uint32_t animation_frame;
    std::uint32_t effects;
    std::uint32_t setup_flags;
    std::uint32_t ik_mask;
    std::uint32_t entity_flags;
    std::array<RageVector, 2> angles;
    std::array<RageVector, 2> origins;
    // Layer 12 +20, touched only when the native layer count exceeds 12.
    float last_layer_weight;
    std::int32_t layer_count;
    // Native optional owner/IK dependencies resolved through entity handles.
    std::uint32_t* owner_setup_flags = nullptr;
    std::uintptr_t* ik_owner_slot = nullptr;
    std::uintptr_t ik_owner = 0;
    std::uint32_t readable_bones;
    std::uint32_t writable_bones;
};
struct BoneRequest {
    RageVector angles;
    RageVector origin;
    float time;
    std::uint32_t mask;
    std::int32_t starting_bone;
    std::uint32_t options;
    bool attachments;
};
struct BoneEngine {
    void* context;
    // Genuine engine virtual +338 called through 433E533F. This supplies local
    // poses, rather than replacing any recovered matrix-building routine.
    void (*accumulate_pose)(void*, BoneEntityState&, const BoneModel&,
                            std::span<RageVector, maximum_bones>,
                            std::span<BoneQuaternion, maximum_bones>, float, std::uint32_t);
    // Genuine engine target 43468F80; optional exactly as in 43341E47.
    void (*update_attachments)(void*, BoneEntityState&, std::span<Matrix3x4>,
                               std::uint32_t) = nullptr;
};

// Complete 433E4ADF; the first column deliberately uses double arithmetic.
Matrix3x4 bone_local_matrix(const RageVector&, const BoneQuaternion&) noexcept;
// Complete 433CB715. Valid studio arrays have at most 256 bones, parent-before-
// child order, an acyclic parent chain, and matching pose/output spans. Unmasked
// output matrices retain their previous contents, including skipped parents.
void compose_bone_hierarchy(const BoneModel&, const Matrix3x4& root, std::span<const RageVector>,
                            std::span<const BoneQuaternion>, std::int32_t starting_bone,
                            std::uint32_t mask, std::span<Matrix3x4> output) noexcept;
// Complete native 43388720 transaction, including owner/IK/layer state restore.
// accumulate_pose and CRT trig bindings are required engine dependencies.
bool build_bone_matrices(BoneEntityState&, const BoneModel&, const BoneRequest&,
                         std::span<Matrix3x4>, const BoneEngine&, const RageTrigonometry&);
// 433899B3 selects at most 19 matrices through model hitbox bone indices.
std::int32_t select_hitbox_bone_matrices(const BoneModel&, std::span<const Matrix3x4>,
                                         std::span<Matrix3x4>) noexcept;
// Complete 433FF63C: zero time, yaw-only angles, 80300 mask, all bones, then
// hitbox matrix selection. Nonpositive count is the resolver failure condition.
std::int32_t build_resolver_hitbox_matrices(BoneEntityState&, const BoneModel&,
                                            const RageVector& entity_origin, float yaw,
                                            std::span<Matrix3x4> hitboxes, const BoneEngine&,
                                            const RageTrigonometry&);
} // namespace skeet::animation

// Declarations from entity_update.hpp

namespace skeet::animation {
// Complete433F526C: clear five dwords at13C; every other byte is preserved.
void initialize_animation_state(void* animation_state) noexcept;

struct AnimationAngleSample {
    std::int32_t frame;
    std::array<float, 2> angles; // pitch,yaw
};
static_assert(sizeof(AnimationAngleSample) == 12);
struct AnimationAngleHistory {
    bool active; //43468D7C
    std::int32_t current_frame; // pointed global43475B2C
    std::array<AnimationAngleSample, 2> samples; //434759DC,434759E8
};
// Production history has independent data cells and an indirect frame cell.
// The standalone aggregate remains useful for explicit isolated inputs.
class AnimationAngleHistoryView {
public:
    AnimationAngleHistoryView(const AnimationAngleHistory& value) noexcept
        : active_(&value.active), frame_(&value.current_frame), frame_slot_(nullptr),
          samples_(value.samples) {}
    AnimationAngleHistoryView(AnimationAngleHistory&&) = delete;
    AnimationAngleHistoryView(const std::uint8_t& active, const std::int32_t* const& frame,
                              const std::array<AnimationAngleSample, 2>& samples) noexcept
        : active_(&active), frame_(nullptr), frame_slot_(&frame), samples_(samples) {}
    bool active() const noexcept;
    std::int32_t frame() const noexcept {
        return frame_slot_ ? **frame_slot_ : *frame_;
    }
    std::span<const AnimationAngleSample, 2> samples() const noexcept {
        return samples_;
    }

private:
    const void* active_;
    const std::int32_t* frame_;
    const std::int32_t* const* frame_slot_;
    std::span<const AnimationAngleSample, 2> samples_;
};
// Complete43384C9E. First eligible sample wins; roll and failed output retain
// their original bits. The comparisons use signed frame numbers.
bool select_animation_angles(const AnimationAngleHistory&, std::int32_t requested_frame,
                             RageVector& angles) noexcept;
bool select_animation_angles(AnimationAngleHistoryView, std::int32_t requested_frame,
                             RageVector&) noexcept;

struct AnimationUpdateEngine {
    void* context;
    // Genuine optional engine entry434693F0. Its native x86 entry receives
    // ECX=state,EDX=0,XMM0=zero,XMM1=yaw,XMM2=pitch and stack int32(force).
    // The binding adapts this external engine ABI, not an internal helper.
    void (*update_state)(void* context, void* state, float zero, float yaw, float pitch,
                         std::int32_t force);
};
struct AnimationUpdateTiming {
    std::int32_t frame; // pointed global434678F8
    float current_time; // pointed global43467EB8
    float interval; // pointed global4346A338
    std::uint8_t* reset_byte; // optional43467B24, cleared after the engine call
    std::uint8_t* const* reset_byte_slot = nullptr; // production rereads43467B24 after the call
    void clear_reset_byte() const noexcept {
        auto* const reset = reset_byte_slot ? *reset_byte_slot : reset_byte;
        if (reset)
            *reset = 0;
    }
};
// Complete433A0F80. Optional engine entry absence leaves every field intact.
// The caller supplies the actual0x348-byte animation state (or its saved copy).
void update_animation_state(void* animation_state, const RageVector& angles,
                            const AnimationUpdateTiming&, const AnimationUpdateEngine&);

// Entity-facing4341D38E/4331ED0B use the canonical1D0-byte lagcomp snapshot.
// Layer descriptor stores data at0/count atC; each engine layer has stride38.
void capture_entity_animation(lagcomp::AnimationState&, const void* entity,
                              const lagcomp::EntityNetvars&) noexcept;
void restore_entity_animation(const lagcomp::AnimationState&, void* entity,
                              const lagcomp::EntityNetvars&) noexcept;
} // namespace skeet::animation

// Declarations from fix_animation_backup.hpp

namespace skeet::animation {
struct FixAnimationBackupGrowth {
    float maximum_load; //4346D6F0, native initial1
    float multiplier; //4346D6F4, native initial2
    std::uint32_t next_resize; //4346D6F8
};
static_assert(sizeof(FixAnimationBackupGrowth) == 12);
struct FixAnimationBackupInsert {
    FixAnimationBackupNode* node;
    FixAnimationBackupNode** bucket;
    bool inserted;
};
// References are the SAME4346D6E4/E8/EC and policy words read by
// FixAnimationsRuntime::restore_animation_backup. There is no second map.
// Native initial buckets point at{nullptr,FFFFFFFF}, count1,size0.
struct FixAnimationBackupBindings {
    FixAnimationBackupNode**& buckets;
    std::uint32_t& bucket_count;
    std::uint32_t& size;
    FixAnimationBackupGrowth& growth;
    const std::int32_t& mode; //same4347C9EC transition status
    const std::int32_t& prediction_frame; //43467B04+4
    const BoneRuntime& bones;
    FixAnimationEntityView entity;
    std::ptrdiff_t life_state_9d920e35;
    engine::JobMemory memory;
    // Actual recorded CRT ceil77512A30. This is an import, not a map helper.
    double (*ceiling)(double);
};
struct FixAnimationBackupDetachBindings {
    engine::VtableHookRegistry& hooks; //same UnloadState::hooks sentinel
    const std::ptrdiff_t& extra_interface_offset; //43479CE4
    std::byte* player_settings; //4347BE00, stride20; marker at+4
    client::TargetResetState target; //canonical shared AA/command globals
};

class FixAnimationBackupRuntime {
public:
    explicit FixAnimationBackupRuntime(const FixAnimationBackupBindings& bindings,
                                       const FixAnimationBackupDetachBindings& detach) noexcept
        : bindings(bindings), detach(detach) {}
    const FixAnimationBackupBindings& bindings;
    const FixAnimationBackupDetachBindings& detach;

    // Full43329CDF/433C140F/4332C319; callbacks are only actual heap imports.
    FixAnimationBackupNode* create_node(const std::uint32_t* key);
    FixAnimationBackupInsert find_or_insert(const std::uint32_t* key);
    void rehash(std::uint32_t bucket_count);
    // Full43393973. The live animation pointer is required under its native
    // nonzero-mode/alive contract. Capture clears the borrowed vector header.
    void capture(void* player);
    // Full433CB965..433CBAD2: unhook three actual interface getters, optional
    // extra interface and base entity, capture, player marker, target reset.
    void detach_entity(void* player);

private:
    struct GrowthDecision {
        bool rehash;
        std::uint32_t bucket_count;
    };
    GrowthDecision plan_growth(std::uint32_t buckets, std::uint32_t size);
    void* allocate(std::size_t);
    void free(void*);
};

// Called by the same preserved initialization/hook bridge that routes the
// ordinary captured43393973/433CB965 entries to their concrete source bodies.
// Binding does not introduce a new frame phase or reinterpret VM operands.
void bind_fix_animation_backup(FixAnimationBackupRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __fastcall capture_fix_animation_backup(void* player, void* ignored_edx);
void __fastcall detach_fix_animation_entity(void* player, void* ignored_edx);
#else
void capture_fix_animation_backup(void* player, void* ignored_edx);
void detach_fix_animation_entity(void* player, void* ignored_edx);
#endif
} // namespace skeet::animation

// Declarations from fix_animation_backup_cleanup.hpp

namespace skeet::client {
struct ExitCallbacks;
class NativeStartupSources;
class ClientRuntimeStorage;
}
namespace skeet::animation {
// The cleanup body consumes only these process map/heap cells. Engine fields,
// pose accumulation, hooks and renderer opacity belong to later producers.
struct FixAnimationBackupMapBindings {
    FixAnimationBackupNode**& buckets;
    std::uint32_t& bucket_count;
    std::uint32_t& size;
    engine::JobMemory memory;
};
// Complete4339D47B over the SAME canonical map used by capture/restore.
// The allocator reference names live43467204, so each node observes changes
// made by the preceding genuine engine/free call. Native leaves the bucket
// pointer/count/growth words intact after releasing their storage.
void cleanup_fix_animation_backups(FixAnimationBackupRuntime&, void* const& allocator);
void cleanup_fix_animation_backups(const FixAnimationBackupMapBindings&, void* const& allocator);

class FixAnimationBackupCleanupContext {
public:
    explicit FixAnimationBackupCleanupContext(client::ClientRuntimeStorage&);
    FixAnimationBackupCleanupContext(client::ExitCallbacks& callbacks,
                                     FixAnimationBackupRuntime& backups,
                                     void* const& allocator) noexcept
        : callbacks(callbacks), allocator(allocator),
          map_{backups.bindings.buckets, backups.bindings.bucket_count, backups.bindings.size,
               backups.bindings.memory} {}
    FixAnimationBackupCleanupContext(const FixAnimationBackupCleanupContext&) = delete;
    FixAnimationBackupCleanupContext& operator=(const FixAnimationBackupCleanupContext&) = delete;
    FixAnimationBackupCleanupContext(FixAnimationBackupCleanupContext&&) = delete;
    FixAnimationBackupCleanupContext& operator=(FixAnimationBackupCleanupContext&&) = delete;

    client::ExitCallbacks& callbacks; //shared512slots/count/mask
    void* const& allocator;
    const FixAnimationBackupMapBindings& map() const noexcept {
        return map_;
    }
    // Late capture/restore must publish into the very map whose finalizer was
    // already registered during CRT. This performs no registration or calls.
    void require_producer(const FixAnimationBackupRuntime&) const;
    void finalize(); //43359C18 ->4339D47B
    void (*source_finalizer(std::uint32_t native_address))();

private:
    FixAnimationBackupMapBindings map_;
};

// Actual initializer433B86B4 at operand0A4 only appends43359C18 through the
// existing433837F5 implementation. The live startup scheduler determines its
// registration order; this binder does not run or append any other finalizer.
// Context, producer runtime and borrowed globals must outliveExitCallbacks::run.
void bind_fix_animation_backup_exit_initializer(client::NativeStartupSources&,
                                                FixAnimationBackupCleanupContext&);
} // namespace skeet::animation

// Declarations from fix_animation_entity.hpp

namespace skeet::animation {
// Fix hooks use these five offsets and the two engine interfaces. They do
// not consume EntityStateRuntime's frame/command/time transaction snapshot.
struct FixAnimationEntityFields {
    std::uint32_t field_0471e13c;
    std::uint32_t field_432b1f5b;
    std::uint32_t field_0d560fac;
    std::uint32_t player_flags;
    std::uint32_t pose_parameters;
};
struct FixAnimationEntityView {
    void* engine;
    void* entity_list;
    FixAnimationEntityFields offsets;
    FixAnimationEntityView(void* engine, void* entity_list,
                           FixAnimationEntityFields fields) noexcept
        : engine(engine), entity_list(entity_list), offsets(fields) {}
    // Existing callers with a real transaction runtime preserve their exact
    // interface/field identities, without retaining its clock snapshot.
    FixAnimationEntityView(const lagcomp::EntityStateRuntime& entity) noexcept
        : engine(entity.engine), entity_list(entity.entity_list),
          offsets{entity.offsets.field_0471e13c, entity.offsets.field_432b1f5b,
                  entity.offsets.field_0d560fac, entity.offsets.player_flags,
                  entity.offsets.pose_parameters} {}
};
} //namespace skeet::animation

// Declarations from fix_animations.hpp

namespace skeet::effects {
struct PlayerOpacityBindings;
}
namespace skeet::animation {
struct FixAnimationFields {
    std::ptrdiff_t life_state_9d920e35;
    std::ptrdiff_t simulation_time_d5fd2e2c;
    std::ptrdiff_t previous_time_1599541f;
    std::ptrdiff_t replicated_time_d3d4230d;
    std::ptrdiff_t body_yaw_a2a8247f;
    std::ptrdiff_t client_animation_8481ea04;
    std::ptrdiff_t suppressed_cde290d2;
    std::ptrdiff_t move_type_7501cb91;
    std::ptrdiff_t observer_mode_2f4e97c4;
    std::ptrdiff_t observer_target_3b704b41;
    std::ptrdiff_t view_offset_c246315e;
    std::ptrdiff_t animation_frame_74019d87;
    std::ptrdiff_t transient_6e9852d6;
    std::ptrdiff_t eye_angles_c1b66ee8;
    std::ptrdiff_t resource_alive_d266b537;
};
// The actual4346D6E4 bucket chains consumed by433C7FC5. The final bucket is
// the sentinel. This is borrowed native/source-owned storage, not a new cache.
struct FixAnimationBackupNode {
    std::uint32_t entity_index;
    std::int32_t acknowledged_command, frame;
    std::array<std::byte, 0x348> animation;
    RageVector angles;
    std::array<float, 24> poses;
    float collision_time;
    FixAnimationBackupNode* next;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(FixAnimationBackupNode, animation) == 0xC);
static_assert(offsetof(FixAnimationBackupNode, angles) == 0x354);
static_assert(offsetof(FixAnimationBackupNode, poses) == 0x360);
static_assert(offsetof(FixAnimationBackupNode, next) == 0x3C4);
#endif
struct FixAnimationGlobals {
    const std::uint8_t& enabled; //SettingsMenuValues.fix_animations,4346A527
    const std::uint8_t& command_mode; //43467224
    const std::uint8_t& alternate_command_mode; //4346A740
    const std::uint8_t& clear_animation_state; //43467CEC
    const std::uint8_t& invalidate_bones; //43468A10
    const std::uint32_t& command_flags; //434759C0
    const std::int32_t& animation_frame; //*434678F8
    const std::int32_t& prediction_frame; //43467B04+4
    const std::int32_t& clock_window; //43467B04+38
    const std::int32_t& clock_offset; //43467B04+3C
    const float& interval; //*4346A338
    const float& receive_interval; //43467B04+20
    float& current_time; //*43467EB8
    const std::int32_t& last_outgoing; //*43475B2C
    client::EntityCommandOwner* const& command_history_owner; //43475898
    void* const& local_player; //43475808
    const RageVector& setup_angles; //43476414
    const RageVector& view_angles; //4347BD30
    std::uint8_t& thinking; //43475BD1
    double* const& engine_timestamp; //optional4346A7E4
    double (*timestamp)(); //genuine engine entry43468C84
    FixAnimationBackupNode** const& backup_buckets; //4346D6E4
    const std::uint32_t& backup_bucket_count; //4346D6E8
    std::uint32_t& backup_count; //4346D6EC
    const std::int32_t& backup_mode; //4347C9EC
};
#if defined(_M_IX86) || defined(__i386__)
using AnimationLayerProxy = void(__cdecl*)(const void*, void*, void*);
#else
using AnimationLayerProxy = void (*)(const void*, void*, void*);
#endif
struct FixAnimationsBindings {
    const BoneRuntime& bones;
    FixAnimationEntityView entity;
    client::PlayerStateCache& players;
    FixAnimationFields fields;
    FixAnimationGlobals globals;
    void* input; //4346A894, actual third-person virtual32
    const void* interpolation_cvar; //4346A850 xor478720C2
    void* const* resource; //optional43468FF0, then its live first pointer
    std::byte* player_settings; //4347BE00, stride20, id at0/settings at8
    network::DataUpdateObserverRegistry& data_update_observers; //43475760/64/68
    const client::PlayerDataUpdateOffsets& data_update_fields;
    engine::OwnerRegistry& update_observers; //43475988/8C/90
    engine::MemberFunction<void, std::int32_t> invalidate_physics; //optional43468C10
    void* allocator; //43467204, Free virtual5
    engine::JobMemory memory;
    AnimationLayerProxy& original_layer_proxy; //saved engine proxy43476974
    const effects::PlayerOpacityBindings& opacity;
};

enum class FixAnimationHook {
    player,
    renderable,
    networkable_history,
    networkable_view,
    thinkable_observers,
    thinkable_opacity
};
// Hook bodies read the canonical enabled byte and the actual receiver on each
// invocation. Original methods always come from installed vtable[-2].
class FixAnimationsRuntime {
public:
    explicit FixAnimationsRuntime(const FixAnimationsBindings& bindings) noexcept
        : bindings(bindings) {}
    const FixAnimationsBindings& bindings;
    void build_transformations(void*, std::array<std::uintptr_t, 6>); //43330F73,slot190
    void standard_blending_rules(void*, void*, RageVector*, BoneQuaternion*,
                                 float); //4335C639,slot207
    void extra_bone_processing(void*, std::array<std::uintptr_t, 6>); //43388201,slot198
    void calculate_view(void*, RageVector*, RageVector*, float*, float*, float*); //4335B798,slot277
    bool setup_bones(void*, Matrix3x4*, std::int32_t, std::uint32_t, float); //433E80F7,slot13
    void networkable_history(void*, std::int32_t); //4337DE0B,slot7
    void networkable_pre_update(void*, std::int32_t); //433E129A ->433B7613,slot6
    void networkable_view(void*, std::int32_t); //433B7093,slot7
    void thinkable_observers(void*); //433FD3BF,slot1
    void thinkable_opacity(void*); //43383CA2,slot1
    void receive_layer(const void*, void*, void*); //433CE396
    void restore_animation_backup(void* player); //433C7FC5

    // The existing initialization scheduler supplies each actual prepared
    // descriptor and receiver. Source entries replace only the recovered slots;
    // installation and whole-unload restoration use the canonical hook list.
    void prepare(FixAnimationHook, engine::VtableHookMetadata&) noexcept;
    engine::VtableHookNode* install(FixAnimationHook, engine::VtableHookRegistry&, void* receiver,
                                    engine::VtableHookMetadata&);
    // Exact captured operand binding for the existing protected scheduler.
    // Invoke after its raw replacement loop and before4331E834. The native
    // entry identity selects the correct networkable/thinkable variant; no
    // class/local-player guess or replacement of unrelated entries is needed.
    void bind_native_replacement(engine::VtableHookMetadata&, std::size_t slot,
                                 std::uint32_t native_entry);
    static void* native_replacement_entry(std::size_t slot, std::uint32_t native_entry);
    void uninstall(engine::VtableHookRegistry&, void* receiver);
    void install_layer_proxy(void** actual_proxy_slot);
    void restore_layer_proxy();

private:
    bool animation_owner_changed(void* player) const; //complete4332FB3C
    void** proxy_slot_ = nullptr;
    void* saved_proxy_ = nullptr;
};
} // namespace skeet::animation

// Declarations from fix_runtime_bindings.hpp

namespace skeet::client {
class ConfigurationRuntimeOwner;
}
namespace skeet::animation {
class RuntimeOwner;
FixAnimationFields native_fix_animation_fields(const engine::NativeOffsetGlobals&);
FixAnimationEntityFields native_fix_animation_entity_fields(const engine::NativeOffsetGlobals&);
// Stable source owners supply all interfaces, settings and live global cells.
// Keep data_update_fields and opacity alive for the returned binding. The
// final owner additionally verifies the actual HookRuntimeOwner identity.
FixAnimationsBindings native_fix_animations_bindings(RuntimeOwner&,
                                                     client::ConfigurationRuntimeOwner&,
                                                     const client::PlayerDataUpdateOffsets&,
                                                     const effects::PlayerOpacityBindings&);
FixAnimationBackupBindings native_fix_animation_backup_bindings(RuntimeOwner&,
                                                                const FixAnimationsBindings&);
// Actual loaded msvcrt.dll!ceil, using its ordinary double(double) CRT ABI.
// No new module is loaded and no replacement math implementation is selected.
using AnimationCeiling = double (*)(double);
AnimationCeiling native_animation_ceiling();
} //namespace skeet::animation

// Declarations from fix_runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class NativeStartupSources;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::effects {
struct PlayerOpacityBindings;
}
namespace skeet::animation {
class RuntimeOwner;
class FixAnimationsRuntime;
class FixAnimationBackupRuntime;
class FixAnimationBackupCleanupContext;

// Construct after the canonical engine/tier0/client-state publications and
// RuntimeOwner. The opacity dependency is the actual renderer's live binding;
// its entity projection must be refreshed by that owner at each transaction.
// The supplied cleanup context was constructed before CRT and already owns
// the registration lifetime for the same process backup map.
// This owner creates no renderer, entity snapshot, hook list or second map.
// All borrowed owners must outlive it; keep it alive through exit callbacks.
class FixRuntimeOwner {
public:
    FixRuntimeOwner(RuntimeOwner&, client::ConfigurationRuntimeOwner&, engine::HookRuntimeOwner&,
                    const effects::PlayerOpacityBindings&, FixAnimationBackupCleanupContext&);
    ~FixRuntimeOwner();
    FixRuntimeOwner(const FixRuntimeOwner&) = delete;
    FixRuntimeOwner& operator=(const FixRuntimeOwner&) = delete;
    FixRuntimeOwner(FixRuntimeOwner&&) = delete;
    FixRuntimeOwner& operator=(FixRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    RuntimeOwner& bones_owner() const noexcept;
    FixAnimationsRuntime& animations() noexcept;
    FixAnimationBackupRuntime& backup() noexcept;
    FixAnimationBackupCleanupContext& cleanup() noexcept;
    // Binds the same early433B86B4 context; the startup scheduler preserves registration
    // order and later calls the shared exit list. No finalizer runs here.
    void bind_initializers(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::animation

// Declarations from layer_receive.hpp

namespace skeet::animation {
class FixAnimationsRuntime;
enum class ReceivedLayerField {
    cycle,
    previous_cycle,
    sequence,
    weight,
    playback_rate,
    weight_delta_rate,
    order
};
class LayerReceiveRuntime {
public:
    LayerReceiveRuntime(engine::NativeDataImage& data, const engine::NativeOffsetStorage& offsets,
                        FixAnimationsRuntime& animations) noexcept
        : data_(data), offsets_(offsets), animations_(animations) {}
    engine::NativeDataImage& data() const noexcept {
        return data_;
    }
    FixAnimationsRuntime& animations() const noexcept {
        return animations_;
    }
    void receive(ReceivedLayerField, const engine::RecvProxyData*, void* layer, void* output);

private:
    engine::NativeDataImage& data_;
    const engine::NativeOffsetStorage& offsets_;
    FixAnimationsRuntime& animations_;
    std::uint32_t offset(std::uint32_t, std::uint32_t, std::uint32_t) const;
};
} //namespace skeet::animation

// Declarations from layer_state.hpp

namespace skeet::animation {

// Complete 433CC618. The input is the seven-word layer copy beginning at
// engine-layer +14 (record +4D8 at caller 43411872), not the engine layer base.
// It reads sequence at +04, playback-rate bits at +14 and cycle at +18.
// The result is exactly 0 or 1. A NaN playback rate fails; a NaN cycle passes
// the final native COMISS/JAE branch. The input must be a valid layer view.
std::uint8_t layer_is_active(const void* layer) noexcept;

} // namespace skeet::animation

// Declarations from local_helpers.hpp

namespace skeet::client {
struct PredictionHistoryOwner;
}
namespace skeet::animation {
struct LocalAnimationOffsets {
    std::ptrdiff_t layers; //B538BBA5, descriptor.data/count+C
    std::ptrdiff_t studio_header; //32E4DAF1
    std::ptrdiff_t animation_state; //3D52A1A7
    std::ptrdiff_t active_weapon; //CD812654
    std::ptrdiff_t weapon_identifier; //5C9F0FBB
    std::ptrdiff_t player_flags; //E2FBE695
    std::ptrdiff_t additional_ground; //AC7F2913
    std::ptrdiff_t movement_state; //52899DF3
};
using InitializeSequenceMapping = engine::MemberFunction<void*>;
using SelectSymbolSequence =
    engine::MemberFunction<std::int32_t, void*, std::int32_t, const std::uint16_t*, std::int32_t>;
using LookupAnimationSymbol = engine::MemberFunction<void, std::uint16_t*, const char*>;
struct LocalLayerWeightEngine {
    void* context;
    // Genuine4346A598 receives ECX=state,EDX=0,stack(index),returnsXMM0.
    // The runtime adapter handles that external ABI only.
    float (*read)(void* context, void* animation_state, std::int32_t layer);
};
struct LocalAnimationBindings {
    LocalAnimationOffsets offsets;
    void* entity_list;
    void* symbol_table; //4346830C
    InitializeSequenceMapping initialize_mapping; //43469348
    SelectSymbolSequence select_symbol_sequence; //4346A4F8
    LookupAnimationSymbol find_symbol; //434691DC
    LookupAnimationSymbol add_symbol; //4346728C
    void* activity_interface; //4346A460; genuine entries+5C/+64
    std::int32_t* prediction_seed; //43468328
    void** prediction_player; //434693A4
    void* const& local_player; //live43475808, read at prediction publication
    client::PredictionHistoryOwner* const& sequence_owner; //live434759B4,canonical owned strings
    engine::MemberFunction<std::int32_t, std::int32_t> sequence_activity; //4346A364
    void* layer_weight_entry; //4346A598; genuine engine ABI adapted by main
    LocalLayerWeightEngine layer_weight;
    engine::MemberFunction<const char*, std::int32_t> sequence_name_entry; //434678C8
    // Explicit standalone context bytes. The supported engine methods at
    // vtable+5C/+64 consume only word7 (player); production reserves these.
    std::array<std::uint32_t, 6> activity_scratch;
};
// Complete433EAE1C, including weapon slot455 and the native prefix table.
const char* weapon_sequence_prefix(void* weapon, const LocalAnimationBindings&);
// Complete433B1A8F, including modifier symbol lookup/add and mapping cache.
std::int32_t select_symbol_weighted_sequence(void* player, std::int32_t layer,
                                             std::int32_t activity, const LocalAnimationBindings&);
// Complete433DF1A9->433C0554. The latter is solely a cdecl return-spoof
// trampoline to the genuine animation-interface entry+64.
std::int32_t map_animation_activity(void* player, const char* name, const LocalAnimationBindings&);
// Complete43387742, with paired prediction seed/player publication/restoration.
std::int32_t select_prediction_sequence(void* player, std::int32_t activity, std::int32_t seed,
                                        const LocalAnimationBindings&);
// Complete433800F5. Valid player and command contract; the full state dword
// is cleared first even when no directional combination is admitted.
std::uint32_t movement_button_state(std::uint32_t buttons) noexcept;
void apply_movement_button_state(void* player, const client::Command&,
                                 const LocalAnimationBindings&) noexcept;
// Complete4340BD50; scalar arguments are XMM0..4, references are ECX/EDX.
void update_lower_body_yaw_timer(float yaw, float horizontal_speed, float vertical_speed, float now,
                                 float target_yaw, float& next_update,
                                 float& previous_yaw) noexcept;
// Complete434011C8; only modifier byte32B is assigned.
void mark_animation_modifiers(void* animation_state) noexcept;
// Called under the native nonnull4346A598 gate; read is its actual ABI adapter.
float read_local_layer_weight(void* animation_state, std::int32_t layer,
                              const LocalAnimationBindings&);
// Complete433EBF17 and43381E04. The animation-state pointer is the caller's
// saved/live0x348-byte state. Both reset the actual layer5 through player slot223.
void update_landing_layer(void* player, void* animation_state, const LocalAnimationBindings&);
// Fourth native argument (previous+110) is passed and purged but never read.
void update_post_animation_layer(void* player, void* animation_state, bool previous_in_air,
                                 float unused_previous_time, const LocalAnimationBindings&);
} // namespace skeet::animation

// Declarations from local_prediction_pose.hpp

namespace skeet::animation {
struct LocalPredictionPoseOffsets {
    std::uint32_t velocity_36375c23, duck_3309c0cf, player_flags_e2fbe695;
    std::uint32_t command_f58e6118, origin_4e66bb34, origin_2a671b42, body_yaw_a2a8247f;
};
struct LocalPredictionPoseState {
    float& timestamp; //4347C640
    HitboxMatrices& hitboxes; //4347C650
    RageVector& origin; //4347C9E0
};
class LocalPredictionCommandRing {
    const client::Command* commands_;
    void* const* input_;

public:
    explicit LocalPredictionCommandRing(std::span<const client::Command, 150> commands) noexcept
        : commands_(commands.data()), input_(nullptr) {}
    explicit LocalPredictionCommandRing(void* const& input) noexcept
        : commands_(nullptr), input_(&input) {}
    const client::Command& operator[](std::uint32_t number) const noexcept {
        return input_ ? client::input_command_slot(*input_, number) : commands_[number % 150u];
    }
};
struct LocalPredictionPoseBindings {
    const lagcomp::EntityStateRuntime& entity;
    LocalPredictionPoseOffsets offsets;
    const lagcomp::TickTimingSamples& latency;
    const anti_aim::MovementCaptureInput& movement_capture;
    const MovementEnvironmentBindings& movement_environment;
    MovementCollisionEngine& collision;
    LocalPredictionCommandRing commands;
    void* const& animation_update; //optional434693F0
    const std::int32_t* const& animation_frame;
    float* const& current_time; //43467EB8
    const float* const& interval; //4346A338
    std::uint8_t* const& animation_reset; //optional43467B24
    std::uint8_t& animation_update_flag; //43475BD3
    void* const& allocator; //43467204; actualFree virtual5
    const BoneRuntime& bones;
    lagcomp::SetAbsoluteVector const& set_absolute_angles; //live optional434688C8
    LocalPredictionPoseState output;
    const float* const& publication_clock; //4346A334
};
// Complete4331EE04. The original is gated by send_packet or a full choke
// budget; this entry runs after the concrete restricted-player command owner.
// Movement, animation update and model-pose publication remain source.
void publish_local_prediction_pose(client::CommandFrame&, void* player, const client::Command&,
                                   std::uint8_t send_packet, const LocalPredictionPoseBindings&);
// Explicit native-frame input seam for differential fixtures. Production uses
// the automatic workspace owned by the entry above, not persistent seed data.
void publish_local_prediction_pose_with_inputs(client::CommandFrame&, void* player,
                                               const client::Command&, std::uint8_t send_packet,
                                               const LocalPredictionPoseBindings&,
                                               const LocalPredictionPoseInputs&);
} // namespace skeet::animation

// Declarations from local_prediction_pose_inputs.hpp

namespace skeet::animation {
// Automatic local storage reserved by4331EE04. Production creates it inside
// the actual invocation without value-initialization. The explicit input API
// uses this layout only to reproduce selected stack bytes in fixtures.
struct LocalPredictionPoseInputs {
    std::array<std::byte, copied_animation_state_size> animation;
    std::array<Matrix3x4, 256> bones;
};
} //namespace skeet::animation

// Declarations from native_trigonometry.hpp

namespace skeet::animation {
// Actual loaded msvcrt.dll __libm_sse2_sinf/__libm_sse2_cosf imports.
// The adapters convert the source float(float) ABI to XMM0 input/result.
// All source consumers borrow the same immutable process instance. Missing
// exports or a non-x86 process fail; this never loads a replacement library.
const RageTrigonometry& native_sse_trigonometry();
//4335C2B0/4335C300 pass y inXMM0 and x inXMM1 to __libm_sse2_atan2.
double native_sse_atan2(double y, double x);
//433BE1A3 passes and receives the low float inXMM0 at __libm_sse2_expf.
float native_sse_exponential(float);
//4334331D uses these two genuine CRT exports with XMM0 input/result.
float native_sse_tangent(float);
float native_sse_arctangent(float);
} //namespace skeet::animation

// Declarations from record_pose_scope.hpp

namespace skeet::animation {
// Rebuild a captured pose without mixing in the entity's current layers.
class RecordPoseScope {
    void* player_;
    lagcomp::EntityNetvars offsets_{};
    lagcomp::AnimationState saved_{};
    bool valid_ = true;

public:
    RecordPoseScope(void* player, const lagcomp::LagRecord& record, const BoneRuntime& bones,
                    bool enabled)
        : player_(enabled ? player : nullptr) {
        if (!player_)
            return;
        offsets_.animation_layers = std::uint32_t(bones.fields.animation_layers_b538bba5);
        offsets_.pose_parameters = std::uint32_t(bones.fields.pose_parameters);
        capture_entity_animation(saved_, player_, offsets_);
        valid_ = record.animation.layer_count == saved_.layer_count &&
                 record.animation.layer_count >= 0 && record.animation.layer_count <= 13;
        for (float pose : record.animation.pose_parameters)
            valid_ = valid_ && std::isfinite(pose);
        if (!valid_) {
            player_ = nullptr;
            return;
        }
        restore_entity_animation(record.animation, player_, offsets_);
    }
    bool valid() const noexcept {
        return valid_;
    }
    ~RecordPoseScope() {
        if (player_)
            restore_entity_animation(saved_, player_, offsets_);
    }
    RecordPoseScope(const RecordPoseScope&) = delete;
    RecordPoseScope& operator=(const RecordPoseScope&) = delete;
};
}

// Declarations from remote_animation_preparation.hpp

namespace skeet::animation {
struct RemoteAnimationPreparationFields {
    explicit RemoteAnimationPreparationFields(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset frame, ik_mask, flags, previous_flags, active_weapon;
    engine::DecodedOffset animation_state, view_gate, observer_mode, observer_target, world_model;
    engine::DecodedOffset eye_origin, duck_amount; //433849B5/433E533F
};
struct RemoteAnimationPreparationBindings {
    const RemoteAnimationPreparationFields& fields;
    const RageTrigonometry& trigonometry;
    void* engine; //4346A514, virtual93
    void* entity_list; //4346A890, virtual4
    void* input; //4346A894, virtual32
    void* const& local_player; //43475808
    const std::int32_t& frame; //43467B04+4
    const RageVector& view_origin; //43479CCC
    const RageVector& view_angles; //43479CD8
};

// Complete ordinary43402385. Recomputes the remote player's animation LOD
// once per frame and propagates its flags to the active weapon's world model.
// The caller owns the real per-thread guard; this body does not change it.
class RemoteAnimationPreparationRuntime {
public:
    explicit RemoteAnimationPreparationRuntime(RemoteAnimationPreparationBindings bindings) noexcept
        : bindings(bindings) {}
    const RemoteAnimationPreparationBindings bindings;
    void prepare(void* player) const;

private:
    bool allows_view_animation(void* player) const; //433BFF50
};
} // namespace skeet::animation

// Declarations from remote_setup_bones.hpp

namespace skeet::animation {
class RemoteAnimationPreparationRuntime;

struct RemoteSetupBonesFields {
    std::ptrdiff_t bone_cache_17978090; // pointer +0, signed count +C
    std::ptrdiff_t recent_bone_counter_2b554ad0;
    std::ptrdiff_t view_angles_e23a9be1; // pitch +0, yaw +4
    std::ptrdiff_t transform_guard_49d93e62; // first child handle -14,next -10
};

#if defined(_M_IX86) || defined(__i386__)
using BoneEngineThreadPredicate = bool(__cdecl*)();
#else
using BoneEngineThreadPredicate = bool (*)();
#endif
using BoneEnginePrepare = engine::MemberFunction<void>;
using BoneEngineAttachments = engine::MemberFunction<void, void*>;

// Actual per-thread TEB byte used by433FF7F6, not a process-wide lock.
std::uint8_t& native_remote_bone_thread_guard() noexcept;

struct RemoteSetupBonesBindings {
    const BoneRuntime& bones;
    client::PlayerStateCache& players;
    const RemoteAnimationPreparationRuntime& preparation; // complete43402385
    RemoteSetupBonesFields fields;
    const std::int32_t* const& current_bone_counter; // optional43469060
    BoneEnginePrepare const& prepare_on_engine_thread; // optional4346A38C
    BoneEngineThreadPredicate const& on_engine_thread; // optional43476920
    BoneEngineAttachments const& update_attachments; // optional4346A6B0
    float& frame_time; //43467B04+14, live global frame duration
    std::uint8_t& (*thread_guard)() noexcept = native_remote_bone_thread_guard;
};

class RemoteSetupBonesRuntime {
public:
    explicit RemoteSetupBonesRuntime(const RemoteSetupBonesBindings& bindings) noexcept
        : bindings(bindings) {}
    const RemoteSetupBonesBindings& bindings;
    // Complete433FF7F6, native thunk4334AD2A. ECX is IClientRenderable
    // (player+4), stack output/max/mask/time, RET16. The incoming EBP copied
    // to EDX by the thunk and the time argument are unused by this body.
    bool setup_bones(void* renderable, Matrix3x4* output, std::int32_t maximum, std::uint32_t mask,
                     float time);
    // Complete4339C7A0. Calls the genuine optional engine attachment entry,
    // then invalidates each linked entity, preserving native field ordering.
    void refresh_attachments(void* player) const;
    // Only slot13 is owned here; slot37 has a different captured handler.
    void bind(engine::VtableHookMetadata&) noexcept;
    static void* source_entry() noexcept;
};
} // namespace skeet::animation

// Declarations from runtime_bindings.hpp

namespace skeet::animation {
// Decode the actual pointer/key/salt operands after native offset publication.
// These value layouts are initialized once for the bound entity schema; they
// do not replace RemoteAnimationPreparationFields' live DecodedOffset members.
BoneRuntimeFields native_bone_runtime_fields(const engine::NativeOffsetGlobals&);
RemoteSetupBonesFields native_remote_setup_bones_fields(const engine::NativeOffsetGlobals&);

// attachment_update is the typed result returned by
// engine::initialize_build_animation_bindings. Its identity is checked against
// canonical43468F80; the entity list comes from canonical4346A890.
BoneRuntime native_bone_runtime(const engine::NativeDataImage&, const engine::NativeOffsetGlobals&,
                                BoneAttachmentUpdate attachment_update, const RageTrigonometry&);

// Call after game-state and animation function publication. The returned
// references borrow the actual optional function/counter cells and gpGlobals
// frame duration, so cell changes are visible to the remote handler. Keep the
// data image, global-vars object and supplied source runtimes alive throughout
// use; RemoteSetupBonesRuntime also borrows the returned bindings object.
RemoteSetupBonesBindings native_remote_setup_bones_bindings(
    const engine::NativeDataImage&, const engine::NativeOffsetGlobals&, const BoneRuntime&,
    client::PlayerStateCache&, const RemoteAnimationPreparationRuntime&);
} // namespace skeet::animation

// Declarations from runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
}
namespace skeet::animation {
// Stable assembly for the three remote bone/animation runtimes. Construct
// after ClientRuntimeStorage::initialize_engine_bindings; keep storage and the
// shared PlayerStateCache alive until hooks and all consumers are detached.
// Owns its fields and bindings, including the binding borrowed by the remote
// hook. Neither another player cache nor a snapshot of dynamic globals exists.
class RuntimeOwner {
public:
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::PlayerStateCache&);
    RuntimeOwner(const RuntimeOwner&) = delete;
    RuntimeOwner& operator=(const RuntimeOwner&) = delete;
    RuntimeOwner(RuntimeOwner&&) = delete;
    RuntimeOwner& operator=(RuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    client::PlayerStateCache& players() const noexcept {
        return players_;
    }
    const RageTrigonometry& trigonometry() const noexcept {
        return trigonometry_;
    }
    const BoneRuntime& bones() const noexcept {
        return bones_;
    }
    const RemoteAnimationPreparationRuntime& preparation() const noexcept {
        return preparation_;
    }
    RemoteSetupBonesRuntime& remote_setup_bones() noexcept {
        return remote_;
    }
    const RemoteSetupBonesRuntime& remote_setup_bones() const noexcept {
        return remote_;
    }

private:
    client::ClientRuntimeStorage& storage_;
    client::PlayerStateCache& players_;
    const RageTrigonometry& trigonometry_;
    RemoteAnimationPreparationFields preparation_fields_;
    BoneRuntime bones_;
    RemoteAnimationPreparationRuntime preparation_;
    RemoteSetupBonesBindings remote_bindings_;
    RemoteSetupBonesRuntime remote_;
};
} //namespace skeet::animation

// Declarations from state_copy.hpp

namespace skeet::animation {

inline constexpr std::size_t copied_animation_state_size = 0x348;

// Complete433F526C. Only the five native32-bit vector-header words at+13C
// are initialized; the remaining animation state bytes are untouched.
void initialize_animation_state_vector(void* animation_state) noexcept;

// Complete433CF41B ->4331E2CD twice. allocator is the actual43467204 engine
// interface. Owned native32-bit storage is freed through its virtual slot5;
// a negative grow-size keeps external storage and its allocation count.
void release_animation_state_vector(void* animation_state, void* allocator);

} // namespace skeet::animation
