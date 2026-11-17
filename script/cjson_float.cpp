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
/* Lua CJSON assumes the locale is the same for all threads within a
 * process and doesn't change after initialisation.
 *
 * This avoids the need for per thread storage or expensive checks
 * for call. */
static char locale_decimal_point = '.';

/* In theory multibyte decimal_points are possible, but
 * Lua CJSON only supports UTF-8 and known locales only have
 * single byte decimal points ([.,]).
 *
 * localconv() may not be thread safe (=>crash), and nl_langinfo() is
 * not supported on some platforms. Use sprintf() instead - if the
 * locale does change, at least Lua CJSON won't crash. */
static void fpconv_update_locale() {
    char buf[8];

    sprintf_s(buf, sizeof(buf), "%g", 0.5);

    /* Failing this test might imply the platform has a buggy dtoa
     * implementation or wide characters */
    if (buf[0] != '0' || buf[2] != '5' || buf[3] != 0) {
        terminate_json_process(0);
    }

    locale_decimal_point = buf[1];
}

/* "fmt" must point to a buffer of at least 6 characters */
static void set_number_format(char* fmt, int precision) {
    int d1, d2, i;

    assert(1 <= precision && precision <= 14);

    /* Create printf format (%.14g) from precision */
    d1 = precision / 10;
    d2 = precision % 10;
    fmt[0] = '%';
    fmt[1] = '.';
    i = 2;
    if (d1) {
        fmt[i++] = '0' + d1;
    }
    fmt[i++] = '0' + d2;
    fmt[i++] = 'g';
    fmt[i] = 0;
}

/* Assumes there is always at least 32 characters available in the target buffer */
int fpconv_g_fmt(char* str, double num, int precision) {
    char buf[FPCONV_G_FMT_BUFSIZE];
    char fmt[6];
    int len;
    char* b;

    set_number_format(fmt, precision);

    /* Pass through when decimal point character is dot. */
    if (locale_decimal_point == '.')
        return sprintf_s(str, FPCONV_G_FMT_BUFSIZE, fmt, num);

    /* snprintf() to a buffer then translate for other decimal point characters */
    len = sprintf_s(buf, FPCONV_G_FMT_BUFSIZE, fmt, num);

    /* Copy into target location. Translate decimal point if required */
    b = buf;
    do {
        *str++ = (*b == locale_decimal_point ? '.' : *b);
    } while (*b++);

    return len;
}

void fpconv_init() {
    fpconv_update_locale();
}

/* vi:ai et sw=4 ts=4:
 */

}
