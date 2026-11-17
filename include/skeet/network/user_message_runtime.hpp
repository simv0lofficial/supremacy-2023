#pragma once
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
}
namespace skeet::engine {
class NativeDataImage;
}
namespace skeet::players {
struct State;
}
namespace skeet::script {
struct Bridge;
}

namespace skeet::network {
// Owns all contexts borrowed by the four SDK message entrypoints.
class UserMessageRuntime {
public:
    UserMessageRuntime(client::ClientRuntimeStorage&, const players::State&, script::Bridge&);
    ~UserMessageRuntime();
    UserMessageRuntime(const UserMessageRuntime&) = delete;
    UserMessageRuntime& operator=(const UserMessageRuntime&) = delete;
    void bind();
    bool bound_to(const engine::NativeDataImage&) const noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
bool user_message_contexts_ready(const engine::NativeDataImage&) noexcept;
}
