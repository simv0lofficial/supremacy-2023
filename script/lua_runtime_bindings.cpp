#include "skeet/script/lua_runtime_bindings.hpp"
#include "initialize_detail.hpp"
#include "detail.hpp"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
void skeet_lua_runtime_layout(unsigned int output[3]);
}
#include <type_traits>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <limits>

namespace skeet::script {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(lua_Integer) == sizeof(std::ptrdiff_t));
static_assert(std::is_same_v<lua_Number, double>);
static_assert(sizeof(LuaApi::CFunction) == sizeof(lua_CFunction));
static_assert(sizeof(CjsonRegistration) == sizeof(luaL_Reg));
static_assert(offsetof(CjsonRegistration, name) == offsetof(luaL_Reg, name));
static_assert(offsetof(CjsonRegistration, function) == offsetof(luaL_Reg, func));

// The bridge's opaque state has the same x86 cdecl ABI as lua_State*.
// Ordinary entries retain their exact public Lua parameter and result types.
template <auto Function> struct Entry;
template <class Result, class... Args, Result (*Function)(lua_State*, Args...)>
struct Entry<Function> {
    static Result invoke(void* state, Args... args) {
        return Function(static_cast<lua_State*>(state), args...);
    }
};
int cpcall(void* state, LuaApi::CFunction function, void* context) {
    return lua_cpcall(static_cast<lua_State*>(state), reinterpret_cast<lua_CFunction>(function),
                      context);
}
void pushcclosure(void* state, LuaApi::CFunction function, int upvalues) {
    lua_pushcclosure(static_cast<lua_State*>(state), reinterpret_cast<lua_CFunction>(function),
                     upvalues);
}
LuaApi::CFunction atpanic(void* state, LuaApi::CFunction function) {
    return reinterpret_cast<LuaApi::CFunction>(
        lua_atpanic(static_cast<lua_State*>(state), reinterpret_cast<lua_CFunction>(function)));
}
void* newstate() {
    return luaL_newstate();
}
void open_library(void* state, const char* name, const CjsonRegistration* entries, int upvalues) {
    // Both public registration records contain {name, x86 cdecl callback}.
    luaL_openlib(static_cast<lua_State*>(state), name, reinterpret_cast<const luaL_Reg*>(entries),
                 upvalues);
}

