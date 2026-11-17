#pragma once
#include "skeet/client/player_state.hpp"
#include "skeet/lagcomp/entity_preparation.hpp"
#include "skeet/legit/historical_bones.hpp"

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
