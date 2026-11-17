#pragma once
#include "targeting.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::rage {
// Process relationships are sampled at the actual enemy-helper invocation,
// after engine/event callbacks have had the opportunity to replace the cells.
struct LiveTargetRelationships {
    void* const& entity_list;
    engine::NativeConVarReference game_type;
    engine::NativeConVarReference teammates_are_enemies;
    const players::State& adjustments;
    const TargetOffsets& offsets;
    TargetBindings capture(void* local_player) const noexcept {
        return {local_player, entity_list, game_type, teammates_are_enemies, adjustments, offsets};
    }
};

// The explicit standalone view keeps existing pure-function fixtures useful;
// production borrows LiveTargetRelationships over canonical process cells.
class TargetRelationshipBindings {
public:
    TargetRelationshipBindings(const TargetBindings& standalone) noexcept
        : standalone_(&standalone) {}
    TargetRelationshipBindings(const LiveTargetRelationships& live) noexcept : live_(&live) {}
    TargetRelationshipBindings(TargetBindings&&) = delete;
    TargetRelationshipBindings(LiveTargetRelationships&&) = delete;
    TargetBindings capture(void* local_player) const noexcept {
        if (live_)
            return live_->capture(local_player);
        auto result = *standalone_;
        result.local_player = local_player;
        return result;
    }

private:
    const TargetBindings* standalone_ = nullptr;
    const LiveTargetRelationships* live_ = nullptr;
};
} //namespace skeet::rage
