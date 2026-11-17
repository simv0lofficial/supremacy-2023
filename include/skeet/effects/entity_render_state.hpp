#pragma once
#include "skeet/effects/camera.hpp"
#include "skeet/esp/effects_config.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <cstdint>
#include <span>

namespace skeet::effects {

// Native list entries occupy eight bytes at argument+0x44. Only the first
// dword is consumed; the signed count is stored at argument+0x4044.
struct EntityRenderListEntry {
    std::uint32_t index;
    std::uint32_t untouched;
};
static_assert(sizeof(EntityRenderListEntry) == 8);

struct EntityRenderOffsets {
    std::uint32_t ragdoll_yaw; // 73DBBB49
    std::uint32_t render_flags; // 38CA5228
    std::uint32_t ragdoll_owner; // 9C14F5A7
    std::uint32_t hidden_byte_1b5f47d5;
    std::uint32_t render_mode; // FC2723F7; hidden mode is 10
    std::uint32_t reset_byte_8481ea04;
    std::uint32_t origin_source; // 258BF82B
    std::uint32_t origin_copy; // A9EE6062
    std::uint32_t origin_invalidated; // 2A671B42
    std::uint32_t angles_invalidated; // 3B1F6ECC
    std::uint32_t angles_copy; // C865F10E
};

using InvalidateEntityPhysics = engine::MemberFunction<void, std::int32_t>;
struct EntityRenderBindings {
    void* const& entity_list; //live4346A890
    void* const& ragdoll_vtable; //live43467DC0, entity's primary vtable
    engine::NativeConVarReference ragdoll_convar; //live4346A67C XOR3155EA46
    InvalidateEntityPhysics const& invalidate_physics; //live optional43468C10
    EntityRenderOffsets offsets;
    const lagcomp::EntityNetvars& animation_offsets;
};

// Complete 43326129. The original observer update is performed first through
// its restored owner. Ragdoll animation uses the same PlayerStateCache and
// newest LagRecord as capture/resolver; no second history or entity model is
// created. weapon_models_enabled references ColoredModelsSettings::weapons
// enabled.value directly: receiver=full owner+0x18, receiver+0x394=full+0x3AC,
// descriptor hash 92EBAD93. It gates the actual entity slot 159 predicate
// followed by slot 238(false).
// Each supplied entry belongs to the original positive-count list; entries
// with index<=64 or any high sixteen bits are skipped exactly as native.
void update_entity_render_state(CameraRuntime&, client::PlayerStateCache&,
                                const EntityRenderBindings&, void* observer_player,
                                std::span<const EntityRenderListEntry>, const esp::EffectsSettings&,
                                const std::uint8_t& weapon_models_enabled);

// Live native receiver form. Packet count and entry storage are inspected
// after the observer update and the ConVar/configuration gates, at 43326184.
// A nonpositive signed count reads no entries.
void update_entity_render_state_packet(CameraRuntime&, client::PlayerStateCache&,
                                       const EntityRenderBindings&, void* observer_player,
                                       const void* packet, const esp::EffectsSettings&,
                                       const std::uint8_t& weapon_models_enabled);

} // namespace skeet::effects
