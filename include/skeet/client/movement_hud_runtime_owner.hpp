#pragma once
#include "movement_hud_factories.hpp"
#include "skeet/movement/extrapolation.hpp"
#include "skeet/movement/release_command.hpp"
#include <memory>

namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class CommandCaptureRuntimeOwner;
class HistoryRuntimeOwner;
class VisualRuntimeOwner;
struct EngineRuntimeBindings;
struct CommandBudgetBindings;

// Explicit borrowed source views for standalone authored fixtures. Production
// obtains these from the canonical owners in the first constructor below.
// No entity, player, command, camera or clock snapshot is stored here.
struct StandaloneMovementHudViews {
    script::Bridge& scripts;
    const CommandBudgetBindings& budget;
    const rage::TargetOffsets& target_offsets;
    const esp::Projection& projection;
    const esp::IndicatorLayout& indicators;
    const Point& screen_size;
};
struct MovementHudRuntimeImports {
    const RageTrigonometry& trigonometry;
    lagcomp::RotationMath rotation;
    float (*exponential)(float);
    float (*tangent)(float);
};
MovementHudRuntimeImports native_movement_hud_runtime_imports();

// Stable assembly for434856D6/4348DA54/4349095E/43481DE4. Construction does
// not allocate/register their owners or publish434759B0. Their real factories
// initialize the same state/settings later in the startup schedule.
class MovementHudRuntimeOwner {
public:
    MovementHudRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                            ConfigurationRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                            VisualRuntimeOwner&, script::SourceScriptRuntimeOwner&,
                            CommandCaptureRuntimeOwner&, HistoryRuntimeOwner&);
    MovementHudRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                            ConfigurationRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                            StandaloneMovementHudViews, MovementHudRuntimeImports);
    ~MovementHudRuntimeOwner();
    MovementHudRuntimeOwner(const MovementHudRuntimeOwner&) = delete;
    MovementHudRuntimeOwner& operator=(const MovementHudRuntimeOwner&) = delete;
    MovementHudRuntimeOwner(MovementHudRuntimeOwner&&) = delete;
    MovementHudRuntimeOwner& operator=(MovementHudRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    const MovementEnvironmentBindings& movement_environment() const noexcept;
    const MovementCaptureInput& movement_capture() const noexcept;
    const MovementExtrapolationLiveBindings& live_movement() const noexcept;
    MovementCommandPhaseOwner& movement_command() noexcept;
    MovementReleaseRuntime& release_movement() noexcept;
    MovementHudFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
