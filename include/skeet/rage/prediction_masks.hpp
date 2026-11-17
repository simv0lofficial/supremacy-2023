#pragma once
#include "skeet/rage/targeting.hpp"
#include "skeet/rage/ballistics.hpp"
#include "skeet/rage/target_relationships.hpp"
#include <functional>
#include <variant>

namespace skeet::esp {
class EnginePlayers;
class EspOwnerRuntime;
}
namespace skeet::anti_aim {
class TargetTraceEntities;
}
namespace skeet::rage {

// Canonical native owner+0C/+14, also read by433C8BD6 after target firing.
struct PredictionMaskState {
    std::array<std::uint32_t, 2> allowed;
    std::array<std::uint32_t, 2> blocked;
    // Native index64 uses index>>5 without masking. allowed[2] aliases
    // blocked[0]; blocked[2] reaches the zeroed heap tail at full-owner+1C.
    // Retain that actual word separately from source-only callback metadata.
    std::uint32_t trailing_word = 0;
};
static_assert(sizeof(PredictionMaskState) == 0x14);
class PredictionMaskPlayers {
    std::variant<std::reference_wrapper<esp::EnginePlayers>,
                 std::reference_wrapper<esp::EspOwnerRuntime* const>>
        source_;

public:
    PredictionMaskPlayers(esp::EnginePlayers& fixture) noexcept : source_(std::ref(fixture)) {}
    PredictionMaskPlayers(esp::EspOwnerRuntime* const& live) noexcept : source_(std::cref(live)) {}
    esp::EnginePlayers& get() const;
};
struct PredictionMaskBindings {
    void* const& engine_client; //4346A514
    const TargetBindings& targeting;
    const std::uint8_t& enabled; //434671E0
    const std::uint32_t& context_flags; //43467E40
    const std::int32_t& entity_limit; //pointed43469300, inclusive in this callback
    PredictionMaskPlayers players;
    BallisticEngine& ballistics;
    void* const* trace_filter_vtable; //actual engine filter interface43475B90
    const anti_aim::TargetTraceEntities& trace_entities;
};
struct LivePredictionMaskBindings {
    void* const& engine_client;
    void* const& local_player;
    const LiveTargetRelationships& targeting;
    const std::uint8_t& enabled;
    const std::uint32_t& context_flags;
    std::int32_t* const& entity_limit;
    esp::EspOwnerRuntime* const& players;
    BallisticEngine& ballistics;
    void* const* const& trace_filter_vtable;
    const anti_aim::TargetTraceEntities& trace_entities;
};
class PredictionMaskRuntimeBinding {
    std::variant<std::reference_wrapper<const PredictionMaskBindings>,
                 std::reference_wrapper<const LivePredictionMaskBindings>>
        source_;

public:
    PredictionMaskRuntimeBinding(const PredictionMaskBindings& decoded) noexcept
        : source_(std::cref(decoded)) {}
    PredictionMaskRuntimeBinding(const LivePredictionMaskBindings& live) noexcept
        : source_(std::cref(live)) {}
    void update(PredictionMaskState&, std::uint8_t) const;
};

void reset_prediction_masks(PredictionMaskState&) noexcept; //4332AD41
// Complete433EDCA9. Inactive frames and airborne eligible targets preserve
// prior bits. Index64 preserves its original overlapping/tail publications.
void update_prediction_masks(PredictionMaskState&, std::uint8_t update,
                             const PredictionMaskBindings&);
void update_prediction_masks(PredictionMaskState&, std::uint8_t, const LivePredictionMaskBindings&);
}
