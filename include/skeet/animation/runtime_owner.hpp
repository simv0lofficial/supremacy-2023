#pragma once
#include "skeet/animation/native_trigonometry.hpp"
#include "skeet/animation/runtime_bindings.hpp"
#include "skeet/animation/remote_animation_preparation.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
}
namespace skeet::animation {
// Stable assembly for the three remote bone/animation runtimes. Construct
// after ClientRuntimeStorage::initialize_engine_bindings; keep storage and the
// shared PlayerStateCache alive until hooks and all consumers are detached.
// Owns its fields and bindings, including the binding borrowed by the remote
// hook. Neither another player cache nor a snapshot of dynamic globals exists.
class RuntimeOwner {
public:
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::PlayerStateCache&);
    RuntimeOwner(const RuntimeOwner&) = delete;
    RuntimeOwner& operator=(const RuntimeOwner&) = delete;
    RuntimeOwner(RuntimeOwner&&) = delete;
    RuntimeOwner& operator=(RuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    client::PlayerStateCache& players() const noexcept {
        return players_;
    }
    const RageTrigonometry& trigonometry() const noexcept {
        return trigonometry_;
    }
    const BoneRuntime& bones() const noexcept {
        return bones_;
    }
    const RemoteAnimationPreparationRuntime& preparation() const noexcept {
        return preparation_;
    }
    RemoteSetupBonesRuntime& remote_setup_bones() noexcept {
        return remote_;
    }
    const RemoteSetupBonesRuntime& remote_setup_bones() const noexcept {
        return remote_;
    }

private:
    client::ClientRuntimeStorage& storage_;
    client::PlayerStateCache& players_;
    const RageTrigonometry& trigonometry_;
    RemoteAnimationPreparationFields preparation_fields_;
    BoneRuntime bones_;
    RemoteAnimationPreparationRuntime preparation_;
    RemoteSetupBonesBindings remote_bindings_;
    RemoteSetupBonesRuntime remote_;
};
} //namespace skeet::animation
