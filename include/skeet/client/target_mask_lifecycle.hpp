#pragma once
#include "target_mask_owner.hpp"
#include "command_phase_registry.hpp"
#include "entity_observer_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/native_data_image.hpp"
#include <vector>

namespace skeet::client {
class NativeStartupSources;
class TargetMaskLifecycleOwner;

struct TargetMaskOwnerRegistries {
    engine::OwnerRegistryBinding primary; //433676A0, 43475B78
    CommandPhaseRegistry& commands; //43345EBC, 434757A0
    engine::OwnerRegistryBinding messages; //4336F73C, 43475B94
    EntityObserverRegistry& entities; //4333BF32, 43475BA8
    engine::OwnerRegistryBinding hud; //433D20B5, 43475A9C
};
struct TargetMaskFactoryBindings {
    TargetMaskOwnerRegistries registries;
    TargetMaskOwner& command;
    TargetMaskSettings& settings; //same descriptor borrowed by CRT43490C68
    std::shared_ptr<engine::NativeDataImage> data;
};

inline constexpr std::uint32_t target_mask_factory = 0x4348066Au;
inline constexpr std::uint32_t target_mask_constructor = 0x434806CEu;
inline constexpr std::uint32_t target_mask_primary_vtable = 0x43470114u;
inline constexpr std::size_t target_mask_native_size = 0x63Cu;

std::size_t target_mask_lifecycle_size() noexcept;
// Whole ordinary434806CE..4348075F. All five interfaces borrow command.state;
// primary, command, SendNetMsg, BA8 and HUD registration retain that order.
// Constructor writes only its proven scalar/mask fields, preserving record
// ticks and the high three bytes of accepted_tick on caller-provided storage.
TargetMaskLifecycleOwner& construct_target_mask_lifecycle(void* storage,
                                                          const TargetMaskFactoryBindings&);

//4348066A successful allocation/tail-jump path. Uses the exact0x63C native
// zeroed allocation and projects those zeros into the one borrowed state.
// The null edge enters original INT1/junk at4348067F; no ordinary continuation
// or publication is proved. Source stops explicitly at that unfinished edge.
TargetMaskLifecycleOwner* create_target_mask_lifecycle(const TargetMaskFactoryBindings&);

// Primary43470114 slot0 is433B1A7A ->433837D0: header-free only, with no
// owner publication clear, state reset or registry removal. Slot6 is the
// existing complete4332F1D9 reset of the same canonical command owner.
void destroy_target_mask_lifecycle(TargetMaskLifecycleOwner*);
void reset_target_mask_lifecycle(TargetMaskLifecycleOwner&) noexcept;
void* target_mask_lifecycle_object(TargetMaskLifecycleOwner&) noexcept;
TargetMaskOwner& target_mask_command_owner(TargetMaskLifecycleOwner&) noexcept;

// Register only the actual4348066A factory identity. Startup's live operand
// order selects when to invoke it; this binding does not construct an owner.
void bind_native_target_mask_factory(NativeStartupSources&, TargetMaskFactoryBindings&);
} // namespace skeet::client