LuaRuntimeBindings make_bindings() noexcept {
    LuaRuntimeBindings result{};
    auto& lua = result.lua;
    lua.gettop = Entry<&lua_gettop>::invoke;
    lua.settop = Entry<&lua_settop>::invoke;
    lua.rawgeti = Entry<&lua_rawgeti>::invoke;
    lua.type = Entry<&lua_type>::invoke;
    lua.createtable = Entry<&lua_createtable>::invoke;
    lua.pushlstring = Entry<&lua_pushlstring>::invoke;
    lua.pushinteger = Entry<&lua_pushinteger>::invoke;
    lua.settable = Entry<&lua_settable>::invoke;
    lua.pcall = Entry<&lua_pcall>::invoke;
    lua.tolstring = Entry<&lua_tolstring>::invoke;
    lua.unref = Entry<&luaL_unref>::invoke;
    lua.pushvalue = Entry<&lua_pushvalue>::invoke;
    lua.ref = Entry<&luaL_ref>::invoke;
    lua.pushstring = Entry<&lua_pushstring>::invoke;
    lua.gettable = Entry<&lua_gettable>::invoke;
    lua.objlen = Entry<&lua_objlen>::invoke;
    lua.rawequal = Entry<&lua_rawequal>::invoke;
    lua.rawseti = Entry<&lua_rawseti>::invoke;
    lua.pushnil = Entry<&lua_pushnil>::invoke;
    lua.checktype = Entry<&luaL_checktype>::invoke;
    lua.pushnumber = Entry<&lua_pushnumber>::invoke;
    lua.call = Entry<&lua_call>::invoke;
    lua.remove = Entry<&lua_remove>::invoke;
    lua.gc = Entry<&lua_gc>::invoke;
    lua.close = Entry<&lua_close>::invoke;
    lua.getfield = Entry<&lua_getfield>::invoke;
    lua.next = Entry<&lua_next>::invoke;
    lua.rawset = Entry<&lua_rawset>::invoke;
    lua.toboolean = Entry<&lua_toboolean>::invoke;
    lua.isstring = Entry<&lua_isstring>::invoke;
    lua.checklstring = Entry<&luaL_checklstring>::invoke;
    lua.checkinteger = Entry<&luaL_checkinteger>::invoke;
    lua.checknumber = Entry<&luaL_checknumber>::invoke;
    lua.pushlightuserdata = Entry<&lua_pushlightuserdata>::invoke;
    lua.touserdata = Entry<&lua_touserdata>::invoke;
    lua.setfield = Entry<&lua_setfield>::invoke;
    lua.argerror = Entry<&luaL_argerror>::invoke;
    lua.tonumber = Entry<&lua_tonumber>::invoke;
    lua.tointeger = Entry<&lua_tointeger>::invoke;
    lua.pushboolean = Entry<&lua_pushboolean>::invoke;
    lua.newuserdata = Entry<&lua_newuserdata>::invoke;
    lua.getmetatable = Entry<&lua_getmetatable>::invoke;
    lua.setmetatable = Entry<&lua_setmetatable>::invoke;
    lua.checkudata = Entry<&luaL_checkudata>::invoke;
    lua.isnumber = Entry<&lua_isnumber>::invoke;
    lua.error = Entry<&lua_error>::invoke;
    lua.insert = Entry<&lua_insert>::invoke;
    lua.lcheckstack = Entry<&luaL_checkstack>::invoke;
    lua.where = Entry<&luaL_where>::invoke;
    lua.concat = Entry<&lua_concat>::invoke;
    lua.cpcall = cpcall;
    lua.pushcclosure = pushcclosure;
    // Varargs stay at the actual library entry; no formatting/argument replay.
    lua.lerror = reinterpret_cast<decltype(lua.lerror)>(&luaL_error);
    unsigned int layout[3]{};
    skeet_lua_runtime_layout(layout);
    result.initialization = {newstate,
                             Entry<&luaJIT_setmode>::invoke,
                             atpanic,
                             Entry<&luaL_openlibs>::invoke,
                             {layout[0], layout[1], layout[2]}};
    result.json = {lua,
                   Entry<&lua_typename>::invoke,
                   Entry<&lua_tonumber>::invoke,
                   Entry<&lua_newuserdata>::invoke,
                   Entry<&lua_setmetatable>::invoke,
                   Entry<&lua_checkstack>::invoke,
                   Entry<&lua_pushboolean>::invoke,
                   Entry<&luaL_checkoption>::invoke,
                   open_library};
    result.vector = {Entry<&luaL_loadbuffer>::invoke, Entry<&luaL_findtable>::invoke};
    return result;
}
} // namespace

namespace detail {
namespace {
int callback_traceback(lua_State* state) {
    const char* message = lua_tostring(state, 1);
    luaL_traceback(state, state, message ? message : luaL_typename(state, 1), 1);
    return 1;
}
}

int callback_pcall(void* opaque_state, int arguments, int results) {
    auto* state = static_cast<lua_State*>(opaque_state);
    if (!lua_checkstack(state, 1))
        return lua_pcall(state, arguments, results, 0);

    const auto handler = lua_gettop(state) - arguments;
    lua_pushcfunction(state, callback_traceback);
    lua_insert(state, handler);
    const auto status = lua_pcall(state, arguments, results, handler);
    lua_remove(state, handler);
    return status;
}

void report_callback_error(Bridge& bridge, void* state) {
    const auto& api = *bridge.api;
    const auto* message = api.tolstring(state, -1, nullptr);
    if (message)
        report_callback_failure(bridge, message);
    api.settop(state, -2);
}

void report_callback_failure(Bridge& bridge, const char* message) {
    if (bridge.report_error) {
        constexpr std::uint64_t repeat_interval_ms = 5000;
        const auto now = GetTickCount64();
        auto* oldest = &bridge.recent_callback_errors.front();
        for (auto& error : bridge.recent_callback_errors) {
            if (error.message == message) {
                if (now - error.reported_at < repeat_interval_ms)
                    return;
                oldest = &error;
                break;
            }
            if (error.reported_at < oldest->reported_at)
                oldest = &error;
        }
        oldest->message = message;
        oldest->reported_at = now;
        bridge.report_error(bridge.error_context, message);
    }
}
}

const LuaRuntimeBindings& source_lua_runtime_bindings() noexcept {
    static const LuaRuntimeBindings bindings = make_bindings();
    return bindings;
}
} // namespace skeet::script

