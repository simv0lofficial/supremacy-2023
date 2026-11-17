#pragma once
#include "extrapolated_record.hpp"
#include "history_alternative.hpp"
#include "matrix_interpolation.hpp"

namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet {
struct MovementExtrapolationLiveBindings;
}
namespace skeet::client {
struct CommandBudgetBindings;
}
namespace skeet::lagcomp {
struct HistorySelectionBindings {
    const ExtrapolatedRecordBindings& extrapolation;
    const HistoryAlternativeBindings& alternative;
    const RotationMath& rotation;
    const HistoryMatrixScratch& matrix_scratch;
    float current_time; // actual *43467EB8
    float interpolation; // actual43476928
    std::int32_t maximum_unlag_ms; // actual43468230
};
// Complete4336C53B for a valid nonzero player networkable index. selected_source, if requested,
// mirrors destination.field_00 without truncating the portable source pointer.
bool select_target_history_record(client::CommandFrame&, void* player, std::int32_t requested_tick,
                                  std::uint32_t options, LagRecord& destination,
                                  std::uint32_t& output_flags, const HistorySelectionBindings&,
                                  const LagRecord** selected_source = nullptr);
bool select_target_history_record(client::CommandFrame&, void* player, std::int32_t requested_tick,
                                  std::uint32_t options, LagRecord& destination,
                                  std::uint32_t& output_flags, const HistorySelectionBindings&,
                                  const animation::BoneRuntime&,
                                  const MovementExtrapolationLiveBindings&,
                                  const client::CommandBudgetBindings&,
                                  const LagRecord** selected_source = nullptr,
                                  std::int32_t* predicted_ticks = nullptr);
} // namespace skeet::lagcomp
