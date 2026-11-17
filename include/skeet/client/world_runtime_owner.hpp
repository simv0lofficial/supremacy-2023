#pragma once
#include "world_runtime_factories.hpp"
#include "game_window_proxy_factory.hpp"
#include <memory>

namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::effects {
class EffectsRuntimeOwner;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class VisualRuntimeOwner;
class WindowRuntimeOwner;
struct EngineRuntimeBindings;
struct WorldRuntimeImports {
    input::NativeWindowLifecycleImports window;
    engine::GameWindowProxyImports proxy;
};
WorldRuntimeImports native_world_runtime_imports();

// Stable contexts for the twelve complete world factories and the genuine
// game-window proxy. Construction binds existing process owners only; their
// factories retain the startup scheduler's allocation/registration order.
class WorldRuntimeOwner {
public:
    WorldRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                      ConfigurationRuntimeOwner&, HistoryRuntimeOwner&, VisualRuntimeOwner&,
                      effects::EffectsRuntimeOwner&, engine::HookRuntimeOwner&, WindowRuntimeOwner&,
                      ConfigStorageOwner&);
    WorldRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                      ConfigurationRuntimeOwner&, HistoryRuntimeOwner&, VisualRuntimeOwner&,
                      effects::EffectsRuntimeOwner&, engine::HookRuntimeOwner&, WindowRuntimeOwner&,
                      ConfigStorageOwner&, WorldRuntimeImports);
    ~WorldRuntimeOwner();
    WorldRuntimeOwner(const WorldRuntimeOwner&) = delete;
    WorldRuntimeOwner& operator=(const WorldRuntimeOwner&) = delete;
    WorldRuntimeOwner(WorldRuntimeOwner&&) = delete;
    WorldRuntimeOwner& operator=(WorldRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    WorldRuntimeFactoryBindings& factory_bindings() noexcept;
    GameWindowProxyFactoryBindings& proxy_factory_bindings() noexcept;
    const effects::HarmlessMolotovBindings& molotov_bindings() const noexcept;
    events::HitMarkerBindings& hit_marker_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
