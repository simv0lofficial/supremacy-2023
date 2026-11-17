#pragma once
#include "skeet/client/command_empty_tick.hpp"
#include "skeet/client/command_owner_dispatch.hpp"
#include "skeet/engine/owner_registry.hpp"
#include <vector>

namespace skeet::legit {
class LifecycleOwner;
struct OwnerBindings {
    engine::OwnerRegistryBinding lifecycle;
    client::CommandPhaseRegistry& command_owners;
    client::CommandEmptyTickRegistry& empty_tick_owners;
    ConfigRegistries& configuration;
    Settings& settings;
    client::LegitCommandOwner& commands;
    LifecycleOwner*& active; // canonical4347588C, cleared by433689DD
};

// RuntimeState projection of43484884..434848F3. Native+800 already aliases
// commands.settings.rage_enabled.value through RuntimeSettings. Unwritten words, the other
// three bytes of the two view components/maximum-lock-time and track interval
// words survive construction. This is distinct from primary slot7's reset.
void initialize_runtime_state(RuntimeState&) noexcept;
std::size_t legit_owner_size() noexcept;
// Whole434845CF: primary -> command -> empty-tick registration, then tables,
// all51 descriptors, selective state construction and final config overrides.
// Storage carries the43387448 header. Canonical state/settings/bindings and
// borrowed registry ranges outlive the owner. active is published by its caller.
LifecycleOwner& construct_legit_owner(void* storage, OwnerBindings);
// Whole4347FEBC, including the explicit mem_int3.h record for the canonical
// pointer store at4347FED5. The native allocator requests0x1310/flags8; the
// source's borrowed settings/state are zeroed on successful allocation before
// constructor registration. They must be fresh and unregistered. Failure
// publishes null and leaves those borrowed values unchanged.
LifecycleOwner* create_legit_owner(OwnerBindings);
// Whole433689DD: clear active even for a null object, then433837D0 header-free.
void destroy_legit_owner(LifecycleOwner* owner, LifecycleOwner*& active);
void* legit_lifecycle_object(LifecycleOwner&) noexcept;
// Resolve the canonical configuration through the nullable owner slot. The
// source LifecycleOwner is not laid out as the native0x1310-byte allocation.
const Settings* legit_owner_settings(const LifecycleOwner*) noexcept;

// Immutable guest verification executes4347FEBC only through4347FED5: the
// five trapped publication bytes remain unmodified. Their source semantics
// come from the recorded MOV handler, rather than guest execution of that MOV.
} // namespace skeet::legit
