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

namespace skeet::script::cjson_detail {
static const char* json_token_type_name[] = {
    "T_OBJ_BEGIN", "T_OBJ_END",    "T_ARR_BEGIN", "T_ARR_END", "T_STRING",
    "T_NUMBER",    "T_BOOLEAN",    "T_NULL",      "T_COLON",   "T_COMMA",
    "T_END",       "T_WHITESPACE", "T_ERROR",     "T_UNKNOWN", NULL};

static void json_process_value(lua_State* l, json_parse_t* json, json_token_t* token);

static int hexdigit2int(char hex) {
    if ('0' <= hex && hex <= '9')
        return hex - '0';

    /* Force lowercase */
    hex |= 0x20;
    if ('a' <= hex && hex <= 'f')
        return 10 + hex - 'a';

    return -1;
}

static int decode_hex4(const char* hex) {
    int digit[4];
    int i;

    /* Convert ASCII hex digit to numeric digit
     * Note: this returns an error for invalid hex digits, including
     *       NULL */
    for (i = 0; i < 4; i++) {
        digit[i] = hexdigit2int(hex[i]);
        if (digit[i] < 0) {
            return -1;
        }
    }

    return (digit[0] << 12) + (digit[1] << 8) + (digit[2] << 4) + digit[3];
}

/* Converts a Unicode codepoint to UTF-8.
 * Returns UTF-8 string length, and up to 4 bytes in *utf8 */
static int codepoint_to_utf8(char* utf8, int codepoint) {
    /* 0xxxxxxx */
    if (codepoint <= 0x7F) {
        utf8[0] = codepoint;
        return 1;
    }

    /* 110xxxxx 10xxxxxx */
    if (codepoint <= 0x7FF) {
        utf8[0] = (codepoint >> 6) | 0xC0;
        utf8[1] = (codepoint & 0x3F) | 0x80;
        return 2;
    }

    /* 1110xxxx 10xxxxxx 10xxxxxx */
    if (codepoint <= 0xFFFF) {
        utf8[0] = (codepoint >> 12) | 0xE0;
        utf8[1] = ((codepoint >> 6) & 0x3F) | 0x80;
        utf8[2] = (codepoint & 0x3F) | 0x80;
        return 3;
    }

    /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint <= 0x1FFFFF) {
        utf8[0] = (codepoint >> 18) | 0xF0;
        utf8[1] = ((codepoint >> 12) & 0x3F) | 0x80;
        utf8[2] = ((codepoint >> 6) & 0x3F) | 0x80;
        utf8[3] = (codepoint & 0x3F) | 0x80;
        return 4;
    }

    return 0;
}

/* Called when index pointing to beginning of UTF-16 code escape: \uXXXX
 * \u is guaranteed to exist, but the remaining hex characters may be
 * missing.
 * Translate to UTF-8 and append to temporary token string.
 * Must advance index to the next character to be processed.
 * Returns: 0   success
 *          -1  error
 */
static int json_append_unicode_escape(json_parse_t* json) {
    char utf8[4]; /* Surrogate pairs require 4 UTF-8 bytes */
    int codepoint;
    int surrogate_low;
    int len;
    int escape_len = 6;

    /* Fetch UTF-16 code unit */
    codepoint = decode_hex4(json->ptr + 2);
    if (codepoint < 0)
        return -1;

    /* UTF-16 surrogate pairs take the following 2 byte form:
     *      11011 x yyyyyyyyyy
     * When x = 0: y is the high 10 bits of the codepoint
     *      x = 1: y is the low 10 bits of the codepoint
     *
     * Check for a surrogate pair (high or low) */
    if ((codepoint & 0xF800) == 0xD800) {
        /* Error if the 1st surrogate is not high */
        if (codepoint & 0x400)
            return -1;

        /* Ensure the next code is a unicode escape */
        if (*(json->ptr + escape_len) != '\\' || *(json->ptr + escape_len + 1) != 'u') {
            return -1;
        }

        /* Fetch the next codepoint */
        surrogate_low = decode_hex4(json->ptr + 2 + escape_len);
        if (surrogate_low < 0)
            return -1;

        /* Error if the 2nd code is not a low surrogate */
        if ((surrogate_low & 0xFC00) != 0xDC00)
            return -1;

        /* Calculate Unicode codepoint */
        codepoint = (codepoint & 0x3FF) << 10;
        surrogate_low &= 0x3FF;
        codepoint = (codepoint | surrogate_low) + 0x10000;
        escape_len = 12;
    }

    /* Convert codepoint to UTF-8 */
    len = codepoint_to_utf8(utf8, codepoint);
    if (!len)
        return -1;

    /* Append bytes and advance parse index */
    strbuf_append_mem_unsafe(json->tmp, utf8, len);
    json->ptr += escape_len;

    return 0;
}

