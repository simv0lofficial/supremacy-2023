#pragma once
#include "command_orchestrator.hpp"
#include <memory>

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class SharedEspRuntimeOwner;
class LocalAnimationRuntimeOwner;
struct EngineRuntimeBindings;

// Process ownership for the complete recovered CreateMove framing and its
// accepted-command transaction. Early construction composes prediction,
// packet-angle, mouse and packet-transition contexts from canonical owners;
// it invokes no engine method, feature factory or command callback.
//
// The three late contexts are mandatory. complete() retains their real source
// owners and rejects mismatched canonical cells before publishing a runtime.
// Hook installation and factory registration remain separate startup phases.
// Every borrowed owner must outlive this object and all installed hooks.
class CommandRuntimeOwner final {
public:
    CommandRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                        CommandCaptureRuntimeOwner&, animation::RuntimeOwner&);
    ~CommandRuntimeOwner();
    CommandRuntimeOwner(const CommandRuntimeOwner&) = delete;
    CommandRuntimeOwner& operator=(const CommandRuntimeOwner&) = delete;
    CommandRuntimeOwner(CommandRuntimeOwner&&) = delete;
    CommandRuntimeOwner& operator=(CommandRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    CommandCaptureRuntimeOwner& capture() const noexcept;
    HistoryRuntimeOwner& history() const noexcept;
    const prediction::CommandBindings& prediction() const noexcept;
    const prediction::CommandPlayerOffsets& prediction_player_offsets() const noexcept;
    const CommandPacketAngleBindings& packet_angles() const noexcept;
    const CommandWeaponPostprocessBindings& postprocess() const noexcept;
    const CommandPacketTransitionBindings& packet_transition() const noexcept;
    const CommandTrigonometry& trigonometry() const noexcept;

    void complete(engine::HookRuntimeOwner&, LocalAnimationRuntimeOwner&,
                  TargetRefreshRuntimeOwner&, SharedEspRuntimeOwner&);
    bool complete() const noexcept;
    // Both operations fail explicitly while any required late context is
    // absent. bind_callback() publishes the concrete source callback context;
    // it installs no engine hook and executes no command.
    CommandCreateMoveRuntime& runtime() const;
    void bind_callback();

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
