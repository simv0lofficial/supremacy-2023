#pragma once
#include "free_look_owner.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class CrtRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::effects {
// Shared command/view state before43484A2B allocates its physical receivers.
class FreeLookRuntimeOwner final {
public:
    FreeLookRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                         client::ConfigurationRuntimeOwner&, client::CrtRuntimeOwner&);
    FreeLookRuntimeOwner(const FreeLookRuntimeOwner&) = delete;
    FreeLookRuntimeOwner& operator=(const FreeLookRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    FreeLookInitialization& initializers() noexcept {
        return initialization_;
    }
    void bind_initializers(client::NativeStartupSources&);

private:
    client::ClientRuntimeStorage& storage_;
    FreeLookOwnerState state_{};
    FreeLookViewBindings view_;
    FreeLookInitialization initialization_;
};
}
