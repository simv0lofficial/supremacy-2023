#pragma once
#include "prediction_mask_owner.hpp"
#include <memory>

namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::rage {
//4347EA07 precedes the actual ESP factory. This source context borrows that
// nullable singleton cell and resolves its EnginePlayers at the native weapon
// metadata use site. No synthetic player or second ESP adapter is created.
class PredictionMaskRuntimeOwner final {
public:
    PredictionMaskRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                               client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                               engine::SceneQueryRuntimeOwner&, esp::EspRuntimeOwner&);
    ~PredictionMaskRuntimeOwner();
    PredictionMaskRuntimeOwner(const PredictionMaskRuntimeOwner&) = delete;
    PredictionMaskRuntimeOwner& operator=(const PredictionMaskRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    PredictionMaskFactoryBindings& factory_bindings() noexcept;
    void bind_initializer(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::rage
