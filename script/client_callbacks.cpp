#include "client_detail.hpp"
#include "skeet/events/event_name_storage.hpp"
#include <algorithm>
#include <stdexcept>

namespace skeet::script {
using namespace client_detail;
bool ClientModule::builtin_event(std::uint32_t hash) const noexcept {
    constexpr std::array<std::uint32_t, 28> names{
        0xf672a4c2, 0x7f6774cf, 0xe4b4bf56, 0xb2c6ac0e, 0x41aed39d, 0xbdc072f9, 0x19d5469d,
        0x944c2915, 0x6bf0a8ef, 0x8b6d6f7d, 0x6aea37ba, 0x78df5f0d, 0x55edb6b2, 0x895f03c7,
        0xd642104c, 0xf3d4301e, 0x8e21b836, 0xf98186b9, 0xec600d46, 0x30084612, 0x7d671b3a,
        0xeb6312f5, 0x9667dcaf, 0x3d7677eb, 0x887cf1ea, 0x983409fe, 0x51d0b047, 0x8370609c};
    return std::find(names.begin(), names.end(), hash) != names.end();
}
void ClientModule::subscribe(const char* name) { //433E2344
    const auto hash = raw_name_hash(name) ^ 0x92ca5df1u;
    if (hash == 0xd642104c) {
        if (bindings.console_input)
            bindings.console_input->enable();
        return;
    }
    if (builtin_event(hash))
        return;
    if (std::find(subscribed_events_.begin(), subscribed_events_.end(), hash) !=
        subscribed_events_.end())
        return;
    subscribed_events_.push_back(hash);
    if (engine::invoke<bool>(bindings.engine, 26)) {
        if (!bindings.subscriptions_dirty)
            throw std::logic_error("Event subscription requires engine dirty flag");
        *bindings.subscriptions_dirty = 1;
    }
}
int ClientModule::callbacks(Export operation, void* state) {
    auto& owner = bridge();
    const auto& api = *owner.api;
    if (operation == Export::fire_event) { //4339EB0E
        const auto count = api.gettop(state);
        if (count < 1)
            return 0;
        const auto* name = api.checklstring(state, 1, nullptr);
        if (!name)
            return 0;
        if (!builtin_event(callback_hash(name)))
            dispatch_custom_event(owner, state, name, count >= 2);
        return 0;
    }
    if (owner.callback_table_ref == -1)
        return 0;
    if (api.gettop(state) < 2)
        return argument_count_error(api, state);
    const auto* name = api.checklstring(state, 1, nullptr);
    if (!name || !*name)
        return api.argerror(state, 1, owner.name(events::EventName::invalid_event_name).get());
    api.checktype(state, 2, 6);
    const auto hash = raw_name_hash(name, true);
    const bool removing = operation == Export::unset_event_callback;
    if (hash == 0x04ad815e || hash == 0x79a94f04) {
        const auto kind =
            hash == 0x04ad815e ? ExclusiveCallback::indicator : ExclusiveCallback::output;
        if (removing)
            unregister_exclusive(owner, state, kind);
        else
            register_exclusive(owner, state, kind, 2);
        return 0;
    }
    if (!removing && hash == 0xf8206a4b) { //shutdown routes through the runtime's defer helper
        api.getfield(state, globals, "defer");
        api.pushvalue(state, 2);
        api.call(state, 1, 0);
        return 0;
    }
    api.rawgeti(state, registry, owner.callback_table_ref);
    api.pushvalue(state, 1);
    api.gettable(state, -2);
    if (api.type(state, -1) == 0) {
        if (!removing) {
            api.pushvalue(state, 1);
            api.createtable(state, 1, 0);
            api.pushvalue(state, 2);
            api.rawseti(state, -2, 1);
            api.settable(state, -4);
            subscribe(name);
        }
    } else {
        const auto table = api.gettop(state);
        const auto index = find_callback(api, state, table, 2);
        if (removing) {
            if (index) {
                const auto event_hash = callback_hash(name);
                const auto active = owner.callback_active.find(event_hash);
                if (active != owner.callback_active.end() && active->second) {
                    // Preserve indices until the current dispatch finishes.
                    owner.callback_dirty[event_hash] = true;
                    api.pushboolean(state, 0);
                    api.rawseti(state, table, index);
                } else {
                    remove_callback(api, state, table, index);
                }
            }
        } else if (index == 0) {
            const auto count = api.objlen(state, -1);
            api.pushvalue(state, 2);
            api.rawseti(state, -2, static_cast<int>(count) + 1);
        }
    }
    api.settop(state, -3);
    return 0;
}
} // namespace skeet::script
