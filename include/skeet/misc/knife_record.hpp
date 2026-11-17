#pragma once
#include "skeet/lagcomp/extrapolated_record.hpp"

namespace skeet::misc {
// Complete43398A76. Captures the transaction snapshot before loading live or
// newest history, extrapolates, applies entity fields, then rebuilds bones and
// attachments. A negative movement result exits before the entity application.
void apply_extrapolated_knife_record(lagcomp::EntityRecordScope&, client::CommandFrame&,
                                     const lagcomp::ExtrapolatedRecordBindings&,
                                     const std::uint32_t* current_bone_counter,
                                     const animation::BoneRuntime&,
                                     const MovementExtrapolationLiveBindings&,
                                     const client::CommandBudgetBindings&);
} // namespace skeet::misc
