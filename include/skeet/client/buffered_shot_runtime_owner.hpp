#pragma once
#include "buffered_shot_lifecycle.hpp"
#include <memory>

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::rage {
class RuntimeOwner;
class FeedbackRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
class CrtRuntimeOwner;
struct EngineRuntimeBindings;
//43485BE3's one canonical diagnostic/frame store. Each phase6 invocation
// composes timing, input rings, movement-button policy and actual RAGE repair
// from its live arguments. Construction captures no player or command.
class BufferedShotRuntimeOwner final {
public:
    BufferedShotRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                             ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                             CommandCaptureRuntimeOwner&, animation::RuntimeOwner&,
                             CrtRuntimeOwner&, esp::EspRuntimeOwner&, rage::RuntimeOwner&,
                             rage::FeedbackRuntimeOwner&);
    ~BufferedShotRuntimeOwner();
    BufferedShotRuntimeOwner(const BufferedShotRuntimeOwner&) = delete;
    BufferedShotRuntimeOwner& operator=(const BufferedShotRuntimeOwner&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    BufferedShotCommandOwner& command() noexcept;
    BufferedShotCommandStorage& state() noexcept;
    BufferedShotFactoryBindings& factory_bindings() noexcept;
    void bind_initializer(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
