#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/client/player_state.hpp"
#include <vector>
#include <optional>

namespace skeet::client {
struct CommandBudgetBindings;
}

namespace skeet::rage {
struct RecordCandidateBindings {
    const void* lag_compensation; // decoded ConVar43467A78 XOR3AE06513
    void* engine_client; // actual engine slots12(max clients),14(time)
    float current_time; // pointed global43467EB8
    float interval, receive_interval;
    float interpolation; // global43476928
    std::int32_t maximum_unlag_ms; // global43468230
    std::optional<std::int32_t> prediction_limit; // explicit decoded fixture input only
    const client::CommandBudgetBindings* live_budget = nullptr;
};

// Complete433AEBA8. Appends simulation ticks to the existing output, retaining
// its prefix and the native priority/cap rules. The canonical player cache is
// supplied by the acquisition owner; the network index still comes from the
// actual target virtual. Its nonzero index resolves the supplied PlayerState.
// Rings contain at most32 nonnull records.
void append_target_record_ticks(const client::CommandFrame&, void* target, client::PlayerState&,
                                std::uint32_t maximum, const RecordCandidateBindings&,
                                std::vector<std::int32_t>& output);
std::vector<std::int32_t> collect_target_record_ticks(const client::CommandFrame&, void* target,
                                                      client::PlayerState&, std::uint32_t maximum,
                                                      const RecordCandidateBindings&);
} // namespace skeet::rage
