#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/effects/model_render.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/lagcomp/entity_preparation.hpp"
#include "skeet/rage/targeting.hpp"
#include <memory>

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
