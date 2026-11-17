#pragma once
#include "frame.hpp"
#include "skeet/events/registry.hpp"

namespace skeet::esp {
// Actual43482632 event bodies. The counter pointer is the borrowed43468D4C
// global; it is reloaded after external calls, like the native owner.
class EspFrameEvents {
public:
    EspFrameEvents(EspFrameOwner& owner, const PlayerEspConfig& config,
                   const PlayerRuntimeEngine& players, std::int32_t* const& frame)
        : owner_(owner), config_(config), players_(players), frame_(frame) {}
    void item_pickup_slerp(events::GameEventReader&, events::EventPlayerIndices&); //433E0236
    void player_spawn(events::GameEventReader&, events::EventPlayerIndices&); //433986DA
    void item_equip(events::GameEventReader&, events::EventPlayerIndices&); //4334C7A3
    void bomb_begin_plant(events::GameEventReader&); //4338A457
    void reset_plant(events::GameEventReader&); //4332CA32
    void round_start(events::GameEventReader&); //433D5AB3
    // Preserve43483E11..43483E6D construction order around the separately
    // recovered death listener at43483E3F.
    void bind_before_death(events::EventRegistry&);
    void bind_after_death(events::EventRegistry&);
    // The owner stores these descriptors at+EE0/+EF0/+F00.4337F9F6 gates
    // item_equip by Dormant and begin/abortplant by WorldESP Bomb. Its update
    // prefix adds/removes them; the event bodies keep their own native gates.
    std::array<events::EventListener, 3> dormant_listeners() noexcept;

private:
    RuntimePlayer* event_player(std::uint32_t user_id, events::EventPlayerIndices&,
                                std::int32_t& index);
    void site(events::GameEventReader&);
    static void pickup(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void spawn(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void equip(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void begin(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void reset(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void round(void*, events::GameEventReader&, events::EventPlayerIndices&);
    EspFrameOwner& owner_;
    const PlayerEspConfig& config_;
    const PlayerRuntimeEngine& players_;
    std::int32_t* const& frame_;
};
} // namespace skeet::esp