static void json_set_token_error(json_token_t* token, json_parse_t* json, const char* errtype) {
    token->type = T_ERROR;
    token->index = json->ptr - json->data;
    token->value.string = errtype;
}

static void json_next_string_token(json_parse_t* json, json_token_t* token) {
    char* escape2char = json->cfg->escape2char;
    char ch;

    /* Caller must ensure a string is next */
    assert(*json->ptr == '"');

    /* Skip " */
    json->ptr++;

    /* json->tmp is the temporary strbuf used to accumulate the
     * decoded string value.
     * json->tmp is sized to handle JSON containing only a string value.
     */
    strbuf_reset(json->tmp);

    while ((ch = *json->ptr) != '"') {
        if (!ch) {
            /* Premature end of the string */
            json_set_token_error(token, json, "unexpected end of string");
            return;
        }

        /* Handle escapes */
        if (ch == '\\') {
            /* Fetch escape character */
            ch = *(json->ptr + 1);

            /* Translate escape code and append to tmp string */
            ch = escape2char[(unsigned char)ch];
            if (ch == 'u') {
                if (json_append_unicode_escape(json) == 0)
                    continue;

                json_set_token_error(token, json, "invalid unicode escape code");
                return;
            }
            if (!ch) {
                json_set_token_error(token, json, "invalid escape code");
                return;
            }

            /* Skip '\' */
            json->ptr++;
        }
        /* Append normal character or translated single character
         * Unicode escapes are handled above */
        strbuf_append_char_unsafe(json->tmp, ch);
        json->ptr++;
    }
    json->ptr++; /* Eat final quote (") */

    strbuf_ensure_null(json->tmp);

    token->type = T_STRING;
    token->value.string = strbuf_string(json->tmp, &token->string_len);
}

/* JSON numbers should take the following form:
 *      -?(0|[1-9]|[1-9][0-9]+)(.[0-9]+)?([eE][-+]?[0-9]+)?
 *
 * The client replaces upstream strtod() with its bundled LuaJIT numeric
 * scanner. It accepts these additional forms when configured:
 * - numbers starting with '+'
 * - NaN, -NaN, infinity, -infinity
 * - hexadecimal numbers
 * - numbers with leading zeros
 *
 * json_is_invalid_number() detects "numbers" which may pass the scanner's
 * error checking, but should not be allowed with strict JSON.
 *
 * json_is_invalid_number() may pass numbers which cause the scanner
 * to generate an error.
 */
static int json_is_invalid_number(json_parse_t* json) {
    const char* p = json->ptr;

    /* Reject numbers starting with + */
    if (*p == '+')
        return 1;

    /* Skip minus sign if it exists */
    if (*p == '-')
        p++;

    /* Reject numbers starting with 0x, or leading zeros */
    if (*p == '0') {
        int ch2 = *(p + 1);

        if ((ch2 | 0x20) == 'x' || /* Hex */
            ('0' <= ch2 && ch2 <= '9')) /* Leading zero */
            return 1;

        return 0;
    } else if (*p <= '9') {
        return 0; /* Ordinary number */
    }

    /* Reject inf/nan */
    if (!_strnicmp(p, "inf", 3))
        return 1;
    if (!_strnicmp(p, "nan", 3))
        return 1;

    /* Pass all other numbers which may still be invalid, but
     * numeric conversion will catch them. */
    return 0;
}

