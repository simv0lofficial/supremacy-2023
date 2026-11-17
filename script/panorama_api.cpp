#include "panorama_detail.hpp"
#include <algorithm>

namespace skeet::script {
using namespace panorama_detail;
namespace {
void* (*closure_userdata)(void*, int) = nullptr;
PanoramaModule& owner(void* state, int upvalues) {
    return *static_cast<PanoramaModule*>(closure_userdata(state, upvalue(upvalues + 1)));
}
}

int PanoramaModule::userdata_tag(void* state, int index) {
    const auto& api = *bridge_->api;
    const auto type = api.type(state, index);
    if (type != 2 && type != 7)
        return 0;
    api.pushvalue(state, index);
    if (handle_types_ref_ == -1) {
        api.createtable(state, 0, 0);
        api.createtable(state, 0, 2);
        api.pushlstring(state, "k", 1);
        api.setfield(state, -2, "__mode");
        api.pushboolean(state, 0);
        api.setfield(state, -2, "__metatable");
        api.setmetatable(state, -2);
        handle_types_ref_ = api.ref(state, registry);
    }
    api.rawgeti(state, registry, handle_types_ref_);
    api.pushvalue(state, -2);
    api.gettable(state, -2);
    const auto tag = api.type(state, -1) ? static_cast<int>(api.checkinteger(state, -1)) : 0;
    api.settop(state, -4);
    return tag;
}

void PanoramaModule::check_userdata(void* state, int index, int tag) {
    if (userdata_tag(state, index) != tag)
        bridge_->api->argerror(state, index, "expected udata");
}

void PanoramaModule::push_userdata(void* state, int tag) {
    const auto& api = *bridge_->api;
    (void)api.newuserdata(state, 0);
    if (handle_types_ref_ == -1) {
        api.createtable(state, 0, 0);
        api.createtable(state, 0, 2);
        api.pushlstring(state, "k", 1);
        api.setfield(state, -2, "__mode");
        api.pushboolean(state, 0);
        api.setfield(state, -2, "__metatable");
        api.setmetatable(state, -2);
        handle_types_ref_ = api.ref(state, registry);
    }
    api.rawgeti(state, registry, handle_types_ref_);
    api.pushvalue(state, -2);
    api.pushinteger(state, tag);
    api.settable(state, -3);
    api.settop(state, -2);
}

void PanoramaModule::register_exports(Bridge& bridge) {
    const auto& api = *bridge.api;
    if (closure_userdata && closure_userdata != api.touserdata)
        throw std::logic_error("Panorama closures require one Lua C API");
    closure_userdata = api.touserdata;
    bridge_ = &bridge;
    browser_.dispose_global = services_.v8.dispose_global;
    for (auto* handles : {&browser_.object_handles, &browser_.script_handles}) {
        for (auto handle : *handles)
            if (handle) {
                if (!services_.v8.dispose_global)
                    throw std::logic_error("Panorama global handles require V8 DisposeGlobal");
                services_.v8.dispose_global(handle);
            }
        handles->assign(1024, nullptr);
    }
    for (auto* indices : {&browser_.free_object_indices, &browser_.free_script_indices}) {
        indices->resize(1024);
        for (std::uint32_t index = 0; index < 1024; ++index)
            (*indices)[index] = 1023 - index;
    }
    const int base = api.gettop(bridge.state) + 1;
    void* state = bridge.state;
    api.createtable(state, 0, 0); // module
    for (int index = 0; index < 3; ++index)
        api.createtable(state, 0, 0);
    api.createtable(state, 0, 3); // object metatable
    api.pushboolean(state, 0);
    api.setfield(state, -2, "__metatable");
    api.createtable(state, 0, 0); // script metatable
    api.pushboolean(state, 0);
    api.setfield(state, -2, "__metatable");
    api.createtable(state, 0, 2); // shared weak-key metatable for three maps
    api.pushlstring(state, "k", 1);
    api.setfield(state, -2, "__mode");
    api.pushboolean(state, 0);
    api.setfield(state, -2, "__metatable");
    api.pushvalue(state, -1);
    api.pushvalue(state, -1);
    api.setmetatable(state, base + 1);
    api.setmetatable(state, base + 2);
    api.setmetatable(state, base + 3);
    auto bind = [&](int table, const char* name, LuaApi::CFunction function, int captures) {
        for (int index = 1; index <= captures; ++index)
            api.pushvalue(state, base + index);
        api.pushlightuserdata(state, this);
        api.pushcclosure(state, function, captures + 1);
        api.setfield(state, table, name);
    };
    bind(base + 4, "__index", index_entry, 4);
    bind(base + 4, "__call", call_entry, 4);
    bind(base + 4, "__tostring", tostring_entry, 1);
    bind(base + 4, "__gc", collect_object_entry, 1);
    bind(base + 5, "__call", run_script_entry, 4);
    bind(base + 5, "__gc", collect_script_entry, 1);
    bind(base, "open", open_entry, 5);
    bind(base, "loadstring", loadstring_entry, 5);
    api.settop(state, base);
    api.setfield(state, globals, "panorama");
}

void PanoramaModule::push_proxy(void* state, void* isolate, V8Local value, void* panel,
                                int parent) {
    const auto& api = *bridge_->api;
    (void)api.gc(state, 5, 0);
    const auto slot = take_handle(isolate, value, false);
    push_userdata(state, object_tag);
    api.pushvalue(state, upvalue(4));
    api.setmetatable(state, -2);
    api.pushvalue(state, -1);
    api.pushinteger(state, slot);
    api.settable(state, upvalue(1));
    if (parent != -1) {
        api.pushvalue(state, -1);
        api.pushvalue(state, parent);
        api.settable(state, upvalue(2));
    }
    api.pushvalue(state, -1);
    api.pushlightuserdata(state, panel);
    api.settable(state, upvalue(3));
}

int PanoramaModule::collect_object(void* state) {
    const auto& api = *bridge_->api;
    if (userdata_tag(state, 1) != object_tag)
        return 0;
    api.pushvalue(state, 1);
    api.gettable(state, upvalue(1));
    if (api.type(state, -1) != 3) {
        api.settop(state, -2);
        return 0;
    }
    const auto index = static_cast<std::uint32_t>(api.checkinteger(state, -1));
    api.settop(state, -2);
    if (index >= browser_.object_handles.size() || !browser_.object_handles[index])
        return 0;
    if (!browser_.zero_callback_storage)
        services_.v8.dispose_global(browser_.object_handles[index]);
    browser_.object_handles[index] = nullptr;
    browser_.free_object_indices.push_back(index);
    return 0;
}

int PanoramaModule::collect_script(void* state) {
    const auto& api = *bridge_->api;
    if (userdata_tag(state, 1) != script_tag)
        return 0;
    api.pushvalue(state, 1);
    api.gettable(state, upvalue(1));
    if (api.type(state, -1) != 3) {
        api.settop(state, -2);
        return 0;
    }
    const auto index = static_cast<std::uint32_t>(api.checkinteger(state, -1));
    api.settop(state, -2);
    if (index >= browser_.script_handles.size() || !browser_.script_handles[index])
        return 0;
    if (!browser_.zero_callback_storage)
        services_.v8.dispose_global(browser_.script_handles[index]);
    browser_.script_handles[index] = nullptr;
    browser_.free_script_indices.push_back(index);
    return 0;
}

int PanoramaModule::open_entry(void* state) {
    return owner(state, 5).open(state);
}
int PanoramaModule::loadstring_entry(void* state) {
    return owner(state, 5).loadstring(state);
}
int PanoramaModule::index_entry(void* state) {
    return owner(state, 4).index(state);
}
int PanoramaModule::call_entry(void* state) {
    return owner(state, 4).call(state);
}
int PanoramaModule::tostring_entry(void* state) {
    return owner(state, 1).tostring(state);
}
int PanoramaModule::collect_object_entry(void* state) {
    return owner(state, 1).collect_object(state);
}
int PanoramaModule::run_script_entry(void* state) {
    return owner(state, 4).run_script(state);
}
int PanoramaModule::collect_script_entry(void* state) {
    return owner(state, 1).collect_script(state);
}

} // namespace skeet::script
