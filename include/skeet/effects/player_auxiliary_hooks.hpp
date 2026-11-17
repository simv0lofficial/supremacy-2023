#pragma once
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
}
namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::effects {
struct EntityVisibilityRuntime;
class EntityNetworkableRuntime;
struct FreeLookCaptureBindings;

// Actual per-player hook inputs shared by the primary and ragdoll families.
// Offsets and player state come from the canonical history owner; callbacks
// resolve their saved local/entity at the original call sites. This owner
// creates no second free-look state, player cache, configuration or hook list.
// Keep these contexts and all borrowed owners alive until hooks are restored.
class PlayerAuxiliaryHooks {
public:
    PlayerAuxiliaryHooks(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                         client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                         animation::RuntimeOwner&, engine::HookRuntimeOwner&);
    ~PlayerAuxiliaryHooks();
    PlayerAuxiliaryHooks(const PlayerAuxiliaryHooks&) = delete;
    PlayerAuxiliaryHooks& operator=(const PlayerAuxiliaryHooks&) = delete;
    PlayerAuxiliaryHooks(PlayerAuxiliaryHooks&&) = delete;
    PlayerAuxiliaryHooks& operator=(PlayerAuxiliaryHooks&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    client::HistoryRuntimeOwner& history() const noexcept;
    EntityVisibilityRuntime& visibility() noexcept;
    EntityNetworkableRuntime& ragdolls() noexcept;
    const FreeLookCaptureBindings& free_look_capture() const noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}