// Native43439986 checks the builtin-loading byte after detecting binary input.
// Resolve the canonical runtime by the VM's shared global reference, so a
// coroutine receives the same gate as its main state. Unowned states have no
// client builtin-loading permission.
extern "C" int skeet_lua_bytecode_allowed(lua_State* state) {
    auto* runtime = skeet::script::initialize_detail::owner(state);
    return runtime && runtime->loading_builtin;
}

// Script addresses refer to the former image, not the relocated source DLL.
// Never let an unrelated allocation at those addresses become a patch target.
extern "C" int skeet_lua_ffi_access(const void* pointer, std::size_t length, int mode) {
    const auto first = reinterpret_cast<std::uintptr_t>(pointer);
    constexpr std::uintptr_t legacy_begin = 0x43310000u, legacy_end = 0x4360c000u;
    if (mode == 3) // Pointer argument: null and OS pseudo-handles remain legal.
        return first < legacy_begin || first >= legacy_end;
    if (!length)
        return 1;
    if (length - 1 > std::numeric_limits<std::uintptr_t>::max() - first)
        return 0;
    const auto last = first + length - 1;
    if (first < legacy_end && last >= legacy_begin)
        return 0;
    for (auto address = first;;) {
        MEMORY_BASIC_INFORMATION region{};
        if (!VirtualQuery(reinterpret_cast<const void*>(address), &region, sizeof(region)) ||
            region.State != MEM_COMMIT || (region.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
            return 0;
        const DWORD readable = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
                               PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
        const DWORD writable =
            PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
        const DWORD executable =
            PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
        if (!(region.Protect & (mode == 2 ? executable : mode == 1 ? writable : readable)))
            return 0;
        const auto end = reinterpret_cast<std::uintptr_t>(region.BaseAddress) + region.RegionSize;
        if (!end || last < end)
            return 1;
        if (end <= address)
            return 0;
        address = end;
    }
}

// The trace recorder also uses this bounded scan; strlen must not cross into
// an inaccessible page before an FFI guard has a chance to run.
extern "C" int skeet_lua_ffi_strlen(const char* text) {
    constexpr std::uintptr_t legacy_begin = 0x43310000u, legacy_end = 0x4360c000u;
    constexpr DWORD readable = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY |
                               PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE |
                               PAGE_EXECUTE_WRITECOPY;
    std::size_t length = 0;
    for (;;) {
        const auto address = reinterpret_cast<std::uintptr_t>(text);
        MEMORY_BASIC_INFORMATION region{};
        if ((address >= legacy_begin && address < legacy_end) ||
            !VirtualQuery(text, &region, sizeof(region)) || region.State != MEM_COMMIT ||
            (region.Protect & (PAGE_GUARD | PAGE_NOACCESS)) || !(region.Protect & readable))
            return -1;
        auto end = reinterpret_cast<std::uintptr_t>(region.BaseAddress) + region.RegionSize;
        if (end <= address)
            return -1;
        // Query each region once and never scan into the former image range.
        if (address < legacy_begin && end > legacy_begin)
            end = legacy_begin;
        const auto available = end - address;
        const auto* terminator = static_cast<const char*>(std::memchr(text, 0, available));
        const auto count = terminator ? static_cast<std::size_t>(terminator - text) : available;
        if (count > static_cast<std::size_t>(INT_MAX) - length)
            return -1;
        length += count;
        if (terminator)
            return static_cast<int>(length);
        text = reinterpret_cast<const char*>(end);
    }
}

extern "C" void skeet_lua_ffi_unavailable(lua_State* state, const void* pointer, int mode) {
    auto* runtime = skeet::script::initialize_detail::owner(state);
    if (!runtime || !runtime->bridge.report_error)
        return;
    const auto address = reinterpret_cast<std::uintptr_t>(pointer);
    if (!runtime->unavailable_ffi_addresses.insert(address).second)
        return;
    const char* operation = mode == 2   ? "call"
                            : mode == 1 ? "write"
                            : mode == 3 ? "call argument"
                                        : "read";
    lua_Debug location{};
    const bool located = lua_getstack(state, 1, &location) && lua_getinfo(state, "Sl", &location);
    char message[512]{};
    std::snprintf(message, sizeof(message), "FFI: address 0x%08X unavailable; %s skipped%s%s:%d",
                  static_cast<unsigned>(address), operation, located ? " in " : "",
                  located ? location.short_src : "FFI", located ? location.currentline : 0);
    runtime->bridge.report_error(runtime->bridge.error_context, message);
}
