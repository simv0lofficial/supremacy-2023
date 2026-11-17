#pragma once
#include "network_console_handlers.hpp"

namespace skeet::misc {
class DisableSvPureOwner;
std::size_t disable_sv_pure_owner_size() noexcept;
// Whole434864C4. settings is the unregistered canonical177AB88C descriptor,
// also consumed directly by GameSystemBindings. Its existing byte survives.
// storage must have a native43387448 allocation header for primaryslot0.
// active is the canonical43475974 slot cleared by433ECC98. Construction binds
// that slot but does not invent an allocating/publishing wrapper around it.
DisableSvPureOwner& construct_disable_sv_pure_owner(void* storage,
                                                    engine::OwnerRegistryBinding lifecycle,
                                                    engine::OwnerRegistryBinding messages,
                                                    ConfigRegistries&, DisablePureConfig& settings,
                                                    DisableSvPureOwner*& active);
const DisablePureConfig& disable_sv_pure_configuration(const DisableSvPureOwner&) noexcept;
} //namespace skeet::misc
