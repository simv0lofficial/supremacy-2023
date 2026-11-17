#pragma once
#include "shutdown.hpp"
#include "source_dll_shutdown.hpp"
#include "skeet/misc/unload.hpp"
#include <memory>

namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::ui {
class MainMenuWindowOwner;
class StartupRenderer;
}
namespace skeet::client {
class ClientRuntimeStorage;
class WindowRuntimeOwner;
struct ClientShutdownImports {
    ShutdownImports finalization;
    ui::WindowLifetimeImports window;
    misc::UnloadImports request;
};
ClientShutdownImports native_client_shutdown_imports();

// Source shutdown/request composition before or after ScriptOwner. All menu,
// Lua, heap, hook and configuration contexts are borrowed from their actual
// owners. Constructing this object neither unloads nor schedules a timer.
class ClientShutdownOwner {
public:
    ClientShutdownOwner(ClientRuntimeStorage&, WindowRuntimeOwner&, ui::MainMenuWindowOwner&,
                        ConfigStorageOwner&, script::SourceScriptRuntimeOwner&,
                        ui::StartupRenderer&);
    ClientShutdownOwner(ClientRuntimeStorage&, WindowRuntimeOwner&, ui::MainMenuWindowOwner&,
                        ConfigStorageOwner&, script::SourceScriptRuntimeOwner&,
                        ui::StartupRenderer&, ClientShutdownImports);
    ~ClientShutdownOwner();
    ClientShutdownOwner(const ClientShutdownOwner&) = delete;
    ClientShutdownOwner& operator=(const ClientShutdownOwner&) = delete;
    ClientShutdownOwner(ClientShutdownOwner&&) = delete;
    ClientShutdownOwner& operator=(ClientShutdownOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    ShutdownRuntime& finalizer() noexcept;
    misc::UnloadRuntime& request() noexcept;
    // Required explicit profile selection before the unload button is usable.
    // The module must contain the source callback; no mapped-image profile
    // is selected when this validation fails or has not yet run.
    void select_source_dll(void* module);
    void select_source_dll(void* module, SourceDllShutdownImports);
    // An owning application supplies its destruction cut explicitly. The
    // callback runs after finalize and may destroy this owner itself.
    void select_source_dll(void* module, SourceDllShutdownCompletion);
    void select_source_dll(void* module, SourceDllShutdownImports, SourceDllShutdownCompletion);
    void cancel_deferred_request() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}
