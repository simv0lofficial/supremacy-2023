#pragma once
#include "acquisition.hpp"
#include "targeting_pipeline.hpp"

namespace skeet::rage {
struct FinalTargetContextBindings;
// The sorted pointer list refers to the canonical contiguous candidate array.
// Record/settings metadata retains that original slot, rather than sort order.
const AcquisitionEvaluation& acquisition_evaluation_for(const AcquisitionStorage&,
                                                        const TargetCandidate&) noexcept;
// Native43354AD8..43354C61 using the complete engine-input consumer on each
// attempted point. Reads for unattempted candidates are never performed.
TargetShotChoice select_acquired_target_for_shot(TargetBatch&, AcquisitionStorage&,
                                                 client::CommandFrame&, PreambleState&,
                                                 const FinalTargetContextBindings&);
} // namespace skeet::rage
