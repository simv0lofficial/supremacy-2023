#pragma once
#include "skeet/skins/native_inventory.hpp"
#include "skeet/skins/command.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/events/feature_listeners.hpp"

namespace skeet::skins {
class InventoryLifecycleOwner;
struct InventoryOwnerBindings {
    WeaponSkinsConfig& weapons;
    CounterRandom& random;
    WeaponRefresh& refresh;
    ui::RootMenu* const& menu; //live43476924; construction does not dereference it
    std::span<const std::uint32_t, 3> classification; //43474580
    void* const& engine_client;
    void* const& entity_list; //captured at43490315, after all three registrations
    const std::byte* const& counter_rows;
    const std::int32_t* const& counters;
    const engine::NativeOffsetGlobals& offsets; //shared live encrypted offset words
    events::DeathBindings& death;
    events::WeaponDeathOffsets& death_offsets;
    events::EventRegistry& events;
};

std::size_t inventory_owner_size() noexcept;
// Actual constructor entry434902DF (the factory's JMP target). Its primary,
// command and entity-update registrations retain that order. This creates the
// one CommandRuntime, NativeInventoryRuntime and StatTrakDeath listener over
// the same borrowed WeaponSkinsConfig/CounterRandom/WeaponRefresh.
// weapons must not already be linked by Settings::initialize: this constructor
// registers only B6218FEC and sets its 65 records' native defaults. It preserves
// each record's reserved word and the descriptor's reserved byte.
// storage is a native-header allocation sized by inventory_owner_size();
// primary slot0 frees it. Registry shutdown retains ownership of its ranges.
InventoryLifecycleOwner& construct_inventory_owner(void* storage,
                                                   engine::OwnerRegistryBinding lifecycle,
                                                   client::CommandPhaseRegistry& commands,
                                                   client::EntityObserverRegistry&,
                                                   ConfigRegistries&, InventoryOwnerBindings);
} //namespace skeet::skins
