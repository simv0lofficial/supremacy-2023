#include "skeet/script/material_api.hpp"
#include "skeet/effects/model_render.hpp"
#include "skeet/engine/interface_call.hpp"
#include <cstring>

namespace skeet::script {
using engine::invoke;

bool MaterialModule::load(Object& object) { // 0x434035AC
    auto* material = invoke<void*>(object.material, 99);
    if (!material)
        return false;
    if (!invoke<bool>(material, 71)) {
        object.loaded = true;
        invoke<void>(material, 62);
    }
    material = invoke<void*>(material, 99);
    return invoke<void*>(material, 69) != nullptr;
}
int MaterialModule::override_material(void* state) { // 0x4335852D
    const auto& api = *bridge_->api;
    if (!ready() || api.gettop(state) < 2)
        return 0;
    auto* destination = static_cast<Object*>(tagged(state, 1, 0x4D54));
    if (!destination || !destination->material)
        return 0;
    auto* source = static_cast<Object*>(tagged(state, 2, 0x4D54));
    if (!source || !source->material)
        return 0;
    auto* to = destination->material;
    auto* from = source->material;
    if (!load(*destination) || !load(*source))
        return 0;
    if (!invoke<void*>(from, 69) || !invoke<void*>(to, 69))
        return 0;
    destination->changed = true;
    invoke<void>(to, 34, invoke<const char*>(from, 49));
    const auto source_count = invoke<int>(from, 40);
    auto** source_variables = invoke<void**>(from, 41);
    const auto destination_count = invoke<int>(to, 40);
    auto** destination_variables = invoke<void**>(to, 41);
    for (int n = 0; n < destination_count; ++n) {
        auto* variable = destination_variables[n];
        if (!variable)
            continue;
        for (int j = 0; j < source_count; ++j) {
            auto* other = source_variables[j];
            if (!other)
                continue;
            const auto* destination_name = invoke<const char*>(variable, 2);
            const auto* source_name = invoke<const char*>(other, 2);
            if (std::strcmp(destination_name, source_name) == 0) {
                invoke<void>(variable, 23, other);
                break;
            }
        }
    }
    for (std::uint32_t n = 0; n < 32; ++n) {
        const auto flag = std::uint32_t(1) << n;
        invoke<void>(to, 29, flag, invoke<bool>(from, 30, flag));
    }
    return 0;
}
int MaterialModule::collect(void* state) { // 0x433E9C60
    const auto& api = *bridge_->api;
    if ((!ready() && !*interfaces_.unloading) || api.gettop(state) < 1)
        return 0;
    auto* object = static_cast<Object*>(api.touserdata(state, 1));
    if (!object || !object->material)
        return 0;
    auto* material = object->material;
    if (object->loaded) {
        if (!invoke<int>(material, 56))
            invoke<void>(material, 61, false);
        else if (object->changed) {
            invoke<void>(material, 37);
            if (models_)
                models_->invalidate_material(material);
        }
    }
    invoke<void>(material, 13);
    return 0;
}

} // namespace skeet::script
