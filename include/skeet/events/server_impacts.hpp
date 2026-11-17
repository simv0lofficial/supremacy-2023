#pragma once
#include "game_events.hpp"
#include "skeet/esp/effects_config.hpp"
#include <cstdint>

namespace skeet::events {
struct ServerImpactBindings {
    void* const& debug_overlay; // live43468260, genuine AddBoxOverlay virtual1
    const std::uint8_t& suspended; // live43468E58
};
struct ServerImpactRuntime {
    const esp::EffectsSettings& settings; // same canonical4348E6E2 descriptors
    const ServerImpactBindings& bindings;
};
// Complete43387294, reached by event-listener thunk43359E08. The enclosing
// effects owner registers19459119 and supplies the captured local player.
void process_server_impact(ServerImpactRuntime&, GameEventReader&, EventPlayerIndices&);
} // namespace skeet::events
