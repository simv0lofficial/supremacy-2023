#pragma once
#include <memory>

namespace skeet::animation {
struct LocalAnimationBindings;
}
namespace skeet::effects {
class EffectsRuntimeOwner;
class ViewRuntime;
struct ViewDuckBindings;
}
namespace skeet::prediction {
struct PlayerSimulationBindings;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
class LocalPlayerAnimationHooks;
class PlayerCommandQueries;
class PlayerPhysicsHooks;
class PlayerCommandHooks;
struct RestrictedPlayerCommandBindings;
struct EngineRuntimeBindings;
// Canonical local-player animation/command contexts, available before the
// prediction-history factory. All nullable owner gates borrow their actual
// cells and see later factory publications. This creates no feature owner,
// second ViewRuntime, hook installation or engine call.
class LocalAnimationRuntimeOwner final {
public:
    LocalAnimationRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                               ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                               CommandCaptureRuntimeOwner&, effects::EffectsRuntimeOwner&);
    ~LocalAnimationRuntimeOwner();
    LocalAnimationRuntimeOwner(const LocalAnimationRuntimeOwner&) = delete;
    LocalAnimationRuntimeOwner& operator=(const LocalAnimationRuntimeOwner&) = delete;
    LocalAnimationRuntimeOwner(LocalAnimationRuntimeOwner&&) = delete;
    LocalAnimationRuntimeOwner& operator=(LocalAnimationRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    HistoryRuntimeOwner& history() const noexcept;
    const RestrictedPlayerCommandBindings& restricted() const noexcept;
    const animation::LocalAnimationBindings& helpers() const noexcept;
    LocalPlayerAnimationHooks& local_hooks() noexcept;
    PlayerCommandQueries& queries() noexcept;
    PlayerPhysicsHooks& physics() noexcept;
    PlayerCommandHooks& command_hooks() noexcept;
    const prediction::PlayerSimulationBindings& simulation() const noexcept;
    effects::ViewDuckBindings& duck() noexcept;
    effects::ViewRuntime& view() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
