#include "panorama_detail.hpp"
#include <algorithm>

namespace skeet::script {
using namespace panorama_detail;

int PanoramaModule::call(void* state) {
    const auto& lua = *bridge_->api;
    const auto& v8 = services_.v8;
    if (!services_.begin_engine_call || !services_.end_engine_call)
        return 0;
    check_userdata(state, 1, object_tag);
    lua.pushvalue(state, 1);
    lua.gettable(state, upvalue(1));
    if (!lua.type(state, -1)) {
        lua.settop(state, -2);
        return 0;
    }
    const auto function_index = static_cast<std::uint32_t>(lua.checkinteger(state, -1));
    lua.settop(state, -2);
    lua.pushvalue(state, 1);
    lua.gettable(state, upvalue(2));
    if (!lua.type(state, -1)) {
        lua.settop(state, -2);
        return 0;
    }
    lua.gettable(state, upvalue(1));
    if (!lua.type(state, -1)) {
        lua.settop(state, -2);
        return 0;
    }
    const auto receiver_index = static_cast<std::uint32_t>(lua.checkinteger(state, -1));
    lua.settop(state, -2);
    if (function_index >= browser_.object_handles.size() ||
        receiver_index >= browser_.object_handles.size())
        return 0;
    lua.pushvalue(state, 1);
    lua.gettable(state, upvalue(3));
    void* panel = lua.touserdata(state, -1);
    lua.settop(state, -2);
    auto* engine = ui_engine();
    if (!engine)
        return 0;
    auto* isolate = engine_call<void*>(engine, 129);
    if (!isolate)
        return 0;
    bool failed = false;
    int results = 0;
    {
        Scope scope(v8, isolate);
        auto* persistent = panel_context(engine, panel);
        if (!persistent)
            return 0;
        ContextScope context(v8, isolate, persistent);
        const auto function = local(v8, isolate, browser_.object_handles[function_index]);
        if (!function.handle)
            return 0;
        const bool direct = v8.is_function(function.handle);
        V8Local object{};
        if (!direct) {
            if (!v8.is_object(function.handle))
                return 0;
            object = v8.to_object(function.handle, context.context);
            if (!object.handle || !v8.is_callable(object.handle))
                return 0;
        }
        const auto receiver = local(v8, isolate, browser_.object_handles[receiver_index]);
        if (!receiver.handle)
            return 0;
        services_.begin_engine_call(panel);
        const auto limit = (std::min)(lua.gettop(state), 16);
        std::array<V8Local, 15> arguments{};
        for (int argument = 2; argument <= limit; ++argument)
            arguments[static_cast<std::size_t>(argument - 2)] =
                to_v8(state, isolate, context.context, argument);
        const auto count = (std::max)(limit - 1, 0);
        auto* values = count ? arguments.data() : nullptr;
        const auto result =
            direct ? v8.function_call(function.handle, context.context, receiver, count, values)
                   : v8.object_call(object.handle, context.context, receiver, count, values);
        services_.end_engine_call();
        if (v8.has_caught(scope.caught.data())) {
            failed = scope.push_error(lua, state, nullptr, true, true);
        } else if (result.handle) {
            push_v8(state, isolate, context.context, result, panel);
            results = 1;
        }
    }
    return failed ? raise_pending(lua, state) : results;
}

int PanoramaModule::tostring(void* state) {
    const auto& lua = *bridge_->api;
    const auto& v8 = services_.v8;
    if (lua.gettop(state) != 1)
        return 0;
    check_userdata(state, 1, object_tag);
    lua.pushvalue(state, 1);
    lua.gettable(state, upvalue(1));
    const auto index =
        lua.type(state, -1) ? static_cast<std::uint32_t>(lua.checkinteger(state, -1)) : 0xffffffffu;
    lua.settop(state, -2);
    if (index >= browser_.object_handles.size())
        return 0;
    auto* engine = ui_engine();
    if (!engine)
        return 0;
    auto* isolate = engine_call<void*>(engine, 129);
    if (!isolate)
        return 0;
    bool failed = false;
    int results = 0;
    {
        Scope scope(v8, isolate);
        auto* persistent = engine_call<void***>(engine, 123);
        ContextScope context(v8, isolate, persistent);
        const auto global = v8.context_global(context.context.handle);
        if (!browser_.owns_handle) {
            if (!global.handle)
                return 0;
            const auto json_key = v8.string_new(isolate, "JSON", 0, -1);
            const auto json = v8.object_get(global.handle, context.context, json_key);
            if (!json.handle || !v8.is_object(json.handle))
                return 0;
            const auto stringify_key = v8.string_new(isolate, "stringify", 0, -1);
            const auto stringify = v8.object_get(json.handle, context.context, stringify_key);
            if (!stringify.handle || !v8.is_function(stringify.handle))
                return 0;
            browser_.global_handle = v8.globalize_reference(isolate, stringify.handle);
            browser_.owns_handle = true;
        }
        if (!browser_.global_handle)
            return 0;
        const auto stringify = local(v8, isolate, browser_.global_handle);
        if (!stringify.handle)
            return 0;
        auto value = local(v8, isolate, browser_.object_handles[index]);
        if (!value.handle)
            return 0;
        const auto result = v8.function_call(stringify.handle, context.context, value, 1, &value);
        if (heap_string(result)) {
            const auto string = v8.to_string(result.handle, context.context);
            if (string.handle) {
                std::array<std::uintptr_t, 2> text{};
                v8.utf8_construct(text.data(), string);
                lua.pushlstring(state, reinterpret_cast<const char*>(text[0]),
                                static_cast<std::uint32_t>(text[1]));
                v8.utf8_destroy(text.data());
                results = 1;
            }
        }
        if (!results)
            failed = scope.push_error(lua, state);
    }
    return failed ? raise_pending(lua, state) : results;
}

} // namespace skeet::script
