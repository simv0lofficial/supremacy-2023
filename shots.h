#pragma once

// Shot Tracking & Hit Registration Declarations
#include <cstdint>
#include "aimbot.h"
#include "skeet/client/command_frame.hpp"
#include "skeet/rage/shot_feedback.hpp"
#include "aimbot.h"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/network/exploit_observers.hpp"
#include "skeet/rage/spread.hpp"
#include <memory>
#include "skeet/client/player_state.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/effects/model_render.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/lagcomp/entity_preparation.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/rage/ballistics.hpp"
#include "skeet/events/game_events.hpp"
#include "skeet/lagcomp/timeline.hpp"
#include "skeet/resolver_runtime/attempts.hpp"
#include "skeet/resolver_runtime/next_attempt.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/script/bridge.hpp"
#include "skeet/players/state.hpp"
#include <deque>
#include "skeet/rage/aim_fire_event.hpp"
#include "skeet/rage/pending_shot_publication.hpp"
#include "skeet/rage/shot_diagnostics.hpp"
#include <span>
#include "skeet/engine/jobs.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include "aimbot.h"


// Declarations from aim_fire_event.hpp

namespace skeet::script {
struct Bridge;
}
namespace skeet::client {
struct CommandFrame;
class PlayerStateCache;
}

namespace skeet::rage {
struct TargetCandidate;
struct AcquisitionEvaluation;

// Complete4340C8C4 ->4332A6B4 ->4336A85F. The native ECX frame argument
// is not read. Candidate point fields and B4 tick refer to the same scan slot.
// The canonical player-cache lookup still precedes callback lookup, even when
// no aim_fire callback is registered. An absent Lua state skips that lookup.
// Predefined event/table names read the Bridge's live native-name rows.
int dispatch_aim_fire_event(script::Bridge&, client::PlayerStateCache&, const client::CommandFrame&,
                            const TargetCandidate&, const AcquisitionEvaluation&,
                            std::int32_t shot_index);
} // namespace skeet::rage

// Declarations from buffered_shot_repair.hpp

namespace skeet::rage {
class RuntimeOwner;
struct BufferedShotRepairBindings {
    std::variant<CommandOwner*, RuntimeOwner*> source;
    BufferedShotRepairBindings(CommandOwner* fixture) noexcept : source(fixture) {}
    BufferedShotRepairBindings(RuntimeOwner& live) noexcept : source(&live) {}
};
// Native43402BA2 supplies sixteen in-object pointer slots. Candidate grouping
// appends only while the begin pointer still aliases that inline storage.
struct BufferedShotRepairScratch {
    TargetBatch candidates;
    bool inline_storage = true;
    BallisticQuery historical_bullet;
};
// Complete433C155F. The stored frame is the original ECX, local player EDX,
// followed by copied command, six-entry diagnostic record and inline scratch.
// Returns the canonical full0x1190 evaluation in owner.acquisition, whose
// lifetime extends through the caller's pending-shot update. Flags+170 mask1
// is set for reconstruction and cleared on every ordinary exit.
const lagcomp::EvaluationState*
repair_buffered_shot(client::CommandFrame& stored, void* local_player,
                     client::Command& copied_command, const ShotDiagnosticEntry&,
                     BufferedShotRepairScratch&, const BufferedShotRepairBindings&);
} // namespace skeet::rage

// Declarations from feedback_command_owner.hpp

namespace skeet::rage {
// Native4346DC2C is the command interface at feedback owner+8, registered
// by4347F721/43345EBC. Its+44/+48 vector is the same full owner's+4C/+50
// pending vector used by shot publication, reconciliation and RAGE selection.
// This interface view owns no second queue and must not outlive feedback.
struct FeedbackCommandOwner {
    ShotFeedbackState& feedback;
    const std::int32_t& last_outgoing_command; // dereferenced43475B2C
    const std::uint8_t& send_gate; // global4346754C; distinct from command send byte
};

// Complete4340C927. A choked frame extends the last command of each pending
// shot whose first command lies strictly inside(last outgoing,current).
void process_feedback_command_phase4(FeedbackCommandOwner&, const client::CommandFrame&) noexcept;
// Complete4331E7AC. Under the process send gate, only the newest pending shot
// with a matching first command receives the current raw three-word origin.
void process_feedback_command_phase6(FeedbackCommandOwner&, const client::CommandFrame&) noexcept;
} // namespace skeet::rage

// Declarations from feedback_entity_owner.hpp

