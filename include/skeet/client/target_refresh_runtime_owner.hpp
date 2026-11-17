#pragma once
#include <cstdint>
#include <memory>

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::anti_aim {
struct TargetState;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class PlayerRuntimeOwner;
class HistoryRuntimeOwner;
class MovementHudRuntimeOwner;
struct EngineRuntimeBindings;
struct Command;

//4335BA8E's shared production selector. All phase inputs are supplied at the
// actual call; canonical history, resource, clock and ConVar cells remain live.
// Construction calls no selector, engine method, owner factory or registration.
class TargetRefreshRuntimeOwner final {
public:
    TargetRefreshRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                              PlayerRuntimeOwner&, HistoryRuntimeOwner&,
                              engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                              MovementHudRuntimeOwner&);
    ~TargetRefreshRuntimeOwner();
    TargetRefreshRuntimeOwner(const TargetRefreshRuntimeOwner&) = delete;
    TargetRefreshRuntimeOwner& operator=(const TargetRefreshRuntimeOwner&) = delete;
    TargetRefreshRuntimeOwner(TargetRefreshRuntimeOwner&&) = delete;
    TargetRefreshRuntimeOwner& operator=(TargetRefreshRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    void refresh(anti_aim::TargetState&, void* current_player, const Command&,
                 std::uint32_t incoming_buttons, float incoming_yaw) const;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
