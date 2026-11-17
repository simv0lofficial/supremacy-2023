#pragma once
#include "skeet/core.hpp"
#include "skeet/animation/local_prediction_pose_inputs.hpp"
#include <memory>

namespace skeet::animation {
class RuntimeOwner;
struct LocalPredictionPoseBindings;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class LocalAnimationRuntimeOwner;
class HistoryRuntimeOwner;
class MovementHudRuntimeOwner;
class CommandCaptureRuntimeOwner;
class PredictionHistoryStorage;
struct PredictionHistoryOwner;
struct PredictionHistoryInitialization;
struct EngineRuntimeBindings;
class NativeStartupSources;
// Stable source assembly for4349081E. Construction borrows the real local
// restricted-command, movement, history, animation and capture owners, without
// reading a player, command-ring pointer or scratch byte. Only the factory
// allocates/registers the canonical history arrays and22sequence strings.
class PredictionHistoryRuntimeOwner final {
public:
    PredictionHistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                                  ConfigurationRuntimeOwner&, LocalAnimationRuntimeOwner&,
                                  HistoryRuntimeOwner&, MovementHudRuntimeOwner&,
                                  animation::RuntimeOwner&, CommandCaptureRuntimeOwner&);
    ~PredictionHistoryRuntimeOwner();
    PredictionHistoryRuntimeOwner(const PredictionHistoryRuntimeOwner&) = delete;
    PredictionHistoryRuntimeOwner& operator=(const PredictionHistoryRuntimeOwner&) = delete;
    PredictionHistoryRuntimeOwner(PredictionHistoryRuntimeOwner&&) = delete;
    PredictionHistoryRuntimeOwner& operator=(PredictionHistoryRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    PredictionHistoryStorage& history_storage() noexcept;
    PredictionHistoryOwner& command() noexcept;
    const animation::LocalPredictionPoseBindings& pose() const noexcept;
    PredictionHistoryInitialization& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
