#include "skeet/script/material_methods.hpp"
#include "skeet/script/material_values.hpp"
#include "skeet/effects/model_render.hpp"
#include "skeet/engine/interface_call.hpp"
#include <cstring>
#include <limits>

namespace skeet::script {
using engine::invoke;
namespace {
std::int32_t native_integer(std::ptrdiff_t value) noexcept {
    // Native luaL_checkinteger ends in CVTTSD2SI EAX at 0x434249FA.
    if (value < std::numeric_limits<std::int32_t>::min() ||
        value > std::numeric_limits<std::int32_t>::max())
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}
}

MaterialModule::Object* MaterialMethods::object(MaterialModule& module, void* state,
                                                int required_arguments) { // 0x43342BD4
    const auto& api = *module.bridge_->api;
    if (api.gettop(state) < required_arguments)
        return nullptr;
    api.checktype(state, 1, 7);
    return static_cast<MaterialModule::Object*>(module.tagged(state, 1, 0x4D54));
}

bool MaterialMethods::parameter(MaterialModule& module, void* state, std::int32_t& index,
                                const char*& name) {
    const auto& api = *module.bridge_->api;
    index = -1;
    name = nullptr;
    if (api.type(state, 2) == 3) {
        index = native_integer(api.checkinteger(state, 2));
        return index >= 0 && index < 13;
    }
    name = api.checklstring(state, 2, nullptr);
    return name && *name;
}

int MaterialMethods::get_name_entry(void* state) {
    auto& module = MaterialModule::owner(state, -10003);
    if (!module.ready())
        return 0;
    auto* value = object(module, state, 1);
    if (!value || !value->material)
        return 0;
    auto* material = value->material;
    const char* name = nullptr;
    auto* models = module.models_;
    if (models && material == models->prepare_material(&models->materials[0]))
        name = "chams";
    else if (models && material == models->prepare_material(&models->materials[5]))
        name = "arms";
    else if (models && material == models->prepare_material(&models->materials[6]))
        name = "vm";
    else
        name = invoke<const char*>(material, 0);
    module.bridge_->api->pushstring(state, name);
    return 1;
}

int MaterialMethods::reload_entry(void* state) {
    auto& module = MaterialModule::owner(state, -10003);
    if (!module.ready())
        return 0;
    auto* value = object(module, state, 1);
    if (!value || !value->material)
        return 0;
    invoke<void>(value->material, 37);
    if (module.models_)
        module.models_->invalidate_material(value->material);
    return 0;
}

int MaterialMethods::alpha_modulate_entry(void* state) {
    auto& module = MaterialModule::owner(state, -10003);
    if (!module.ready())
        return 0;
    auto* value = object(module, state, 2);
    if (!value || !value->material)
        return 0;
    const auto alpha = static_cast<float>(module.bridge_->api->checknumber(state, 2)) / 255.0f;
    if (module.load(*value)) {
        value->changed = true;
        invoke<void>(value->material, 27, alpha);
    }
    return 0;
}

int MaterialMethods::color_modulate_entry(void* state) {
    auto& module = MaterialModule::owner(state, -10003);
    if (!module.ready())
        return 0;
    auto* value = object(module, state, 4);
    if (!value || !value->material)
        return 0;
    const auto& api = *module.bridge_->api;
    const auto red = static_cast<float>(api.checknumber(state, 2)) / 255.0f;
    const auto green = static_cast<float>(api.checknumber(state, 3)) / 255.0f;
    const auto blue = static_cast<float>(api.checknumber(state, 4)) / 255.0f;
    if (module.load(*value)) {
        value->changed = true;
        invoke<void>(value->material, 28, red, green, blue);
    }
    return 0;
}

int MaterialMethods::set_shader_param_entry(void* state) {
    auto& module = MaterialModule::owner(state, -10003);
    const auto& api = *module.bridge_->api;
    if (!module.ready() || api.gettop(state) < 3)
        return 0;
    std::int32_t index;
    const char* name;
    if (!parameter(module, state, index, name))
        return 0;
    auto* value = object(module, state, 3);
    if (!value || !value->material || !module.load(*value))
        return 0;
    auto* system = module.interfaces_.material_system;
    const auto lock = invoke<std::uint32_t>(system, 118);
    auto* material = invoke<void*>(value->material, 99);
    const auto count = invoke<std::int32_t>(material, 40);
    auto** variables = invoke<void**>(material, 41);
    if (variables) {
        if (index != -1) {
            if (index < count && variables[index]) {
                MaterialValues::set(module, state, variables[index], 3);
                value->changed = true;
            }
        } else {
            void* variable = nullptr;
            for (std::int32_t i = 0; i < count; ++i) {
                if (variables[i] && std::strcmp(invoke<const char*>(variables[i], 2), name) == 0) {
                    variable = variables[i];
                    break;
                }
            }
            if (variable) {
                MaterialValues::set(module, state, variable, 3);
                value->changed = true;
                invoke<void>(material, 53);
            } else if ((variable = MaterialValues::create(module, material, name))) {
                invoke<void>(variable, 19);
                MaterialValues::set(module, state, variable, 3);
                value->changed = true;
                invoke<void>(material, 84, variable);
                invoke<void>(material, 53);
            }
        }
    }
    invoke<void>(system, 119, lock);
    return 0;
}

int MaterialMethods::get_shader_param_entry(void* state) {
    auto& module = MaterialModule::owner(state, -10003);
    const auto& api = *module.bridge_->api;
    if (!module.ready() || api.gettop(state) < 2)
        return 0;
    std::int32_t index;
    const char* name;
    if (!parameter(module, state, index, name))
        return 0;
    auto* value = object(module, state, 2);
    if (!value || !value->material || !module.load(*value))
        return 0;
    const auto count = invoke<std::int32_t>(value->material, 40);
    auto** variables = invoke<void**>(value->material, 41);
    if (!variables)
        return 0;
    if (index != -1)
        return variables[index] ? MaterialValues::get(module, state, variables[index]) : 0;
    for (std::int32_t i = 0; i < count; ++i)
        if (variables[i] && std::strcmp(invoke<const char*>(variables[i], 2), name) == 0)
            return MaterialValues::get(module, state, variables[i]);
    return 0;
}

int MaterialMethods::set_material_var_flag_entry(void* state) {
    auto& module = MaterialModule::owner(state, -10003);
    if (!module.ready())
        return 0;
    auto* value = object(module, state, 3);
    if (!value || !value->material || !module.load(*value))
        return 0;
    value->changed = true;
    const auto& api = *module.bridge_->api;
    const auto enabled = api.toboolean(state, 3) != 0;
    const auto index = static_cast<std::uint32_t>(native_integer(api.checkinteger(state, 2)));
    invoke<void>(value->material, 29, 1u << (index & 31), enabled);
    return 0;
}

int MaterialMethods::get_material_var_flag_entry(void* state) {
    auto& module = MaterialModule::owner(state, -10003);
    if (!module.ready())
        return 0;
    bool enabled = false;
    auto* value = object(module, state, 2);
    if (value && value->material && module.load(*value)) {
        const auto index =
            static_cast<std::uint32_t>(native_integer(module.bridge_->api->checkinteger(state, 2)));
        enabled = invoke<bool>(value->material, 30, 1u << (index & 31));
    }
    module.bridge_->api->pushboolean(state, enabled);
    return 1;
}

} // namespace skeet::script
