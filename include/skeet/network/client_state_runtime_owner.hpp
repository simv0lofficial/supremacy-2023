#pragma once
#include "skeet/client/entity_update_dispatch.hpp"
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::ui {
class MainMenuWindowOwner;
}
namespace skeet::esp {
struct EffectsSettings;
}
namespace skeet::events {
class EventPacketHookRuntime;
}
namespace skeet::network {
class ClientStateHooksRuntime;
class ClientStateHookFamily;
struct TickUpdateRuntime;
struct SharedEspRuntime;
struct IncomingUpdateRuntime;
struct ClientStateRuntimeImports {
    client::EntityUpdateBindings::ExchangeWord exchange;
};
ClientStateRuntimeImports native_client_state_runtime_imports();

// Concrete contexts for all thirteen ClientState replacement entries, including
// the nullable live ESP-owner cell and the command producer's actual sent view.
// Construction borrows actual menu/scripts/history/configuration and the
// existing channel runtime. No callback, registration or table swap runs here.
class ClientStateRuntimeOwner {
public:
    ClientStateRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                            client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                            client::CommandCaptureRuntimeOwner&, engine::HookRuntimeOwner&,
                            script::SourceScriptRuntimeOwner&, ui::MainMenuWindowOwner&);
    ClientStateRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                            client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                            client::CommandCaptureRuntimeOwner&, engine::HookRuntimeOwner&,
                            script::SourceScriptRuntimeOwner&, ui::MainMenuWindowOwner&,
                            ClientStateRuntimeImports);
    ~ClientStateRuntimeOwner();
    ClientStateRuntimeOwner(const ClientStateRuntimeOwner&) = delete;
    ClientStateRuntimeOwner& operator=(const ClientStateRuntimeOwner&) = delete;
    ClientStateRuntimeOwner(ClientStateRuntimeOwner&&) = delete;
    ClientStateRuntimeOwner& operator=(ClientStateRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    ClientStateHooksRuntime& core() noexcept;
    TickUpdateRuntime& ticks() noexcept;
    IncomingUpdateRuntime& incoming() noexcept;
    SharedEspRuntime& shared_esp() noexcept;
    events::EventPacketHookRuntime& event_packets() noexcept;
    client::EntityUpdateRuntime& entities() noexcept;
    const esp::EffectsSettings& effects() const noexcept;
    ClientStateHookFamily& family() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}
