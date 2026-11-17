#pragma once
#include "hooks.hpp"
#include <memory>

namespace skeet::client {
class CrtRuntimeOwner;
class ClientRuntimeStorage;
class PostInitializationOwner;
class MiscRuntimeOwner;
class VisualRuntimeOwner;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
struct EngineRuntimeBindings;
struct ChannelHookState;
}
namespace skeet::misc {
struct AutoAcceptPatchState;
struct AutoAcceptOwnerBindings;
class AutoAcceptHookFamily;
class PingSpikeChannelRuntime;
struct NetworkMessageRuntime;
}
namespace skeet::animation {
class FixRuntimeOwner;
class RuntimeOwner;
}
namespace skeet::effects {
class PlayerAuxiliaryHooks;
}
namespace skeet::script {
class ScriptOwner;
class SourceScriptRuntimeOwner;
struct ConsoleInputHook;
}
namespace skeet::ui {
class StartupRenderer;
class MainMenuWindowOwner;
}
namespace skeet::network {
class ClientStateRuntimeOwner;
}
namespace skeet::engine {
class HookDescriptorStorage;
struct HookDescriptorSystem;
struct NativeImageEnvironment;
class FocusOverlayHooks;
class BuildHookCallbacks;

// Process-lifetime hook composition shared by animation/effects owners and
// the later menu/post-initialization stage. It creates no second hook list,
// data image, script runtime, renderer, configuration or descriptor storage.
class HookRuntimeOwner {
public:
    HookRuntimeOwner(client::CrtRuntimeOwner&, script::SourceScriptRuntimeOwner&,
                     ui::StartupRenderer&);
    ~HookRuntimeOwner();
    HookRuntimeOwner(const HookRuntimeOwner&) = delete;
    HookRuntimeOwner& operator=(const HookRuntimeOwner&) = delete;
    HookRuntimeOwner(HookRuntimeOwner&&) = delete;
    HookRuntimeOwner& operator=(HookRuntimeOwner&&) = delete;
    VtableHookRegistry& registry() noexcept;
    client::ClientRuntimeStorage& storage() const noexcept;
    UnloadState& unload_state() const noexcept;
    HookDescriptorStorage& descriptor_storage() noexcept;
    HookDescriptorSystem descriptor_system() const noexcept;
    NativeImageEnvironment descriptor_images() const noexcept;
    misc::AutoAcceptPatchState& auto_accept_patch() noexcept;
    misc::AutoAcceptHookFamily& auto_accept_hooks() noexcept;
    misc::AutoAcceptOwnerBindings auto_accept_owner_bindings() const;
    script::ConsoleInputHook& console_input() noexcept;
    script::ScriptOwner& script_owner() const;
    // Compose the two channel handlers from the actual misc feature owner.
    // These contexts borrow the canonical history/acknowledgement cells and
    // live clock/message-transition aliases; no hook is installed here.
    void initialize_network(client::MiscRuntimeOwner&);
    misc::PingSpikeChannelRuntime& channel();
    client::ChannelHookState& channel_state();
    misc::NetworkMessageRuntime& messages();
    // FixRuntimeOwner itself borrows this hook registry. Construct it first,
    // then bind the shared short-hook context to its actual animation state.
    void initialize_callbacks(animation::FixRuntimeOwner&);
    BuildHookCallbacks& callbacks();
    void initialize_player_auxiliary(const client::EngineRuntimeBindings&,
                                     client::ConfigurationRuntimeOwner&,
                                     client::HistoryRuntimeOwner&, animation::RuntimeOwner&);
    effects::PlayerAuxiliaryHooks& player_auxiliary();
    void initialize_client_state(const client::EngineRuntimeBindings&,
                                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                                 client::CommandCaptureRuntimeOwner&,
                                 script::SourceScriptRuntimeOwner&, ui::MainMenuWindowOwner&);
    network::ClientStateRuntimeOwner& client_state();
    // This actual phase runs after RootMenu/PostInitializationOwner exist.
    // It publishes the source callbacks, resolves the actual engine panel,
    // and installs the one-shot entry. Its first real invocation initializes
    // startup, then publishes/enters the permanent HUD focus entry.
    void initialize_focus(client::PostInitializationOwner&, client::VisualRuntimeOwner&);
    FocusOverlayHooks& focus();

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}
