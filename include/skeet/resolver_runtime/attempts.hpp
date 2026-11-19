#pragma once
#include "skeet/resolver.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::lagcomp {
struct LagRecord;
}

namespace skeet::resolver_runtime {

struct RecordHistory {
    std::array<ResolverRecord*, 32> records;
    std::uint32_t first;
    std::uint32_t count;
};
struct LagRecordHistory {
    std::array<lagcomp::LagRecord*, 32> records;
    std::uint32_t first;
    std::uint32_t count;
};

struct SelectionState {
    // Native owner +10 + mode*24. Modes 1 and 2 share their winning attempt
    // when feedback publishes a new selection (+34 and +58 respectively).
    std::array<std::uint8_t, 4> attempt_by_mode;
    std::uint8_t status; // native owner +A0
};

// 0x43349D0C..0x43349D27: convert the winning hypothesis pose to attempt.
std::uint8_t classify_winning_pose(float pose_yaw) noexcept;

// 0x4336C64E..0x4336C674, before the selected lag record is copied.
void advance_pending_attempt(ResolverRecord& record, std::uint32_t options) noexcept;
// Direct adapter for the complete captured record layout, without projecting or
// copying the animation layers and unrelated capture fields.
void select_variant(lagcomp::LagRecord& record, std::uint8_t attempt) noexcept;
constexpr std::size_t yaw_variant_index(std::uint8_t attempt) noexcept {
    return (attempt % 3u + 1u) % 3u;
}
void select_yaw_variant(lagcomp::LagRecord& record, std::uint8_t attempt) noexcept;
void advance_pending_attempt(lagcomp::LagRecord& record, std::uint32_t options) noexcept;

// Shared transition at 0x43349D92..0x43349E21 and
// 0x43413823..0x4341389A. Processing stops at the first incompatible record;
// flag 0x200 stops propagation after that record has been updated.
std::uint32_t propagate_attempt(RecordHistory& history, std::uint8_t mode,
                                std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;
std::uint32_t propagate_attempt(LagRecordHistory& history, std::uint8_t mode,
                                std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;

// The shot-feedback owner marks its source even when propagation stops early.
void apply_feedback(RecordHistory& history, ResolverRecord& source, std::uint8_t previous_attempt,
                    std::uint8_t new_attempt) noexcept;
void apply_feedback(LagRecordHistory& history, lagcomp::LagRecord& source,
                    std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;

// 0x43349D69..0x43349E25: publish the selected attempt, synchronize the two
// movement modes, propagate it through compatible history, and mark the source.
void publish_feedback(SelectionState& state, RecordHistory& history, ResolverRecord& source,
                      std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;
void publish_feedback(SelectionState& state, LagRecordHistory& history, lagcomp::LagRecord& source,
                      std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;

} // namespace skeet::resolver_runtime
