#include "skeet/script/command_owner.hpp"

namespace skeet::script {
namespace {
struct CommandEvent {
    const client::CommandFrame& frame;
    const client::Command& command;
};

void integer_field(Bridge& bridge, events::EventNameRef name, std::int32_t value) { //4339C6D0
    const auto& api = *bridge.api;
    api.pushlstring(bridge.state, name.get(), 14);
    api.pushinteger(bridge.state, value);
    api.settable(bridge.state, -3);
}

int marshal_command_event(Bridge& bridge, void* context) { //43331EB0
    const auto& event = *static_cast<const CommandEvent*>(context);
    bridge.api->createtable(bridge.state, 0, 2);
    integer_field(bridge, bridge.name(events::EventName::chokedcommands), event.frame.choked_15c);
    integer_field(bridge, bridge.name(events::EventName::command_number), event.command.number);
    return 1;
}

int dispatch_command_event(Bridge& bridge, events::EventNameRef event_name,
                           const client::CommandFrame& frame, const client::Command& command) {
    if (!bridge.state)
        return 0;
    CommandEvent event{frame, command};
    return dispatch_callbacks(bridge, event_name.get(), marshal_command_event, &event);
}
}

ScriptCommandOwner::ScriptCommandOwner(Bridge& bridge, ScriptCommandOwnerBindings bindings,
                                       client::CommandPhaseRegistry& actual_order)
    : bridge_(bridge), bindings_(bindings) {
    actual_order.append(receiver_, this); //4347FF6A, complete owner+8
}

void ScriptCommandOwner::publish_table(client::CommandPhaseRegistry& actual_order) noexcept {
    actual_order.publish(receiver_, client::CommandOwnerId::owner_43470834); //4347FFF6
}

int ScriptCommandOwner::phase2(client::Command& command, std::array<float, 3>& incoming,
                               std::uint8_t& send_packet) {
    if (!bridge_.state || bridge_.runtime_ref == -1)
        return 0;
    const client::ScriptCommandBindings live{&bridge_,
                                             bindings_.snapshot,
                                             "setup_command",
                                             &bindings_.choked_commands,
                                             &bindings_.marshalling,
                                             bridge_.api->pushlightuserdata,
                                             bindings_.environment};
    return client::dispatch_command_script(live, command, incoming[1], send_packet,
                                           bindings_.scheduler_flags);
}

int ScriptCommandOwner::phase4(const client::CommandFrame& frame, const client::Command& command) {
    return dispatch_command_event(bridge_, bridge_.name(events::EventName::run_command), frame,
                                  command);
}

int ScriptCommandOwner::phase6(const client::CommandFrame& frame, const client::Command& command) {
    return dispatch_command_event(bridge_, bridge_.name(events::EventName::finish_command), frame,
                                  command);
}

} // namespace skeet::script
