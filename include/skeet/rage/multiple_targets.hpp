#pragma once
#include "acquisition.hpp"
#include "preamble.hpp"
#include "single_target.hpp"
#include "body_aim_probe.hpp"

namespace skeet::rage {
struct MultipleTargetBindings {
    const TargetPreparationBindings& preparation;
    const BodyAimProbeContext& body_probe;
    const TargetPredictionBindings& prediction;
    const RecordCandidateBindings& record_candidates;
    const AcquisitionRecordBindings& records;
    const players::State& adjustments;
    std::int32_t maximum_clients;
    engine::JobDispatcher& jobs;
};
// Complete multi-target acquisition43351C37..433547C9. The canonical
// preamble owns flags/masks/completion and storage supplies the same candidates
// consumed by the subsequent rank_completed_target_candidates call.
void acquire_multiple_targets(PreambleState&, PreambleResult&, AcquisitionStorage&,
                              const MultipleTargetBindings&);
} // namespace skeet::rage
