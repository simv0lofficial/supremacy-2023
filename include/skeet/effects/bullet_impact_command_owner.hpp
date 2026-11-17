#pragma once
#include "camera.hpp"
#include "world.hpp"
#include "entity_render_state.hpp"
#include "model_render.hpp"
#include "skeet/client/command_empty_tick.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/rage/targeting_commit.hpp"

namespace skeet::effects {
struct BulletImpactCommandBindings {
    void** const& game_rules; //4346A3A4, then the actual rules object
    std::ptrdiff_t game_rule_override; //D720AEA6, byte; complete433DF22C gate
    std::ptrdiff_t weapon_definition; //5C9F0FBB, read at both native consumers
    void* const& debug_overlay; //43468260, actual virtual1 AddBoxOverlay
    engine::NativeConVarReference shotgun_patterns; //live43468520 XOR394BFF56, read during spread
    BallisticEngine& ballistics;
    void* const* trace_filter_vtable; //genuine external43475B90, slots0..4
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    const RageTrigonometry& trigonometry;
};
// One native effects/model owner4348E6E2 supplies normal434700E4,
// HUD434704B8 and empty-tick4347270C. These are references to its existing
// canonical state; this adapter does not construct another camera/world owner.
struct EffectsCommandOwner {
    CameraRuntime& camera;
    WorldRuntime& world;
    ModelRenderRuntime& models;
    client::PlayerStateCache& players;
    const EntityRenderBindings& entities;
    const BulletImpactCommandBindings& bindings;
    client::NativeCommandEmptyTickReceiver empty_tick_receiver{};
};
// Complete normal phase6 body4339EC1B. Uses live weapon-definition getters,
// command angles/seed, frame recoil/spread/origin, concrete direction ballistics
// and an engine debug box for every recorded penetration/impact point.
void process_bullet_impacts(EffectsCommandOwner&, const client::CommandFrame&, void* player,
                            const client::Command&);
// Whole4346FAFC slot1, consuming the actual incoming packet only after the
// camera/ragdoll gates in the existing43326129 implementation have run.
void update_effects_entities(EffectsCommandOwner&, void* player, std::uintptr_t incoming);
} // namespace skeet::effects
