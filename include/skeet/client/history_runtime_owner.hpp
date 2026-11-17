#pragma once
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/lagcomp/native_history_boundaries.hpp"
#include <memory>

namespace skeet {
struct ShotFeedbackState;
}
namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
struct NativeOffsetGlobals;
}
namespace skeet::lagcomp {
struct HistoryRecordingBindings;
struct HistoryRecordingScratch;
}
namespace skeet::rage {
struct TargetOffsets;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
struct HistoryRuntimeFactoryBindings;
class SourceTerminalCleanupBindings;
class NativeStartupSources;
// Genuine external engine/CRT boundary inputs; the explicit overload permits
// authored standalone interface/ABI spies without replacing recovered bodies.
struct HistoryRuntimeImports {
    void* thread_pool;
    lagcomp::HistoryNumericBindings numeric;
};
HistoryRuntimeImports native_history_runtime_imports();

lagcomp::EntityNetvars native_history_entity_fields(const engine::NativeOffsetGlobals&);
// Stable complete history/prediction factory contexts, composed after genuine
// engine discovery. Production recording owns both native automatic scratch
// directions per invocation; explicit-input overloads serve differential fixtures.
// No owner is registered and no engine function is called by construction.
// All borrowed process owners must remain alive through workers and exit calls.
class HistoryRuntimeOwner {
public:
    HistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                        engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                        script::SourceScriptRuntimeOwner&);
    HistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                        engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                        script::SourceScriptRuntimeOwner&, HistoryRuntimeImports);
    HistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                        engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                        script::SourceScriptRuntimeOwner&, lagcomp::HistoryRecordingScratch&);
    HistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                        engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                        script::SourceScriptRuntimeOwner&, lagcomp::HistoryRecordingScratch&,
                        HistoryRuntimeImports);
    ~HistoryRuntimeOwner();
    HistoryRuntimeOwner(const HistoryRuntimeOwner&) = delete;
    HistoryRuntimeOwner& operator=(const HistoryRuntimeOwner&) = delete;
    HistoryRuntimeOwner(HistoryRuntimeOwner&&) = delete;
    HistoryRuntimeOwner& operator=(HistoryRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    PlayerRuntimeOwner& players() const noexcept;
    const lagcomp::EntityStateRuntime& entity_runtime() const noexcept;
    const rage::TargetOffsets& target_offsets() const noexcept;
    engine::NativeConVarReference game_type() const noexcept;
    engine::NativeConVarReference teammates_are_enemies() const noexcept;
    ShotFeedbackState& feedback_state() noexcept;
    const lagcomp::HistoryRecordingBindings& recording_bindings() const noexcept;
    HistoryRuntimeFactoryBindings& factory_bindings() noexcept;
    SourceTerminalCleanupBindings& source_terminal_cleanup() noexcept;
    // Bind only the two actual factories. Startup order and source terminal
    // profile selection remain responsibilities of NativeStartupSourceGraph.
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
