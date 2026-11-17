#pragma once
#include "record_selection.hpp"
#include "skeet/animation/bone_runtime.hpp"

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
