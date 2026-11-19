#pragma once
#include "interpolation.hpp"

namespace skeet::lagcomp {
// Retained native scratch layout. The reconstructed inverse transform now
// derives its translation from the current rotation instead of these incoming
// stack values, which were read before initialization in the captured code.
struct HistoryMatrixScratch {
    Vector inverse_row1; // stackA0/A4/A8
    Vector inverse_row2; // stackB0/B4/B8
};
// Complete4336C968..D739, including primary and all three hypothesis matrices.
// Destination is the record already selected/copied by4336C572..C692.
float interpolate_history_record(LagRecord& destination, const LagRecord& newer,
                                 const LagRecord& older, float requested_time, float interval,
                                 const HistoryMatrixScratch&, std::uint32_t& output_flags,
                                 const RotationMath&);
} // namespace skeet::lagcomp
