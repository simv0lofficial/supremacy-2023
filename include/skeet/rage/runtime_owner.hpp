#pragma once
#include "owner.hpp"
#include "skeet/lagcomp/history_selection.hpp"
#include <functional>
#include <memory>

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::effects {
class EffectsRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
class CrtRuntimeOwner;
class MovementHudRuntimeOwner;
class TargetRefreshRuntimeOwner;
class VisualRuntimeOwner;
class NativeStartupSources;
}
namespace skeet::rage {
struct ShotDiagnosticEntry;
struct BufferedShotRepairScratch;
struct RuntimeImports {
    void* thread_pool;
};
// Layout for native per-invocation automatic storage. The explicit-input
// constructor is a differential-fixture seam, not a bootstrap dependency.
struct CommandFrameInputs {
    lagcomp::RecordInterpolationScratch record_interpolation;
    lagcomp::HistoryMatrixScratch history_interpolation;
    std::array<std::byte, animation::copied_animation_state_size> alternative_animation;
    std::array<Matrix3x4, 256> attachment_matrices;
};
// All four views borrow invocation-local bindings. They are valid only while
// the synchronous with_frame consumer is executing and cannot be retained.
struct RuntimeFrameBindings {
    const TargetBindings& targeting;
    const RecordCandidateBindings& candidates;
    const lagcomp::RecordSelectionContext& records;
    const lagcomp::ExtrapolatedRecordBindings& extrapolation;
};
// The complete4348DA25 source factory and four RAGE command phases. Settings,
// acquisition and firing publications share their existing canonical owners.
// Construction registers nothing, calls no game method and retains no frame,
// command, target or local-player snapshot.
class RuntimeOwner final {
public:
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&, effects::EffectsRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&, CommandFrameInputs&);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&, effects::EffectsRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&);
    // The same source contexts with an explicit external thread-pool import.
    // Production still owns each phase's automatic workspace at invocation.
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::MovementHudRuntimeOwner&,
                 client::TargetRefreshRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&, effects::EffectsRuntimeOwner&, client::VisualRuntimeOwner&,
                 script::SourceScriptRuntimeOwner&, RuntimeImports);
    ~RuntimeOwner();
    RuntimeOwner(const RuntimeOwner&) = delete;
    RuntimeOwner& operator=(const RuntimeOwner&) = delete;
    RuntimeOwner(RuntimeOwner&&) = delete;
    RuntimeOwner& operator=(RuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    PreambleState& state() noexcept;
    AcquisitionStorage& acquisition() noexcept;
    CommandEndpoint& commands() noexcept;
    OwnerBindings& factory_bindings() noexcept;
    void with_frame(client::CommandFrame&, void* current_player, client::Command&,
                    const std::function<void(const RuntimeFrameBindings&)>&);
    const lagcomp::EvaluationState* repair_buffered(client::CommandFrame&, void* current_player,
                                                    client::Command&, const ShotDiagnosticEntry&,
                                                    BufferedShotRepairScratch&);
    void bind_initializer(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::rage
