#pragma once
#include "records.hpp"
#include "skeet/resolver_runtime/build.hpp"
#include "skeet/core.hpp"

namespace skeet::lagcomp {
struct HistoryHypothesisPolicy {
    bool enabled;
    bool force;
    std::int32_t game_type;
    std::uint32_t local_handle;
    void* entity_list;
    const std::uint8_t* player_table; // native 4347BE08, stride0x20; value+0x14
    HotkeyBinding hotkey;
    InputBitmap keys;
    std::uint32_t settings_mask;
    std::uint32_t active_mode_mask;
    bool physical_override;
    std::uint32_t input_flags;
};
// Shared complete policy4341639C..43416476 /4336A638..4336A6ED. Entity-list[4]
// and networkable(+8)[10] remain actual engine calls; the hotkey uses the
// existing concrete key bitmap evaluator.
bool should_build_history_hypotheses(const HistoryHypothesisPolicy&, void* player);
// Complete valid-player deferred worker4336A5B3. Its history lookup is provided
// as the recovered HypothesisInput projection, while both bone and hypothesis
// builders execute concrete recovered functions, not internal callback stubs.
void build_deferred_history_record(LagRecord&, void* player, float speed_fraction,
                                   const HistoryHypothesisPolicy&,
                                   const resolver_runtime::HypothesisInput&,
                                   resolver_runtime::HypothesisBuildContext&);
} // namespace skeet::lagcomp
