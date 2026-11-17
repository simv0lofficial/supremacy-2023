#pragma once
#include "receive_proxies.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
}
namespace skeet::effects {
// Source lifetime for the five camera/world receive callbacks. Construction
// borrows canonical owners only; explicit initialization belongs to the later
// receive-patch startup cut, after the real feature owners/classes exist.
class VisualReceiveOwner {
public:
    VisualReceiveOwner(client::ClientRuntimeStorage&, client::ConfigurationRuntimeOwner&,
                       client::PlayerRuntimeOwner&);
    VisualReceiveOwner(const VisualReceiveOwner&) = delete;
    VisualReceiveOwner& operator=(const VisualReceiveOwner&) = delete;
    VisualReceiveOwner(VisualReceiveOwner&&) = delete;
    VisualReceiveOwner& operator=(VisualReceiveOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    VisualReceiveRuntime& runtime() noexcept {
        return runtime_;
    }

private:
    client::ClientRuntimeStorage& storage_;
    VisualReceiveRuntime runtime_;
};
} //namespace skeet::effects
