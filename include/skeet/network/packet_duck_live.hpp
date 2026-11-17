#pragma once
#include <cstdint>

namespace skeet::network {
struct PacketCommandOwner;

// Live Air duck/FakeLag phase2 tail 433EA9DA..433EADAB. The owner's canonical
// settings and the current player fields are read on each native branch.
void update_packet_duck_live(PacketCommandOwner&, void* player, std::uint32_t& buttons,
                             std::uint32_t& command_flags);
} // namespace skeet::network
