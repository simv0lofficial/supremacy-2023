#pragma once
#include "camera.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::effects {
// Camera callbacks and receive proxies use the same player cache and live
// source globals. Construction neither reads a player nor changes the camera.
class CameraRuntimeOwner {
public:
    CameraRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                       client::ConfigurationRuntimeOwner&, client::PlayerRuntimeOwner&);
    CameraRuntimeOwner(const CameraRuntimeOwner&) = delete;
    CameraRuntimeOwner& operator=(const CameraRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    CameraRuntime& runtime() noexcept {
        return runtime_;
    }
    CameraInterfaces& interfaces() noexcept {
        return interfaces_;
    }
    CameraHistory& history() noexcept {
        return history_;
    }

private:
    client::ClientRuntimeStorage& storage_;
    CameraInterfaces interfaces_;
    CameraHistory history_;
    CameraRuntime runtime_;
};
}
