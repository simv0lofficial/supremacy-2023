#include "skeet/script/client_api.hpp"
#include "skeet/events/event_name_storage.hpp"
#include "detail.hpp"

namespace skeet::script {
namespace {
int marshal_console_input(Bridge& bridge, void* context) { //433B2646
    bridge.api->pushstring(bridge.state, *static_cast<const char* const*>(context));
    return 1;
}
}
bool dispatch_boolean_callbacks(Bridge& bridge, const char* name, Marshaller marshal,
                                void* context) {
    const std::lock_guard lock(bridge.vm_mutex);
    if (!bridge.state)
        return false;
    const auto hash = callback_hash(name);
    auto& active = bridge.callback_active[hash];
    if (active)
        return false;
    const auto& api = *bridge.api;
    auto* state = bridge.state;
    detail::CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = api.gettop(state);
    detail::RtlPushFrame(&guard.active);
    bool accepted = false;
    active = true;
    auto dispatch = [&] {
        api.lcheckstack(state, 12, "dispatching console callbacks");
        if (!push_callback_list(bridge, state, name))
            return;
        const auto count = static_cast<std::int32_t>(api.objlen(state, -1));
        if (count > 0) {
            const auto arguments = marshal(bridge, context);
            if (arguments != -1) {
                const auto list = api.gettop(state) - arguments;
                for (std::int32_t index = 1; index <= count; ++index) {
                    api.rawgeti(state, list, index);
                    if (api.type(state, -1) == 6) {
                        for (int argument = 1; argument <= arguments; ++argument)
                            api.pushvalue(state, list + argument);
                        if (detail::callback_pcall(state, arguments, 1) != 0) {
                            detail::report_callback_error(bridge, state);
                            continue;
                        }
                        //433309E4 accumulates explicit boolean true and still
                        // invokes all later callbacks from the captured array.
                        if (api.type(state, -1) == 1 && api.toboolean(state, -1))
                            accepted = true;
                    }
                    api.settop(state, -2);
                }
                if (arguments)
                    api.settop(state, list);
            }
            const auto dirty = bridge.callback_dirty.find(hash);
            if (dirty != bridge.callback_dirty.end() && dirty->second) {
                dirty->second = false;
                compact_callback_list(api, state, -1);
            }
        }
        api.settop(state, -3);
    };
    if (!detail::protected_region(dispatch, guard))
        accepted = false;
    api.settop(state, guard.saved_top);
    active = false;
    detail::RtlPopFrame(&guard.active);
    return accepted;
}
bool dispatch_console_input(Bridge& bridge, const char* text) {
    return dispatch_boolean_callbacks(bridge, bridge.name(events::EventName::console_input).get(),
                                      marshal_console_input, &text);
}
} // namespace skeet::script
