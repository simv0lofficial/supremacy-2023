#include "skeet/script/cvar_api.hpp"
#include "skeet/engine/interface_call.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace skeet::script {
namespace {
constexpr int registry = -10000;
constexpr int globals = -10002;
constexpr int first_upvalue = -10003;
constexpr int second_upvalue = -10004;
constexpr int cvar_tag = 0x4356;
void* (*closure_context)(void*, int) = nullptr;
template <class T> T read(void* object, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<std::byte*>(object) + offset, sizeof(value));
    return value;
}
template <class T> void write(void* object, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}
std::uint32_t pointer_bits(void* object) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object));
}
std::uint32_t name_hash(const char* text) {
    std::uint32_t hash = 0x811c9dc5u;
    for (const auto* byte = reinterpret_cast<const unsigned char*>(text); *byte; ++byte)
        hash = (hash ^ *byte) * 0x01000193u;
    return hash ^ 0x92ca5df1u;
}
// Exact 434746A0..43474730 table used by the native __index.
constexpr std::array<std::uint32_t, 36> restricted_names{
    0x57bc12c0, 0x014bc0c9, 0x1e04b46d, 0xf6cb5776, 0xf969f31d, 0x7edebae3, 0x2a063f34, 0x39829225,
    0x137fda69, 0x498a8011, 0xae54cc97, 0xf49c0388, 0x8ebc768d, 0xc413d155, 0x5ae3fcf5, 0x1f4127b3,
    0xbd1b638e, 0xdb968492, 0x774029de, 0xab653f51, 0x242c97c1, 0x2d2ff39c, 0xb3a9f5e0, 0x0c392d6c,
    0x60764d21, 0x13cdaf42, 0xab587065, 0xd18c3fbe, 0x7f213e8d, 0x8de31295, 0xbaaba9e0, 0xbd2b4884,
    0x9f20ee89, 0xcd4aec1d, 0x1db798a3, 0x82c61cb0};
void lock_metatable(const LuaApi& api, void* state) { // 433E41F7
    api.pushboolean(state, 0);
    api.setfield(state, -2, "__metatable");
}
CvarModule& owner(void* state) {
    return *static_cast<CvarModule*>(closure_context(state, first_upvalue));
}
}

void* cvar_parent(void* variable) {
    for (;;) {
        void* next = read<void*>(variable, 0x1c);
        if (next == variable)
            return variable;
        variable = next;
    }
}
float cvar_float(void* variable) {
    void* parent = cvar_parent(variable);
    return std::bit_cast<float>(read<std::uint32_t>(parent, 0x2c) ^ pointer_bits(parent));
}
std::int32_t cvar_int(void* variable) {
    void* parent = cvar_parent(variable);
    return std::bit_cast<std::int32_t>(read<std::uint32_t>(parent, 0x30) ^ pointer_bits(parent));
}
void set_cvar_raw_float(void* variable, float value) {
    void* parent = cvar_parent(variable);
    const auto encoded = std::bit_cast<std::uint32_t>(value) ^ pointer_bits(parent);
    write(parent, 0x2c, encoded);
    write(parent, 0x3c, encoded);
}
void set_cvar_raw_int(void* variable, std::int32_t value) {
    void* parent = cvar_parent(variable);
    const auto encoded = std::bit_cast<std::uint32_t>(value) ^ pointer_bits(parent);
    write(parent, 0x30, encoded);
    write(parent, 0x40, encoded);
}
void* find_cvar(void* engine_cvar, const char* name) {
    return engine::invoke<void*>(engine_cvar, 15, name);
}

void CvarModule::push_tagging_table(void* state) {
    const auto& api = *bridge_->api;
    if (tagging_ref_ == -1) {
        api.createtable(state, 0, 0);
        api.createtable(state, 0, 2);
        api.pushlstring(state, "k", 1);
        api.setfield(state, -2, "__mode");
        lock_metatable(api, state);
        api.setmetatable(state, -2);
        tagging_ref_ = api.ref(state, registry);
    }
    api.rawgeti(state, registry, tagging_ref_);
}
void CvarModule::push_tagged_pointer(void* state, void* pointer) {
    const auto& api = *bridge_->api;
    api.pushlightuserdata(state, pointer);
    push_tagging_table(state);
    api.pushvalue(state, -2);
    api.pushinteger(state, cvar_tag);
    api.settable(state, -3);
    api.settop(state, -2);
}
void* CvarModule::argument(void* state, int minimum) {
    const auto& api = *bridge_->api;
    if (api.gettop(state) < minimum)
        return nullptr;
    api.checktype(state, 1, 5);
    api.rawgeti(state, 1, 1);
    api.checktype(state, -1, 2);
    const auto top = api.gettop(state);
    api.pushvalue(state, -1);
    push_tagging_table(state);
    api.pushvalue(state, -2);
    api.gettable(state, -2);
    void* pointer = nullptr;
    if (api.type(state, -1) != 0 && api.checkinteger(state, -1) == cvar_tag)
        pointer = api.touserdata(state, -3);
    api.settop(state, top - 1);
    return pointer;
}

