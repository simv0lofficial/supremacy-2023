#pragma once
#include "skeet/config.hpp"
#include "skeet/engine/bindings.hpp"
#include "skeet/engine/native_data_image.hpp"
#include "skeet/movement/global_config.hpp"
#include "skeet/skins/config.hpp"
#include <span>
#include <stdexcept>
#include <unordered_map>

namespace skeet::anti_aim {
struct StartupInitializerBindings;
}
namespace skeet::engine {
class PreservedInitialization;
class MapGameSystemRuntime;
class StartupVectorCleanupBindings;
class StartupContainerCleanup;
struct FirstStartupServiceBindings;
}
namespace skeet::script {
struct StartupInitializerBindings;
struct UiTypeInitialization;
}
namespace skeet::animation {
class FixAnimationBackupCleanupContext;
}
namespace skeet::rage {
struct GroupInitializerBindings;
struct FeedbackOwnerBindings;
struct PredictionMaskFactoryBindings;
}
namespace skeet::effects {
struct FreeLookInitialization;
}
namespace skeet::misc {
struct ViewmodelLagFactoryContext;
}
namespace skeet::hud {
struct SimpleHudOwnersContext;
}
namespace skeet::esp {
class FlagTableInitialization;
}
namespace skeet::events {
class EventNameStartupBindings;
}

