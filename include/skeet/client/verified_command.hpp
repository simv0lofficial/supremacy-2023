#pragma once
#include "commands.hpp"

namespace skeet::client {
struct VerifiedCommand {
    Command command;
    std::uint32_t checksum;
};
static_assert(sizeof(VerifiedCommand) == 0x68);
using CommandChecksum = engine::MemberFunction<std::uint32_t>;
//4335849B..433584DD: snapshot is prepared by save_final_command first;
// the optional actual engine43467C48 getter receives the verified entry.
void publish_verified_command(VerifiedCommand&, const Command&, CommandChecksum);
// Complete433D11F7. The supplied object is the genuine prediction interface.
void invalidate_command_prediction(void*) noexcept;
} // namespace skeet::client
