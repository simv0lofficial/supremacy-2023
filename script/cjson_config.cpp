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
 * Native bindings and local changes are checked against 433C8D0F and its
 * eight exported methods; no unique upstream release marker survives. */
#include "cjson_internal.hpp"

namespace skeet::script::cjson_detail {
#define DEFAULT_SPARSE_CONVERT 0
#define DEFAULT_SPARSE_RATIO 2
#define DEFAULT_SPARSE_SAFE 10
#define DEFAULT_ENCODE_MAX_DEPTH 1000
#define DEFAULT_DECODE_MAX_DEPTH 1000
#define DEFAULT_ENCODE_INVALID_NUMBERS 0
#define DEFAULT_DECODE_INVALID_NUMBERS 1
#define DEFAULT_ENCODE_NUMBER_PRECISION 14

json_config_t* json_fetch_config(lua_State* l) {
    json_config_t* cfg;

    cfg = static_cast<json_config_t*>(lua_touserdata(l, lua_upvalueindex(1)));
    if (!cfg)
        luaL_error(l, "BUG: Unable to fetch CJSON configuration");

    return cfg;
}

/* Ensure the correct number of arguments have been provided.
 * Pad with nil to allow other functions to simply check arg[i]
 * to find whether an argument was provided */
static json_config_t* json_arg_init(lua_State* l, int args) {
    luaL_argcheck(l, lua_gettop(l) <= args, args + 1, "found too many arguments");

    while (lua_gettop(l) < args)
        lua_pushnil(l);

    return json_fetch_config(l);
}

/* Process integer options for configuration functions */
static int json_integer_option(lua_State* l, int optindex, int* setting, int min, int max) {
    char errmsg[64];
    int value;

    if (!lua_isnil(l, optindex)) {
        value = luaL_checkinteger(l, optindex);
        sprintf_s(errmsg, sizeof(errmsg), "expected integer between %d and %d", min, max);
        luaL_argcheck(l, min <= value && value <= max, 1, errmsg);
        *setting = value;
    }

    lua_pushinteger(l, *setting);

    return 1;
}

/* Process enumerated arguments for a configuration function */
static int json_enum_option(lua_State* l, int optindex, int* setting, const char** options,
                            int bool_true) {
    static const char* bool_options[] = {"off", "on", NULL};

    if (!options) {
        options = bool_options;
        bool_true = 1;
    }

    if (!lua_isnil(l, optindex)) {
        if (bool_true && lua_isboolean(l, optindex))
            *setting = lua_toboolean(l, optindex) * bool_true;
        else
            *setting = luaL_checkoption(l, optindex, NULL, options);
    }

    if (bool_true && (*setting == 0 || *setting == bool_true))
        lua_pushboolean(l, *setting);
    else
        lua_pushstring(l, options[*setting]);

    return 1;
}

/* Configures handling of extremely sparse arrays:
 * convert: Convert extremely sparse arrays into objects? Otherwise error.
 * ratio: 0: always allow sparse; 1: never allow sparse; >1: use ratio
 * safe: Always use an array when the max index <= safe */
int json_cfg_encode_sparse_array(lua_State* l) {
    json_config_t* cfg = json_arg_init(l, 3);

    json_enum_option(l, 1, &cfg->encode_sparse_convert, NULL, 1);
    json_integer_option(l, 2, &cfg->encode_sparse_ratio, 0, INT_MAX);
    json_integer_option(l, 3, &cfg->encode_sparse_safe, 0, INT_MAX);

    return 3;
}

/* Configures the maximum number of nested arrays/objects allowed when
 * encoding */
int json_cfg_encode_max_depth(lua_State* l) {
    json_config_t* cfg = json_arg_init(l, 1);

    return json_integer_option(l, 1, &cfg->encode_max_depth, 1, INT_MAX);
}

/* Configures the maximum number of nested arrays/objects allowed when
 * encoding */
int json_cfg_decode_max_depth(lua_State* l) {
    json_config_t* cfg = json_arg_init(l, 1);

    return json_integer_option(l, 1, &cfg->decode_max_depth, 1, INT_MAX);
}

/* Configures number precision when converting doubles to text */
int json_cfg_encode_number_precision(lua_State* l) {
    json_config_t* cfg = json_arg_init(l, 1);

    return json_integer_option(l, 1, &cfg->encode_number_precision, 1, 14);
}

int json_cfg_encode_invalid_numbers(lua_State* l) {
    static const char* options[] = {"off", "on", "null", NULL};
    json_config_t* cfg = json_arg_init(l, 1);

    json_enum_option(l, 1, &cfg->encode_invalid_numbers, options, 1);

    return 1;
}

int json_cfg_decode_invalid_numbers(lua_State* l) {
    json_config_t* cfg = json_arg_init(l, 1);

    json_enum_option(l, 1, &cfg->decode_invalid_numbers, NULL, 1);

    return 1;
}

static int json_destroy_config(lua_State* l) {
    json_config_t* cfg;

    cfg = static_cast<json_config_t*>(lua_touserdata(l, 1));
    if (cfg)
        strbuf_free(&cfg->encode_buf);
    cfg = NULL;

    return 0;
}

void json_create_config(lua_State* l) {
    json_config_t* cfg;
    int i;

    cfg = static_cast<json_config_t*>(lua_newuserdata(l, sizeof(*cfg)));
    std::memset(cfg, 0, sizeof(*cfg));

    /* Create GC method to clean up strbuf */
    lua_newtable(l);
    lua_pushcfunction(l, json_destroy_config);
    lua_setfield(l, -2, "__gc");
    lua_setmetatable(l, -2);

    cfg->encode_sparse_convert = DEFAULT_SPARSE_CONVERT;
    cfg->encode_sparse_ratio = DEFAULT_SPARSE_RATIO;
    cfg->encode_sparse_safe = DEFAULT_SPARSE_SAFE;
    cfg->encode_max_depth = DEFAULT_ENCODE_MAX_DEPTH;
    cfg->decode_max_depth = DEFAULT_DECODE_MAX_DEPTH;
    cfg->encode_invalid_numbers = DEFAULT_ENCODE_INVALID_NUMBERS;
    cfg->decode_invalid_numbers = DEFAULT_DECODE_INVALID_NUMBERS;
    cfg->encode_number_precision = DEFAULT_ENCODE_NUMBER_PRECISION;

    strbuf_init(&cfg->encode_buf, 0);

    /* Decoding init */

    /* Tag all characters as an error */
    for (i = 0; i < 256; i++)
        cfg->ch2token[i] = T_ERROR;

    /* Set tokens that require no further processing */
    cfg->ch2token[static_cast<unsigned char>('{')] = T_OBJ_BEGIN;
    cfg->ch2token[static_cast<unsigned char>('}')] = T_OBJ_END;
    cfg->ch2token[static_cast<unsigned char>('[')] = T_ARR_BEGIN;
    cfg->ch2token[static_cast<unsigned char>(']')] = T_ARR_END;
    cfg->ch2token[static_cast<unsigned char>(',')] = T_COMMA;
    cfg->ch2token[static_cast<unsigned char>(':')] = T_COLON;
    cfg->ch2token[static_cast<unsigned char>('\0')] = T_END;
    cfg->ch2token[static_cast<unsigned char>(' ')] = T_WHITESPACE;
    cfg->ch2token[static_cast<unsigned char>('\t')] = T_WHITESPACE;
    cfg->ch2token[static_cast<unsigned char>('\n')] = T_WHITESPACE;
    cfg->ch2token[static_cast<unsigned char>('\r')] = T_WHITESPACE;

    /* Update characters that require further processing */
    cfg->ch2token[static_cast<unsigned char>('f')] = T_UNKNOWN; /* false? */
    cfg->ch2token[static_cast<unsigned char>('i')] = T_UNKNOWN; /* inf, ininity? */
    cfg->ch2token[static_cast<unsigned char>('I')] = T_UNKNOWN;
    cfg->ch2token[static_cast<unsigned char>('n')] = T_UNKNOWN; /* null, nan? */
    cfg->ch2token[static_cast<unsigned char>('N')] = T_UNKNOWN;
    cfg->ch2token[static_cast<unsigned char>('t')] = T_UNKNOWN; /* true? */
    cfg->ch2token[static_cast<unsigned char>('"')] = T_UNKNOWN; /* string? */
    cfg->ch2token[static_cast<unsigned char>('+')] = T_UNKNOWN; /* number? */
    cfg->ch2token[static_cast<unsigned char>('-')] = T_UNKNOWN;
    for (i = 0; i < 10; i++)
        cfg->ch2token['0' + i] = T_UNKNOWN;

    /* Lookup table for parsing escape characters */
    for (i = 0; i < 256; i++)
        cfg->escape2char[i] = 0; /* String error */
    cfg->escape2char[static_cast<unsigned char>('"')] = '"';
    cfg->escape2char[static_cast<unsigned char>('\\')] = '\\';
    cfg->escape2char[static_cast<unsigned char>('/')] = '/';
    cfg->escape2char[static_cast<unsigned char>('b')] = '\b';
    cfg->escape2char[static_cast<unsigned char>('t')] = '\t';
    cfg->escape2char[static_cast<unsigned char>('n')] = '\n';
    cfg->escape2char[static_cast<unsigned char>('f')] = '\f';
    cfg->escape2char[static_cast<unsigned char>('r')] = '\r';
    cfg->escape2char[static_cast<unsigned char>('u')] = 'u'; /* Unicode parsing required */
}

}
