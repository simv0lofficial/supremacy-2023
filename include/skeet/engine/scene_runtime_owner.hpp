#pragma once
#include "scene_queries.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
class PlayerRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
// Real source adapters used by movement, bullet penetration and grenade
// prediction. All interface, class, clock and ConVar cells stay live.
class SceneQueryRuntimeOwner {
public:
    SceneQueryRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                           client::PlayerRuntimeOwner&);
    SceneQueryRuntimeOwner(const SceneQueryRuntimeOwner&) = delete;
    SceneQueryRuntimeOwner& operator=(const SceneQueryRuntimeOwner&) = delete;
    SceneQueryRuntimeOwner(SceneQueryRuntimeOwner&&) = delete;
    SceneQueryRuntimeOwner& operator=(SceneQueryRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    client::PlayerRuntimeOwner& players() const noexcept {
        return players_;
    }
    SceneQueries& queries() noexcept {
        return queries_;
    }

private:
    client::ClientRuntimeStorage& storage_;
    client::PlayerRuntimeOwner& players_;
    SceneQueryBindings bindings_;
    SceneQueries queries_;
};
} //namespace skeet::engine
