#pragma once
#include "timeline.hpp"

namespace skeet::client {
struct CommandBudgetBindings;
}
namespace skeet::lagcomp {
// Live433F94AA. Calls concrete433A13ED at each native pattern clamp; the
// arrival budget is captured before engine time14. timing.prediction_limit
// belongs to the separate semantic entry and is not read by this overload.
std::int32_t estimate_missing_ticks(const LagTimeline&, const MissingTickTiming&,
                                    const client::CommandBudgetBindings&);
} // namespace skeet::lagcomp
