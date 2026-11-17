#pragma once
#include "skeet/anti_aim/command_owner.hpp"
#include <memory>

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class MovementHudRuntimeOwner;
class CommandCaptureRuntimeOwner;
class TargetRefreshRuntimeOwner;
class VisualRuntimeOwner;
class NativeStartupSources;
struct EngineRuntimeBindings;
struct AntiAimRuntimeFactoryBindings;
}
namespace skeet::anti_aim {
struct RuntimeImports {
    using Seed = void(__cdecl*)(std::int32_t);
    using Integer = std::int32_t(__cdecl*)(std::int32_t, std::int32_t);
    using Range = float(__cdecl*)(float, float);
    Seed seed;
    Integer integer;
    Range range;
    const RageTrigonometry& trigonometry;
    double (*atan2)(double, double);
    RageBoxClipper clipper;
};
RuntimeImports native_runtime_imports(client::ClientRuntimeStorage&,
                                      const client::EngineRuntimeBindings&);

// Process-lifetime composition for43485CCC and the three real command phases.
// No player, model, command or frame is sampled by construction. All players
// are supplied at dispatch; live BoneRuntime transactions produce body-yaw
// hypotheses and the matrices consumed by the actual model-effects owner.
// Construction registers no owner and executes no engine or command method.
class RuntimeOwner final {
public:
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                 animation::RuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::HistoryRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                 animation::RuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::HistoryRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&, RuntimeImports);
    ~RuntimeOwner();
    RuntimeOwner(const RuntimeOwner&) = delete;
    RuntimeOwner& operator=(const RuntimeOwner&) = delete;
    RuntimeOwner(RuntimeOwner&&) = delete;
    RuntimeOwner& operator=(RuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    CommandOwner& command() noexcept;
    AngleStageState& state() noexcept;
    PrecalculationState& precalculation() noexcept;
    FinisherState& finisher() noexcept;
    client::AntiAimRuntimeFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::anti_aim
