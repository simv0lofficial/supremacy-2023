#include "skeet/script/material_api.hpp"
#include "skeet/script/material_registry.hpp"
#include <array>
#include <stdexcept>

namespace skeet::script {
namespace {
using Export = std::pair<const char*, LuaApi::CFunction>;
}
int MaterialModule::find_material_entry(void* s) {
    return owner(s).find_material(s);
}
int MaterialModule::find_materials_entry(void* s) {
    return owner(s).find_materials(s);
}
int MaterialModule::find_texture_entry(void* s) {
    return owner(s).find_texture(s);
}
int MaterialModule::get_model_materials_entry(void* s) {
    return owner(s).get_model_materials(s);
}
int MaterialModule::override_material_entry(void* s) {
    return owner(s).override_material(s);
}
int MaterialModule::arms_material_entry(void* s) {
    return owner(s).arms_material(s);
}
int MaterialModule::chams_material_entry(void* s) {
    return owner(s).chams_material(s);
}
int MaterialModule::viewmodel_material_entry(void* s) {
    return owner(s).viewmodel_material(s);
}
int MaterialModule::collect_entry(void* s) {
    return owner(s, -10003).collect(s);
}
void MaterialModule::register_exports(Bridge& bridge) { // 0x433E2E5E
    bridge_ = &bridge;
    if (!method_registry_.available())
        return;
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Lua material exports require an initialized state");
    const auto& api = *bridge.api;
    auto* state = bridge.state;
    context_ = api.touserdata;
    api.createtable(state, 0, 2); // material userdata metatable
    api.createtable(state, 0, 0);
    for (const auto& method : method_registry_.registrations()) {
        api.pushlightuserdata(state, this);
        api.pushcclosure(state, method.function, 1);
        api.setfield(state, -2, method.name);
    }
    api.setfield(state, -2, "__index");
    api.pushlightuserdata(state, this);
    api.pushcclosure(state, collect_entry, 1);
    api.setfield(state, -2, "__gc");
    api.pushboolean(state, false);
    api.setfield(state, -2, "__metatable");
    api.createtable(state, 0, 0); // native shared material-name cache
    api.createtable(state, 1, 0);
    constexpr std::array<Export, 8> exports{{{"find_material", find_material_entry},
                                             {"find_materials", find_materials_entry},
                                             {"find_texture", find_texture_entry},
                                             {"get_model_materials", get_model_materials_entry},
                                             {"override_material", override_material_entry},
                                             {"arms_material", arms_material_entry},
                                             {"chams_material", chams_material_entry},
                                             {"viewmodel_material", viewmodel_material_entry}}};
    for (const auto& [name, callback] : exports) {
        api.pushvalue(state, -2); // upvalue 1: cache
        api.pushvalue(state, -4); // upvalue 2: userdata metatable
        api.pushlightuserdata(state, this); // explicit restored owner
        api.pushcclosure(state, callback, 3);
        api.setfield(state, -2, name);
    }
    api.pushvalue(state, -2);
    api.rawseti(state, -2, 1);
    api.setfield(state, -10002, "materialsystem");
    api.settop(state, -3);
}

} // namespace skeet::script
