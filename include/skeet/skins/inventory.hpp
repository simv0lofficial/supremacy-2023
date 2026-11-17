#pragma once
#include "skeet/skins/apply.hpp"

namespace skeet::skins {
struct PlayerIdentity {
    std::uint32_t low = 0, high = 0;
};
struct WeaponOwnership {
    PlayerIdentity original_owner;
    // Exact engine field identities: encrypted resolver 0x43468E5C and
    // 0x4346A868. Their values are read, without assuming an SDK offset.
    std::uint32_t zero_marker = 0, owner_handle = 0xFFFFFFFF;
};
bool inventory_owner_matches(const WeaponOwnership&, std::uint32_t inventory_tag,
                             bool identity_valid, PlayerIdentity local,
                             std::int32_t local_index) noexcept;
struct InventoryEntry {
    std::uint32_t handle = 0, tag = 0;
};
struct InventoryWeapon {
    std::uint16_t definition = 0;
    bool is_weapon = false;
    WeaponOwnership ownership;
    std::string_view entity_class;
    FallbackAttributes attributes;
};
class InventoryEngine : public CounterRandom, public Refresh {
public:
    virtual bool local_player_exists() const = 0;
    virtual bool local_player_dead() const = 0;
    virtual std::int32_t local_player_index() const = 0;
    virtual std::int32_t local_team() const = 0;
    virtual bool local_identity(PlayerIdentity&) = 0;
    // Resolve the handle/networkable/unknown/base-entity chain. Return null
    // when any engine interface in that chain returns null.
    virtual InventoryWeapon* resolve(std::uint32_t handle) = 0;
    virtual std::span<const CounterClass> counter_classes() const = 0;
};
// Complete policy and inventory traversal of 0x4339D6B2. Entity access and
// resolved field storage are explicit engine adapter responsibilities.
void apply_inventory(const WeaponSkinsConfig&, std::span<const InventoryEntry>, InventoryEngine&);
} // namespace skeet::skins
