#pragma once
#include "skeet/client/player_state.hpp"
#include "skeet/animation/bones.hpp"
#include "skeet/engine/interface_call.hpp"

namespace skeet::client {
// Parsed arguments of Lua client.draw_hitboxes. Only yaw/alternative set native
// option bits; omitted tick retains the simulation-time default.
struct ClientHitboxesRequest {
    std::int32_t entity_index;
    float duration;
    std::uint32_t hitbox_mask;
    std::uint32_t color;
    bool has_tick;
    std::int32_t tick;
    float yaw;
    std::int32_t alternative;
    std::uint32_t flags;
};
struct ClientHitboxOffsets {
    std::uint32_t simulation_time;
    std::uint32_t studio_wrapper;
    std::uint32_t hitbox_set;
    std::uint32_t bone_cache;
    std::uint32_t absolute_angles;
    std::uint32_t origin;
    std::uint32_t absolute_origin;
    std::uint32_t entity_flags;
    std::uint32_t field_432b1f5b;
    std::uint32_t field_0d560fac;
    std::uint32_t bone_accessor;
};
struct ClientHitboxRuntime {
    PlayerStateCache& players;
    ClientHitboxOffsets offsets;
    void* debug_overlay;
    RageTrigonometry trigonometry;
    double (*arc_tangent)(double, double); // binding for recorded __libm_sse2_atan2
    // Actual optional engine target43468F80, called by43341E47 with mask300.
    engine::MemberFunction<void, Matrix3x4*, std::uint32_t> update_attachments;
    const float& interval;
    const std::int32_t& current_tick;
    const float& current_time;
};
// Complete433B60DD. The two debug-overlay virtuals are the only draw boundary.
void draw_client_hitbox(const ClientHitboxRuntime&, const RageLocalHitbox&, const Matrix3x4&,
                        float duration, std::uint32_t color);
// Complete post-parse4334C981, including433E004F history search and4338AC35
// scatter/attachments/select transaction. Entity was resolved by the Lua entry.
// Valid studio models have at most19 hitboxes and bone indices below256.
void draw_client_hitboxes(const ClientHitboxRuntime&, void* entity, const ClientHitboxesRequest&);
} // namespace skeet::client
