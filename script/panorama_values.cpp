#include "panorama_detail.hpp"
#include <cmath>
#include <limits>

namespace skeet::script {
using namespace panorama_detail;
namespace {
std::int32_t truncate_sse(double value) {
    if (value >= -2147483648.0 && value < 2147483648.0)
        return static_cast<std::int32_t>(value);
    return (std::numeric_limits<std::int32_t>::min)();
}
}

V8Local PanoramaModule::to_v8(void* state, void* isolate, V8Local context, int index) {
    const auto& lua = *bridge_->api;
    const auto& v8 = services_.v8;
    auto root = [&](int offset) {
        return V8Local{
            reinterpret_cast<void**>(static_cast<std::byte*>(isolate) + offset * sizeof(void*))};
    };
    switch (lua.type(state, index)) {
    case 1:
        return root(lua.toboolean(state, index) ? 19 : 20);
    case 3:
        return v8.number_new(isolate, lua.tonumber(state, index));
    case 4:
        return v8.string_new(isolate, lua.tolstring(state, index, nullptr), 0, -1);
    case 5: {
        const int table = index < 0 ? lua.gettop(state) + index + 1 : index;
        const auto length = lua.objlen(state, table);
        bool array = true;
        lua.pushnil(state);
        while (lua.next(state, table)) {
            if (lua.type(state, -2) != 3 ||
                truncate_sse(lua.tonumber(state, -2)) > static_cast<std::int32_t>(length)) {
                lua.settop(state, -3);
                array = false;
                break;
            }
            lua.settop(state, -2);
        }
        if (array) {
            const auto result = v8.array_new(isolate, static_cast<int>(length));
            for (std::size_t item = 1; item <= length; ++item) {
                lua.rawgeti(state, table, static_cast<int>(item));
                const auto value = to_v8(state, isolate, context, -1);
                (void)v8.array_set(result.handle, context, static_cast<std::uint32_t>(item - 1),
                                   value);
                lua.settop(state, -2);
            }
            return result;
        }
        const auto result = v8.object_new(isolate);
        lua.pushnil(state);
        while (lua.next(state, table)) {
            lua.pushvalue(state, -2);
            if (const auto* name = lua.tolstring(state, -1, nullptr)) {
                const auto key = v8.string_new(isolate, name, 0, -1);
                if (key.handle && !restricted_object(key)) {
                    const auto value = to_v8(state, isolate, context, -2);
                    (void)v8.object_set(result.handle, key, value);
                }
            }
            lua.settop(state, -3);
        }
        return result;
    }
    case 7:
        if (userdata_tag(state, index) == object_tag) {
            lua.pushvalue(state, index);
            lua.gettable(state, upvalue(1));
            if (lua.type(state, -1)) {
                const auto slot = static_cast<std::uint32_t>(lua.tointeger(state, -1));
                lua.settop(state, -2);
                if (slot < browser_.object_handles.size())
                    return local(v8, isolate, browser_.object_handles[slot]);
            } else
                lua.settop(state, -2);
        }
        break;
    default:
        break;
    }
    return root(18); // native isolate root: undefined
}

void PanoramaModule::push_v8(void* state, void* isolate, V8Local context, V8Local value,
                             void* panel, int parent) {
    const auto& lua = *bridge_->api;
    const auto& v8 = services_.v8;
    if (!value.handle || restricted_object(value)) {
        lua.pushnil(state);
        return;
    }
    if (v8.is_array(value.handle) || v8.is_function(value.handle) || v8.is_object(value.handle)) {
        push_proxy(state, isolate, value, panel, parent);
    } else if (v8.is_boolean(value.handle)) {
        lua.pushboolean(state, v8.boolean_value(value.handle));
    } else if (v8.is_int32(value.handle)) {
        const auto integer = v8.to_int32(value.handle, context);
        if (integer.handle)
            lua.pushinteger(state, v8.int32_value(integer.handle));
        else
            lua.pushnil(state);
    } else if (v8.is_uint32(value.handle)) {
        const auto integer = v8.to_uint32(value.handle, context);
        if (integer.handle)
            lua.pushinteger(state, v8.uint32_value(integer.handle));
        else
            lua.pushnil(state);
    } else if (v8.is_number(value.handle)) {
        const auto number = v8.to_number(value.handle, context);
        if (number.handle)
            lua.pushnumber(state, v8.number_value(number.handle));
        else
            lua.pushnil(state);
    } else if (heap_string(value)) {
        const auto string = v8.to_string(value.handle, context);
        if (string.handle) {
            std::array<std::uintptr_t, 2> text{};
            v8.utf8_construct(text.data(), string);
            lua.pushstring(state, reinterpret_cast<const char*>(text[0]));
            v8.utf8_destroy(text.data());
        } else
            lua.pushnil(state);
    } else
        lua.pushnil(state);
}

int PanoramaModule::index(void* state) {
    const auto& lua = *bridge_->api;
    const auto& v8 = services_.v8;
    check_userdata(state, 1, object_tag);
    lua.pushvalue(state, 1);
    lua.gettable(state, upvalue(1));
    const auto index = static_cast<std::uint32_t>(lua.checkinteger(state, -1));
    lua.settop(state, -2);
    if (index >= browser_.object_handles.size()) {
        lua.pushnil(state);
        return 1;
    }
    lua.pushvalue(state, 1);
    lua.gettable(state, upvalue(3));
    void* panel = lua.touserdata(state, -1);
    lua.settop(state, -2);
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
    Scope scope(v8, isolate);
    auto* persistent = panel_context(engine, panel);
    if (!persistent) {
        lua.pushnil(state);
        return 1;
    }
    ContextScope context(v8, isolate, persistent);
    const auto object = local(v8, isolate, browser_.object_handles[index]);
    if (!object.handle) {
        lua.pushnil(state);
        return 1;
    }
    if (v8.is_object(object.handle)) {
        if (!lua.isstring(state, 2)) {
            lua.pushnil(state);
            return 1;
        }
        const auto* name = lua.tolstring(state, 2, nullptr);
        if (!name) {
            lua.pushnil(state);
            return 1;
        }
        const auto key = v8.string_new(isolate, name, 0, -1);
        const auto value = v8.object_get(object.handle, context.context, key);
        push_v8(state, isolate, context.context, value, panel, 1);
    } else if (v8.is_array(object.handle) && lua.type(state, 2) == 3) {
        const auto offset = static_cast<std::uint32_t>(lua.tointeger(state, 2));
        if (static_cast<std::int32_t>(offset) >= 0 && offset < v8.array_length(object.handle)) {
            const auto value = v8.array_get(object.handle, context.context, offset);
            push_v8(state, isolate, context.context, value, panel);
        } else
            lua.pushnil(state);
    } else
        lua.pushnil(state);
    return 1;
}

} // namespace skeet::script
