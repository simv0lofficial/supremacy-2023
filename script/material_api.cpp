#include "skeet/script/material_api.hpp"
#include "skeet/effects/model_render.hpp"
#include "skeet/engine/interface_call.hpp"
#include <array>
#include <cstring>
#include <limits>

namespace skeet::script {
using engine::invoke;
namespace {
constexpr int registry = -10000;
constexpr int cache_upvalue = -10003, metatable_upvalue = -10004;
constexpr std::uint32_t material_tag = 0x4D54, texture_tag = 0x5458;
std::int32_t native_integer(std::ptrdiff_t value) noexcept {
    // The embedded luaL_checkinteger uses CVTTSD2SI EAX (0x434249FA).
    if (value < std::numeric_limits<std::int32_t>::min() ||
        value > std::numeric_limits<std::int32_t>::max())
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}
}
void* (*MaterialModule::context_)(void*, int) = nullptr;
MaterialModule& MaterialModule::owner(void* state, int upvalue) {
    return *static_cast<MaterialModule*>(context_(state, upvalue));
}
bool MaterialModule::ready() const { // 0x43403660
    return invoke<bool>(interfaces_.engine_client, 26) && *interfaces_.activity != 1;
}
void MaterialModule::push_handle_types(void* state) { // 0x433880A2
    const auto& api = *bridge_->api;
    if (handle_types_ref_ == -1) {
        api.createtable(state, 0, 0);
        api.createtable(state, 0, 2);
        api.pushlstring(state, "k", 1);
        api.setfield(state, -2, "__mode");
        api.pushboolean(state, false);
        api.setfield(state, -2, "__metatable");
        api.setmetatable(state, -2);
        handle_types_ref_ = api.ref(state, registry);
    }
    api.rawgeti(state, registry, handle_types_ref_);
}
void MaterialModule::tag_top(void* state, std::uint32_t tag) {
    const auto& api = *bridge_->api;
    push_handle_types(state);
    api.pushvalue(state, -2);
    api.pushinteger(state, tag);
    api.settable(state, -3);
    api.settop(state, -2);
}
void* MaterialModule::tagged(void* state, int index, std::uint32_t tag) { // 0x4338A612
    const auto& api = *bridge_->api;
    const auto type = api.type(state, index);
    if (type != 2 && type != 7)
        return nullptr;
    api.pushvalue(state, index);
    push_handle_types(state);
    api.pushvalue(state, -2);
    api.gettable(state, -2);
    void* result = nullptr;
    if (api.type(state, -1) != 0 &&
        std::uint32_t(native_integer(api.checkinteger(state, -1))) == tag)
        result = api.touserdata(state, -3);
    api.settop(state, -4);
    return result;
}
void MaterialModule::push_object(void* state, void* material, const char* name) { // 0x433CB5B4
    const auto& api = *bridge_->api;
    auto* object = static_cast<Object*>(api.newuserdata(state, sizeof(Object)));
    tag_top(state, material_tag);
    if (!object) {
        api.pushnil(state);
        return;
    }
    object->material = material;
    object->loaded = object->changed = false;
    invoke<void>(material, 12);
    api.pushvalue(state, metatable_upvalue);
    api.setmetatable(state, -2);
    api.pushstring(state, name);
    api.pushvalue(state, -2);
    api.settable(state, cache_upvalue);
}
void* MaterialModule::search(const char* name, bool force) { // 0x4339FD67
    auto* system = interfaces_.material_system;
    const auto lock = invoke<std::uint32_t>(system, 118);
    auto handle = invoke<std::uint16_t>(system, 86);
    void* found = nullptr;
    while (handle != invoke<std::uint16_t>(system, 88)) {
        auto* material = invoke<void*>(system, 89, handle);
        if (material && !invoke<bool>(material, 42) && invoke<int>(material, 56) > 0 &&
            invoke<bool>(material, 71)) {
            const auto* current = invoke<const char*>(material, 0);
            if (current && std::strcmp(current, name) == 0) {
                found = material;
                break;
            }
        }
        handle = invoke<std::uint16_t>(system, 87, handle);
    }
    invoke<void>(system, 119, lock);
    if (!found && force) {
        found = invoke<void*>(system, 84, name, "", false, static_cast<const char*>(nullptr));
        if (found && invoke<bool>(found, 42))
            found = nullptr;
    }
    return found;
}
void MaterialModule::push_material(void* state, const char* name, bool force) {
    if (auto* material = search(name, force))
        push_object(state, material, name);
    else
        bridge_->api->pushnil(state);
}
void MaterialModule::push_named_material(void* state, void* material) { // 0x43341199
    const auto& api = *bridge_->api;
    const auto* name = material ? invoke<const char*>(material, 0) : nullptr;
    if (!name) {
        api.pushnil(state);
        return;
    }
    api.getfield(state, cache_upvalue, name);
    if (api.type(state, -1) == 0) {
        api.settop(state, -2);
        push_material(state, name, false);
    }
}
int MaterialModule::find_material(void* state) {
    const auto& api = *bridge_->api;
    if (!ready())
        return 0;
    const auto count = api.gettop(state);
    if (count < 1)
        return 0;
    const auto force = count >= 2 && api.toboolean(state, 2) != 0;
    std::size_t length = 0;
    const auto* name = api.checklstring(state, 1, &length);
    if (!name || !length) {
        api.pushnil(state);
        return 1;
    }
    api.getfield(state, cache_upvalue, name);
    if (api.type(state, -1) == 0) {
        api.settop(state, -2);
        push_material(state, name, force);
    }
    return 1;
}
int MaterialModule::find_materials(void* state) {
    const auto& api = *bridge_->api;
    if (!ready())
        return 0;
    api.createtable(state, 0, 0);
    if (api.gettop(state) < 1)
        return 1;
    std::size_t length = 0;
    const auto* requested = api.checklstring(state, 1, &length);
    if (!requested || !length)
        return 1;
    auto* system = interfaces_.material_system;
    const auto lock = invoke<std::uint32_t>(system, 118);
    auto handle = invoke<std::uint16_t>(system, 86);
    int count = 0;
    while (handle != invoke<std::uint16_t>(system, 88)) {
        auto* material = invoke<void*>(system, 89, handle);
        if (material && !invoke<bool>(material, 42)) {
            const auto* name = invoke<const char*>(material, 0);
            if (name && std::strstr(name, requested)) {
                push_named_material(state, material);
                if (api.type(state, -1) == 0)
                    api.settop(state, -2);
                else {
                    api.rawseti(state, -2, ++count);
                    if (count >= 2048)
                        break;
                }
            }
        }
        handle = invoke<std::uint16_t>(system, 87, handle);
    }
    invoke<void>(system, 119, lock);
    return 1;
}
int MaterialModule::find_texture(void* state) {
    const auto& api = *bridge_->api;
    if (!ready() || api.gettop(state) < 1)
        return 0;
    std::size_t length = 0;
    const auto* name = api.checklstring(state, 1, &length);
    if (!name || !length)
        return 0;
    auto* texture = invoke<void*>(interfaces_.material_system, 91, name, "", true, std::int32_t(0));
    if (!texture)
        return 0;
    api.pushlightuserdata(state, texture);
    tag_top(state, texture_tag);
    return 1;
}
int MaterialModule::get_model_materials(void* state) {
    const auto& api = *bridge_->api;
    if (!ready())
        return 0;
    std::array<void*, 128> materials;
    int count = 0;
    if (api.gettop(state) >= 1) {
        const auto index = std::uint32_t(native_integer(api.checkinteger(state, 1)));
        if (index - 1 < 8191 && !(index & 0xFFFF0000u)) {
            auto* entity = invoke<void*>(interfaces_.entity_list, 3, index);
            auto* renderable = entity ? static_cast<std::byte*>(entity) + 4 : nullptr;
            auto* model = renderable ? invoke<void*>(renderable, 8) : nullptr;
            if (model) {
                auto* studio = invoke<void*>(interfaces_.model_info, 32, model);
                if (studio)
                    count = invoke<int>(interfaces_.studio_data_cache, 45, studio, 128,
                                        materials.data());
                if (!studio || !count) {
                    count = invoke<int>(interfaces_.model_info, 18, model);
                    if (count) {
                        if (count > 128)
                            count = 128;
                        invoke<void>(interfaces_.model_info, 19, model, count, materials.data());
                    }
                }
            }
        }
    }
    api.createtable(state, count, 0);
    int output = 1;
    for (int n = 0; n < count; ++n) {
        push_named_material(state, materials[std::size_t(n)]);
        if (api.type(state, -1) == 0)
            api.settop(state, -2);
        else
            api.rawseti(state, -2, output++);
    }
    return 1;
}
int MaterialModule::cached_material(void* state, std::size_t reference, std::size_t material,
                                    const char* name) {
    const auto& api = *bridge_->api;
    if (!ready()) {
        api.pushnil(state);
        return 1;
    }
    auto& stored = references_.references[reference];
    if (stored == -1) {
        auto* selected =
            models_ ? models_->prepare_material(&models_->materials[material]) : nullptr;
        if (selected) {
            push_object(state, selected, name);
            if (api.type(state, -1) != 0)
                stored = api.ref(state, registry);
        }
        if (stored == -1)
            return 0;
    }
    api.rawgeti(state, registry, stored);
    return 1;
}
int MaterialModule::arms_material(void* s) {
    return cached_material(s, 0, 5, "arms");
}
int MaterialModule::chams_material(void* s) {
    return cached_material(s, 1, 0, "chams");
}
int MaterialModule::viewmodel_material(void* s) {
    return cached_material(s, 2, 6, "vm");
}

} // namespace skeet::script
