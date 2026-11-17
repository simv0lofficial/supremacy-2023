#pragma once
#include "skeet/client/startup_reset.hpp"
#include <cstdint>

namespace skeet::client {

// References retain the shared globals rather than capturing their values
// before the engine queries in this cut. The per-call packet_context argument
// is send_packet+1 in the outer wrapper's saved EBX word.
struct CommandPacketTransitionBindings {
    const std::uint8_t& enabled; // 43467224
    const std::uint8_t& transition_enabled; // 4346754C
    void* const& engine_client; // 4346A514
    void** const& shared_player; // 4346A3A4, nullable holder
    std::uint32_t* const& player_guard_word; // 43468378
    const std::uint32_t& player_guard_key; // 43468244, XOR BF567619
    std::uint32_t* const& last_outgoing; // 43475B2C
    std::span<StartupAcknowledgement, 150> acknowledgements; // 43475BE0
};

// Inline433D5888..433D594F compares the mutable ESP+1A decision with the
// saved engine EBX byte. A requested choke while the engine wants to send,
// under all native gates, records the sequence and calls SendDatagram(nullptr).
// The decoded channel virtual byte offset is B8 (slot 46). 433E533F changes
// its return path only; the real thiscall receiver and argument are retained.
// The live channel +2C/+18 words are decremented after that call, then the
// engine byte receives the requested decision. Flag/entity restoration follows.
void apply_command_packet_transition(std::uint8_t requested_packet,
                                     std::uint8_t& engine_send_packet,
                                     const std::uint8_t& packet_context,
                                     const CommandPacketTransitionBindings&);

} // namespace skeet::client
