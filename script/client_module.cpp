#include "client_detail.hpp"
#include <array>
#include <stdexcept>

namespace skeet::script {
namespace {
void* (*closure_context)(void*, int) = nullptr;
std::ptrdiff_t (*closure_integer)(void*, int) = nullptr;
constexpr std::array names{"set_event_callback",
                           "unset_event_callback",
                           "fire_event",
                           "log",
                           "color_log",
                           "error_log",
                           "exec",
                           "userid_to_entindex",
                           "get_cvar",
                           "set_cvar",
                           "random_int",
                           "random_float",
                           "draw_debug_text",
                           "draw_hitboxes",
                           "screen_size",
                           "visible",
                           "trace_line",
                           "trace_bullet",
                           "scale_damage",
                           "latency",
                           "real_latency",
                           "camera_angles",
                           "camera_position",
                           "eye_position",
                           "timestamp",
                           "set_clan_tag",
                           "system_time",
                           "unix_time",
                           "reload_active_scripts",
                           "create_interface",
                           "find_signature",
                           "key_state",
                           "update_player_list",
                           "get_model_name",
                           "register_esp_flag",
                           "current_threat",
                           "request_full_update"};
}
int ClientModule::entry(void* state) {
    auto* owner = static_cast<ClientModule*>(closure_context(state, client_detail::first_upvalue));
    return owner->invoke(static_cast<Export>(closure_integer(state, client_detail::second_upvalue)),
                         state);
}
void ClientModule::register_exports(Bridge& bridge) {
    const auto& api = *bridge.api;
    if (!bridge.state)
        throw std::logic_error("Client module requires initialized Lua state");
    bridge_ = &bridge;
    //433B584A destroys the strings, but does not unref their Lua callbacks.
    //433FC9D0 adds that separate unref pass when the runtime is closed.
    flags.entries.clear();
    closure_context = api.touserdata;
    closure_integer = api.tointeger;
    api.createtable(bridge.state, 0, 0);
    for (std::size_t index = 0; index < names.size(); ++index) {
        api.pushlightuserdata(bridge.state, this);
        api.pushinteger(bridge.state, index);
        api.pushcclosure(bridge.state, entry, 2);
        api.setfield(bridge.state, -2, names[index]);
    }
    api.setfield(bridge.state, client_detail::globals, "client");
}
int ClientModule::invoke(Export operation, void* state) {
    switch (operation) {
    case Export::set_event_callback:
    case Export::unset_event_callback:
    case Export::fire_event:
        return callbacks(operation, state);
    case Export::log:
    case Export::color_log:
    case Export::error_log:
    case Export::exec:
    case Export::draw_debug_text:
    case Export::set_clan_tag:
        return logging(operation, state);
    case Export::visible:
    case Export::trace_line:
    case Export::trace_bullet:
    case Export::scale_damage:
        return tracing(operation, state);
    case Export::timestamp:
    case Export::system_time:
    case Export::unix_time:
    case Export::random_int:
    case Export::random_float:
    case Export::draw_hitboxes:
        return diagnostics(operation, state);
    case Export::create_interface:
    case Export::find_signature:
        return unsafe(operation, state);
    default:
        return queries(operation, state);
    }
}
} // namespace skeet::script
