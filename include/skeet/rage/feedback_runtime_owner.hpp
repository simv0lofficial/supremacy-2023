#pragma once
#include "feedback_owner.hpp"
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
namespace skeet::script {
class ScriptApiRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CrtRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::rage {
struct PendingShotPublicationBindings;
// One production feedback context over HistoryRuntimeOwner's actual pending
// shot/resolver state. Engine adapters read local-player/entity/model/clock
// cells at each method invocation; construction captures no player or frame.
class FeedbackRuntimeOwner final {
public:
    FeedbackRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                         client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                         engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                         client::CrtRuntimeOwner&, esp::EspRuntimeOwner&,
                         script::ScriptApiRuntimeOwner&);
    ~FeedbackRuntimeOwner();
    FeedbackRuntimeOwner(const FeedbackRuntimeOwner&) = delete;
    FeedbackRuntimeOwner& operator=(const FeedbackRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    ShotFeedbackRuntime& runtime() noexcept;
    FeedbackOwnerBindings& factory_bindings() noexcept;
    const PendingShotPublicationBindings& pending_shots() const noexcept;
    void bind_initializer(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::rage
