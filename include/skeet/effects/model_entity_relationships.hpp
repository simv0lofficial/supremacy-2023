#pragma once
#include "skeet/engine/startup_convars.hpp"

namespace skeet::effects {
struct ModelEntityBindings;
struct ModelRelationshipOffsets {
    std::uint32_t life_state, move_type, observer_mode, observer_target, team, survival_team;
};
// The teammate and camera helpers need no render frame or timing snapshot.
// Read local_player after the genuine engine93 call; read each ConVar only
// when its native branch reaches it, after observer resolution if applicable.
struct ModelEntityRelationships {
    void* engine;
    void* entity_list;
    void* const& local_player;
    ModelRelationshipOffsets offsets;
    engine::NativeConVarReference game_type;
    engine::NativeConVarReference teammates_are_enemies;
};
bool model_teammate(void*, const ModelEntityRelationships&);
bool model_teammate(void*, const ModelEntityBindings&);
} //namespace skeet::effects
