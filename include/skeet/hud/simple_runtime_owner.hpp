#pragma once
#include "simple_owners.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class VisualRuntimeOwner;
class NativeStartupSources;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::hud {
class SimpleHudRuntimeOwner final {
public:
    SimpleHudRuntimeOwner(client::ClientRuntimeStorage&, client::ConfigurationRuntimeOwner&,
                          client::VisualRuntimeOwner&, engine::HookRuntimeOwner&,
                          script::SourceScriptRuntimeOwner&);
    SimpleHudRuntimeOwner(const SimpleHudRuntimeOwner&) = delete;
    SimpleHudRuntimeOwner& operator=(const SimpleHudRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    SimpleHudOwnersContext& context() noexcept {
        return context_;
    }
    void bind_initializers(client::NativeStartupSources&);

private:
    client::ClientRuntimeStorage& storage_;
    SimpleHudOwnersContext context_;
};
}
