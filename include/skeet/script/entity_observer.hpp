#pragma once
#include "skeet/script/bridge.hpp"
#include "skeet/client/entity_observer_registry.hpp"

namespace skeet::script {

// Script-owner interface434729C0, appended at constructor4347FF7A to the
// actual43475BA8 registry. Native slots0/2 are433CCE8E; slots1/3 below borrow
// the same Lua Bridge as the owner's command, prediction and view interfaces.
class ScriptEntityObserver final {
public:
    ScriptEntityObserver(Bridge&, client::EntityObserverRegistry&);
    ScriptEntityObserver(const ScriptEntityObserver&) = delete;
    ScriptEntityObserver& operator=(const ScriptEntityObserver&) = delete;
    //43480004 publishes after all whole-owner interfaces were registered.
    void publish_table(client::EntityObserverRegistry&) noexcept;

    //433998EB ignores both incoming engine arguments and dispatches the
    // parameterless net_update_start event.
    int update_entities(void* player, std::uintptr_t incoming);
    //4336A3FD likewise ignores player/delta and dispatches net_update_end.
    int incoming_complete(void* saved_player, std::int32_t acknowledged_delta);

private:
    client::NativeEntityObserverReceiver receiver_;
    Bridge& bridge_;
};

} // namespace skeet::script
