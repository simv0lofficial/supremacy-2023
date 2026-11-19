#pragma once
#include "skeet/resolver_runtime/attempts.hpp"
#include "skeet/resolver_runtime/environment_probe.hpp"
#include "skeet/resolver_runtime/environment_score.hpp"
#include <cstddef>

namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet::client {
struct PlayerState;
}
namespace skeet::resolver_runtime {
struct EnvironmentOwner {
    SelectionState selection;
    float probe_time; // native owner +B0
    float probe_stamp; // +B4
    std::int32_t probe_count; // +B8
    float reference_time; // +BC
    RageVector reference_origin; // +C0
    std::int32_t last_update_tick; // +15C
    float reference_yaw; // +1AC
};
struct EnvironmentUpdateInput {
    float local_time; // local entity field decoded with 9F8E7F71 (433FC87C)
    float current_time;
    std::int32_t command_tick;
    const std::int32_t* previous_command;
    const std::int32_t* current_command;
    RageVector local_origin;
    RageVector target_origin;
    // Native model hitboxes 0 and 3. Null represents an unavailable model/set.
    const std::array<EnvironmentCapsule, 2>* hitboxes;
    bool strict = false;
};
struct EnvironmentLiveUpdateInput {
    void* target;
    const animation::BoneRuntime& bones;
    // Actual nullable4346A3A4 pointer chain. Both native433FC87C calls read it
    // at their own gate, rather than receiving a precomputed local clock.
    void* const* const& local_time_entity;
    std::ptrdiff_t local_time_offset; //9F8E7F71
    const float& current_time; //*4346A334
    const std::int32_t& command_tick;
    const std::int32_t* const& previous_command;
    const std::int32_t* const& current_command;
    const RageVector& local_origin; //43475BD4, read only at probe entry
    std::ptrdiff_t target_origin_offset; //A9EE6062
    bool strict = false;
};

// Shared native owner/history/time gates43348478..43348558, after the entry's
// genuine engine, pending-shot and player-adjustment checks have succeeded.
bool environment_update_ready(EnvironmentOwner&, const LagRecordHistory&,
                              std::uint8_t resolver_index, const EnvironmentUpdateInput&) noexcept;
bool environment_update_ready(client::PlayerState&, const EnvironmentLiveUpdateInput&) noexcept;

// Complete environment update body 43348558..43349E47. The caller has passed
// the preceding engine/UI/pending-shot gates and supplies a nonempty history.
// Returns whether a new environment selection reached the publication stage.
bool update_environment(EnvironmentOwner&, LagRecordHistory&, const EnvironmentUpdateInput&,
                        const EnvironmentEngine&);
// Same body using the canonical owner's live fields and ring. Studio/set and
// hitboxes0/3 are read here, after the world probe; there is no model factory or
// internal lookup callback. Publication changes only the native written fields.
bool update_environment(client::PlayerState&, const EnvironmentLiveUpdateInput&,
                        const EnvironmentEngine&);
} // namespace skeet::resolver_runtime
