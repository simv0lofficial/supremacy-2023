#include "skeet/script/initialize.hpp"
#include "initialize_detail.hpp"
#include "detail.hpp"
#include <stdexcept>

namespace skeet::script {
int ScriptRuntime::panic(void* state) { // 43359BD4
    if (auto* runtime = initialize_detail::owner(state))
        detail::report_lua_error(runtime->bridge, state);
    for (auto* frame = detail::RtlGetFrame(); frame; frame = frame->previous) {
        auto hash = 0x811c9dc5u;
        for (unsigned shift = 0; shift != 32; shift += 8)
            hash = (hash ^ ((frame->flags >> shift) & 255)) * 0x01000193u;
        if ((hash ^ 0x92ca5df1u) == 0xe18c2fa4u)
            detail::resume_panic_frame(frame);
    }
    return 0;
}
int ScriptRuntime::bootstrap(void* state) { // 433EC568
    auto* runtime = initialize_detail::owner(state);
    if (!runtime)
        throw std::logic_error("Lua state has no native script owner");
    runtime->register_modules();
    return 0;
}
void* ScriptRuntime::ensure_state() { // 433C05F9
    const std::lock_guard lock(bridge.vm_mutex);
    if (bridge.state) {
        initialize_detail::bind(*this);
        if (applied_mode != mode) {
            if (!api.setmode)
                throw std::logic_error("LuaJIT mode API is not bound");
            applied_mode = mode;
            api.setmode(bridge.state, 0, mode);
        }
        return bridge.state;
    }
    if (!enabled)
        return nullptr;
    unavailable_ffi_addresses.clear();
    if (!bridge.api || !api.newstate || !api.atpanic || !api.setmode)
        throw std::logic_error("Native Lua initialization API is not bound");
    bridge.state = api.newstate();
    if (!bridge.state)
        return nullptr;
    initialize_detail::bind(*this);
    applied_mode = 0x100;
    if (mode != 0x100) {
        applied_mode = mode;
        api.setmode(bridge.state, 0, mode);
    }
    builtin_context_ref = -1;
    bridge.runtime_ref = -1;
    api.atpanic(bridge.state, panic);
    bridge.api->gc(bridge.state, 0, 0);
    initializing = true;
    detail::CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    detail::RtlPushFrame(&guard.active);
    auto invoke = [&] {
        if (bridge.api->cpcall(bridge.state, bootstrap, nullptr) != 0)
            detail::report_lua_error(bridge);
    };
    const auto completed = detail::protected_region(invoke, guard);
    detail::RtlPopFrame(&guard.active);
    if (!completed)
        detail::report_native_exception(bridge, guard, "initializing Lua");
    initializing = false;
    bridge.api->gc(bridge.state, 1, -1);
    bridge.api->gc(bridge.state, 2, 0);
    return bridge.state;
}
} // namespace skeet::script
