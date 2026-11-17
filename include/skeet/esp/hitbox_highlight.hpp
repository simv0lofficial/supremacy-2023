#pragma once
#include "config.hpp"
#include "player_runtime.hpp"
#include "skeet/animation/bones.hpp"
#include "skeet/rage/targeting.hpp"

namespace skeet::esp {
struct HitboxHighlightEntry {
    std::int32_t hitbox;
    std::uint32_t flags;
};
struct HitboxHighlightState {
    std::int32_t target_handle = -1; // full aimbot visualization owner +180
    std::array<HitboxHighlightEntry, 4> entries{}; // +184..+1A3
};
struct HitboxHighlightTarget {
    RuntimePlayer* player = nullptr;
    std::uint32_t movement_flags = 0;
    const animation::BoneModel* model = nullptr;
    // Studio hitbox order, its actual bone-index mapping, and the entity's
    // live bone-cache matrices. Model hitbox_bones matches hitboxes.size().
    std::span<const RageLocalHitbox> hitboxes;
    std::span<const Matrix3x4> bones;
};
struct HitboxHighlightContext {
    bool active; // native global byte 4346A65C
    const Projection& projection;
    RageVector view_origin; // 43479CCC
    const RageTrigonometry& trigonometry;
    const rage::TargetBindings& targets;
    void* engine_context = nullptr;
    // Genuine entity-list GetClientEntityFromHandle plus model/netvar/cache
    // reads; eligibility, hitbox selection and all drawing remain concrete.
    HitboxHighlightTarget (*by_handle)(void*, std::int32_t) = nullptr;
};
// Complete 433AFDF0: 12 capsule support directions, floating-point projection,
// and a 36-vertex center/transparent-edge triangle fan, native queue opcode7.
void draw_capsule_highlight(ui::RenderQueue&, const Projection&, const RageVector& view_origin,
                            const RageCapsule&, std::uint32_t color, const RageTrigonometry&);
// Complete successor 433D12F7. Both config booleans form only the outer gate;
// entry flags select its color. Invalid target eligibility clears the handle,
// while model/radius/index/projection exits retain it and prior queued draws.
void draw_hitbox_highlight(ui::RenderQueue&, const AimbotVisualizationConfig&,
                           HitboxHighlightState&, const RuntimePlayer& local,
                           const HitboxHighlightContext&);
} // namespace skeet::esp
