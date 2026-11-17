#pragma once
#include "native_data_image.hpp"
#include "skeet/misc/automatic_weapons.hpp"
#include "skeet/misc/clan_tag.hpp"

namespace skeet::engine {
struct BuildMiscEngineBindings {
    misc::SetClanTag set_clan_tag;
    void** game_rules;
    void** player_resource;
    const misc::AutomaticWeaponsBombList* planted_bombs;
};

// Verify actual loaded client/engine builds and instruction/data operands,
// then publish43467550/4346A3A4/43468FF0/434688C0/4346D558/4346D554 together.
// The four data aliases borrow live engine-owned storage. Auto-accept follows
// the genuine SetLocalPlayerReady registration and its stdcall string bridge.
//4346D560/4346D55C retain the actual two performance-owner instruction records.
// No game helper, constructor or interface method executes during discovery.
BuildMiscEngineBindings initialize_build_misc_engine_bindings(NativeDataImage&);
} // namespace skeet::engine
