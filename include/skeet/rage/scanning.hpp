#pragma once
#include "targeting.hpp"
#include "skeet/rage_points.hpp"
#include "targeting_commit.hpp"
#include "scan_hypotheses.hpp"
#include <atomic>

namespace skeet::anti_aim {
class TargetTraceEntities;
}

namespace skeet::rage {
struct ExperimentalDamageContext;

// Per-record settings copied by433543BF into evaluation+BC..E3. Fields unused
// by the scanner stay with the scheduler's canonical evaluation owner.
struct ScanPolicy {
    RageHitboxMasks hitboxes; // C0/C4
    float point_scale; // C8
    std::uint32_t multipoint_mask; // CC
    std::int32_t minimum_damage; // D0
    float minimum_chance; // D4
    std::uint32_t body_disabler_reasons; // B8
    bool prefer_safe, prefer_body; // DC/DD
    bool additional_hypotheses; // E1
};

// Views into the scheduler's existing target/record owners, not another copy
// of the native evaluation allocation. E8 counts concrete ballistic evaluations.
struct ScanEvaluation {
    TargetCandidate& candidate;
    lagcomp::LagRecord& record;
    const lagcomp::LagRecord* source_record;
    ScanPolicy policy;
    std::uint32_t& ballistic_evaluations;
};
struct ScanModelOffsets {
    std::ptrdiff_t studio_header; //32E4DAF1
    std::ptrdiff_t hitbox_set; //C2B6FB10
    std::ptrdiff_t bone_cache; //17978090
    std::ptrdiff_t network_origin; //A9EE6062
};
struct ScanContext {
    BallisticEngine& ballistics;
    const TargetBindings& targeting;
    BallisticEntity source; //actual native43475808 entity address
    void* weapon;
    std::ptrdiff_t weapon_item_offset;
    const std::uint8_t& allow_penetration; //live BooleanConfig.value4346A3E0, read per query
    std::int32_t weapon_id; //aim owner+D8, distinct from weapon's netvar
    RageVector origin; //aim owner+DC
    float spread; //aim owner+160
    std::int32_t record_count; //aim owner+19C
    std::int32_t team; //aim owner+1D0
    std::int32_t accuracy_percent; //configuration+2C
    std::uint32_t& configuration_flags; //configuration+178
    std::span<std::uint32_t, 2> target_mask; //configuration+1AC
    const std::array<RageSpreadSample, 32>& samples;
    ScanModelOffsets model_offsets;
    RageTrigonometry trigonometry;
    RageBoxClipper box_clipper;
    client::PlayerStateCache& players;
    const lagcomp::EntityPreparationContext& preparation;
    ScanHypothesisOffsets hypothesis_offsets;
    legit::AttachmentUpdate update_attachments;
    FinalTeammateConvars teammate_convars;
    // Genuine external43475B90 table and actual entity virtual/netvar reads.
    // The43470404 rejection predicate is recovered source, per-call storage.
    void* const* trace_filter_vtable;
    const anti_aim::TargetTraceEntities& trace_entities;
    const ExperimentalDamageContext* damage_context = nullptr;
};
struct ScanRequest {
    void* target_player;
    BallisticEntity target_entity; //actual native target address, not its index
    ScanEvaluation evaluation;
    std::uint32_t descriptor_flags;
    std::span<std::uint32_t, 3> completed;
    std::uint32_t ordinal;
};

// Native4335CD37..43364782. The context owns concrete engine/model bindings;
// generation, geometry, ballistics, refinement and admission remain source.
void scan_candidate(ScanContext&, ScanRequest&);

} // namespace skeet::rage
