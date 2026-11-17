#pragma once
#include "live_record_attachments.hpp"

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