namespace skeet::rage {
// Required live producer bindings. Pointer-global references preserve native
// rebinding; encrypted ConVar references are decoded only at consumption.
struct FeedbackEntityBindings {
    const std::uint8_t& recording_enabled; //4346754C
    const float* const& current_time; //4346A334 -> float
    const float* const& tick_interval; //4346A338 -> float
    const void* const& receive_clock; //43467B04 -> object, interval+20
    const std::uint32_t& update_rate_convar; //4346747C XOR03B4819C
    const std::uint32_t& interpolation_convar; //4346A748 XOR7D9DC86F
    const std::uint32_t& interpolation_ratio_convar; //4346A330 XORBE64D223
    const std::uint32_t& disable_ray_tests_convar; //43468EA4 XORFCD2DA7A
    const float& correction_floor; //43469254
    const float& correction_addend; //43476928
    const std::int32_t& maximum_unlag; //43468230, milliseconds
    const std::uint8_t* resolver_rework = nullptr;

    bool enabled(const ShotFeedbackState&) const noexcept;
    float now() const noexcept;
    float interval() const noexcept;
    float receive_interval() const noexcept;
    float lerp() const noexcept; // complete4331E237, update/interpolation/ratio order
    float correction_minimum() const noexcept;
    float correction_extra() const noexcept;
    std::int32_t maximum_unlag_milliseconds() const noexcept;
    bool ray_tests_disabled() const noexcept;
    bool reworked() const noexcept {
        return resolver_rework && *resolver_rework;
    }
};
struct FeedbackEntityOwner {
    ShotFeedbackState& feedback;
    ShotFeedbackRuntime& runtime;
    const FeedbackEntityBindings& bindings;
};
// BA8 interface4346F68C, full feedback owner+10. Slot2 clears only the event
// queue and recent-position count; pending shots/fire snapshots stay owned.
void reset_feedback_events(FeedbackEntityOwner&) noexcept; //complete4333D358
// Slot3 enters the existing complete reconciliation body with required live
// reads at its original consumption sites, after the same native gates.
void reconcile_feedback_events(FeedbackEntityOwner&); //complete433240DD
} //namespace skeet::rage

// Declarations from feedback_owner.hpp

namespace skeet::client {
class NativeStartupSources;
}
namespace skeet::rage {
struct FeedbackPacketOffsets {
    engine::DecodedOffset player_index, origin, spread, inaccuracy, recoil_index;
    engine::DecodedOffset angles, seed, fire_mode, weapon_id;
};
FeedbackPacketOffsets feedback_packet_offsets(const engine::NativeOffsetStorage&);
struct FeedbackPacketBindings {
    const std::uint8_t& capture_enabled; //43467224
    const std::uint8_t& recording_enabled; //4346754C
    void* const& input; //4346A894, actual150-command ring
    engine::ConVarBinding shotgun_patterns; //live decoded43468520 ConVar
    FeedbackPacketOffsets offsets;
    const SpreadTables& spread_tables;
    const SpreadRandom& random;
    const ShotgunPattern& shotgun_pattern;
    const RageTrigonometry& trigonometry;
};

class FeedbackOwner;
struct FeedbackOwnerBindings {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    client::EntityObserverRegistry& entities; //43475BA8
    engine::OwnerRegistryBinding packets; //4347584C
    engine::OwnerRegistryBinding frames; //43475AD8
    engine::OwnerRegistryBinding positions; //43475930/34/38
    network::ExploitRechargeObserverRegistry& recharge; //43475784
    ConfigRegistries& configuration;
    events::EventRegistry& events;
    ShotLogSettings& settings;
    ShotFeedbackState& state;
    ShotFeedbackRuntime& runtime;
    const FeedbackEntityBindings& live;
    const FeedbackPacketBindings& packet;
    const std::int32_t& last_outgoing_command; //*43475B2C
    void* const& engine_client; //4346A514, captured once at4347F79C
    FeedbackOwner*& active; //canonical43475A7C, full primary object
};

std::size_t feedback_owner_size() noexcept;
// Complete ordinary4347F708 constructor. Bindings and all shared storage
// outlive the owner. State queues/descriptors are fresh and unregistered;
// no separate command/entity/recharge owner may be preregistered for them.
FeedbackOwner& construct_feedback_owner(void* native_aligned_storage, const FeedbackOwnerBindings&);
FeedbackOwner& construct_feedback_owner(void*, FeedbackOwnerBindings&&) = delete;
FeedbackOwner& construct_feedback_owner(void*, const FeedbackOwnerBindings&&) = delete;
// Complete43485C3A. Native allocation is110 bytes; mem_int3.h records the
//43485C53 MOV[43475A7C],EAX on both success and allocation failure.
FeedbackOwner* create_feedback_owner(const FeedbackOwnerBindings&);
FeedbackOwner* create_feedback_owner(FeedbackOwnerBindings&&) = delete;
FeedbackOwner* create_feedback_owner(const FeedbackOwnerBindings&&) = delete;
// Binds only the ordinary factory identity. Preserved startup metadata keeps
// its actual operand/order; this function does not execute or rewrite the VM.
void bind_feedback_owner_factory(client::NativeStartupSources&, FeedbackOwnerBindings&);
// Complete433A0098/433CDB60: clear active first, unsubscribe only the dynamic
// impact listener, release snapshot/event/text/pending storage, header-free.
void destroy_feedback_owner(FeedbackOwner*, FeedbackOwner*& active);
void* feedback_lifecycle_object(FeedbackOwner&) noexcept;
ShotFeedbackState* feedback_owner_state(FeedbackOwner*) noexcept;
const ShotFeedbackState* feedback_owner_state(const FeedbackOwner*) noexcept;
const ShotLogSettings* feedback_owner_settings(const FeedbackOwner*) noexcept;
const ShotFeedbackRuntime* feedback_owner_runtime(const FeedbackOwner*) noexcept;

void reset_feedback_owner(FeedbackOwner&) noexcept; //433C8E1D, primary7
void capture_feedback_packet(FeedbackOwner&, const void* packet); //complete433C7A41
void capture_feedback_position(FeedbackOwner&, const RageVector&) noexcept; //complete433ED9DA
void update_feedback_subscription(FeedbackOwner&, std::uint8_t update); //43331AE5
// Complete ordinary433A00C4..433A0124 caller: flags+1C require0B, then the
// three packet coordinates+8/+C/+14 are sent through the native XMM ABI.
void dispatch_feedback_position_packet(const engine::OwnerRegistry&, const void* packet);
struct FeedbackPositionRuntime {
    const engine::OwnerRegistry& positions; //actual43475930/34/38
};
void bind_feedback_position_runtime(FeedbackPositionRuntime&) noexcept;
void unbind_feedback_position_runtime(FeedbackPositionRuntime&) noexcept;
//433A00C4 reads its only argument from[EBP+8] and returns with plainRET.
// Its proven body does not invoke any original callback/trampoline.
#if defined(_M_IX86) || defined(__i386__)
void __cdecl feedback_position_hook(const void* packet);
#else
void feedback_position_hook(const void* packet);
#endif
} //namespace skeet::rage

