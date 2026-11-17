#pragma once
#include "history.hpp"
#include "interpolation.hpp"
#include "entity_state.hpp"
#include "entity_preparation.hpp"
#include "skeet/legit/historical_bones.hpp"
#include "skeet/animation/bone_source_reference.hpp"

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
