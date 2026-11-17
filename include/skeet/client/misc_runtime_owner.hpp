#pragma once
#include "misc_runtime_factories.hpp"
#include <memory>
#include <variant>

namespace skeet::engine {
class SceneQueryRuntimeOwner;
class HookDescriptorStorage;
}
namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::rage {
class RuntimeOwner;
}
namespace skeet::misc {
class Runtime;
struct AutoAcceptPatchState;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
struct EngineRuntimeBindings;

// Explicit supplied frame views for authored standalone boundary fixtures.
// Production uses the actual RAGE owner and never retains these frame views.
struct StandaloneMiscKnifeViews {
    const rage::TargetBindings& targeting;
    const rage::RecordCandidateBindings& candidates;
    const lagcomp::RecordSelectionContext& records;
    const lagcomp::ExtrapolatedRecordBindings& extrapolation;
};
struct MiscKnifeFrameSource {
    MiscKnifeFrameSource(rage::RuntimeOwner& owner) noexcept : source(&owner) {}
    MiscKnifeFrameSource(StandaloneMiscKnifeViews views) noexcept : source(views) {}
    std::variant<rage::RuntimeOwner*, StandaloneMiscKnifeViews> source;
};
// Shared physical contexts already used by combat/history, movement and HUD.
// RAGE supplies invocation-local knife views only after native entry gates.
struct MiscRuntimeSharedBindings {
    script::Bridge& scripts;
    engine::SceneQueryRuntimeOwner& scene;
    animation::RuntimeOwner& animation;
    MiscKnifeFrameSource knife;
    const MovementExtrapolationLiveBindings& live_movement;
    const CommandBudgetBindings& budget;
    const esp::Projection& projection;
    ui::RendererBackend& renderer;
    const esp::GrenadeWarningTexture& explosive_texture;
    const esp::GrenadeWarningTexture& fire_texture;
    const effects::ProjectileRibbonBindings& ribbons;
    const esp::IndicatorLayout& indicators;
    MovementReleaseBinding release_movement;
    MovementFeedbackBinding movement_feedback;
};

struct MiscRuntimeImports {
    misc::AutomaticWeaponsRandomInteger random_integer;
    misc::LowFpsClock performance_clock;
    esp::GrenadeDamageMath damage_math;
    double (*arc_tangent2)(double, double);
};
MiscRuntimeImports native_misc_runtime_imports(const EngineRuntimeBindings&);

// One source owner for the nine misc factory contexts, shared runtime/channel,
// feature state and concrete engine bindings. Construction registers nothing;
// bind_initializers only supplies the existing native factory bodies. Their
// real startup order still allocates/registers the nine complete owners.
// Storage, engine/configuration/descriptor and shared-context owners must
// outlive this object, and this object must outlive the lifecycle owners.
class MiscRuntimeOwner {
public:
    MiscRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                     ConfigurationRuntimeOwner&, engine::HookDescriptorStorage&,
                     MiscRuntimeSharedBindings);
    MiscRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                     ConfigurationRuntimeOwner&, engine::HookDescriptorStorage&,
                     MiscRuntimeSharedBindings, MiscRuntimeImports);
    ~MiscRuntimeOwner();
    MiscRuntimeOwner(const MiscRuntimeOwner&) = delete;
    MiscRuntimeOwner& operator=(const MiscRuntimeOwner&) = delete;
    MiscRuntimeOwner(MiscRuntimeOwner&&) = delete;
    MiscRuntimeOwner& operator=(MiscRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    misc::Runtime& runtime() noexcept;
    ChannelHookState& channel() noexcept;
    misc::AutoAcceptPatchState& auto_accept_patch() noexcept;
    misc::QuickSwitchRuntime& quick_switch() noexcept;
    misc::LowFpsRuntime* low_fps() const noexcept;
    MiscRuntimeFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    static ClientRuntimeStorage& require_available(ClientRuntimeStorage&);
    struct Data;
    std::unique_ptr<Data> data_;
};
} // namespace skeet::client
