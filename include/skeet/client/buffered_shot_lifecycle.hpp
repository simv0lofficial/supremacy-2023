#pragma once
#include "buffered_shot_command.hpp"
#include "command_phase_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include <vector>

namespace skeet::engine {
class NativeOwnerRegistryStorage;
}
namespace skeet::client {
class NativeStartupSources;
class BufferedShotLifecycleOwner;
struct BufferedShotLifecycleBindings {
    engine::OwnerRegistryBinding primary; //canonical 43475B78/7C/80
    CommandPhaseRegistry& commands; //canonical normal-command order
    BufferedShotCommandOwner& command;
    // The one full-owner slot43475968. The factory publishes it after
    // construction; the constructor itself performs no global publication.
    BufferedShotLifecycleOwner*& active;
};

inline constexpr std::uint32_t buffered_shot_constructor = 0x4347FE52u;
inline constexpr std::uint32_t buffered_shot_primary_vtable = 0x4346FC00u;
inline constexpr std::uint32_t buffered_shot_factory = 0x43485BE3u;
inline constexpr std::size_t buffered_shot_native_size = 0x126B8u;

std::size_t buffered_shot_lifecycle_size() noexcept;
// Complete ordinary4347FE52 on caller storage carrying the43387448 header.
// Primary registration precedes F01C command registration, primary table
// publication, then sparse construction of the ONE borrowed state owner.
// No configuration, empty-tick, data-update or other registry is added.
// The F01C interface uses the existing typed phase-registry representation.
// The borrowed command/state/registries/active cell must outlive this object.
BufferedShotLifecycleOwner& construct_buffered_shot_lifecycle(void* storage,
                                                              BufferedShotLifecycleBindings);

// Complete43485BE3..43485C01: allocate126B8/HEAP_ZERO_MEMORY and construct on
// success. Both branches publish the returned owner/null to the same active
// cell. The five trapped bytes at43485BFC encode the MOV[43475968],EAX
// recorded in mem_int3.h:560; the ordinary suffix at43485C01 is RET.
BufferedShotLifecycleOwner* create_buffered_shot_owner(BufferedShotLifecycleBindings);

// Complete primary slot0: clear active first, then433837D0 header-free.
// Native destruction leaves the primary and command registry entries intact.
void destroy_buffered_shot_lifecycle(BufferedShotLifecycleOwner*,
                                     BufferedShotLifecycleOwner*& active);
// Complete primary slot7,433D6C8F: count0 and all150 command tags -1 only.
void reset_buffered_shot_lifecycle(BufferedShotLifecycleOwner&) noexcept;
// Complete primary slot5,433B06E3: call the current primary slot7.
void refresh_buffered_shot_lifecycle(BufferedShotLifecycleOwner&);
void* buffered_shot_lifecycle_object(BufferedShotLifecycleOwner&) noexcept;
// Resolve views from the one live full-owner slot. No command/state/history
// pointer is published in another canonical cell.
BufferedShotCommandOwner* buffered_shot_command_owner(BufferedShotLifecycleOwner*) noexcept;
BufferedShotCommandStorage* buffered_shot_command_storage(BufferedShotLifecycleOwner*) noexcept;
rage::ShotDiagnosticHistory* buffered_shot_diagnostics(BufferedShotLifecycleOwner*) noexcept;

// Stable source binding for the actual parameterless43485BE3 initializer.
// The image-backed registry and active pointer cell are checked by the binder
// before any factory call. Retain this context through startup invocation.
struct BufferedShotFactoryBindings {
    engine::NativeOwnerRegistryStorage& registries;
    const BufferedShotLifecycleBindings lifecycle;
    BufferedShotFactoryBindings(engine::NativeOwnerRegistryStorage& registries,
                                BufferedShotLifecycleBindings lifecycle) noexcept
        : registries(registries), lifecycle(lifecycle) {}
    BufferedShotFactoryBindings(const BufferedShotFactoryBindings&) = delete;
    BufferedShotFactoryBindings& operator=(const BufferedShotFactoryBindings&) = delete;
    BufferedShotFactoryBindings(BufferedShotFactoryBindings&&) = delete;
    BufferedShotFactoryBindings& operator=(BufferedShotFactoryBindings&&) = delete;
};
// Register the confirmed factory identity only. The existing live startup
// scheduler selects when it runs; binding does not construct or publish an owner.
void bind_buffered_shot_factory(NativeStartupSources&, BufferedShotFactoryBindings&);
}
