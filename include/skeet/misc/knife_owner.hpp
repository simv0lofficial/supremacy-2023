#pragma once
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/misc/knife_runtime.hpp"
#include <vector>

namespace skeet::misc {
class KnifeLifecycleOwner;
std::size_t knife_owner_size() noexcept;

// Complete constructor434802E2, primary4346F358 and command43471EB8. The
// supplied storage has the native43387448 allocation header; primary slot0
// releases it through433B1A7A ->433837D0. The canonical settings, bindings and
// scheduler flags must outlive the owner, as must the borrowed registry ranges.
// Registers primary -> command, preserves the existing enabled value while
// registering7A5909EE, then registers modeA7C81D9B with value3. Do not also call
// initialize_knife_config for the same descriptors. There is no knife reset.
KnifeLifecycleOwner& construct_knife(void* storage, engine::OwnerRegistryBinding lifecycle,
                                     client::CommandPhaseRegistry& command_owners,
                                     ConfigRegistries&, KnifeConfig&, KnifeCommandBindings,
                                     std::uint32_t& scheduler_flags);

// Original allocation entry43483FF8 requests0x34 bytes and tail-calls this
// constructor. Its allocation-failure target4348400B is not recovered; this
// API starts at the confirmed constructor and does not replace that edge.
} // namespace skeet::misc
