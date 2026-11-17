#pragma once
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class LocalAnimationRuntimeOwner;
class CommandCaptureRuntimeOwner;
class NativeStartupSources;
struct EngineRuntimeBindings;
struct EntityCommandState;
struct EntityCommandBindings;
struct EntityCommandOwner;
struct EntityCommandFactoryBindings;
struct EntityCommandRuntimeImports {
    double (*ceiling)(double);
    float (*exponential)(float);
};
EntityCommandRuntimeImports native_entity_command_runtime_imports();

// Stable contexts for43486705. Its real constructor allocates the only
// revolver/prediction rings and model-category table. No player/model/history
// buffer or registration is manufactured while composing these contexts.
class EntityCommandRuntimeOwner final {
public:
    EntityCommandRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                              ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                              LocalAnimationRuntimeOwner&, CommandCaptureRuntimeOwner&);
    EntityCommandRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                              ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                              LocalAnimationRuntimeOwner&, CommandCaptureRuntimeOwner&,
                              EntityCommandRuntimeImports);
    ~EntityCommandRuntimeOwner();
    EntityCommandRuntimeOwner(const EntityCommandRuntimeOwner&) = delete;
    EntityCommandRuntimeOwner& operator=(const EntityCommandRuntimeOwner&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    EntityCommandState& state() noexcept;
    EntityCommandOwner& command() noexcept;
    EntityCommandBindings& bindings() noexcept;
    EntityCommandFactoryBindings& factory_bindings() noexcept;
    void initialize_prediction_scratch(bool local_creation);
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
