#pragma once
#include "skeet/resolver_runtime/environment_update.hpp"

namespace skeet::client {
struct PlayerState;
class PlayerStateCache;
}
namespace skeet::resolver_runtime {
struct EnvironmentTarget {
    std::uint32_t flags_38ca5228;
    std::uint8_t byte_043fa575, byte_334b460c;
    bool needs_update; // global4347BE0E, stride20, selected low16-bit index
    RageVector origin;
    const std::array<EnvironmentCapsule, 2>* hitboxes;
};
struct EnvironmentEntryEngine {
    void* context;
    bool (*enabled_68)(void*); // original owner+28, actual virtual +68
    EnvironmentTarget* (*lookup_10)(void*, std::int32_t); // entity-list virtual +10
    bool (*dormant_24)(void*, EnvironmentTarget&); // target networkable virtual +24
};
struct EnvironmentEntryInput {
    float enabled_cvar; // 43407DE2 decoded cvar value
    bool local_exists;
    std::uint8_t local_field_9d920e35;
    std::int32_t selected_entity; // native global4346DB1C
    std::span<const lagcomp::HistoryEvaluation> pending_shots;
    EnvironmentUpdateInput update;
};

// The projected entry/input overloads retain explicit logical inputs for
// semantic callers. The engine-owned HistoryOwner uses the live overload.
// Complete433279A4: pending shot flags20/40 and native target field+94.
bool has_pending_environment_shot(std::span<const lagcomp::HistoryEvaluation>,
                                  std::uint32_t selected_index) noexcept;
// Complete outer4334832C gates through43348558. Active selected indices are
// nonzero, as required by the native valid control-flow path at4334844C.
client::PlayerState* prepare_environment_update(client::PlayerStateCache&,
                                                const EnvironmentEntryInput&,
                                                const EnvironmentEntryEngine&);
// Complete4334832C: concrete shared cache, entry guards, environment probe,
// penetration/scoring and owner publication. Only genuine engine calls bind.
bool update_environment_entry(client::PlayerStateCache&, const EnvironmentEntryInput&,
                              const EnvironmentEntryEngine&, const EnvironmentEngine&);
// Live owner adapter after its actual engine/UI/local/entity/pending/adjustment
// gates and cache lookup. Shared owner gates and the complete body stay here.
bool update_environment_entry(client::PlayerState&, const EnvironmentLiveUpdateInput&,
                              const EnvironmentEngine&);
} // namespace skeet::resolver_runtime
