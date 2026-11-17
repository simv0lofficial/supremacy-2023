#pragma once
#include "hook_descriptor_storage.hpp"
#include "hook_descriptors.hpp"
#include <span>
#include <vector>

namespace skeet::animation {
class FixAnimationsRuntime;
class FixAnimationBackupRuntime;
class RemoteSetupBonesRuntime;
}
namespace skeet::effects {
class EntityNetworkableRuntime;
class HarmlessMolotovRuntime;
class ModelRenderHooks;
}
namespace skeet::misc {
class PingSpikeChannelRuntime;
struct NetworkMessageRuntime;
class ViewmodelHooksRuntime;
class AutoAcceptHookFamily;
}
namespace skeet::network {
class ClientStateHookFamily;
struct PacketDispatchRuntime;
}
namespace skeet::script {
struct ConsoleInputHook;
}
namespace skeet::ui {
class FrameRuntime;
}

namespace skeet::engine {
class PlayerHookFamily;
struct NativeOffsetStorage;
enum class BuildHookDescriptor {
    channel,
    local_player,
    local_thinkable,
    local_renderable,
    local_networkable,
    remote_player,
    remote_thinkable,
    remote_renderable,
    remote_networkable,
    game_event_listener,
    ragdoll_networkable,
    fire_bullets_networkable,
    molotov,
    incendiary,
    viewmodel,
    inferno,
    particle_effect,
    particle_unknown,
    particle_renderable,
    focus_overlay,
    client_state,
    studio,
    leaf_system,
    auto_accept,
    console,
    overlay_stream
};
struct BuildHookDescriptorRecipe {
    BuildHookDescriptor kind;
    std::uint32_t descriptor, module_cell, type_hash, base_hash;
    std::uint32_t descriptor_operand, replacement_operand, replacement_count;
};
struct BuildHookNativeReplacement {
    std::uint32_t slot, native_entry, entry_operand;
};
// Fixed-build metadata, not an implementation of the protected operand reader.
// Existing family manifests remain the sole source of their replacement rows.
std::span<const BuildHookDescriptorRecipe, 26> build_hook_descriptors() noexcept;
std::vector<BuildHookNativeReplacement> build_hook_native_replacements(BuildHookDescriptor);

// Canonical state for the short hooks which previously had no source entry.
// FocusOverlayHooks separately owns the canonical post-init/HUD callbacks.
class BuildHookCallbacks {
public:
    BuildHookCallbacks(HookDescriptorStorage&, VtableHookRegistry&, const NativeOffsetStorage&,
                       animation::FixAnimationsRuntime&, animation::FixAnimationBackupRuntime&,
                       misc::PingSpikeChannelRuntime&, ui::FrameRuntime&);
    void bind(std::uint32_t native_entry, VtableHookMetadata&, std::uint32_t slot);
    static void bind_static(std::uint32_t native_entry, VtableHookMetadata&, std::uint32_t slot);
    std::uintptr_t channel_reset(void*, std::uintptr_t caller);
    void channel_packet(void*, std::uintptr_t, std::uintptr_t);
    void detach_thinkable(void*);
    void detach_networkable(void*, bool local);
    void record_local_frame(void*);
    void before_remote_data_change(void*, std::int32_t);
    std::uintptr_t renderable_shadow(void*);
    std::uintptr_t game_event(void*, void*);
    std::uintptr_t weapon_query(void*);
    std::uintptr_t overlay_stream(void*, std::uintptr_t caller);

private:
    const NativeDataImage& data_;
    const NativeOffsetStorage& offsets_;
    animation::FixAnimationsRuntime& animations_;
    animation::FixAnimationBackupRuntime& backups_;
    misc::PingSpikeChannelRuntime& channel_;
    ui::FrameRuntime& frame_;
};

struct BuildHookFamilies {
    PlayerHookFamily& players;
    animation::FixAnimationsRuntime& animations;
    animation::RemoteSetupBonesRuntime& remote_bones;
    BuildHookCallbacks& callbacks;
    effects::EntityNetworkableRuntime& ragdolls;
    effects::HarmlessMolotovRuntime& particles;
    network::PacketDispatchRuntime& packets;
    misc::PingSpikeChannelRuntime& channel;
    misc::NetworkMessageRuntime& messages;
    misc::ViewmodelHooksRuntime& viewmodels;
    network::ClientStateHookFamily& client_state;
    effects::ModelRenderHooks& models;
    misc::AutoAcceptHookFamily& auto_accept;
    script::ConsoleInputHook& console;
};
// Actual early phase: twenty-six genuine RTTI clones and94 source entry
// addresses, before all57 feature factories and RootMenu. It publishes no
// runtime context, installs no receiver and needs no feature/menu object.
void prepare_build_hook_descriptors(HookDescriptorStorage&, JobMemory, NativeImageEnvironment,
                                    HookDescriptorSystem);
// Later context publication after the actual owners exist. Reuses those same
// descriptors without allocating, querying memory or resolving RTTI again.
// Complete this before installing any prepared table on a real receiver.
void bind_build_hook_runtime_contexts(HookDescriptorStorage&, VtableHookRegistry&,
                                      const BuildHookFamilies&);
// Whole descriptor-production phase. Resolve actual engine RTTI and clone its
// executable prefix, then bind source entries in the native26-group order.
// Actual receiver installation remains in the existing object lifecycles.
// Source startup uses the two phases above at their separate native cuts.
void initialize_build_hooks(HookDescriptorStorage&, VtableHookRegistry&, NativeImageEnvironment,
                            HookDescriptorSystem, const BuildHookFamilies&);
}
