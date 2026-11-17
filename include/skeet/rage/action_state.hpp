#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/esp/hitbox_highlight.hpp"
#include "preamble.hpp"
#include "single_target.hpp"
#include "skeet/lagcomp/history_selection.hpp"
#include "skeet/movement/collision.hpp"
#include "skeet/animation/bone_runtime.hpp"

namespace skeet::rage {
class FeedbackOwner;
struct ActionStateBindings {
    bool active; // actual global byte4346A65C
    const esp::AimbotVisualizationConfig& visualization;
    PreambleState& state;
    const PreambleBindings& preamble;
    const TargetPreparationBindings& target_preparation;
    const players::State& player_adjustments;
    FeedbackOwner* const& feedback_owner; //canonical43475A7C slot; resolved after health getter
    const lagcomp::HistorySelectionBindings& history;
    ScanContext& scan;
    MovementCollisionEngine& collision;
    std::uint8_t* trace_flag;
    // Production history/attachments always read the action's actual target.
    const animation::BoneRuntime& bone_runtime;
    const MovementExtrapolationLiveBindings& live_movement;
    const client::CommandBudgetBindings& live_budget;
};
// Native433320D4, called at4335773F with the selected frame/player/command.
// Produces the same four hitbox flags and target handle consumed by433D12F7.
// The configuration owner is represented by its existing typed settings and
// shared RAGE state in ActionStateBindings, rather than another native layout.
void update_action_state(esp::HitboxHighlightState&, client::CommandFrame&, void* local_player,
                         client::Command&, const ActionStateBindings&);
} // namespace skeet::rage
