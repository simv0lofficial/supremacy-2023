#pragma once
#include "single_target.hpp"
#include "multiple_targets.hpp"

namespace skeet::rage {
// Refreshed timing/reference owners for one acquisition branch. Production
// record paths use records.bone_runtime for fresh per-target model/pose reads;
// existing semantic helper bindings retain their separate fixture contracts.
struct AcquisitionBranchFrameBindings {
    TargetBindings targeting;
    lagcomp::HistoryTickTiming prediction_timing, scanner_timing, extrapolation_timing,
        history_timing;
    lagcomp::EntityPreparationContext prediction_preparation, scanner_preparation,
        extrapolation_preparation, history_preparation;
    lagcomp::RecordSelectionContext prediction_records;
    lagcomp::ExtrapolatedRecordBindings extrapolation, history_extrapolation;
    lagcomp::HistorySelectionBindings history;
    TargetPreparationBindings preparation;
    RecordCandidateBindings record_candidates;
    ScanContext scanner;
    AcquisitionRecordBindings records;
    BodyAimProbeContext body_probe;
    TargetPredictionBindings prediction;
    AcquisitionBranchFrameBindings(PreambleState&, const client::CommandFrame&,
                                   float prediction_time, std::int32_t remembered_target_handle,
                                   std::uint32_t scheduler_flags,
                                   const std::array<RageSpreadSample, 32>&,
                                   const TargetPreparationBindings&, const BodyAimProbeContext&,
                                   const TargetPredictionBindings&, const RecordCandidateBindings&,
                                   const AcquisitionRecordBindings&,
                                   const players::State& adjustments,
                                   const ui::SettingsMenuValues&);
    AcquisitionBranchFrameBindings(const AcquisitionBranchFrameBindings&) = delete;
    AcquisitionBranchFrameBindings& operator=(const AcquisitionBranchFrameBindings&) = delete;
};
class AcquisitionFrameBindings {
public:
    // Construct immediately after prepare_rage_command. All owned reference
    // targets outlive .single/.multiple and every queued scan until its drain.
    AcquisitionFrameBindings(PreambleState&, const PreambleResult&, float live_prediction_time,
                             std::int32_t live_yaw_target_handle,
                             std::uint32_t live_scheduler_flags, const SingleTargetBindings&,
                             const MultipleTargetBindings&, const ui::SettingsMenuValues&);
    AcquisitionFrameBindings(const AcquisitionFrameBindings&) = delete;
    AcquisitionFrameBindings& operator=(const AcquisitionFrameBindings&) = delete;
    const std::array<RageSpreadSample, 32> samples;
    AcquisitionBranchFrameBindings single_frame, multiple_frame;
    SingleTargetBindings single;
    MultipleTargetBindings multiple;
};
// The post-firing stop projection changes the selected frame and clock again.
// Owned reference targets remain alive through its prediction and scan calls.
class TargetFrameBindings {
public:
    TargetFrameBindings(const TargetPredictionBindings&, ScanContext&, const client::CommandFrame&,
                        float live_prediction_time, std::uint32_t live_scheduler_flags);
    TargetFrameBindings(const TargetFrameBindings&) = delete;
    TargetFrameBindings& operator=(const TargetFrameBindings&) = delete;
    lagcomp::HistoryTickTiming prediction_timing, scanner_timing;
    lagcomp::EntityPreparationContext prediction_preparation, scanner_preparation;
    lagcomp::RecordSelectionContext prediction_records;
    BodyAimProbeContext body_probe;
    TargetPredictionBindings prediction;
    ScanContext scanner;
};
} // namespace skeet::rage
