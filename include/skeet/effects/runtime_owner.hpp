#pragma once
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class VisualRuntimeOwner;
class PlayerRuntimeOwner;
class CrtRuntimeOwner;
class HistoryRuntimeOwner;
class NativeStartupSources;
struct EffectsRuntimeFactoryBindings;
struct EngineRuntimeBindings;
}
namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
struct BuildMaterialBindings;
struct BuildEspBindings;
class SceneQueryRuntimeOwner;
}
namespace skeet::rage {
class ShotModelBoneStorage;
}
namespace skeet::effects {
class ModelRenderRuntime;
class WorldRuntime;
class ViewRuntime;
class CameraRuntimeOwner;
struct SmokeOwnerBindings;
struct ScopeOwnerBindings;
struct EffectsCommandOwner;
struct ModelShotState;
class ModelRenderHooks;
class ModelRenderLifecycle;
class WorldCallbacks;
// Stable material/world/view lifetime shared by Lua, World factories and the
// later full effects owner. Phase one binds genuine acquired engine cells and
// canonical settings only; no resource, feature or hook is initialized.
class EffectsRuntimeOwner {
public:
    EffectsRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                        client::ConfigurationRuntimeOwner&, client::VisualRuntimeOwner&,
                        const engine::BuildMaterialBindings&);
    // Explicit genuine engine boundary for independent source composition and
    // authored ABI fixtures. The production overload above obtains this exact
    // singleton from its already verified VisualRuntimeOwner.
    EffectsRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                        client::ConfigurationRuntimeOwner&, const engine::BuildMaterialBindings&,
                        void* key_values_system);
    ~EffectsRuntimeOwner();
    EffectsRuntimeOwner(const EffectsRuntimeOwner&) = delete;
    EffectsRuntimeOwner& operator=(const EffectsRuntimeOwner&) = delete;
    EffectsRuntimeOwner(EffectsRuntimeOwner&&) = delete;
    EffectsRuntimeOwner& operator=(EffectsRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    ModelRenderRuntime& models() noexcept;
    WorldRuntime& world() noexcept;
    ViewRuntime& view() noexcept;
    SmokeOwnerBindings& smoke_bindings() noexcept;
    ScopeOwnerBindings& scope_bindings() noexcept;
    ModelShotState& shot_state() noexcept;
    rage::ShotModelBoneStorage& shot_bones() noexcept;
    // Phase two borrows the real camera, player cache, SceneQueries ballistics,
    // CRT spread/RNG/math and the one genuine shotgun producer. It creates no
    // primary owner and does not publish43475A1C or install a hook.
    void compose_command(CameraRuntimeOwner&, client::PlayerRuntimeOwner&,
                         engine::SceneQueryRuntimeOwner&, client::CrtRuntimeOwner&,
                         const engine::BuildEspBindings&);
    EffectsCommandOwner& command();
    // The primary phase joins the same command, history, bone and hook-table
    // owners. Factory registration/publication retains its startup position.
    void compose_primary(CameraRuntimeOwner&, client::HistoryRuntimeOwner&,
                         animation::RuntimeOwner&, client::CrtRuntimeOwner&);
    client::EffectsRuntimeFactoryBindings& factory_bindings();
    ModelRenderHooks& model_hooks();
    ModelRenderLifecycle& lifecycle();
    WorldCallbacks& updates();
    void bind_initializers(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::effects
