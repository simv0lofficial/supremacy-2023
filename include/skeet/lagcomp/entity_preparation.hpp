#pragma once
#include "entity_state.hpp"
#include "history.hpp"
#include <cstring>

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::lagcomp {
struct EntityPreparationLiveGlobals {
    void* const& engine_globals; //43467B04, first word is comparison realtime
    float* const& simulation_time; //43467EB8, raw bits copied to entity stamp
    float* const& current_time; //4346A334
    const TickTimingSamples& latency; //43478C70
};
struct EntityPreparationContext {
    const EntityStateRuntime& entity;
    std::uint32_t global_frame; // *0x43467EB8
    float comparison_time; // *0x43467B04
    float current_time; // *0x4346A334
    TickTimingSamples latency; // 0x43478C70/74/78
    // Native optional current timing state. A present state supplies D4+18C;
    // otherwise mean latency plus corrected command tick supplies the clock.
    const HistoryTickTiming* current_timing = nullptr;
    const EntityPreparationLiveGlobals* live_globals = nullptr;
    std::uint32_t frame_value() const noexcept {
        if (!live_globals)
            return global_frame;
        std::uint32_t value;
        std::memcpy(&value, live_globals->simulation_time, sizeof value);
        return value;
    }
    float comparison_value() const noexcept {
        if (!live_globals)
            return comparison_time;
        float value;
        std::memcpy(&value, live_globals->engine_globals, sizeof value);
        return value;
    }
    float current_value() const noexcept {
        return live_globals ? *live_globals->current_time : current_time;
    }
    const TickTimingSamples& latency_values() const noexcept {
        return live_globals ? live_globals->latency : latency;
    }
};
// Complete4341DDA0. The target has a valid nonzero networkable index, and its
// shared cache has the native at-most-six packet samples. Writes the two
// collision fields and eye angles, and may stamp cache+F4. Return codes1..7
// are retained verbatim for callers that publish the native preparation mode.
std::uint32_t prepare_entity_angles(client::PlayerStateCache&, void* target,
                                    const LagRecord& source, bool copy_yaw,
                                    const EntityPreparationContext&);
} // namespace skeet::lagcomp