static void json_next_number_token(json_parse_t* json, json_token_t* token) {
    // Native 43394FF7 indexes the LuaJIT character table with MOVSX. Preserve
    // the complete byte predicate, including that client's signed-byte lookup.
    static constexpr unsigned char identifier_bits[32] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xfe, 0xff, 0xff,
        0x87, 0xfe, 0xff, 0xff, 0x07, 0xe3, 0x59, 0x8d, 0x01, 0x50, 0x80,
        0x02, 0x18, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    const char* end = json->ptr;
    unsigned char previous = *end;
    if (previous == '-' || previous == '+')
        previous = *++end;
    unsigned char exponent = 'e';
    if (previous == '0') {
        ++end;
        if ((*end | 0x20) == 'x')
            exponent = 'p';
    }
    for (;;) {
        const auto ch = static_cast<unsigned char>(*end);
        if (!(identifier_bits[ch >> 3] & (1u << (ch & 7))) && ch != '.' &&
            !((ch == '-' || ch == '+') && (previous | 0x20) == exponent))
            break;
        previous = ch;
        ++end;
    }
    char number[64];
    const auto length = end - json->ptr < 63 ? end - json->ptr : 63;
    std::memcpy(number, json->ptr, length);
    number[length] = 0;
    if (end != json->ptr && json_scan_number(number, &token->value.number)) {
        json->ptr = end;
        token->type = T_NUMBER;
        return;
    }
    json_set_token_error(token, json, "invalid number");
}

/* Fills in the token struct.
 * T_STRING will return a pointer to the json_parse_t temporary string
 * T_ERROR will leave the json->ptr pointer at the error.
 */
static void json_next_token(json_parse_t* json, json_token_t* token) {
    const json_token_type_t* ch2token = json->cfg->ch2token;
    int ch;

    /* Eat whitespace. */
    while (1) {
        ch = (unsigned char)*(json->ptr);
        token->type = ch2token[ch];
        if (token->type != T_WHITESPACE)
            break;
        json->ptr++;
    }

    /* Store location of new token. Required when throwing errors
     * for unexpected tokens (syntax errors). */
    token->index = json->ptr - json->data;

    /* Don't advance the pointer for an error or the end */
    if (token->type == T_ERROR) {
        json_set_token_error(token, json, "invalid token");
        return;
    }

    if (token->type == T_END) {
        return;
    }

    /* Found a known single character token, advance index and return */
    if (token->type != T_UNKNOWN) {
        json->ptr++;
        return;
    }

    /* Process characters which triggered T_UNKNOWN
     *
     * Must use strncmp() to match the front of the JSON string.
     * JSON identifier must be lowercase.
     * When strict_numbers if disabled, either case is allowed for
     * Infinity/NaN (since we are no longer following the spec..) */
    if (ch == '"') {
        json_next_string_token(json, token);
        return;
    } else if (ch == '-' || ('0' <= ch && ch <= '9')) {
        if (!json->cfg->decode_invalid_numbers && json_is_invalid_number(json)) {
            json_set_token_error(token, json, "invalid number");
            return;
        }
        json_next_number_token(json, token);
        return;
    } else if (!strncmp(json->ptr, "true", 4)) {
        token->type = T_BOOLEAN;
        token->value.boolean = 1;
        json->ptr += 4;
        return;
    } else if (!strncmp(json->ptr, "false", 5)) {
        token->type = T_BOOLEAN;
        token->value.boolean = 0;
        json->ptr += 5;
        return;
    } else if (!strncmp(json->ptr, "null", 4)) {
        token->type = T_NULL;
        json->ptr += 4;
        return;
    } else if (json->cfg->decode_invalid_numbers && json_is_invalid_number(json)) {
        /* When decode_invalid_numbers is enabled, only attempt to process
         * numbers we know are invalid JSON (Inf, NaN, hex)
         * This is required to generate an appropriate token error,
         * otherwise all bad tokens will register as "invalid number"
         */
        json_next_number_token(json, token);
        return;
    }

    /* Token starts with t/f/n but isn't recognised above. */
    json_set_token_error(token, json, "invalid token");
}

/* This function does not return.
 * DO NOT CALL WITH DYNAMIC MEMORY ALLOCATED.
 * The only supported exception is the temporary parser string
 * json->tmp struct.
 * json and token should exist on the stack somewhere.
 * luaL_error() will long_jmp and release the stack */
static void json_throw_parse_error(lua_State* l, json_parse_t* json, const char* exp,
                                   json_token_t* token) {
    const char* found;

    strbuf_free(json->tmp);

    if (token->type == T_ERROR)
        found = token->value.string;
    else
        found = json_token_type_name[token->type];

    /* Note: token->index is 0 based, display starting from 1 */
    luaL_error(l, "Expected %s but found %s at character %d", exp, found, token->index + 1);
}

static inline void json_decode_ascend(json_parse_t* json) {
    json->current_depth--;
}

