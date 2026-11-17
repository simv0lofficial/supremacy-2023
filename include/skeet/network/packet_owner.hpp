#pragma once
#include "command_owner.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include <vector>

namespace skeet::network {
class PacketLifecycleOwner;
std::size_t packet_owner_size() noexcept;

// Complete434901DA on supplied native43387448 aligned storage. settings is
// the same mutable configuration referenced by command.settings; its amount,
// variance and limit descriptors must still be unregistered.
// Registration order is primary433676A0, command43345EBC, then the three owner
// descriptors. The enabled/hotkey CRT descriptors have independent callers.
// State is borrowed from command.state; only the proven sparse tail is written.
PacketLifecycleOwner& construct_packet_owner(void* storage, engine::OwnerRegistryBinding primary,
                                             client::CommandPhaseRegistry& commands,
                                             ConfigRegistries&, anti_aim::FakeLagSettings& settings,
                                             PacketCommandOwner& command);

// Primary4347290C slot0 is433B1A7A→433837D0. It frees the native allocation
// without unregistering descriptors/commands, resetting state or clearing slots.
void destroy_packet_owner(void* storage);
} //namespace skeet::network
