#pragma once
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/misc/automatic_weapons.hpp"
#include <vector>

namespace skeet::misc {
class AutomaticWeaponsLifecycleOwner;
std::size_t automatic_weapons_owner_size() noexcept;

// Complete4347D991/4346F358 composition. storage is the caller's native aligned
// heap allocation; lifecycle slot0 releases it through433B1A7A ->433837D0.
// settings is the ONE canonical, still-unregistered configuration. Do not
// separately call initialize_automatic_weapons_config for the same descriptors.
// The canonical state/settings/bindings must outlive the lifecycle owner.
// Registers primary -> command, initializes descriptors, then clears only the
// native+5C/+60 state. The actual command vector stores a stable CommandOwner
// variant, so the existing master directly executes433EA2AA in phase4.
AutomaticWeaponsLifecycleOwner&
construct_automatic_weapons(void* storage, engine::OwnerRegistryBinding lifecycle,
                            client::CommandPhaseRegistry& command_owners, ConfigRegistries&,
                            AutomaticWeaponsConfig&, AutomaticWeaponsState&,
                            const AutomaticWeaponsBindings&);
} //namespace skeet::misc
