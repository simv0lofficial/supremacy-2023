#pragma once
#include "command_runtime.hpp"
#include "skeet/client/command_frame.hpp"

namespace skeet {

// The normal command registry and generated commands borrow the same movement
// owner. The command count is the live pointer cell at434757DC.
struct MovementCommandPhaseBindings {
    const MovementCommandBindings& command;
    std::int32_t* const& choked_commands;
    const HotkeyBinding& jump_at_edge_key; //4346760C
    std::uint32_t movement_flags_38ca5228;
};
struct MovementCommandPhaseOwner {
    MovementCommandOwner& state;
    const MovementCommandPhaseBindings& bindings;
};

void movement_owner_phase1(MovementCommandPhaseOwner&, client::Command&);
// Complete4334659B on the native valid-player contract. The enclosing master
// handles rejected/null players before invoking this phase.
void movement_owner_phase2(MovementCommandPhaseOwner&, void* player, void* weapon, client::Command&,
                           std::array<float, 3>& incoming_angles, std::uint8_t& send_packet);
// Complete433D1F99, using the same settings and callback flags as phase2.
void movement_owner_phase4(MovementCommandPhaseOwner&, client::CommandFrame&, client::Command&);

} //namespace skeet
