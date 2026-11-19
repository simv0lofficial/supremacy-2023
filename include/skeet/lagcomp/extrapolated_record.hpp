#pragma once
#include "record_selection.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/resolver_runtime/build.hpp"
#include "skeet/rage/experimental_switch.hpp"
#include <optional>

namespace skeet {
struct MovementExtrapolationBindings;
struct MovementExtrapolationLiveBindings;
}
namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet::client {
struct CommandBudgetBindings;
}
namespace skeet::lagcomp {

struct ExtrapolatedRecordBindings {
    client::PlayerStateCache& players;
    const EntityPreparationContext& preparation;
    const RecordAttachmentContext& attachments;
    animation::BoneSourceReference<resolver_runtime::HypothesisBuildContext> bones;
    const MovementExtrapolationBindings& movement;
    std::optional<std::int32_t> prediction_limit; // decoded fixture result; absent in live entry
    rage::ExperimentalSwitch validate_record{};
    rage::ExperimentalSwitch rebuild_pose{};
};

// Complete4339C8E2. Unlike the hypothesis builder's80300 mask, this entry
// requests300, takes the origin/yaw from the destination record and publishes
// flag4/clears20 whenever the engine/model build succeeds, even with no set.
void build_extrapolated_record_matrices(LagRecord&, resolver_runtime::HypothesisBuildContext&,
                                        std::span<Matrix3x4, 256> scratch);
// Live4339C8E2 entry. The model/set is read from this player after the build.
void build_extrapolated_record_matrices(void* player, LagRecord&, const animation::BoneRuntime&,
                                        std::span<Matrix3x4, 256> scratch, bool strict = false,
                                        float interval = 0.0f);

// Concrete input projection for433FAC08. The original valid-player contract
// requires its animation state for the ensuing hypothesis computation.
resolver_runtime::HypothesisInput
capture_record_hypothesis_input(void* player, const client::PlayerState&, const LagRecord&,
                                const EntityStateRuntime&, bool use_record_yaw = false) noexcept;

// Complete43387BCE transaction. The networkable index/cache lookup is repeated
// here as in native. A stale/empty history captures live state; the other path
// copies all10A0 bytes. A negative movement result exits before flags/bones/
// attachments. Destination.field_00 is copied or left by capture unchanged.
bool capture_extrapolated_record(client::CommandFrame&, void* player, LagRecord& destination,
                                 std::uint32_t& output_flags, const ExtrapolatedRecordBindings&);
// Full live timeline/movement/bone entry. Budget reads occur inside433F94AA,
// after source capture and at its three concrete clamp sites.
bool capture_extrapolated_record(client::CommandFrame&, void* player, LagRecord& destination,
                                 std::uint32_t& output_flags, const ExtrapolatedRecordBindings&,
                                 const animation::BoneRuntime&,
                                 const MovementExtrapolationLiveBindings&,
                                 const client::CommandBudgetBindings&,
                                 std::int32_t* predicted_ticks = nullptr);

} // namespace skeet::lagcomp
