#pragma once
#include "shared_esp_owner.hpp"
#include <memory>

namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
class HistoryRuntimeOwner;
// Stable production contexts for the separate4348DA78 shared-ESP owner.
// Main ESP, history, menu, Lua and the command master borrow one player cache
// and adjustment table. This context runs no factory or callback on creation.
class SharedEspRuntimeOwner {
public:
    SharedEspRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                          ConfigurationRuntimeOwner&, PlayerRuntimeOwner&, HistoryRuntimeOwner&,
                          script::SourceScriptRuntimeOwner&);
    ~SharedEspRuntimeOwner();
    SharedEspRuntimeOwner(const SharedEspRuntimeOwner&) = delete;
    SharedEspRuntimeOwner& operator=(const SharedEspRuntimeOwner&) = delete;
    SharedEspRuntimeOwner(SharedEspRuntimeOwner&&) = delete;
    SharedEspRuntimeOwner& operator=(SharedEspRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    SharedEspInitialization& initializers() noexcept;
    const RejectedPlayerUpdateContext& rejected_player_update() const noexcept;
    network::SharedEspRuntime& incoming() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} // namespace skeet::client
