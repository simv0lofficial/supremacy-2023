#pragma once
#include "settings.hpp"
#include "skeet/weapon/fire_state.hpp"

namespace skeet::rage {
struct PreambleState;
struct CommandPhase2Offsets {
    // These are the same live decoded fields used by evaluate_fire_state.
    const weapon::FireStateOffsets& fire_state;
    std::ptrdiff_t tick_base; //434693D8 /3D10B94C, int32
    std::ptrdiff_t reserve_ammunition; //4346749C /49C770CA, int32
};
struct CommandPhase2Bindings {
    const GlobalSettings& settings;
    InputBitmap input;
    const std::int32_t& field_4347c9ec;
    const void* accuracy_convar; //resolved43468EA4 XOR FCD2DA7A
    void* const* game_rules; //4346A3A4; either level may be absent
    float interval; //*4346A338
    CommandPhase2Offsets offsets;
};
inline constexpr std::uint32_t rage_command_phase2_body = 0x43358716u;
// Complete43358716..43358951. This runs before CommandFrame construction.
// The +174 word belongs to the same PreambleState consumed by acquisition
// and firing; it is cleared even when RAGE is disabled. Native engine input
// supplies a nonnull player. Both trailing phase arguments are unused here.
void process_rage_command_phase2(PreambleState&, void* player, void* weapon, client::Command&,
                                 std::array<float, 3>& incoming_angles, std::uint8_t& send_packet,
                                 const CommandPhase2Bindings&);
} //namespace skeet::rage
