#pragma once
#include "records.hpp"
#include "skeet/engine/interface_call.hpp"
#include <span>

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
