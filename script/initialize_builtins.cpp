#include "skeet/script/initialize.hpp"
#include "detail.hpp"
#include <stdexcept>

namespace skeet::script {
int ScriptRuntime::run_builtin(BuiltinModule& module) { // 4336EF9E
    if (bridge.runtime_ref == -1)
        return 1;
    if (module.encrypted)
        throw std::logic_error("Builtin source is encrypted at the Lua call boundary");
    const auto& lua = *bridge.api;
    int status = 1;
    detail::CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    detail::RtlPushFrame(&guard.active);
    auto invoke = [&] {
        lua.rawgeti(bridge.state, -10000, bridge.runtime_ref);
        lua.rawgeti(bridge.state, -1, 4);
        lua.remove(bridge.state, -2);
        lua.pushstring(bridge.state, module.name.c_str());
        lua.pushlstring(bridge.state, reinterpret_cast<const char*>(module.source.data()),
                        module.source.size());
        status = lua.pcall(bridge.state, 2, 0, 0);
        if (status)
            detail::report_lua_error(bridge);
    };
    const auto completed = detail::protected_region(invoke, guard);
    detail::RtlPopFrame(&guard.active);
    if (!completed)
        detail::report_native_exception(bridge, guard, module.name.c_str());
    return status;
}
void ScriptRuntime::run_builtins() { // 433824BE
    for (auto& module : builtins) {
        if (!module.enabled)
            continue;
        if (!module.encrypted)
            throw std::logic_error("Builtin source must be encrypted in native storage");
        loading_builtin = true;
        cipher.transform(module.source);
        module.encrypted = false;
        if (run_builtin(module))
            module.enabled = false;
        loading_builtin = false;
        cipher.transform(module.source);
        module.encrypted = true;
    }
}
} // namespace skeet::script
