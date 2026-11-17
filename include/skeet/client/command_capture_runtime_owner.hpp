#pragma once
#include "command_orchestrator.hpp"
#include <memory>

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
struct EngineRuntimeBindings;

// Concrete canonical field production for the existing CreateMove capture
// bodies. Construct after engine/offset publication and the shared player and
// history owners. This owns no duplicate EntityStateRuntime or player cache.
// It performs no registration, engine callback, prediction or command phase.
// All borrowed owners and their engine module objects must outlive this owner.
class CommandCaptureRuntimeOwner final {
public:
    CommandCaptureRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                               ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                               animation::RuntimeOwner&, const lagcomp::EntityStateRuntime&);
    ~CommandCaptureRuntimeOwner();
    CommandCaptureRuntimeOwner(const CommandCaptureRuntimeOwner&) = delete;
    CommandCaptureRuntimeOwner& operator=(const CommandCaptureRuntimeOwner&) = delete;
    CommandCaptureRuntimeOwner(CommandCaptureRuntimeOwner&&) = delete;
    CommandCaptureRuntimeOwner& operator=(CommandCaptureRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    const CommandMasterGlobals& globals() const noexcept;
    const CommandPlayerGates& player_gates() const noexcept;
    const CommandMouseOffsets& mouse_offsets() const noexcept;
    const CommandRecaptureOffsets& physical_offsets() const noexcept;
    const CommandCaptureOffsets& capture_offsets() const noexcept;
    const weapon::FireStateBindings& firing() const noexcept;
    const CommandEntityBindings& entities() const noexcept;
    const ScriptCommandEnvironment& angle_environment() const noexcept;
    const CommandAngleBindings& view_angles() const noexcept;
    const CommandPacketCatchupBindings& packet_catchup() const noexcept;
    const CommandBudgetBindings& budget() const noexcept;
    const CommandTargetValidation& target_validation() const noexcept;
    const CommandSentTimingBindings& sent_timing_bindings() const noexcept;
    const CommandEyeAdjustmentSourceContext& eye_adjustment() const noexcept;
    engine::NativeConVarReference copy_gate() const noexcept;
    std::vector<Matrix3x4>& bone_storage() noexcept;
    SentCommandHistoryView& sent_history() noexcept;

    // Call at the master's existing native capture position, after its fresh
    // SentCommandTiming local has been formed. The returned views borrow the
    // actual player, physical context and timing; none is retained as a fake
    // initial frame. The unchanged capture/recapture bodies consume them.
    CommandRecaptureContext recapture(void* current_player) const;
    CommandCaptureContext capture(const CommandRecaptureContext&, const SentCommandTiming&) const;
    CommandCaptureContext capture(CommandRecaptureContext&&,
                                  const SentCommandTiming&) const = delete;
    CommandCaptureContext capture(const CommandRecaptureContext&&,
                                  const SentCommandTiming&) const = delete;
    CommandCaptureContext capture(const CommandRecaptureContext&,
                                  SentCommandTiming&&) const = delete;
    CommandCaptureContext capture(const CommandRecaptureContext&,
                                  const SentCommandTiming&&) const = delete;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
