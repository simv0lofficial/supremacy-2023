#pragma once
#include "skeet/script/cjson.hpp"
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace skeet::script::cjson_detail {
using lua_State = void;
using lua_Integer = std::ptrdiff_t;
using luaL_Reg = CjsonRegistration;
inline constexpr int LUA_TNIL = 0, LUA_TBOOLEAN = 1, LUA_TLIGHTUSERDATA = 2, LUA_TNUMBER = 3,
                     LUA_TSTRING = 4, LUA_TTABLE = 5;
const CjsonApi& api();
void bind_api(const CjsonApi&);
void terminate_json_process(unsigned int);
inline int lua_upvalueindex(int index) {
    return -10002 - index;
}
inline bool lua_isnil(lua_State* state, int index) {
    return api().lua.type(state, index) == LUA_TNIL;
}
inline bool lua_isboolean(lua_State* state, int index) {
    return api().lua.type(state, index) == LUA_TBOOLEAN;
}
inline void lua_pop(lua_State* state, int count) {
    api().lua.settop(state, -count - 1);
}
inline void lua_newtable(lua_State* state) {
    api().lua.createtable(state, 0, 0);
}
inline void lua_pushcfunction(lua_State* state, LuaApi::CFunction function) {
    api().lua.pushcclosure(state, function, 0);
}

#define lua_gettop(s) api().lua.gettop(s)
#define lua_type(s, i) api().lua.type(s, i)
#define lua_typename(s, i) api().type_name(s, i)
#define lua_touserdata(s, i) api().lua.touserdata(s, i)
#define lua_tonumber(s, i) api().to_number(s, i)
#define lua_toboolean(s, i) api().lua.toboolean(s, i)
#define lua_tolstring(s, i, n) api().lua.tolstring(s, i, n)
#define lua_pushinteger(s, n) api().lua.pushinteger(s, n)
#define lua_pushnumber(s, n) api().lua.pushnumber(s, n)
#define lua_pushboolean(s, n) api().push_boolean(s, n)
#define lua_pushnil(s) api().lua.pushnil(s)
#define lua_pushstring(s, v) api().lua.pushstring(s, v)
#define lua_pushlstring(s, v, n) api().lua.pushlstring(s, v, n)
#define lua_pushlightuserdata(s, v) api().lua.pushlightuserdata(s, v)
#define lua_setfield(s, i, k) api().lua.setfield(s, i, k)
#define lua_setmetatable(s, i) api().set_metatable(s, i)
#define lua_newuserdata(s, n) api().new_userdata(s, n)
#define lua_next(s, i) api().lua.next(s, i)
#define lua_rawgeti(s, i, n) api().lua.rawgeti(s, i, n)
#define lua_rawseti(s, i, n) api().lua.rawseti(s, i, n)
#define lua_rawset(s, i) api().lua.rawset(s, i)
#define lua_checkstack(s, n) api().check_stack(s, n)
#define luaL_checkinteger(s, i) api().lua.checkinteger(s, i)
#define luaL_checklstring(s, i, n) api().lua.checklstring(s, i, n)
#define luaL_checkoption(s, i, d, v) api().check_option(s, i, d, v)
#define luaL_error(...) api().lua.lerror(__VA_ARGS__)
#define luaL_argcheck(s, c, i, m) ((void)((c) || api().lua.argerror(s, i, m)))

// Native compact strbuf at +0x500: pointer, size, length, dynamic flag.
struct strbuf_t {
    char* buf;
    int size, length, dynamic;
};
void strbuf_init(strbuf_t*, int);
strbuf_t* strbuf_new(int);
void strbuf_free(strbuf_t*);
void strbuf_resize(strbuf_t*, int);
void strbuf_append_string(strbuf_t*, const char*);
inline int strbuf_empty_length(const strbuf_t* s) {
    return s->size - s->length - 1;
}
inline int strbuf_length(const strbuf_t* s) {
    return s->length;
}
inline char* strbuf_empty_ptr(strbuf_t* s) {
    return s->buf + s->length;
}
inline void strbuf_extend_length(strbuf_t* s, int length) {
    s->length += length;
}
inline void strbuf_reset(strbuf_t* s) {
    s->length = 0;
}
inline void strbuf_ensure_empty_length(strbuf_t* s, int length) {
    if (length > strbuf_empty_length(s))
        strbuf_resize(s, s->length + length);
}
inline void strbuf_append_char_unsafe(strbuf_t* s, char value) {
    s->buf[s->length++] = value;
}
inline void strbuf_append_char(strbuf_t* s, char value) {
    strbuf_ensure_empty_length(s, 1);
    strbuf_append_char_unsafe(s, value);
}
inline void strbuf_append_mem_unsafe(strbuf_t* s, const char* data, int length) {
    std::memcpy(s->buf + s->length, data, length);
    s->length += length;
}
inline void strbuf_append_mem(strbuf_t* s, const char* data, int length) {
    strbuf_ensure_empty_length(s, length);
    strbuf_append_mem_unsafe(s, data, length);
}
inline void strbuf_ensure_null(strbuf_t* s) {
    s->buf[s->length] = 0;
}
inline char* strbuf_string(strbuf_t* s, int* length) {
    if (length)
        *length = s->length;
    return s->buf;
}

enum json_token_type_t {
    T_OBJ_BEGIN,
    T_OBJ_END,
    T_ARR_BEGIN,
    T_ARR_END,
    T_STRING,
    T_NUMBER,
    T_BOOLEAN,
    T_NULL,
    T_COLON,
    T_COMMA,
    T_END,
    T_WHITESPACE,
    T_ERROR,
    T_UNKNOWN
};
struct json_config_t {
    json_token_type_t ch2token[256];
    char escape2char[256];
    strbuf_t encode_buf;
    int encode_sparse_convert, encode_sparse_ratio, encode_sparse_safe, encode_max_depth;
    int encode_invalid_numbers, encode_number_precision, decode_invalid_numbers, decode_max_depth;
};
struct json_parse_t {
    const char* data;
    const char* ptr;
    strbuf_t* tmp;
    json_config_t* cfg;
    int current_depth;
};
struct json_token_t {
    json_token_type_t type;
    int index;
    union {
        const char* string;
        double number;
        int boolean;
    } value;
    int string_len;
};
json_config_t* json_fetch_config(lua_State*);
void json_create_config(lua_State*);
int json_encode(lua_State*);
int json_decode(lua_State*);
int json_cfg_encode_sparse_array(lua_State*);
int json_cfg_encode_max_depth(lua_State*);
int json_cfg_decode_max_depth(lua_State*);
int json_cfg_encode_number_precision(lua_State*);
int json_cfg_encode_invalid_numbers(lua_State*);
int json_cfg_decode_invalid_numbers(lua_State*);
bool json_scan_number(const char*, double*);
int fpconv_g_fmt(char*, double, int);
void fpconv_init();
inline constexpr int FPCONV_G_FMT_BUFSIZE = 32;
} // namespace skeet::script::cjson_detail
