#pragma once
#include "player_receive.hpp"
#include "skeet/animation/layer_receive.hpp"
#include <span>

namespace skeet::animation {
class FixRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class PlayerRuntimeOwner;
struct PlayerReceiveDescriptor {
    std::uint32_t operand, class_hash, table_hash, property_hash, native_entry, patch_address;
    engine::RecvVarProxy replacement;
};
std::span<const PlayerReceiveDescriptor, 15> player_receive_descriptors() noexcept;
// Separate capture and publication preserve startup's prepare/install split.
// Neither function installs a property replacement or mutates patch links.
void initialize_player_receive_patch_targets(engine::NativeDataImage&);
void bind_player_receive_patches(engine::NativeDataImage&, PlayerReceiveRuntime&,
                                 animation::LayerReceiveRuntime&);
// Shared runtime assembly for the fifteen player/layer receive properties.
// Keep this owner through patch restoration. It creates no second player cache
// and uses the existing FixAnimations sequence receiver and configuration.
class PlayerReceiveOwner {
public:
    PlayerReceiveOwner(ClientRuntimeStorage&, PlayerRuntimeOwner&, animation::FixRuntimeOwner&);
    PlayerReceiveOwner(const PlayerReceiveOwner&) = delete;
    PlayerReceiveOwner& operator=(const PlayerReceiveOwner&) = delete;
    ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    PlayerReceiveRuntime& players() noexcept {
        return players_;
    }
    animation::LayerReceiveRuntime& layers() noexcept {
        return layers_;
    }
    void bind_receive_patches();
    // Capture the actual VClient018 RecvProp targets and current originals.
    // All-fifteen validation is explicit source policy, not a claim about the
    // protected initializer's missing-property path. No next/head/slot writes.
    void initialize_receive_patches();

private:
    ClientRuntimeStorage& storage_;
    PlayerReceiveRuntime players_;
    animation::LayerReceiveRuntime layers_;
};
} //namespace skeet::client
