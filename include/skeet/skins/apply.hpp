#pragma once
#include "skeet/skins/config.hpp"
#include "skeet/skins/catalog.hpp"
#include <span>
#include <string_view>

namespace skeet::skins {
// Semantic entity fields supplied by the engine's resolved field accessors.
struct FallbackAttributes {
    std::int32_t item_id_high = 0, item_id_low = 0;
    std::uint32_t account_id = 0;
    std::int32_t paintkit = 0, seed = 0;
    float wear = 0;
    std::int32_t stattrak = 0;
};
struct CounterClass {
    std::string_view name;
    std::int32_t catalog_index = -1, counter = -1;
    bool valid = false;
};
class CounterRandom {
public:
    virtual ~CounterRandom() = default;
    // The native vstdlib RandomInt call uses inclusive bounds 1 and 10000.
    virtual std::int32_t random_integer(std::int32_t minimum, std::int32_t maximum) = 0;
};
// Native attribute-writing slice 0x4339D965..0x4339DBD1. Ownership, entity
// lookup, item classification and the enabled-record gate belong to its caller.
// Returns whether changing the paintkit scheduled a full update.
bool apply_attributes(const WeaponSkin&, FallbackAttributes&, std::uint32_t local_account,
                      std::string_view entity_class, std::span<const CounterClass>, CounterRandom&,
                      Refresh&);
// 0x4339DA75..0x4339DA97, including the native lower bound of 0.00001.
float quality_to_wear(std::int32_t quality) noexcept;
} // namespace skeet::skins
