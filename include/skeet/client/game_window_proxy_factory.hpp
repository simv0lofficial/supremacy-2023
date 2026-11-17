#pragma once
#include "skeet/engine/game_window_proxy.hpp"
#include "skeet/engine/native_owner_registry_storage.hpp"

namespace skeet::client {
class NativeStartupSources;
struct GameWindowProxyFactoryBindings {
    std::shared_ptr<engine::NativeDataImage> data;
    engine::NativeOwnerRegistryStorage& registries;
    engine::JobMemory memory;
    engine::GameWindowProxyBindings proxy;
};
inline constexpr std::uint32_t game_window_proxy_factory = 0x4347D94Fu;
engine::GameWindowProxyOwner* create_game_window_proxy_owner(GameWindowProxyFactoryBindings&);
// Requires canonical cells/registries and real loaded-build guard/interface
// bindings. Binding this factory allocates nothing and publishes no proxy.
void bind_game_window_proxy_factory(NativeStartupSources&, GameWindowProxyFactoryBindings&);
} // namespace skeet::client
