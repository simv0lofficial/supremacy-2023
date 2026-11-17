#pragma once
#include "runtime_owner_bindings.hpp"
#include <memory>

namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::skins {
class SkinsRuntimeOwner;
}
namespace skeet::rage {
class RuntimeOwner;
}
namespace skeet::legit {
class RuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::script {
class ScriptApiRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;

// Process lifetime for the eight factories shared by combat, rendering and
// events. Each domain supplies its existing physical state. Construction only
// composes references; the native factory order still controls registration.
class FeatureRuntimeOwner final {
public:
    FeatureRuntimeOwner(ClientRuntimeStorage&, ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                        skins::SkinsRuntimeOwner&, rage::RuntimeOwner&, legit::RuntimeOwner&,
                        esp::EspRuntimeOwner&, script::ScriptApiRuntimeOwner&,
                        engine::HookRuntimeOwner&);
    ~FeatureRuntimeOwner();
    FeatureRuntimeOwner(const FeatureRuntimeOwner&) = delete;
    FeatureRuntimeOwner& operator=(const FeatureRuntimeOwner&) = delete;
    FeatureRuntimeOwner(FeatureRuntimeOwner&&) = delete;
    FeatureRuntimeOwner& operator=(FeatureRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    RuntimeOwnerFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