// Declarations from feedback_runtime_owner.hpp

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::script {
class ScriptApiRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CrtRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::rage {
struct PendingShotPublicationBindings;
// One production feedback context over HistoryRuntimeOwner's actual pending
// shot/resolver state. Engine adapters read local-player/entity/model/clock
// cells at each method invocation; construction captures no player or frame.
class FeedbackRuntimeOwner final {
public:
    FeedbackRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                         client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                         engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                         client::CrtRuntimeOwner&, esp::EspRuntimeOwner&,
                         script::ScriptApiRuntimeOwner&);
    ~FeedbackRuntimeOwner();
    FeedbackRuntimeOwner(const FeedbackRuntimeOwner&) = delete;
    FeedbackRuntimeOwner& operator=(const FeedbackRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    ShotFeedbackRuntime& runtime() noexcept;
    FeedbackOwnerBindings& factory_bindings() noexcept;
    const PendingShotPublicationBindings& pending_shots() const noexcept;
    void bind_initializer(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::rage

// Declarations from pending_shot_publication.hpp

namespace skeet::rage {
class FeedbackOwner;
struct PendingShotEntityOffsets {
    std::ptrdiff_t life_count; // CA5EC698, captured minus one
    std::ptrdiff_t model_index; // 47877DEB, signed short
    std::ptrdiff_t hitbox_set; // C2B6FB10
};
struct PendingShotPublicationBindings {
    FeedbackOwner*& active; // actual nullable43475A7C cell, full primary owner
    const std::uint8_t& recording_enabled; // actual4346754C, read independently
    client::PlayerStateCache& players;
    void* engine_client;
    void* entity_list;
    void* weapon_system; // actual43467B54, virtual2; null uses native4346F1B8
    const void* life_count_convar; // decoded434670D0 ^ 1320FFAA
    PendingShotEntityOffsets offsets;
    const float* current_time; // *4346A334
    const float* prediction_time; // *43467EB8
    const float* interval; // *4346A338
    const std::uint32_t* scheduler_flags; //434759C0
    resolver_runtime::AttemptEngineBinding attempts;
    const std::uint8_t* resolver_rework = nullptr;
};

// Complete433CCA5C. The original null-system path copies4346F1B8; otherwise
// real weapon-system virtual2 supplies data. The upper identifier word stays
// untouched on the nonnull path, matching the native16-bit store.
void capture_shot_weapon_bullet(void* weapon_system, std::uint16_t identifier,
                                client::CommandBullet&);

// Complete433D1221. The entity index must be nonzero, as on the native valid
// path. Cache acquisition precedes entity-list virtual3. A missing entity
// leaves the destination unchanged; a present entity captures all0x1190 bytes.
void shot_capture_target(ShotFeedbackRecord&, const lagcomp::EvaluationState&,
                         const RageVector& origin, std::uint8_t low_flags,
                         const PendingShotPublicationBindings&);

// Complete4341D449. Native ABI: ECX=frame, EDX=shot_index, stack=evaluation.
// Appends the canonical0x1220 pending record before reading engine state, then
// captures the full evaluation and updates the same PlayerStateCache resolver
// owner used by history acquisition. Resolves active at entry, after preceding
// callbacks; only the native captured record survives the later engine calls.
// False is the native absent-owner/disabled-recording gate.
bool publish_pending_shot(const client::CommandFrame&, const lagcomp::EvaluationState&,
                          std::int32_t shot_index, const PendingShotPublicationBindings&,
                          std::int32_t extrapolation_ticks = 0);
}

// Declarations from shot_diagnostics.hpp

namespace skeet::client {
class BufferedShotLifecycleOwner;
}
namespace skeet::rage {
struct ShotDiagnosticEntry {
    std::uint32_t clock;
    std::int32_t command, target_tick;
    std::uint32_t target_field_168, target_flags;
    float target_height, target_duck, target_reference_yaw;
    std::uint32_t local_field_7f5ed2d0;
    std::int32_t local_tick;
    std::uint32_t target_handle;
    client::CommandBullet bullet;
};
static_assert(sizeof(ShotDiagnosticEntry) == 0x48);
struct ShotDiagnosticHistory {
    std::array<std::byte, 0xC> untouched;
    std::uint32_t count = 0;
    std::array<ShotDiagnosticEntry, 6> entries;
};
static_assert(offsetof(ShotDiagnosticHistory, entries) == 0x10);
static_assert(sizeof(ShotDiagnosticHistory) == 0x1C0);
struct ShotDiagnosticOffsets {
    std::ptrdiff_t simulation_time; //D5FD2E2C
    std::ptrdiff_t player_flags; //E2FBE695
    std::ptrdiff_t duck_amount; //3309C0CF
    std::ptrdiff_t local_field_7f5ed2d0;
};
struct ShotDiagnosticBindings {
    // The actual full-owner slot43475968. Resolve its diagnostics only where
    // the native producer reads that global, after the caller's engine lookup.
    client::BufferedShotLifecycleOwner*& buffered_shot_owner;
    client::PlayerStateCache& players;
    ShotDiagnosticOffsets offsets;
    const std::uint32_t* clock; //dereferenced4346A334, copied without conversion
    float interval;
    void* weapon_system; //actual43467B54, data lookup slot2
};
// Complete43329B30, including the retained six-entry0x48-byte history and
// concrete433CCA5C weapon-data capture. Target/local are native-valid entities.
void capture_shot_diagnostics(const client::CommandFrame&, void* local_player, void* target,
                              const ShotDiagnosticBindings&);

// Complete4335C604/4336EB3A matrix-vector resize. Previously retained matrices
// keep their bytes; every appended matrix is cleared, including reused capacity.
class ShotModelBoneStorage {
public:
    explicit ShotModelBoneStorage(engine::JobMemory = engine::native_job_memory());
    ShotModelBoneStorage(engine::NativeDataImage&, engine::JobMemory);
    ~ShotModelBoneStorage();
    ShotModelBoneStorage(const ShotModelBoneStorage&) = delete;
    ShotModelBoneStorage& operator=(const ShotModelBoneStorage&) = delete;
    std::span<Matrix3x4> matrices() const noexcept;
    void resize(std::size_t count);
    void finalize();

private:
    Matrix3x4* standalone_begin_ = nullptr;
    Matrix3x4* standalone_end_ = nullptr;
    Matrix3x4* standalone_limit_ = nullptr;
    Matrix3x4*& begin_;
    Matrix3x4*& end_;
    Matrix3x4*& limit_;
    engine::JobMemory memory_;
};
struct ShotModelOffsets {
    std::ptrdiff_t studio_header; //32E4DAF1: pointer to wrapper, first word is studiohdr
    std::ptrdiff_t model_index; //47877DEB, signed16
    std::ptrdiff_t entity_flags; //38CA5228
    std::ptrdiff_t byte_8481ea04;
    std::ptrdiff_t interpolation_flags; //57B2D535
    std::ptrdiff_t hitbox_set; //C2B6FB10
    std::ptrdiff_t field_950f7b20;
    std::ptrdiff_t model_instance; //5173A531
    std::ptrdiff_t field_a25d8271;
    std::ptrdiff_t local_origin; //2A671B42
    std::ptrdiff_t local_angles; //3B1F6ECC
    std::ptrdiff_t render_mode; //FC2723F7
    std::ptrdiff_t render_effect; //688E0292
    std::ptrdiff_t color; //4999428D
};
#if defined(_M_IX86) || defined(__i386__)
using ShotEntityCreate = void*(__cdecl*)(std::int32_t, std::int32_t);
#else
using ShotEntityCreate = void* (*)(std::int32_t, std::int32_t);
#endif
using RegisterShotEntity = engine::MemberFunction<void, std::uint32_t>;
using UnregisterShotEntity = engine::MemberFunction<void, std::uint32_t*, void*>;
using InvalidateShotTransform = engine::MemberFunction<void, std::int32_t>;
struct ShotModelPublicationBindings {
    const esp::ColoredModelsSettings& settings;
    void* entity_list; //4346A890, actual slots3/4 and highest-index+24
    void* model_info; //4346A614, slots1/2/3
    void* model_cache; //43469330, slots33/34
    ShotEntityCreate create_entity; //actual first word of native class434685FC
    // Genuine resolved entity-list entries4346A884/43468E88. Both are required
    // by the native registration branch, which adjusts the base-list receiver.
    RegisterShotEntity register_entity;
    UnregisterShotEntity unregister_entity;
    void* registration_receiver;
    ShotModelOffsets offsets;
    InvalidateShotTransform invalidate_transform; //optional43468C10
    const engine::RecvVarProxy* color_proxy; //optional native43476B44
    std::uint32_t& shot_entity_handle; //shared4346D674
    effects::ModelShotState& published; //shared4347BDE8/BDF0/434797A0/A4
    ShotModelBoneStorage& bones;
    client::PlayerStateCache& players;
    const lagcomp::EntityPreparationContext& preparation;
    const lagcomp::HistoryTickTiming& timing;
    const std::uint32_t* current_bone_counter;
    const animation::BoneRuntime& bone_runtime;
    void* clock; //same actual43467118 used by model rendering; virtual5 returns double
};
// Complete4331F3F1. Applies the accepted full record under EntityRecordScope,
// creates/updates the real render entity, builds matrices through43388720's
// recovered body, publishes the draw owner's origin/expiry, and restores target.
void publish_accepted_shot_model(const client::CommandFrame&, const TargetCandidate&,
                                 const lagcomp::LagRecord&, const ShotModelPublicationBindings&);
} // namespace skeet::rage

// Declarations from shot_feedback.hpp

namespace skeet::rage {
struct ShotLogSettings;
struct FeedbackEntityBindings;
}
namespace skeet::misc {
class NormalLog;
}

namespace skeet {
// The pending-shot element is the already recovered full 0x1220-byte history
// evaluation. Prefix fields are named through this view without projecting or
// dropping its captured LagRecord, hitbox matrices, or padding.
using ShotFeedbackRecord = lagcomp::HistoryEvaluation;
struct ShotRecordView {
    ShotFeedbackRecord& record;
    std::uint32_t& flags() const noexcept {
        return record.words[0];
    }
    std::int32_t command() const noexcept;
    float time() const noexcept;
    std::int32_t shot_tick() const noexcept;
    std::uint32_t target_handle() const noexcept {
        return record.words[7];
    }
    std::int32_t target_life_id() const noexcept;
    RageVector origin() const noexcept;
    std::int32_t model() const noexcept;
    std::int32_t hitbox_set() const noexcept;
    std::uint32_t fire_weapon() const noexcept {
        return record.words[14];
    }
    std::uint32_t hurt_weapon() const noexcept {
        return record.words[15];
    }
    std::int32_t target() const noexcept;
    std::int32_t target_health() const noexcept;
    std::int32_t record_tick() const noexcept; // shot +144, evaluation prefix
    RageVector target_point() const noexcept;
    std::uint32_t shot_flags() const noexcept {
        return record.state.words[13];
    }
    script::AimShot script_shot() const noexcept;
};
struct ShotFireSnapshot {
    std::uint32_t marker = 0;
    RageVector origin{}, direction{};
};
static_assert(sizeof(ShotFireSnapshot) == 0x1c);
enum class ShotFeedbackNoticeKind {
    miss,
    hit
};
struct ShotFeedbackNotice {
    ShotFeedbackNoticeKind kind{};
    script::AimShot shot{};
    std::uint32_t reason = 0;
    std::int32_t target = 0, hitgroup = 0, damage = 0;
};
struct ShotFeedbackState {
    explicit ShotFeedbackState(script::Bridge& shared_scripts) noexcept : scripts(shared_scripts) {}
    events::GameEventState events;
    script::Bridge& scripts;
    std::vector<ShotFeedbackRecord> pending;
    std::deque<ShotFireSnapshot> fire_snapshots;
    // Native owner+64 count. The inferred-health view numbers its unused
    // slot zero from+5C; actual433ED9DA positions occupy slots1..10,+68..D3.
    // Only count==1/slot1 participates in inferred-health confirmation.
    std::uint32_t recent_position_count = 0;
    std::array<RageVector, 11> recent_positions{};
    // Captured wrapper arguments. Lua dispatch itself runs immediately, before
    // the native flag/counter updates, through the bound scripts member.
    std::vector<ShotFeedbackNotice> notices;
    struct ExtrapolationShot {
        std::int32_t original_command, id, ticks, simulation_tick;
        bool reported = false;
    };
    std::vector<ExtrapolationShot> extrapolation_shots;
};
// Production entity lookups return identity first. Each field is read only
// when the recovered consumer reaches it; health's engine virtual must not be
// introduced by an earlier exists-only test. Decoded fixture values retain
// the same aggregate construction surface.
template <class T> class ShotFeedbackEntityField {
    struct Reader {
        void* object;
        std::ptrdiff_t offset;
        T (*read)(void*, std::ptrdiff_t);
    };
    std::variant<T, Reader> source_;

public:
    ShotFeedbackEntityField(T value = T{}) noexcept : source_(value) {}
    static ShotFeedbackEntityField live(void* object, std::ptrdiff_t offset,
                                        T (*read)(void*, std::ptrdiff_t)) noexcept {
        ShotFeedbackEntityField result;
        result.source_ = Reader{object, offset, read};
        return result;
    }
    operator T() const {
        if (const auto* value = std::get_if<T>(&source_))
            return *value;
        const auto& reader = std::get<Reader>(source_);
        return reader.read(reader.object, reader.offset);
    }
    ShotFeedbackEntityField& operator=(T value) noexcept {
        source_ = value;
        return *this;
    }
};
struct ShotFeedbackEntity {
    bool exists = false;
    ShotFeedbackEntityField<bool> hidden = false;
    ShotFeedbackEntityField<std::int32_t> life_id = -1, health = 0;
    ShotFeedbackEntityField<float> simulation_time = 0; // decoded 0176A381
    ShotFeedbackEntityField<float> last_shot_time = 0; // decoded AE838771
    ShotFeedbackEntityField<float> field_d5fd2e2c = 0;
    ShotFeedbackEntityField<std::uint32_t> weapon_handle = 0xffffffffU;
};
class ShotFeedbackEngine {
public:
    virtual ~ShotFeedbackEngine() = default;
    virtual std::int32_t current_command() = 0; // engine virtual +334
    virtual std::int32_t local_player_index() = 0; // local networkable +28
    virtual float engine_time() = 0; // engine virtual +38, x87 float return
    virtual ShotFeedbackEntity local_player() = 0;
    virtual ShotFeedbackEntity entity_by_index(std::int32_t) = 0;
    virtual ShotFeedbackEntity entity_by_handle(std::uint32_t) = 0;
    // Model-info virtual +4/+80 and studio hitbox-set lookup. Empty is the
    // native missing-model/set/count path, which classifies the ray as spread.
    virtual std::span<const RageLocalHitbox> hitboxes(std::int32_t model, std::int32_t set) = 0;
    // Genuine engine 43468C3C, reached through 4339FA8E for capsules and boxes.
    virtual int clip_hitbox(const RageRay&, const RageLocalHitbox&, const Matrix3x4&,
                            RageTrace&) = 0;
};
struct ShotFeedbackRuntime {
    BallisticEngine& ballistics;
    ShotFeedbackEngine& engine;
    resolver_runtime::AttemptEngineBinding attempt_engine;
    const RageTrigonometry& trigonometry;
    // Publication4341D449 and feedback433866BD/433240DD all call433B8A2B.
    // Resolver slots, counters, timing and history live in this one cache.
    client::PlayerStateCache& players;
    // Native4347BE0E+i*20 is the canonical adjustment's correction_active byte.
    skeet::players::State& player_adjustments;
    const rage::ShotLogSettings& spread_log_settings; //live owner+48, E82BE8DD
    misc::NormalLog& normal_log; //4339DC04, before Lua miss publication
    float now = 0, interval = 0, receive_interval = 0;
    float update_rate = 0, interpolation = 0, interpolation_ratio = 0;
    float correction_minimum = 0, correction_extra = 0;
    std::int32_t maximum_unlag_milliseconds = 0;
    // Native convar read at 43468EA4; a nonzero value suppresses ray tests.
    bool ray_tests_disabled = false;
    const std::uint8_t* resolver_rework = nullptr;
};

float quantize_shot_coordinate(float) noexcept; // complete 433E104F
RageVector quantize_shot_origin(const RageVector&) noexcept; // 43337967
// Complete 43368C91, including x87 receive-time conversion and ring-head time.
bool shot_record_expired(const ShotRecordView&, const client::PlayerState&, ShotFeedbackRuntime&);
// Complete 433866BD. Reason 0 means unknown/resolver; 1 spread; 2 prediction
// error. Only reason zero updates resolver attempts and miss history.
void classify_shot_ray(ShotFeedbackState&, ShotFeedbackRecord&, const RageVector* origin,
                       const RageVector* direction, ShotFeedbackRuntime&);
// Complete 433240DD: expiry, captured timing, game-event grouping/matching,
// ray reconstruction, hit/miss updates, death/life changes and inferred hits.
void reconcile_shot_feedback(ShotFeedbackState&, ShotFeedbackRuntime&);
// Required-live production forms used by the canonical BA8 owner. The
// two-argument/value-projection forms above remain explicit fixture APIs.
bool shot_record_expired(const ShotRecordView&, const client::PlayerState&, ShotFeedbackRuntime&,
                         const rage::FeedbackEntityBindings&);
void classify_shot_ray(ShotFeedbackState&, ShotFeedbackRecord&, const RageVector* origin,
                       const RageVector* direction, ShotFeedbackRuntime&,
                       const rage::FeedbackEntityBindings&);
void reconcile_shot_feedback(ShotFeedbackState&, ShotFeedbackRuntime&,
                             const rage::FeedbackEntityBindings&);
} // namespace skeet

// Declarations from shot_publication.hpp

// Declarations from spread.hpp

namespace skeet::rage {
struct SpreadRandom {
    void* context;
    // Required genuine vstdlib RandomSeed/RandomFloat bindings.
    void (*seed)(void*, std::int32_t);
    float (*uniform)(void*, float, float);
};
struct SpreadSeed {
    float first_cosine, first_sine, second_cosine, second_sine;
    float first_radius, second_radius;
};
static_assert(sizeof(SpreadSeed) == 0x18);
// Readonly view of a canonical native pointer cell. Indexing resolves its
// current allocation, including initialization and finalizer publication.
class SpreadPlane {
public:
    explicit SpreadPlane(SpreadSeed* const& allocation) noexcept : allocation_(&allocation) {}
    const SpreadSeed* data() const noexcept {
        return *allocation_;
    }
    const SpreadSeed& operator[](std::size_t index) const noexcept {
        return data()[index];
    }
    static constexpr std::size_t size() noexcept {
        return 256;
    }
    const SpreadSeed* begin() const noexcept {
        return data();
    }
    const SpreadSeed* end() const noexcept {
        return data() + size();
    }
    std::span<const SpreadSeed, 256> seeds() const noexcept {
        return std::span<const SpreadSeed, 256>(data(), size());
    }

private:
    SpreadSeed* const* allocation_;
};
struct SpreadTables {
    SpreadPlane standard;
    SpreadPlane shotgun_patterns;
    SpreadTables(SpreadSeed* const& first, SpreadSeed* const& second) noexcept
        : standard(first), shotgun_patterns(second) {}
};
// Numerical loops43402EA8..43402FFD after allocation. Each completed draw or
// trigonometric call writes its native field before the next external call.
// SpreadStorage owns the complete43402E73 allocation/publication sequence.
void fill_spread_plane(std::span<SpreadSeed, 256>, bool shotgun_patterns, const SpreadRandom&,
                       const RageTrigonometry&);

struct SpreadParameters {
    std::uint32_t weapon_id; // weapon context +D8
    std::int32_t fire_mode; // +B0
    float spread; // +D0
    float inaccuracy; // +C4
    float recoil_index; // +100
    std::int32_t pellet_count; // +19C
    bool shotgun_patterns; // resolved ConVar at 43468520
    std::uint8_t weapon_type = 0; // +124; shotgun class is 4
};
struct ShotgunPattern {
    void* context = nullptr;
    // Genuine engine function at 43467DDC. The native code initializes both
    // outputs to zero before calling this binding.
    void (*get)(void*, std::uint32_t, std::int32_t, std::int32_t, float& angle,
                float& radius) = nullptr;
};
// Complete 433C0041, including Revolver secondary and early Negev recoil.
float transform_spread_radius(std::uint32_t weapon_id, std::int32_t fire_mode, float recoil_index,
                              float radius) noexcept;
// Complete 433ADB4F; seed is the native zero-based table index, 0..255.
// Shotgun averaging uses the real engine pattern and RNG dependencies above.
RageSpreadSample generate_spread_sample(const SpreadParameters&, std::uint32_t seed,
                                        const SpreadTables&, const SpreadRandom&,
                                        const ShotgunPattern&, const RageTrigonometry&);

struct SpreadCache {
    std::array<RageSpreadSample, 128> primary;
    std::array<RageSpreadSample, 128> compensated;
    std::uint32_t generation;
};
// Full preparation loop 433C1977..433C1B9A. Enabled is context flags +178 mask10;
// generation is the weapon marker +AC, cached at aim context +1C4. combined_spread
// is weapon context +160; compensated inaccuracy is combined_spread - spread.
bool prepare_spread_cache(SpreadCache&, const SpreadParameters&, float combined_spread,
                          std::uint32_t generation, bool enabled, const SpreadTables&,
                          const SpreadRandom&, const ShotgunPattern&, const RageTrigonometry&);
// The 32-sample consumer at 433600FE advances 0x20 bytes through the 8-byte cache.
std::array<RageSpreadSample, 32> hitchance_spread_samples(const SpreadCache&) noexcept;
} // namespace skeet::rage

// Declarations from spread_miss_log.hpp

namespace skeet::misc {
class NormalLog;
}
namespace skeet::rage {
struct ShotLogSettings;
//43386C0F..43386C65, after every hitbox missed the reconstructed bullet ray.
// This log precedes aim_miss and is independent of the shot's notified bit20.
void log_spread_miss(const ShotLogSettings&, misc::NormalLog&);
}

// Declarations from spread_storage.hpp

namespace skeet::rage {
struct SpreadStorageBindings {
    // Genuine ntdll RtlAllocateHeap/RtlFreeHeap and PEB process-heap binding.
    engine::JobMemory memory;
    const SpreadRandom& random;
    const RageTrigonometry& trigonometry;
};

// Direct views of the retained initialization image's43478C60/43478C64 cells.
// Construction keeps that image alive without resetting either pointer.
// Consumer tables() views and the registered4337D954 finalizer use these same
// cells as the preserved VM; there is no second source pointer-cell owner.
class SpreadStorage {
public:
    explicit SpreadStorage(engine::PreservedInitialization&);
    explicit SpreadStorage(std::shared_ptr<engine::NativeDataImage>);
    SpreadStorage(const SpreadStorage&) = delete;
    SpreadStorage& operator=(const SpreadStorage&) = delete;
    SpreadStorage(SpreadStorage&&) = delete;
    SpreadStorage& operator=(SpreadStorage&&) = delete;

    // Complete43402E73. Ordinary startup supplies bound imports and successful
    // allocations. No allocation-failure continuation or reinit guard exists
    // in the recovered producer.
    // Native return is the address of the first pointer cell, not its payload.
    const SpreadSeed* const* initialize(const SpreadStorageBindings&);
    // Complete4337D954 ->433AD1CD ->433837D0. Explicit CRT finalization owns
    // this operation; ordinary C++ destruction does not add another cleanup.
    void finalize(const SpreadStorageBindings&);
    const SpreadTables& tables() const noexcept {
        return tables_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    std::shared_ptr<engine::NativeDataImage> image_owner() const noexcept {
        return image_;
    }

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    SpreadSeed*& standard_; //actual image cell43478C60
    SpreadSeed*& shotgun_patterns_; //actual image cell43478C64
    const SpreadTables tables_;
};
} // namespace skeet::rage

// Declarations from rage_shot.hpp

namespace skeet {
struct RageShotCandidate {
    std::uint32_t flags;
    RageVector aim_point;
};
struct RageShotState {
    std::uint32_t flags;
    std::int32_t weapon_id;
    float special_gate_threshold;
    RageVector selected_aim_point;
    std::int32_t marker;
};
struct RageShotCommitState {
    std::uint32_t context_flags;
    std::uint32_t command_flags;
    std::int32_t last_committed_marker;
    std::uint8_t send_packet;
};
enum class RageShotCommit {
    attack_button_only,
    recorded
};
// Native accepted-shot commit 0x43355DEF..0x43355EA9. The attack button is
// written before the weapon-64 gate. Later shot history/entity notifications
// after 0x43355EA9 are separate work, not represented as completed here.
RageShotCommit commit_accepted_rage_shot(RageShotState&, RageShotCommitState&, UserCommand&,
                                         const RageShotCandidate&, float special_gate_value,
                                         bool alternate_path, bool send_mode) noexcept;
} // namespace skeet
