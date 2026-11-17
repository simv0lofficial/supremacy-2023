#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/rage/shot_feedback.hpp"

namespace skeet::rage {
class FeedbackOwner;
struct PendingShotEntityOffsets {
    std::ptrdiff_t life_count; // CA5EC698, captured minus one
    std::ptrdiff_t model_index; // 47877DEB, signed short
    std::ptrdiff_t hitbox_set; // C2B6FB10
};
struct PendingShotPublicationBindings {
    FeedbackOwner*& active; // actual nullable43475A7C cell, full primary owner
    const std::uint8_t& recording_enabled; // actual4346754C, read independently
    client::PlayerStateCache& players;
    void* engine_client;
    void* entity_list;
    void* weapon_system; // actual43467B54, virtual2; null uses native4346F1B8
    const void* life_count_convar; // decoded434670D0 ^ 1320FFAA
    PendingShotEntityOffsets offsets;
    const float* current_time; // *4346A334
    const float* prediction_time; // *43467EB8
    const float* interval; // *4346A338
    const std::uint32_t* scheduler_flags; //434759C0
    resolver_runtime::AttemptEngineBinding attempts;
    const std::uint8_t* resolver_rework = nullptr;
};

// Complete433CCA5C. The original null-system path copies4346F1B8; otherwise
// real weapon-system virtual2 supplies data. The upper identifier word stays
// untouched on the nonnull path, matching the native16-bit store.
void capture_shot_weapon_bullet(void* weapon_system, std::uint16_t identifier,
                                client::CommandBullet&);

// Complete433D1221. The entity index must be nonzero, as on the native valid
// path. Cache acquisition precedes entity-list virtual3. A missing entity
// leaves the destination unchanged; a present entity captures all0x1190 bytes.
void shot_capture_target(ShotFeedbackRecord&, const lagcomp::EvaluationState&,
                         const RageVector& origin, std::uint8_t low_flags,
                         const PendingShotPublicationBindings&);

// Complete4341D449. Native ABI: ECX=frame, EDX=shot_index, stack=evaluation.
// Appends the canonical0x1220 pending record before reading engine state, then
// captures the full evaluation and updates the same PlayerStateCache resolver
// owner used by history acquisition. Resolves active at entry, after preceding
// callbacks; only the native captured record survives the later engine calls.
// False is the native absent-owner/disabled-recording gate.
bool publish_pending_shot(const client::CommandFrame&, const lagcomp::EvaluationState&,
                          std::int32_t shot_index, const PendingShotPublicationBindings&,
                          std::int32_t extrapolation_ticks = 0);
}
