#pragma once
#include "hitbox_highlight.hpp"
#include "player_engine.hpp"
#include <functional>
#include <variant>

namespace skeet::esp {
class EspOwnerRuntime;
class HitboxHighlightPlayers {
    std::variant<std::reference_wrapper<EnginePlayers>,
                 std::reference_wrapper<EspOwnerRuntime* const>>
        source_;

public:
    HitboxHighlightPlayers(EnginePlayers& fixture) noexcept : source_(std::ref(fixture)) {}
    HitboxHighlightPlayers(EspOwnerRuntime* const& live) noexcept : source_(std::cref(live)) {}
    EnginePlayers& get() const;
};
// Native433D12F7 reads these globals at draw time. The player identities and
// decoded offset words are the same owners used by the ESP frame and RAGE.
struct HitboxHighlightEngineContext {
    HitboxHighlightPlayers players;
    const engine::NativeOffsetStorage& offsets;
    const std::uint8_t& active; //4346A65C
    const Projection& projection;
    const RageVector& view_origin; //43479CCC
    const RageTrigonometry& trigonometry;
    const skeet::players::State& player_adjustments; //same4347BE08 whitelist table
    std::uint8_t life_state(const void* local) const;
};

// Complete live433D12F7. Model/set/descriptor/cache reads occur after actual
// entity eligibility, and each enabled entry reloads its descriptor and bone.
// This path does not populate a HitboxHighlightTarget snapshot or use its
// by_handle callback. Capsule construction/drawing remain recovered source.
void draw_live_hitbox_highlight(ui::RenderQueue&, const AimbotVisualizationConfig&,
                                HitboxHighlightState&, void* local,
                                const HitboxHighlightEngineContext&);
} //namespace skeet::esp
