#pragma once
#include "skeet/animation/bones.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/engine/interface_call.hpp"

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
