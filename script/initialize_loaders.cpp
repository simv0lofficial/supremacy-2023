#include "skeet/script/initialize.hpp"
#include "initialize_detail.hpp"
#include "detail.hpp"
#include <cstring>
#include <stdexcept>

namespace skeet::script {
void ScriptRuntime::install_loaders() { // 433B6C6E
    if (required_modules.empty())
        return;
    const auto& lua = *bridge.api;
    auto* state = bridge.state;
    lua.getfield(state, -10002, "table");
    lua.getfield(state, -1, "insert");
    lua.remove(state, -2);
    lua.getfield(state, -10002, "package");
    lua.getfield(state, -1, "loaders");
    lua.remove(state, -2);
    lua.pushinteger(state, 1);
    lua.pushcclosure(state, loader, 0);
    if (lua.pcall(state, 3, 0, 0))
        detail::report_lua_error(bridge);
}
int ScriptRuntime::loader(void* state) { // 43331646
    auto* runtime = initialize_detail::owner(state);
    if (!runtime)
        throw std::logic_error("Lua loader has no native script owner");
    auto& bridge = runtime->bridge;
    const auto& lua = *bridge.api;
    if (bridge.runtime_ref == -1 || runtime->required_modules.empty() || lua.type(state, 1) != 4)
        return 0;
    std::size_t length = 0;
    const auto* requested = lua.tolstring(state, 1, &length);
    constexpr std::size_t prefix_length = 10;
    if (!requested || length <= prefix_length)
        return 0;
    const std::string_view prefix(requested, prefix_length);
    if ((_strnicmp(prefix.data(), "supremacy", prefix_length - 1) != 0 &&
         _strnicmp(prefix.data(), "gamesense", prefix_length - 1) != 0) ||
        (prefix.back() != '/' && prefix.back() != '.'))
        return 0;
    for (auto& module : runtime->required_modules) {
        // Native774FB200 is msvcrt!_stricmp; suffix case is ignored too.
        if (module.name.size() != length - prefix_length ||
            _stricmp(module.name.c_str(), requested + prefix_length))
            continue;
        if (!module.encrypted)
            throw std::logic_error("Required module source is not encrypted in native storage");
        lua.rawgeti(state, -10000, bridge.runtime_ref);
        lua.rawgeti(state, -1, 5);
        lua.remove(state, -2);
        lua.pushvalue(state, 1);
        runtime->cipher.transform(module.source);
        module.encrypted = false;
        lua.pushlstring(state, reinterpret_cast<const char*>(module.source.data()),
                        module.source.size());
        runtime->cipher.transform(module.source);
        module.encrypted = true;
        const bool was_loading = runtime->loading_builtin;
        runtime->loading_builtin = true;
        const auto status = lua.pcall(state, 2, 1, 0);
        runtime->loading_builtin = was_loading;
        if (status)
            return lua.error(state);
        return 1;
    }
    return 0;
}
} // namespace skeet::script
