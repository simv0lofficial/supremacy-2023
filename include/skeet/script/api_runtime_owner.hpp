#pragma once
#include "source_runtime_owner.hpp"
#include <memory>

namespace skeet::client {
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
class HistoryRuntimeOwner;
class VisualRuntimeOwner;
class MiscRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
class HookRuntimeOwner;
}
namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::effects {
class EffectsRuntimeOwner;
}
namespace skeet::ui {
class RendererAssets;
}
namespace skeet::script {
struct ScriptApiRuntimeImports {
    ClientRandom random;
    ClientClock clock;
    PanoramaV8Api v8;
    engine::LoadedModuleEnvironment modules;
    double (*arc_tangent2)(double, double);
};
// Resolve only genuine exports of already loaded modules and OS/CRT services.
// Clock origin/frequency sampling remains4347E307's process-initializer work.
ScriptApiRuntimeImports native_script_api_runtime_imports(const client::EngineRuntimeBindings&);

// Production inputs for all eleven ScriptOwner modules. This lifetime is
// composed before feature factories and owns no duplicate EnginePlayers,
// renderer, cache, configuration, material runtime, bridge or error log.
// ScriptOwner13 precedes ESP33 and ConsoleOutput55: their actual singleton
// cells are borrowed and resolved at the corresponding later Lua API uses.
class ScriptApiRuntimeOwner {
public:
    ScriptApiRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                          client::ConfigurationRuntimeOwner&, client::PlayerRuntimeOwner&,
                          client::HistoryRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                          animation::RuntimeOwner&, client::VisualRuntimeOwner&,
                          effects::EffectsRuntimeOwner&, engine::HookRuntimeOwner&,
                          ui::RendererAssets&, client::MiscRuntimeOwner&,
                          SourceScriptRuntimeOwner&);
    ScriptApiRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                          client::ConfigurationRuntimeOwner&, client::PlayerRuntimeOwner&,
                          client::HistoryRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                          animation::RuntimeOwner&, client::VisualRuntimeOwner&,
                          effects::EffectsRuntimeOwner&, engine::HookRuntimeOwner&,
                          ui::RendererAssets&, client::MiscRuntimeOwner&, SourceScriptRuntimeOwner&,
                          ScriptApiRuntimeImports);
    ~ScriptApiRuntimeOwner();
    ScriptApiRuntimeOwner(const ScriptApiRuntimeOwner&) = delete;
    ScriptApiRuntimeOwner& operator=(const ScriptApiRuntimeOwner&) = delete;
    ScriptApiRuntimeOwner(ScriptApiRuntimeOwner&&) = delete;
    ScriptApiRuntimeOwner& operator=(ScriptApiRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    ClientBindings& client() noexcept;
    EntityBindings& entity() noexcept;
    const GlobalsBindings& globals() const noexcept;
    PanoramaServices& panorama() noexcept;
    MaterialInterfaces& materials() noexcept;
    const RendererBindings& renderer() const noexcept;
    misc::NormalLog& normal_log() noexcept;
    misc::ErrorLog& error_log() noexcept;
    client::ClientHitboxRuntime& hitboxes() noexcept;
    SourceScriptModuleInputs module_inputs(ConfigStorageOwner&, ui::MainMenuWindowOwner&);
    // Validate and supply the real factory/module context only. It creates no
    // ScriptOwner, Lua state, OS handler, feature or renderer resource.
    void bind_modules(ConfigStorageOwner&, ui::MainMenuWindowOwner&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}