static void json_decode_descend(lua_State* l, json_parse_t* json, int slots) {
    json->current_depth++;

    if (json->current_depth <= json->cfg->decode_max_depth && lua_checkstack(l, slots)) {
        return;
    }

    strbuf_free(json->tmp);
    luaL_error(l, "Found too many nested data structures (%d) at character %d", json->current_depth,
               json->ptr - json->data);
}

static void json_parse_object_context(lua_State* l, json_parse_t* json) {
    json_token_t token;

    /* 3 slots required:
     * .., table, key, value */
    json_decode_descend(l, json, 3);

    lua_newtable(l);

    json_next_token(json, &token);

    /* Handle empty objects */
    if (token.type == T_OBJ_END) {
        json_decode_ascend(json);
        return;
    }

    while (1) {
        if (token.type != T_STRING)
            json_throw_parse_error(l, json, "object key string", &token);

        /* Push key */
        lua_pushlstring(l, token.value.string, token.string_len);

        json_next_token(json, &token);
        if (token.type != T_COLON)
            json_throw_parse_error(l, json, "colon", &token);

        /* Fetch value */
        json_next_token(json, &token);
        json_process_value(l, json, &token);

        /* Set key = value */
        lua_rawset(l, -3);

        json_next_token(json, &token);

        if (token.type == T_OBJ_END) {
            json_decode_ascend(json);
            return;
        }

        if (token.type != T_COMMA)
            json_throw_parse_error(l, json, "comma or object end", &token);

        json_next_token(json, &token);
    }
}

/* Handle the array context */
static void json_parse_array_context(lua_State* l, json_parse_t* json) {
    json_token_t token;
    int i;

    /* 2 slots required:
     * .., table, value */
    json_decode_descend(l, json, 2);

    lua_newtable(l);

    json_next_token(json, &token);

    /* Handle empty arrays */
    if (token.type == T_ARR_END) {
        json_decode_ascend(json);
        return;
    }

    for (i = 1;; i++) {
        json_process_value(l, json, &token);
        lua_rawseti(l, -2, i); /* arr[i] = value */

        json_next_token(json, &token);

        if (token.type == T_ARR_END) {
            json_decode_ascend(json);
            return;
        }

        if (token.type != T_COMMA)
            json_throw_parse_error(l, json, "comma or array end", &token);

        json_next_token(json, &token);
    }
}

/* Handle the "value" context */
static void json_process_value(lua_State* l, json_parse_t* json, json_token_t* token) {
    switch (token->type) {
    case T_STRING:
        lua_pushlstring(l, token->value.string, token->string_len);
        break;
        ;
    case T_NUMBER:
        lua_pushnumber(l, token->value.number);
        break;
        ;
    case T_BOOLEAN:
        lua_pushboolean(l, token->value.boolean);
        break;
        ;
    case T_OBJ_BEGIN:
        json_parse_object_context(l, json);
        break;
        ;
    case T_ARR_BEGIN:
        json_parse_array_context(l, json);
        break;
        ;
    case T_NULL:
        /* In Lua, setting "t[k] = nil" will delete k from the table.
         * Hence a NULL pointer lightuserdata object is used instead */
        lua_pushlightuserdata(l, NULL);
        break;
        ;
    default:
        json_throw_parse_error(l, json, "value", token);
    }
}

int json_decode(lua_State* l) {
    json_parse_t json;
    json_token_t token;
    size_t json_len;

    luaL_argcheck(l, lua_gettop(l) == 1, 1, "expected 1 argument");

    json.cfg = json_fetch_config(l);
    json.data = luaL_checklstring(l, 1, &json_len);
    json.current_depth = 0;
    json.ptr = json.data;

    /* Detect Unicode other than UTF-8 (see RFC 4627, Sec 3)
     *
     * CJSON can support any simple data type, hence only the first
     * character is guaranteed to be ASCII (at worst: '"'). This is
     * still enough to detect whether the wrong encoding is in use. */
    if (json_len >= 2 && (!json.data[0] || !json.data[1]))
        luaL_error(l, "JSON parser does not support UTF-16 or UTF-32");

    /* Ensure the temporary buffer can hold the entire string.
     * This means we no longer need to do length checks since the decoded
     * string must be smaller than the entire json string */
    json.tmp = strbuf_new(json_len);

    json_next_token(&json, &token);
    json_process_value(l, &json, &token);

    /* Ensure there is no more input left */
    json_next_token(&json, &token);

    if (token.type != T_END)
        json_throw_parse_error(l, &json, "the end", &token);

    strbuf_free(json.tmp);

    return 1;
}

}
