/* Lua CJSON - JSON support for Lua
 *
 * Copyright (c) 2010-2012  Mark Pulford <mark@kyne.com.au>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Adapted from the identical conversion core of mpx/lua-cjson tags 2.0.0
 * and 2.1.0, official https://github.com/mpx/lua-cjson. The client removes
 * configurable buffer persistence and uses its compact four-field strbuf.
 * Native bindings and local changes correspond to 433C8D0F and its eight
 * exported methods; no unique upstream release marker survives. */
#include "cjson_internal.hpp"
#include <bit>
#include <emmintrin.h>

namespace skeet::script::cjson_detail {
static const char* char2escape[256] = {
    "\\u0000", "\\u0001", "\\u0002", "\\u0003", "\\u0004", "\\u0005", "\\u0006", "\\u0007",
    "\\b",     "\\t",     "\\n",     "\\u000b", "\\f",     "\\r",     "\\u000e", "\\u000f",
    "\\u0010", "\\u0011", "\\u0012", "\\u0013", "\\u0014", "\\u0015", "\\u0016", "\\u0017",
    "\\u0018", "\\u0019", "\\u001a", "\\u001b", "\\u001c", "\\u001d", "\\u001e", "\\u001f",
    NULL,      NULL,      "\\\"",    NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      "\\/",
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      "\\\\",    NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      "\\u007f",
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
    NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,
};

static void json_encode_exception(lua_State* l, json_config_t* cfg, strbuf_t* json, int lindex,
                                  const char* reason) {
    (void)cfg;
    (void)json;
    luaL_error(l, "Cannot serialise %s: %s", lua_typename(l, lua_type(l, lindex)), reason);
}

/* json_append_string args:
 * - lua_State
 * - JSON strbuf
 * - String (Lua stack index)
 *
 * Returns nothing. Doesn't remove string from Lua stack */
static void json_append_string(lua_State* l, strbuf_t* json, int lindex) {
    const char* escstr;
    int i;
    const char* str;
    size_t len;

    str = lua_tolstring(l, lindex, &len);

    /* Worst case is len * 6 (all unicode escapes).
     * This buffer is reused constantly for small strings
     * If there are any excess pages, they won't be hit anyway.
     * This gains ~5% speedup. */
    strbuf_ensure_empty_length(json, len * 6 + 2);

    strbuf_append_char_unsafe(json, '\"');
    for (i = 0; static_cast<std::size_t>(i) < len; i++) {
        escstr = char2escape[(unsigned char)str[i]];
        if (escstr)
            strbuf_append_string(json, escstr);
        else
            strbuf_append_char_unsafe(json, str[i]);
    }
    strbuf_append_char_unsafe(json, '\"');
}

/* Find the size of the array on the top of the Lua stack
 * -1   object (not a pure array)
 * >=0  elements in array
 */
static int lua_array_length(lua_State* l, json_config_t* cfg, strbuf_t* json) {
    double k;
    int max;
    int items;

    max = 0;
    items = 0;

    lua_pushnil(l);
    /* table, startkey */
    while (lua_next(l, -2) != 0) {
        /* table, key, value */
        if (lua_type(l, -2) == LUA_TNUMBER && (k = lua_tonumber(l, -2))) {
            /* Integer >= 1 ? */
            if (floor(k) == k && k >= 1) {
                if (k > max)
                    max = _mm_cvttsd_si32(_mm_set_sd(k));
                items++;
                lua_pop(l, 1);
                continue;
            }
        }

        /* Must not be an array (non integer key) */
        lua_pop(l, 2);
        return -1;
    }

    /* Encode excessively sparse arrays as objects (if enabled) */
    const auto sparse_limit = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(items) * static_cast<std::uint32_t>(cfg->encode_sparse_ratio));
    if (cfg->encode_sparse_ratio > 0 && max > sparse_limit && max > cfg->encode_sparse_safe) {
        if (!cfg->encode_sparse_convert)
            json_encode_exception(l, cfg, json, -1, "excessively sparse array");

        return -1;
    }

    return max;
}

static void json_check_encode_depth(lua_State* l, json_config_t* cfg, int current_depth,
                                    strbuf_t* json) {
    (void)json;
    /* Ensure there are enough slots free to traverse a table (key,
     * value) and push a string for a potential error message.
     *
     * Unlike "decode", the key and value are still on the stack when
     * lua_checkstack() is called.  Hence an extra slot for luaL_error()
     * below is required just in case the next check to lua_checkstack()
     * fails.
     *
     * While this won't cause a crash due to the EXTRA_STACK reserve
     * slots, it would still be an improper use of the API. */
    if (current_depth <= cfg->encode_max_depth && lua_checkstack(l, 3))
        return;

    luaL_error(l, "Cannot serialise, excessive nesting (%d)", current_depth);
}

static void json_append_data(lua_State* l, json_config_t* cfg, int current_depth, strbuf_t* json);

