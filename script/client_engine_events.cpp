#include "client_detail.hpp"
#include "detail.hpp"
#include "skeet/events/engine_event.hpp"

namespace skeet::script {
namespace {
// Engine GameEvent::visit at slot19 uses seven consecutive visitor methods.
// There is no destructor slot in native43470434.
class LuaEventVisitor final : public events::EngineEventVisitor {
public:
    LuaEventVisitor(const LuaApi& api, void* state) : api(api), state(state) {}
    bool pointer(const char* key, void* value) override {
        api.pushstring(state, key);
        api.pushlightuserdata(state, value);
        return set();
    }
    bool string(const char* key, const char* value) override {
        api.pushstring(state, key);
        api.pushstring(state, value);
        return set();
    }
    bool floating(const char* key, float value) override {
        api.pushstring(state, key);
        api.pushnumber(state, double(value));
        return set();
    }
    bool integer(const char* key, std::int32_t value) override {
        api.pushstring(state, key);
        api.pushinteger(state, value);
        return set();
    }
    bool wide_integer(const char* key, std::uint64_t value) override {
        api.pushstring(state, key);
        api.pushinteger(state, std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(value)));
        return set();
    }
    bool opaque_word(const char*, std::uint32_t) override {
        return true;
    }
    bool boolean(const char* key, bool value) override {
        api.pushstring(state, key);
        api.pushboolean(state, value);
        return set();
    }

private:
    const LuaApi& api;
    void* state;
    bool set() {
        api.settable(state, -3);
        return true;
    }
};
int marshal_game_event(Bridge& bridge, void* event) { //434022CF
    const auto& api = *bridge.api;
    api.createtable(bridge.state, 0, 3);
    LuaEventVisitor visitor(api, bridge.state);
    engine::invoke<void, events::EngineEventVisitor*>(event, 19, &visitor);
    return 1;
}
}
void ClientModule::dispatch_engine_event(void* event) { //433D71C7
    auto& owner = bridge();
    const std::lock_guard lock(owner.vm_mutex);
    if (!owner.state)
        return;
    const auto* name = engine::invoke<const char*>(event, 1);
    detail::CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = owner.api->gettop(owner.state);
    detail::RtlPushFrame(&guard.active);
    auto dispatch = [&] { dispatch_callbacks(owner, name, marshal_game_event, event); };
    detail::protected_region(dispatch, guard);
    owner.api->settop(owner.state, guard.saved_top);
    detail::RtlPopFrame(&guard.active);
}
} // namespace skeet::script
