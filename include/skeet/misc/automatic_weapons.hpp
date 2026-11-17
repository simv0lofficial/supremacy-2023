#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/misc/config.hpp"
#include <cstddef>

namespace skeet::misc {

// The live vector at 434688C0. Only entries[0] participates in this owner;
// native x86 storage has entries +0 and count +0C.
struct AutomaticWeaponsBombList {
    void* const* entries;
    std::int32_t allocation_count;
    std::int32_t grow_size;
    std::int32_t count;
};

struct AutomaticWeaponsOffsets {
    std::ptrdiff_t team; // 434688B0 / 8B5586F6, int32
    std::ptrdiff_t has_defuser; // 4346A4F4 / A15F5884, byte
    std::ptrdiff_t bomb_ticking; // 4346A6C4 / 0E5BECE5, byte
    std::ptrdiff_t bomb_detonation_time; // 4346834C / D280D4E1, float
    std::ptrdiff_t player_blocks_attack; // 4346799C / BCB263A5, byte
    std::ptrdiff_t shots_fired; // 4346A6DC / 8CFC633A, int32
    std::ptrdiff_t weapon_definition; // 4346908C / 5C9F0FBB, uint16
    std::ptrdiff_t weapon_last_shot; // 43467200 / 0176A381, float
};

using AutomaticWeaponsRandomInteger = std::int32_t (*)(std::int32_t, std::int32_t);
struct AutomaticWeaponsBindings {
    // Bind the live list pointer, rather than a copied first-bomb snapshot.
    const AutomaticWeaponsBombList* const* planted_bombs; // 434688C0
    const float* current_time; // *43467EB8, prediction time in phase4
    AutomaticWeaponsOffsets offsets;
    // Actual vstdlib!RandomInt import at 63C2FFB0, cdecl on x86.
    AutomaticWeaponsRandomInteger random_integer;
};

struct AutomaticWeaponsState {
    std::int32_t delay_adjustment = 0; // complete owner +5C, phase receiver +54
    float last_shot_time = 0.0f; // complete owner +60, phase receiver +58
};

inline constexpr std::uint32_t automatic_weapons_constructor = 0x4347D991u;
inline constexpr std::uint32_t automatic_weapons_command_body = 0x433EA2AAu;
inline constexpr std::uint32_t automatic_weapons_command_vtable = 0x4346F244u;
inline constexpr std::uint32_t automatic_weapons_command_phase = 4;

// Complete native phase4 body 433EA2AA, called with the owner +8 interface.
// Last second defuse runs before the Automatic weapons enable gate. The three
// settings are the same live descriptors used by the MISC menu and config I/O.
// Engine boundaries are entity GetAbsOrigin slot10, the full-auto predicate at
// weapon slot430, the fire-cycle query at slot433(mode=0), and RandomInt.
void process_automatic_weapons(AutomaticWeaponsState&, const AutomaticWeaponsConfig&,
                               const AutomaticWeaponsBindings&, const client::CommandFrame&,
                               void* player, client::Command&, InputBitmap);

} // namespace skeet::misc
