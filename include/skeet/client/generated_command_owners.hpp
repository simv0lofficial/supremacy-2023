#pragma once
#include "command_frame.hpp"

namespace skeet {
struct MovementCommandOwner;
struct MovementCommandBindings;
namespace anti_aim {
struct CommandOwner;
}
namespace client {

// The generated-command path reads these two nullable source-owner slots at
// each entry, just as native434759B0 and43475940 do. Movement uses the same
// state and concrete engine bindings as the normal command scheduler.
struct GeneratedCommandOwnersBindings {
    MovementCommandOwner* const& movement_owner; //434759B0
    anti_aim::CommandOwner* const& angle_owner; //43475940
    const MovementCommandBindings& movement;
    const std::uint8_t& infinite_duck; //43467CEC
    std::uint32_t shot_counter_8cfc633a;
};

// Complete43328EE7: phase1, movement scheduler, grounded feedback and the
// live shot-counter publication. send_packet is frame+8's separate byte.
void process_generated_movement_command(const GeneratedCommandOwnersBindings&, CommandFrame&,
                                        void* player, Command&, std::uint8_t& send_packet);

// Complete43320B61: low-speed alternating movement followed by the actual
// AA phase4 owner. Its micro-movement does not publish previous_duck_1A4.
void process_generated_angle_command(const GeneratedCommandOwnersBindings&, CommandFrame&,
                                     void* player, Command&, std::uint8_t& send_packet);

} //namespace client
} //namespace skeet
