#include "panorama_detail.hpp"

namespace skeet::script {
using namespace panorama_detail;

int PanoramaModule::open(void* state) {
    const auto& lua = *bridge_->api;
    const auto& v8 = services_.v8;
    if (!unsafe_enabled())
        return lua.lerror(state, "Blocked unsafe script");
    void* panel = nullptr;
    if (lua.gettop(state) >= 1) {
        if (const auto* name = lua.checklstring(state, 1, nullptr))
            panel = find_panel(name);
    }
    if (!panel)
        panel = default_panel();
    if (!panel)
        return lua.lerror(state, "no active panel");
    auto* engine = ui_engine();
    if (!engine)
        return 0;
    auto* isolate = engine_call<void*>(engine, 129);
    if (!isolate)
        return 0;
    auto* current = engine_call<void***>(engine, 123);
    if (!*current)
        return 0;
    int index = -1;
    v8.isolate_enter(isolate);
    std::array<std::uintptr_t, 3> handles{};
    v8.handle_scope_construct(handles.data(), isolate);
    const auto context = local(v8, isolate, *current);
    v8.context_enter(context.handle);
    if (auto* persistent = panel_context(engine, panel)) {
        const auto panel_local = local(v8, isolate, *persistent);
        const auto global = v8.context_global(panel_local.handle);
        index = take_handle(isolate, global, false);
    }
    v8.context_exit(context.handle);
    v8.handle_scope_destroy(handles.data());
    v8.isolate_exit(isolate);
    if (index == -1)
        return 0;
    push_userdata(state, object_tag);
    lua.pushvalue(state, upvalue(4));
    lua.setmetatable(state, -2);
    lua.pushvalue(state, -1);
    lua.pushlightuserdata(state, panel);
    lua.settable(state, upvalue(3));
    lua.pushvalue(state, -1);
    lua.pushinteger(state, index);
    lua.settable(state, upvalue(1));
    return 1;
}

int PanoramaModule::loadstring(void* state) {
    const auto& lua = *bridge_->api;
    const auto& v8 = services_.v8;
    if (!unsafe_enabled())
        return lua.lerror(state, "Blocked unsafe script");
    const auto arguments = lua.gettop(state);
    if (arguments < 1)
        return lua.lerror(state, "invalid number of arguments");
    const auto* source = lua.checklstring(state, 1, nullptr);
    if (!source || !*source)
        return lua.argerror(state, 1, "empty string");
    void* panel;
    if (arguments >= 2) {
        const auto* name = lua.checklstring(state, 2, nullptr);
        panel = name ? find_panel(name) : default_panel();
        if (name && !panel)
            return lua.argerror(state, 2, "panel not found");
    } else
        panel = default_panel();
    if (!panel)
        return lua.lerror(state, "no active panel");
    auto* engine = ui_engine();
    if (!engine)
        return 0;
    auto* isolate = engine_call<void*>(engine, 129);
    if (!isolate)
        return 0;
    bool failed = false;
    {
        Scope scope(v8, isolate);
        auto* persistent = panel_context(engine, panel);
        if (!persistent) {
            lua.pushnil(state);
            return 1;
        }
        ContextScope context(v8, isolate, persistent);
        const auto string = v8.string_maybe_new(isolate, source, 0, -1);
        if (string.handle) {
            // The native Source initializer is seven pointer-width fields.
            std::array<std::uintptr_t, 7> compilation{};
            compilation[0] = reinterpret_cast<std::uintptr_t>(string.handle);
            const auto script = v8.compile_unbound_script(isolate, compilation.data(), 0);
            if (script.handle) {
                const auto index = take_handle(isolate, script, true);
                push_userdata(state, script_tag);
                lua.pushvalue(state, upvalue(5));
                lua.setmetatable(state, -2);
                lua.pushvalue(state, -1);
                lua.pushinteger(state, index);
                lua.settable(state, upvalue(1));
                lua.pushvalue(state, -1);
                lua.pushlightuserdata(state, panel);
                lua.settable(state, upvalue(3));
            } else {
                failed = true;
                if (!scope.push_error(lua, state, "compilation failed", false)) {
                    lua.where(state, 1);
                    lua.pushstring(state, "?");
                }
            }
            if (compilation[6]) {
                auto* cached = reinterpret_cast<void*>(compilation[6]);
                v8.cached_data_destroy(cached);
                auto* allocation = static_cast<std::byte*>(cached) - 0x10;
                const auto displacement = *reinterpret_cast<const std::uint32_t*>(allocation + 8);
                (void)v8.heap_free(v8.process_heap, 0, allocation - displacement);
            }
        }
    }
    return failed ? raise_pending(lua, state) : 1;
}

int PanoramaModule::run_script(void* state) {
    const auto& lua = *bridge_->api;
    const auto& v8 = services_.v8;
    if (lua.gettop(state) < 1)
        return 0;
    check_userdata(state, 1, script_tag);
    lua.pushvalue(state, 1);
    lua.gettable(state, upvalue(1));
    if (lua.type(state, -1) != 3) {
        lua.settop(state, -2);
        return 0;
    }
    const auto index = static_cast<std::uint32_t>(lua.tointeger(state, -1));
    lua.settop(state, -2);
    if (index >= browser_.script_handles.size())
        return 0;
    auto* engine = ui_engine();
    if (!engine) {
        lua.pushnil(state);
        return 1;
    }
    auto* isolate = engine_call<void*>(engine, 129);
    if (!isolate) {
        lua.pushnil(state);
        return 1;
    }
    lua.pushvalue(state, 1);
    lua.gettable(state, upvalue(3));
    void* panel = lua.touserdata(state, -1);
    lua.settop(state, -2);
    bool failed = false;
    int results = 0;
    {
        Scope scope(v8, isolate);
        auto* persistent = panel_context(engine, panel);
        if (!persistent) {
            lua.pushnil(state);
            return 1;
        }
        ContextScope context(v8, isolate, persistent);
        const auto unbound = local(v8, isolate, browser_.script_handles[index]);
        if (!services_.begin_engine_call || !services_.end_engine_call)
            throw std::logic_error("Panorama requires its engine VM call boundaries");
        services_.begin_engine_call(panel);
        V8Local result{};
        if (unbound.handle) {
            const auto script = v8.bind_to_current_context(unbound.handle);
            if (script.handle)
                result = v8.script_run(script.handle, context.context);
        }
        services_.end_engine_call();
        if (result.handle) {
            push_v8(state, isolate, context.context, result, panel);
            results = 1;
        } else if ((failed = scope.push_error(lua, state, "execution failed", false))) {
            // Raise only after the Context, TryCatch, HandleScope and isolate
            // have followed their native destruction order.
        }
    }
    return failed ? raise_pending(lua, state) : results;
}

} // namespace skeet::script