namespace skeet::client {
struct ExitRegistrationContext;
struct ConfigInitializerBindings;
struct GlobalConfigInitializerBindings;
struct PredictionHistoryInitialization;
struct EntityCommandFactoryBindings;
struct TargetMaskFactoryBindings;
struct SharedEspInitialization;
struct BufferedShotFactoryBindings;
struct RuntimeOwnerFactoryBindings;
struct MiscRuntimeFactoryBindings;
struct CombatRuntimeFactoryBindings;
struct SkinRuntimeFactoryBindings;
struct MovementHudFactoryBindings;
struct HistoryRuntimeFactoryBindings;
struct WorldRuntimeFactoryBindings;
struct GameWindowProxyFactoryBindings;
class SourceTerminalCleanupBindings;
class HookDescriptorCleanupContext;
struct StaticCrtInitialization;
struct NativeStartupBitMasks {
    std::span<std::uint32_t, 32 * 33> matrix; //43477BE0
    std::span<std::uint32_t, 32> prefixes; //43477B60
};
void initialize_native_startup_bit_masks(NativeStartupBitMasks) noexcept; //433FB71E
struct NativeInitializerCall {
    std::uint32_t native_address;
    void* owner;
    void (*invoke)(void*);
};
struct NativePreflightCall {
    std::uint32_t native_address;
    void* owner;
    std::uint32_t (*invoke)(void*);
};

// The graph binds the same configuration/hotkey and mask storage consumed by
// the rest of the source. A source initializer entry names its actual body and
// borrowed owner; it is never a captured native-address function-pointer cast.
// Keep this graph and every binding context at stable addresses until all
// owner callbacks and registered terminal cleanup calls have finished.
struct NativeStartupSourceGraph {
    ConfigRegistries& configuration;
    RegisteredHotkey& menu_key;
    MovementGlobalConfig& movement;
    skins::Settings& skins;
    anti_aim::StartupInitializerBindings& anti_aim;
    ExitRegistrationContext& exits;
    ConfigInitializerBindings& static_configuration;
    engine::NativeDataImage& data;
    // Required exact43484F06 source binding. An unfinished final operation
    // must be reported by an absent binding, never a default empty callback.
    NativeInitializerCall final_initializer;
    engine::FirstStartupServiceBindings& first_startup_services;
    HookDescriptorCleanupContext& hook_descriptors;
    script::StartupInitializerBindings& script_extensions;
    animation::FixAnimationBackupCleanupContext& animation_backups;
    rage::GroupInitializerBindings& rage_groups;
    PredictionHistoryInitialization& prediction_history;
    EntityCommandFactoryBindings& entity_commands;
    effects::FreeLookInitialization& free_look;
    TargetMaskFactoryBindings& target_mask;
    SharedEspInitialization& shared_esp;
    rage::FeedbackOwnerBindings& shot_feedback;
    BufferedShotFactoryBindings& buffered_shots;
    misc::ViewmodelLagFactoryContext& viewmodel_lag;
    GlobalConfigInitializerBindings& global_configuration;
    hud::SimpleHudOwnersContext& simple_hud;
    esp::FlagTableInitialization& esp_flags;
    script::UiTypeInitialization& ui_types;
    RuntimeOwnerFactoryBindings& runtime_owners;
    rage::PredictionMaskFactoryBindings& prediction_masks;
    engine::MapGameSystemRuntime& map_game_system;
    MiscRuntimeFactoryBindings& misc_owners;
    CombatRuntimeFactoryBindings& combat_owners;
    SkinRuntimeFactoryBindings& skin_owners;
    MovementHudFactoryBindings& movement_hud;
    HistoryRuntimeFactoryBindings& history_owners;
    WorldRuntimeFactoryBindings& world_owners;
    GameWindowProxyFactoryBindings& game_window_proxy;
    StaticCrtInitialization& static_crt;
    events::EventNameStartupBindings& event_names;
    // These exact native cells have separate ownership from the semantic
    // Bridge/UiBindings/RecordPool STL allocations, which retain their own
    // source cleanup. This context does not claim to release those owners.
    engine::StartupVectorCleanupBindings& native_vector_cleanup;
    // Retained native container headers and the actual source renderer
    // listener share this terminal context; separate STL owners retain their
    // own destructors and are not released through the native cell views.
    engine::StartupContainerCleanup& native_container_cleanup;
    // The complete source graph releases the containers used by its actual
    // producers. Retained header cleanup cannot substitute for this profile.
    SourceTerminalCleanupBindings& source_terminal_cleanup;
};
class UnmappedNativeInitializer final : public std::runtime_error {
public:
    UnmappedNativeInitializer(std::uint32_t operand, std::uint32_t native_address, bool preflight);
    const std::uint32_t operand, native_address;
    const bool preflight;
};

class NativeStartupSources {
public:
    // Low-level source callback table. This constructor runs no initializer
    // and does not claim that the complete client graph has been composed.
    // The runtime preflights the fixed build plan before any startup effects.
    NativeStartupSources(engine::NativeDataImage&, NativeInitializerCall final_initializer,
                         std::span<const NativeInitializerCall> initializers = {},
                         std::span<const NativePreflightCall> preflight = {});
    NativeStartupSources(NativeStartupSourceGraph& graph,
                         std::span<const NativeInitializerCall> initializers,
                         std::span<const NativePreflightCall> preflight);
    NativeStartupSources(const NativeStartupSources&) = delete;
    NativeStartupSources& operator=(const NativeStartupSources&) = delete;
    void bind_initializer(NativeInitializerCall);
    void bind_preflight(NativePreflightCall);
    void require_initializer(std::uint32_t operand, std::uint32_t native_address) const;
    void require_preflight(std::uint32_t operand, std::uint32_t native_address) const;
    void initialize(std::uint32_t operand, std::uint32_t native_address) const;
    std::uint32_t check(std::uint32_t operand, std::uint32_t native_address) const;
    engine::NativeDataImage& data() const noexcept {
        return data_;
    }

private:
    engine::NativeDataImage& data_;
    NativeInitializerCall final_initializer_;
    std::unordered_map<std::uint32_t, NativeInitializerCall> initializers_;
    std::unordered_map<std::uint32_t, NativePreflightCall> preflight_;
};

#if defined(_M_IX86) || defined(__i386__)
using StartupTerminateThread = std::int32_t(__stdcall*)(void*, std::int32_t);
using StartupExitProcess = void(__stdcall*)(std::uint32_t);
#else
using StartupTerminateThread = std::int32_t (*)(void*, std::int32_t);
using StartupExitProcess = void (*)(std::uint32_t);
#endif
struct NativeStartupImports {
    void (*clear_unhandled_exception_filter)(); //SetUnhandledExceptionFilter(NULL)
    StartupTerminateThread terminate_thread; //ntdll!ZwTerminateThread
    StartupExitProcess exit_process; //ntdll!RtlExitUserProcess
};
NativeStartupImports native_startup_runtime_imports();
struct NativeStartupState {
    std::uintptr_t& initialization_argument; //43475AE8, third native entry argument
    void (*&fatal_handler)(); //43475BC4, native433FB70F
};

// Source scheduler for this build's explicit crt_initialization_plan. It
// preflights the whole plan before changing data or invoking an import/body.
// Observed identifiers select source bindings;43493903 is not executed or
// claimed recovered. The separately required final binding owns its actual
// remaining work; no successful bootstrap is manufactured here.
class NativeStartupRuntime {
public:
    NativeStartupRuntime(NativeStartupSources& sources, NativeStartupImports imports);
    std::uint32_t initialize(std::uintptr_t third_argument);
    void fatal(); //433FB70F -> RtlExitUserProcess(-10)
private:
    NativeStartupState state_;
    NativeStartupSources& sources_;
    NativeStartupImports imports_;
};
// Explicit legacy route only. Its caller supplies the real executable image
// and live operand-accessor environment, with the same canonical data owner.
// This remains separate from ordinary source data/storage construction.
class RetainedNativeStartupRuntime {
public:
    RetainedNativeStartupRuntime(NativeStartupSources&, engine::PreservedInitialization&,
                                 NativeStartupImports);
    std::uint32_t initialize(std::uintptr_t third_argument);
    void fatal();

private:
    NativeStartupState state_;
    NativeStartupSources& sources_;
    engine::PreservedInitialization& metadata_;
    NativeStartupImports imports_;
};
// Opt-in adapter for the still-protected final operation. Source composition
// does not select it automatically and requires its actual retained lifetime.
NativeInitializerCall retained_final_startup_initializer(engine::PreservedInitialization&);
// Source counterpart of the no-argument433FB70F callback. The scheduler owner
// must remain alive while this published callback can be invoked.
void native_startup_fatal();
}
