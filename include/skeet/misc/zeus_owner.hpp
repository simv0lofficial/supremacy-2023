#pragma once
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/misc/zeus.hpp"
#include <vector>

namespace skeet::misc {
class ZeusLifecycleOwner;
std::size_t zeus_owner_size() noexcept;

// Complete43485D4C. storage is the caller's native-header allocation sized by
// zeus_owner_size(); primary slot0 frees it through433B1A7A/433837D0.
// Primary registration precedes command registration, descriptor registration,
// then clearing state.rage_enabled. The boolean's existing value survives.
// settings is the ONE still-unregistered canonical ZeusConfig. Do not also
// initialize_zeus_config on it. State/settings/bindings and both registries
// outlive this owner; its CommandOwner variant remains at a stable address.
// Primary slot1 binds the shared RAGE descriptor through434824A7 later in the
// caller's actual lifecycle order. No protected startup position is imposed.
ZeusLifecycleOwner& construct_zeus(void* storage, engine::OwnerRegistryBinding lifecycle,
                                   client::CommandPhaseRegistry& command_owners, ConfigRegistries&,
                                   ZeusConfig&, ZeusState&, const ZeusBindings&);
} //namespace skeet::misc
