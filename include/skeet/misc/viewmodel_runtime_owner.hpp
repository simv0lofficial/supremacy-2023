#pragma once
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class NativeStartupSources;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::effects {
class EffectsRuntimeOwner;
}
namespace skeet::skins {
class SkinsRuntimeOwner;
}
namespace skeet::misc {
struct ViewmodelLagState;
class ViewmodelHooksRuntime;
struct ViewmodelLagFactoryContext;
// One source viewmodel state and the actual skins sequence-history producer.
// Sparse native initialization remains in4347E13E/43486C3D; construction
// registers no feature, installs no hook and samples no player/model.
class ViewmodelRuntimeOwner final {
public:
    ViewmodelRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                          client::ConfigurationRuntimeOwner&, skins::SkinsRuntimeOwner&,
                          effects::EffectsRuntimeOwner&, engine::HookRuntimeOwner&);
    ~ViewmodelRuntimeOwner();
    ViewmodelRuntimeOwner(const ViewmodelRuntimeOwner&) = delete;
    ViewmodelRuntimeOwner& operator=(const ViewmodelRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    ViewmodelLagState& state() noexcept;
    ViewmodelHooksRuntime& hooks() noexcept;
    ViewmodelLagFactoryContext& factory_bindings() noexcept;
    void bind_initializers(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::misc
