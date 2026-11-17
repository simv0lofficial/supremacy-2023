#pragma once
#include "skeet/skins/config.hpp"
#include "skeet/events/game_events.hpp"
#include <span>

namespace skeet::skins {
struct AttributeCounter {
    std::uint32_t id = 0, value = 0;
};
struct StatTrakWeapon {
    std::uint16_t definition = 0;
    std::uint32_t owner_low = 0, owner_high = 0, counter = 0;
    std::span<AttributeCounter> attributes;
};
class StatTrakEngine : public events::EventPlayerIndices {
public:
    virtual std::int32_t entity_index() const = 0;
    virtual std::int32_t team() const = 0;
    virtual StatTrakWeapon* active_weapon() = 0;
    virtual bool local_identity(std::uint32_t& low, std::uint32_t& high) = 0;
};
// 0x43336CC1, registered for player_death by 0x4349035C.
void player_death(const WeaponSkinsConfig&, events::GameEventReader&, StatTrakEngine&);
} // namespace skeet::skins
