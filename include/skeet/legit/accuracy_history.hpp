#pragma once
#include "acquisition.hpp"
#include "skeet/lagcomp/interpolation.hpp"

namespace skeet::legit {
struct AccuracyMatrixScratch {
    Vector inverse_row1; // automatic433EEDFF frame+80/84/88, opaque first use
    Vector inverse_row2; // automatic frame+90/94/98
};
// Complete433EEDFF. Unlike43367778, a noninterpolated success still returns
// the requested tick. Source/destination do not alias; hitbox is0 or3 here.
std::int32_t select_accuracy_hitbox(const lagcomp::RecordRing&, std::int32_t requested_tick,
                                    std::uint32_t hitbox, Vector& origin, Matrix3x4&,
                                    float interval, AccuracyMatrixScratch&,
                                    const lagcomp::RotationMath&);
struct AccuracyHistoryRay {
    Vector origin, direction;
    float maximum_origin_distance;
};
struct StudioOffsets;
// Complete433983BB after the model/studio-hitbox descriptor lookup. Traverses
// the inclusive tick window backwards and updates both outputs only for a
// strictly better cosine. newest_origin is the newest history origin, or the
// live decodedA9EE6062 origin for an empty history.
void search_accuracy_history(const lagcomp::RecordRing&, const AccuracyHistoryRay&,
                             const Vector& newest_origin, const AcquisitionHitbox&,
                             std::uint32_t hitbox, std::int32_t first_tick, std::int32_t last_tick,
                             std::int32_t& best_tick, float& best_cosine, float interval,
                             AccuracyMatrixScratch&, const lagcomp::RotationMath&);
// Whole433983BB owner, including the nullable wrapper/header/hitbox-set gate,
// native descriptor lookup and newest-record/live-origin reference selection.
void search_target_accuracy_history(const void* target, const StudioOffsets&,
                                    std::ptrdiff_t origin_offset, const lagcomp::RecordRing&,
                                    const AccuracyHistoryRay&, std::uint32_t hitbox,
                                    std::int32_t first_tick, std::int32_t last_tick,
                                    std::int32_t& best_tick, float& best_cosine, float interval,
                                    AccuracyMatrixScratch&, const lagcomp::RotationMath&);
} // namespace skeet::legit