int CvarModule::index(void* state) {
    const auto& api = *bridge_->api;
    std::size_t length = 0;
    const char* name = api.checklstring(state, 2, &length);
    if (!name || !length)
        return 0;
    if (!unrestricted_ && std::find(restricted_names.begin(), restricted_names.end(),
                                    name_hash(name)) != restricted_names.end())
        return 0;
    void* variable = find_cvar(engine_cvar_, name);
    if (!variable)
        return 0;
    api.createtable(state, 1, 0);
    push_tagged_pointer(state, variable);
    api.rawseti(state, -2, 1);
    api.pushvalue(state, second_upvalue);
    api.setmetatable(state, -2);
    api.pushvalue(state, 2);
    api.pushvalue(state, -2);
    api.settable(state, 1);
    return 1;
}
int CvarModule::set_string(void* state) {
    void* variable = argument(state, 2);
    if (variable && !engine::invoke<bool>(variable, 1)) {
        if (const char* value = bridge_->api->checklstring(state, 2, nullptr))
            engine::invoke<void>(variable, 14, value);
    }
    return 0;
}
int CvarModule::get_string(void* state) {
    void* variable = argument(state, 1);
    if (variable && !engine::invoke<bool>(variable, 1)) {
        if (const char* value = read<const char*>(variable, 0x24)) {
            bridge_->api->pushstring(state, value);
            return 1;
        }
    }
    return 0;
}
int CvarModule::set_float(void* state) {
    void* variable = argument(state, 2);
    if (variable && !engine::invoke<bool>(variable, 1))
        engine::invoke<void>(variable, 15, static_cast<float>(bridge_->api->tonumber(state, 2)));
    return 0;
}
int CvarModule::set_raw_float(void* state) {
    void* variable = argument(state, 2);
    if (variable && !engine::invoke<bool>(variable, 1))
        set_cvar_raw_float(variable, static_cast<float>(bridge_->api->tonumber(state, 2)));
    return 0;
}
int CvarModule::get_float(void* state) {
    void* variable = argument(state, 1);
    if (!variable || engine::invoke<bool>(variable, 1))
        return 0;
    bridge_->api->pushnumber(state, static_cast<double>(cvar_float(variable)));
    return 1;
}
int CvarModule::set_int(void* state) {
    void* variable = argument(state, 2);
    if (variable && !engine::invoke<bool>(variable, 1))
        engine::invoke<void>(variable, 16,
                             static_cast<std::int32_t>(bridge_->api->tointeger(state, 2)));
    return 0;
}
int CvarModule::set_raw_int(void* state) {
    void* variable = argument(state, 2);
    if (variable && !engine::invoke<bool>(variable, 1))
        set_cvar_raw_int(variable, static_cast<std::int32_t>(bridge_->api->tointeger(state, 2)));
    return 0;
}
int CvarModule::get_int(void* state) {
    void* variable = argument(state, 1);
    if (!variable || engine::invoke<bool>(variable, 1))
        return 0;
    bridge_->api->pushinteger(state, cvar_int(variable));
    return 1;
}
int CvarModule::invoke_callback(void* state) {
    if (void* variable = argument(state, 1)) {
        if (engine::invoke<bool>(variable, 1))
            invoke_command(state, variable);
        else
            invoke_variable(state, variable);
    }
    return 0;
}

int CvarModule::index_entry(void* state) {
    return owner(state).index(state);
}
int CvarModule::set_string_entry(void* state) {
    return owner(state).set_string(state);
}
int CvarModule::get_string_entry(void* state) {
    return owner(state).get_string(state);
}
int CvarModule::set_float_entry(void* state) {
    return owner(state).set_float(state);
}
int CvarModule::set_raw_float_entry(void* state) {
    return owner(state).set_raw_float(state);
}
int CvarModule::get_float_entry(void* state) {
    return owner(state).get_float(state);
}
int CvarModule::set_int_entry(void* state) {
    return owner(state).set_int(state);
}
int CvarModule::set_raw_int_entry(void* state) {
    return owner(state).set_raw_int(state);
}
int CvarModule::get_int_entry(void* state) {
    return owner(state).get_int(state);
}
int CvarModule::invoke_callback_entry(void* state) {
    return owner(state).invoke_callback(state);
}

void CvarModule::register_exports(Bridge& bridge) {
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Lua cvar exports require an initialized state");
    bridge_ = &bridge;
    const auto& api = *bridge.api;
    void* state = bridge.state;
    closure_context = api.touserdata;
    api.createtable(state, 0, 0);
    api.createtable(state, 0, 1);
    api.createtable(state, 0, 1);
    api.createtable(state, 0, 0);
    for (const auto& function : std::array<std::pair<const char*, LuaApi::CFunction>, 9>{
             {{"set_string", set_string_entry},
              {"get_string", get_string_entry},
              {"set_float", set_float_entry},
              {"set_raw_float", set_raw_float_entry},
              {"get_float", get_float_entry},
              {"set_int", set_int_entry},
              {"set_raw_int", set_raw_int_entry},
              {"get_int", get_int_entry},
              {"invoke_callback", invoke_callback_entry}}}) {
        api.pushlightuserdata(state, this);
        api.pushcclosure(state, function.second, 1);
        api.setfield(state, -2, function.first);
    }
    api.setfield(state, -2, "__index");
    lock_metatable(api, state);
    api.pushlightuserdata(state, this);
    api.pushvalue(state, -2);
    api.pushcclosure(state, index_entry, 2);
    api.remove(state, -2);
    api.setfield(state, -2, "__index");
    lock_metatable(api, state);
    api.setmetatable(state, -2);
    api.setfield(state, globals, "cvar");
}

} // namespace skeet::script