/* json_append_array args:
 * - lua_State
 * - JSON strbuf
 * - Size of passwd Lua array (top of stack) */
static void json_append_array(lua_State* l, json_config_t* cfg, int current_depth, strbuf_t* json,
                              int array_length) {
    int comma, i;

    strbuf_append_char(json, '[');

    comma = 0;
    for (i = 1; i <= array_length; i++) {
        if (comma)
            strbuf_append_char(json, ',');
        else
            comma = 1;

        lua_rawgeti(l, -1, i);
        json_append_data(l, cfg, current_depth, json);
        lua_pop(l, 1);
    }

    strbuf_append_char(json, ']');
}

static void json_append_number(lua_State* l, json_config_t* cfg, strbuf_t* json, int lindex) {
    double num = lua_tonumber(l, lindex);
    int len;

    if (cfg->encode_invalid_numbers == 0) {
        /* Prevent encoding invalid numbers */
        if (isinf(num) || isnan(num))
            json_encode_exception(l, cfg, json, lindex, "must not be NaN or Infinity");
    } else if (cfg->encode_invalid_numbers == 1) {
        /* Encode invalid numbers, but handle "nan" separately
         * since some platforms may encode as "-nan". */
        if (isnan(num)) {
            strbuf_append_mem(json, "NaN", 3);
            return;
        }
        if (isinf(num)) {
            if (num < 0)
                strbuf_append_mem(json, "-Infinity", 9);
            else
                strbuf_append_mem(json, "Infinity", 8);
            return;
        }
    } else {
        /* Encode invalid numbers as "null" */
        if (isinf(num) || isnan(num)) {
            strbuf_append_mem(json, "null", 4);
            return;
        }
    }

    strbuf_ensure_empty_length(json, FPCONV_G_FMT_BUFSIZE);
    len = fpconv_g_fmt(strbuf_empty_ptr(json), num, cfg->encode_number_precision);
    strbuf_extend_length(json, len);
}

static void json_append_object(lua_State* l, json_config_t* cfg, int current_depth,
                               strbuf_t* json) {
    (void)json;
    int comma, keytype;

    /* Object */
    strbuf_append_char(json, '{');

    lua_pushnil(l);
    /* table, startkey */
    comma = 0;
    while (lua_next(l, -2) != 0) {
        if (comma)
            strbuf_append_char(json, ',');
        else
            comma = 1;

        /* table, key, value */
        keytype = lua_type(l, -2);
        if (keytype == LUA_TNUMBER) {
            strbuf_append_char(json, '"');
            json_append_number(l, cfg, json, -2);
            strbuf_append_mem(json, "\":", 2);
        } else if (keytype == LUA_TSTRING) {
            json_append_string(l, json, -2);
            strbuf_append_char(json, ':');
        } else {
            json_encode_exception(l, cfg, json, -2, "table key must be a number or string");
            /* never returns */
        }

        /* table, key, value */
        json_append_data(l, cfg, current_depth, json);
        lua_pop(l, 1);
        /* table, key */
    }

    strbuf_append_char(json, '}');
}

/* Serialise Lua data into JSON string. */
static void json_append_data(lua_State* l, json_config_t* cfg, int current_depth, strbuf_t* json) {
    (void)json;
    int len;

    switch (lua_type(l, -1)) {
    case LUA_TSTRING:
        json_append_string(l, json, -1);
        break;
    case LUA_TNUMBER:
        json_append_number(l, cfg, json, -1);
        break;
    case LUA_TBOOLEAN:
        if (lua_toboolean(l, -1))
            strbuf_append_mem(json, "true", 4);
        else
            strbuf_append_mem(json, "false", 5);
        break;
    case LUA_TTABLE:
        current_depth++;
        json_check_encode_depth(l, cfg, current_depth, json);
        len = lua_array_length(l, cfg, json);
        if (len > 0)
            json_append_array(l, cfg, current_depth, json, len);
        else
            json_append_object(l, cfg, current_depth, json);
        break;
    case LUA_TNIL:
        strbuf_append_mem(json, "null", 4);
        break;
    case LUA_TLIGHTUSERDATA:
        if (lua_touserdata(l, -1) == NULL) {
            strbuf_append_mem(json, "null", 4);
            break;
        }
    default:
        /* Remaining types (LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD,
         * and LUA_TLIGHTUSERDATA) cannot be serialised */
        json_encode_exception(l, cfg, json, -1, "type not supported");
        /* never returns */
    }
}

int json_encode(lua_State* l) {
    json_config_t* cfg = json_fetch_config(l);
    luaL_argcheck(l, lua_gettop(l) == 1, 1, "expected 1 argument");
    strbuf_reset(&cfg->encode_buf);
    json_append_data(l, cfg, 0, &cfg->encode_buf);
    lua_pushlstring(l, cfg->encode_buf.buf, cfg->encode_buf.length);
    return 1;
}

}
