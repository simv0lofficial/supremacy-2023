#pragma once
#include "player_draw.hpp"
#include "frame_events.hpp"
#include "sound_overlay.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/players/state.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::esp {
struct LineOfSightBindings {
    void* const& engine; //4346A514; connected26/demo82/HLTV93
    void* const& debug_overlay; //optional43468260, AddLineOverlayAlpha20
    engine::NativeConVarReference interpolation_cvar; //4346A850^478720C2
    std::int32_t* const& maximum_players; //43469300, inclusive upper bound
    std::uint8_t* const& trace_flag; //optional434683D0, exact guard byte
    MovementCollisionEngine& collision;
    RageTrigonometry trigonometry; //actual SSE sine/cosine CRT boundaries
    std::ptrdiff_t duck_amount; //3309C0CF, float
    std::ptrdiff_t eye_angles; //E23A9BE1, pitch/yaw
};
struct LineOfSightInputs {
    const PlayerEspConfig& config;
    const players::State& adjustments;
    const PlayerFrameState& frame; //canonical4346A65C activation result
    const std::uint8_t& suppressed; //43468E58, reread at native gate
    const LineOfSightBindings& bindings;
};
struct LineOfSightContext : LineOfSightInputs {
    LineOfSightContext(LineOfSightInputs inputs, EnginePlayers& source)
        : LineOfSightInputs(inputs), players(source) {}
    EnginePlayers& players;
};
// Complete4337FA44..4337FDC9: engine/player gates, smoothstep eye origin,
// rounded2048-unit ray, reject-entities trace and alpha debug-overlay line.
void draw_line_of_sight(const LineOfSightContext&);

// The actual secondary interface at43482632 owner+28, vtable434708D0.
// Its one slot4337F9F6 synchronizes event subscriptions before sight drawing.
class LineOfSightHandler final : public engine::FrameUpdateEndpoint {
public:
    LineOfSightHandler(engine::OwnerRegistryBinding, SoundOverlay&, EspFrameEvents&,
                       events::EventRegistry&, const WorldEspConfig&, const LineOfSightContext&);

private:
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall update(void*, std::uint8_t);
#else
    static void update(void*, std::uint8_t);
#endif
    inline static constexpr engine::FrameUpdateEntry entries_[1]{update};
    SoundOverlay& sounds_;
    EspFrameEvents& events_;
    events::EventRegistry& event_registry_;
    const WorldEspConfig& world_;
    const LineOfSightContext& context_;
};
} // namespace skeet::esp
