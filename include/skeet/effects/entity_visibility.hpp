#pragma once
#include "skeet/esp/effects_config.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/engine/hooks.hpp"
#include "skeet/effects/entity_render_state.hpp"
#include "skeet/esp/model_color_config.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::effects {
// Relationship inputs carry no selected player. Native callbacks choose their
// actual saved entity/local argument at their own read sites; both ConVar
// pointer cells remain live until that per-call binding is formed.
struct EntityVisibilityRelationships {
    void* const& entity_list;
    engine::NativeConVarReference game_type, teammates_are_enemies;
    const players::State& adjustments;
    const rage::TargetOffsets& offsets;
    rage::TargetBindings for_player(void* actual_saved_player) const noexcept;
};
struct EntityVisibilityBindings {
    void* const& engine; //live4346A514,slots82/93 demo/HLTV bypasses
    void* const* local_player; //43475808,read after both actual engine calls
    const EntityVisibilityRelationships& relationships;
};
// Complete4336EE16. Original player slot194 receives both original stack
// words; only the proven teammate branch returns zero without forwarding.
std::uint32_t filter_teammate_render(const esp::EffectsSettings&, const EntityVisibilityBindings&,
                                     void* entity, std::uintptr_t first, std::uintptr_t second);
struct EntityVisibilityRuntime {
    const esp::EffectsSettings& settings;
    const EntityVisibilityBindings& bindings;
    // Adds only slot194 to the already prepared canonical player table. The
    // existing producer remains responsible for its single installation.
    void bind_player_table(engine::VtableHookMetadata&);
    static void* source_entry() noexcept;
};

struct EntityNetworkableOffsets {
    EntityRenderOffsets render;
    std::uint32_t render_fx; //688E0292
    std::uint32_t local_player_flag; //D070684C, bit0
    std::uint32_t observer_mode; //2F4E97C4
    std::uint32_t observer_target; //3B704B41
};
struct EntityNetworkableBindings {
    const EntityVisibilityBindings& visibility;
    void* const& entity_list; //live4346A890
    const std::uint32_t& encoded_ragdoll_convar; //live4346A67C^3155EA46
    const lagcomp::EntityNetvars& animation_offsets;
    EntityNetworkableOffsets offsets;
    const InvalidateEntityPhysics& invalidate_physics; //optional live43468C10
    engine::MemberFunction<void> const& invalidate_render_flags; //optional live43468320
    void* const& globals; //live43467B04; time+10 read after child mode dispatch
    const std::uint32_t& encoded_shadow_history_convar; //live43467A78^3AE06513
    engine::VtableHookRegistry& hook_registry; //canonical43476B2C, outlives owner
};
// Complete4340C3A8, paired slot1 of descriptor43475AEC. The restored slot1
// is entered only after the actual per-object registry node is removed.
void release_hooked_networkable(engine::VtableHookRegistry&, void* receiver);
// Complete433E622F. FLT_MAX means sample the live clock; an already-set
// effect15 with that sentinel returns before any engine interface call.
void set_entity_fade(void*, float start, float duration, const EntityNetworkableBindings&);
class EntityNetworkableRuntime {
public:
    const esp::EffectsSettings& settings;
    const esp::ColoredModelsSettings& models;
    client::PlayerStateCache& players;
    const EntityNetworkableBindings& bindings;
    // Complete433990E9. receiver is the actual entity+8 networkable, stage is
    // forwarded unchanged to its saved original slot5 between both mutations.
    void post_data_update(void* receiver, std::int32_t stage);
    void bind_networkable_table(engine::VtableHookMetadata&);
    static void bind_static_networkable_table(engine::VtableHookMetadata&);

private:
    bool visible_ragdoll_pass(void*);
};
} // namespace skeet::effects
