#pragma once
#include "player_opacity.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
}
namespace skeet::effects {
// No material, D3D or renderer resources are needed to compute opacity. The
// actual nullable model-owner cell still gates43340082 on each invocation;
// descriptors come from the one source configuration owner. No placeholder
// renderer is constructed or published by this owner.
class PlayerOpacityOwner {
public:
    PlayerOpacityOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                       client::ConfigurationRuntimeOwner&, client::PlayerRuntimeOwner&);
    PlayerOpacityOwner(const PlayerOpacityOwner&) = delete;
    PlayerOpacityOwner& operator=(const PlayerOpacityOwner&) = delete;
    PlayerOpacityOwner(PlayerOpacityOwner&&) = delete;
    PlayerOpacityOwner& operator=(PlayerOpacityOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    const PlayerOpacityBindings& bindings() const noexcept {
        return bindings_;
    }
    void apply(void* player) const {
        apply_player_opacity(player, bindings_);
    }

private:
    client::ClientRuntimeStorage& storage_;
    ModelEntityRelationships entities_;
    PlayerOpacityBindings bindings_;
};
} //namespace skeet::effects
