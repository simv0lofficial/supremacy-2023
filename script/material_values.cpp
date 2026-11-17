#include "skeet/script/material_values.hpp"
#include "skeet/script/material_api.hpp"
#include "skeet/engine/interface_call.hpp"
#include <array>
#include <limits>
#include <stdexcept>

namespace skeet::script {
namespace {
constexpr std::uint32_t texture_tag = 0x5458, material_tag = 0x4d54;
std::uint8_t flags(void* variable) noexcept {
    return *(static_cast<std::uint8_t*>(variable) + 0x1c);
}
std::int32_t native_integer(std::ptrdiff_t value) noexcept {
    // Native43425031 uses CVTTSD2SI EAX; wider host Lua integers must retain its indefinite value.
    if constexpr (sizeof(std::ptrdiff_t) > sizeof(std::int32_t)) {
        if (value < std::numeric_limits<std::int32_t>::min() ||
            value > std::numeric_limits<std::int32_t>::max())
            return std::numeric_limits<std::int32_t>::min();
    }
    return static_cast<std::int32_t>(value);
}
}

std::uint32_t MaterialValues::tag(MaterialModule& module, void* state, int index) {
    const auto& api = *module.bridge_->api;
    const auto type = api.type(state, index);
    if (type != 2 && type != 7)
        return 0;
    api.pushvalue(state, index);
    module.push_handle_types(state);
    api.pushvalue(state, -2);
    api.gettable(state, -2);
    const auto result =
        api.type(state, -1) == 0 ? 0u : std::uint32_t(native_integer(api.checkinteger(state, -1)));
    api.settop(state, -4);
    return result;
}

void MaterialValues::set(MaterialModule& module, void* state, void* variable,
                         int index) { // 43400987
    const auto& api = *module.bridge_->api;
    const auto lua_type = api.type(state, index);
    if (lua_type == 0) {
        engine::invoke<void>(variable, 19);
        return;
    }
    auto type = flags(variable) & 15;
    if (type == 6) {
        if (lua_type == 1)
            type = 4;
        else if (lua_type == 2) {
            if (tag(module, state, index) == texture_tag)
                type = 3;
        } else if (lua_type == 3)
            type = api.gettop(state) >= 4 ? 2 : 0;
        else if (lua_type == 4)
            type = 1;
        else if (lua_type == 7 && tag(module, state, index) == material_tag)
            type = 8;
    }
    switch (type) {
    case 0:
        engine::invoke<void>(variable, 4, static_cast<float>(api.tonumber(state, index)));
        break;
    case 1:
        engine::invoke<void>(variable, 6, api.tolstring(state, index, nullptr));
        break;
    case 2: {
        const auto top = api.gettop(state);
        if (top < 4)
            return;
        std::array<float, 4> values{static_cast<float>(api.checknumber(state, index)),
                                    static_cast<float>(api.checknumber(state, index + 1)), 0, 0};
        int count = 2;
        if (top >= 5) {
            count = 3;
            values[2] = static_cast<float>(api.checknumber(state, index + 2));
        }
        if (top >= 6) {
            count = 4;
            values[3] = static_cast<float>(api.checknumber(state, index + 3));
        }
        engine::invoke<void>(variable, 13, values.data(), count);
        break;
    }
    case 3: {
        void* texture;
        if (api.type(state, index) == 4)
            texture = engine::invoke<void*>(module.interfaces_.material_system, 91,
                                            api.tolstring(state, index, nullptr), "", true,
                                            std::int32_t(0));
        else
            texture = module.tagged(state, index, texture_tag);
        if (texture)
            engine::invoke<void>(variable, 15, texture);
        break;
    }
    case 4:
        engine::invoke<void>(variable, 5,
                             lua_type == 1 ? api.toboolean(state, index)
                                           : native_integer(api.tointeger(state, index)));
        break;
    case 7: {
        if (api.gettop(state) < 4)
            return;
        const auto scale = static_cast<float>(api.tonumber(state, index));
        const auto x = static_cast<float>(api.tonumber(state, index + 1));
        const auto y = static_cast<float>(api.tonumber(state, index + 2));
        const std::array<float, 16> matrix{scale, 0, 0, x, 0, scale, 0, y, 0, 0, 1, 0, 0, 0, 0, 1};
        engine::invoke<void>(variable, 20, matrix.data());
        break;
    }
    case 8: {
        if (api.gettop(state) < index)
            return;
        api.checktype(state, 1, 7);
        auto* object =
            static_cast<MaterialModule::Object*>(module.tagged(state, index, material_tag));
        if (object && object->material)
            engine::invoke<void>(variable, 17, object->material);
        break;
    }
    default:
        break;
    }
}

int MaterialValues::get(MaterialModule& module, void* state, void* variable) { // 433D66C3
    const auto& api = *module.bridge_->api;
    if (!engine::invoke<bool>(variable, 18))
        return 0;
    const auto value_flags = flags(variable);
    switch (value_flags & 15) {
    case 0:
        api.pushnumber(state, double(engine::invoke<float>(variable, 28)));
        return 1;
    case 1:
        api.pushstring(state, engine::invoke<const char*>(variable, 7));
        return 1;
    case 2: {
        const auto count = (value_flags >> 4) & 7;
        if (!count || count > 4)
            return 0;
        std::array<float, 4> values{};
        engine::invoke<void>(variable, 29, values.data(), std::uint32_t(count));
        for (int i = 0; i < count; ++i)
            api.pushnumber(state, double(values[std::size_t(i)]));
        return count;
    }
    case 3:
        if (auto* texture = engine::invoke<void*>(variable, 0)) {
            api.pushlightuserdata(state, texture);
            module.tag_top(state, texture_tag);
            return 1;
        }
        return 0;
    case 4:
        api.pushinteger(state, engine::invoke<std::int32_t>(variable, 27));
        return 1;
    case 8:
        if (!engine::invoke<void*>(variable, 16))
            return 0;
        // 433D6724 passes the tag as a Lua index into4339E7A8. Preserve the
        // resulting argument error for ordinary stacks.
        {
            std::size_t length = 0;
            const auto* name = api.checklstring(state, int(material_tag), &length);
            if (!name || !length)
                api.pushnil(state);
            else {
                api.getfield(state, -10003, name);
                if (api.type(state, -1) == 0) {
                    api.settop(state, -2);
                    module.push_material(state, name, true);
                }
            }
        }
        return 1;
    default:
        return 0;
    }
}

void* MaterialValues::create(MaterialModule& module, void* material, const char* name) {
    if (!module.interfaces_.create_variable)
        throw std::logic_error("Material variable factory is not bound");
    return module.interfaces_.create_variable(material, name);
}
} // namespace skeet::script
