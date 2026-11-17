#pragma once
#include "commands.hpp"

namespace skeet::client {
// Complete43331EFC: CInput+F0 owns150 commands, indexed by unsigned remainder.
Command& input_command_slot(void* input, std::uint32_t command_number) noexcept;
const Command& input_command_slot(const void* input, std::uint32_t command_number) noexcept;
// Complete433B9184 exact-number check following the same actual ring lookup.
const Command* find_input_command(const void* input, std::int32_t command_number) noexcept;
} // namespace skeet::client
