/*
LuaJIT -- a Just-In-Time Compiler for Lua. http://luajit.org/

Copyright (C) 2005-2017 Mike Pall. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

[ MIT license: http://www.opensource.org/licenses/mit-license.php ]
*/

// The numeric core matches native 4342E495 and helpers43438AB3/4343930C/
// 43438986/43439493. Official LuaJIT snapshot71ff7ef8a7e0592dedb3527d4aacc68bf04af0bd:
// https://github.com/LuaJIT/LuaJIT/blob/71ff7ef8a7e0592dedb3527d4aacc68bf04af0bd/src/lj_strscan.c
// Original file SHA256 a7a3145a4a7fae60fe0d9cb7a7bfb3a7a2e12c0b600022a062e4242a34960f45.
// This fingerprints the scanner core; it does not identify the complete client
// LuaJIT release. C++ adaptations are namespace/types, fixed x86 character data,
// and explicit unsigned negation preserving the original wrapping operations.
#include "cjson_internal.hpp"
#include <bit>
#include <emmintrin.h>

namespace skeet::script::cjson_detail {
namespace {
using std::uint8_t;
using std::uint32_t;
using std::uint64_t;
using std::int32_t;
using std::int64_t;
using lua_Number = double;
union TValue {
    double n;
    int32_t i;
    uint64_t u64;
};
using StrScanFmt = int;
enum {
    STRSCAN_ERROR,
    STRSCAN_NUM,
    STRSCAN_IMAG,
    STRSCAN_INT,
    STRSCAN_U32,
    STRSCAN_I64,
    STRSCAN_U64
};
inline constexpr uint32_t STRSCAN_OPT_TOINT = 1, STRSCAN_OPT_TONUM = 2, STRSCAN_OPT_IMAG = 4,
                          STRSCAN_OPT_LL = 8, STRSCAN_OPT_C = 16;
inline void setnanV(TValue* o) {
    o->u64 = 0xfff8000000000000ULL;
}
inline void setminfV(TValue* o) {
    o->u64 = 0xfff0000000000000ULL;
}
inline void setpinfV(TValue* o) {
    o->u64 = 0x7ff0000000000000ULL;
}
inline uint32_t lj_fls(uint32_t value) {
    return std::bit_width(value) - 1;
}
inline int32_t lj_num2int(double value) {
    return _mm_cvttsd_si32(_mm_set_sd(value));
}
#define LJ_LIKELY(c) (c)
#define LJ_UNLIKELY(c) (c)
#define LJ_64 0
#define lua_assert(c) ((void)0)
#define U64x(hi, lo) ((uint64_t(0x##hi) << 32) | uint64_t(0x##lo))
#define LJ_CHAR_DIGIT 8
#define LJ_CHAR_XDIGIT 16
#define lj_char_isa(c, t) (char_bits[uint8_t(c)] & (t))
#define lj_char_isdigit(c) lj_char_isa(c, 8)
#define lj_char_isspace(c) lj_char_isa(c, 2)
static constexpr uint8_t char_bits[256] = {
    1,   1,   1,   1,   1,   1,   1,   1,   1,   3,   3,   3,   3,   3,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   4,   4,   4,   4,   4,
    4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   152, 152, 152, 152, 152, 152, 152, 152, 152,
    152, 4,   4,   4,   4,   4,   4,   4,   176, 176, 176, 176, 176, 176, 160, 160, 160, 160, 160,
    160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 4,   4,   4,   4,
    132, 4,   208, 208, 208, 208, 208, 208, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192, 192, 4,   4,   4,   4,   1,   128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128};

/* -- Scanning numbers ---------------------------------------------------- */

/*
** Rationale for the builtin string to number conversion library:
**
** It removes a dependency on libc's strtod(), which is a true portability
** nightmare. Mainly due to the plethora of supported OS and toolchain
** combinations. Sadly, the various implementations
** a) are often buggy, incomplete (no hex floats) and/or imprecise,
** b) sometimes crash or hang on certain inputs,
** c) return non-standard NaNs that need to be filtered out, and
** d) fail if the locale-specific decimal separator is not a dot,
**    which can only be fixed with atrocious workarounds.
**
** Also, most of the strtod() implementations are hopelessly bloated,
** which is not just an I-cache hog, but a problem for static linkage
** on embedded systems, too.
**
** OTOH the builtin conversion function is very compact. Even though it
** does a lot more, like parsing long longs, octal or imaginary numbers
** and returning the result in different formats:
** a) It needs less than 3 KB (!) of machine code (on x64 with -Os),
** b) it doesn't perform any dynamic allocation and,
** c) it needs only around 600 bytes of stack space.
**
** The builtin function is faster than strtod() for typical inputs, e.g.
** "123", "1.5" or "1e6". Arguably, it's slower for very large exponents,
** which are not very common (this could be fixed, if needed).
**
** And most importantly, the builtin function is equally precise on all
** platforms. It correctly converts and rounds any input to a double.
** If this is not the case, please send a bug report -- but PLEASE verify
** that the implementation you're comparing to is not the culprit!
**
** The implementation quickly pre-scans the entire string first and
** handles simple integers on-the-fly. Otherwise, it dispatches to the
** base-specific parser. Hex and octal is straightforward.
**
** Decimal to binary conversion uses a fixed-length circular buffer in
** base 100. Some simple cases are handled directly. For other cases, the
** number in the buffer is up-scaled or down-scaled until the integer part
** is in the proper range. Then the integer part is rounded and converted
** to a double which is finally rescaled to the result. Denormals need
** special treatment to prevent incorrect 'double rounding'.
*/

/* Definitions for circular decimal digit buffer (base 100 = 2 digits/byte). */
#define STRSCAN_DIG 1024
#define STRSCAN_MAXDIG 800 /* 772 + extra are sufficient. */
#define STRSCAN_DDIG (STRSCAN_DIG / 2)
#define STRSCAN_DMASK (STRSCAN_DDIG - 1)

/* Helpers for circular buffer. */
#define DNEXT(a) (((a) + 1) & STRSCAN_DMASK)
#define DPREV(a) (((a) - 1) & STRSCAN_DMASK)
#define DLEN(lo, hi) ((int32_t)(((lo) - (hi)) & STRSCAN_DMASK))

#define casecmp(c, k) (((c) | 0x20) == k)

/* Final conversion to double. */
static void strscan_double(uint64_t x, TValue* o, int32_t ex2, int32_t neg) {
    double n;

    /* Avoid double rounding for denormals. */
    if (LJ_UNLIKELY(ex2 <= -1075 && x != 0)) {
        /* NYI: all of this generates way too much code on 32 bit CPUs. */
#if defined(__GNUC__) && LJ_64
        int32_t b = (int32_t)(__builtin_clzll(x) ^ 63);
#else
        int32_t b =
            (x >> 32) ? 32 + (int32_t)lj_fls((uint32_t)(x >> 32)) : (int32_t)lj_fls((uint32_t)x);
#endif
        if ((int32_t)b + ex2 <= -1023 && (int32_t)b + ex2 >= -1075) {
            uint64_t rb = (uint64_t)1 << (-1075 - ex2);
            if ((x & rb) && ((x & (rb + rb + rb - 1))))
                x += rb + rb;
            x = (x & ~(rb + rb - 1));
        }
    }

    /* Convert to double using a signed int64_t conversion, then rescale. */
    lua_assert((int64_t)x >= 0);
    n = (double)(int64_t)x;
    if (neg)
        n = -n;
    if (ex2)
        n = ldexp(n, ex2);
    o->n = n;
}

/* Parse hexadecimal number. */
static StrScanFmt strscan_hex(const uint8_t* p, TValue* o, StrScanFmt fmt, uint32_t opt,
                              int32_t ex2, int32_t neg, uint32_t dig) {
    uint64_t x = 0;
    uint32_t i;

    /* Scan hex digits. */
    for (i = dig > 16 ? 16 : dig; i; i--, p++) {
        uint32_t d = (*p != '.' ? *p : *++p);
        if (d > '9')
            d += 9;
        x = (x << 4) + (d & 15);
    }

    /* Summarize rounding-effect of excess digits. */
    for (i = 16; i < dig; i++, p++)
        x |= ((*p != '.' ? *p : *++p) != '0'), ex2 += 4;

    /* Format-specific handling. */
    switch (fmt) {
    case STRSCAN_INT:
        if (!(opt & STRSCAN_OPT_TONUM) && x < 0x80000000u + neg) {
            o->i =
                neg ? static_cast<int32_t>(0u - static_cast<uint32_t>(x)) : static_cast<int32_t>(x);
            return STRSCAN_INT; /* Fast path for 32 bit integers. */
        }
        if (!(opt & STRSCAN_OPT_C)) {
            fmt = STRSCAN_NUM;
            break;
        }
        /* fallthrough */
    case STRSCAN_U32:
        if (dig > 8)
            return STRSCAN_ERROR;
        o->i = neg ? static_cast<int32_t>(0u - static_cast<uint32_t>(x)) : static_cast<int32_t>(x);
        return STRSCAN_U32;
    case STRSCAN_I64:
    case STRSCAN_U64:
        if (dig > 16)
            return STRSCAN_ERROR;
        o->u64 = neg ? uint64_t(0) - x : x;
        return fmt;
    default:
        break;
    }

    /* Reduce range, then convert to double. */
    if ((x & U64x(c0000000, 0000000))) {
        x = (x >> 2) | (x & 3);
        ex2 += 2;
    }
    strscan_double(x, o, ex2, neg);
    return fmt;
}

/* Parse octal number. */
static StrScanFmt strscan_oct(const uint8_t* p, TValue* o, StrScanFmt fmt, int32_t neg,
                              uint32_t dig) {
    uint64_t x = 0;

    /* Scan octal digits. */
    if (dig > 22 || (dig == 22 && *p > '1'))
        return STRSCAN_ERROR;
    while (dig-- > 0) {
        if (!(*p >= '0' && *p <= '7'))
            return STRSCAN_ERROR;
        x = (x << 3) + (*p++ & 7);
    }

    /* Format-specific handling. */
    switch (fmt) {
    case STRSCAN_INT:
        if (x >= 0x80000000u + neg)
            fmt = STRSCAN_U32;
        /* fallthrough */
    case STRSCAN_U32:
        if ((x >> 32))
            return STRSCAN_ERROR;
        o->i = neg ? static_cast<int32_t>(0u - static_cast<uint32_t>(x)) : static_cast<int32_t>(x);
        break;
    default:
    case STRSCAN_I64:
    case STRSCAN_U64:
        o->u64 = neg ? uint64_t(0) - x : x;
        break;
    }
    return fmt;
}

/* Parse decimal number. */
static StrScanFmt strscan_dec(const uint8_t* p, TValue* o, StrScanFmt fmt, uint32_t opt,
                              int32_t ex10, int32_t neg, uint32_t dig) {
    uint8_t xi[STRSCAN_DDIG], *xip = xi;

    if (dig) {
        uint32_t i = dig;
        if (i > STRSCAN_MAXDIG) {
            ex10 += (int32_t)(i - STRSCAN_MAXDIG);
            i = STRSCAN_MAXDIG;
        }
        /* Scan unaligned leading digit. */
        if (((ex10 ^ i) & 1))
            *xip++ = ((*p != '.' ? *p : *++p) & 15), i--, p++;
        /* Scan aligned double-digits. */
        for (; i > 1; i -= 2) {
            uint32_t d = 10 * ((*p != '.' ? *p : *++p) & 15);
            p++;
            *xip++ = d + ((*p != '.' ? *p : *++p) & 15);
            p++;
        }
        /* Scan and realign trailing digit. */
        if (i)
            *xip++ = 10 * ((*p != '.' ? *p : *++p) & 15), ex10--, dig++, p++;

        /* Summarize rounding-effect of excess digits. */
        if (dig > STRSCAN_MAXDIG) {
            do {
                if ((*p != '.' ? *p : *++p) != '0') {
                    xip[-1] |= 1;
                    break;
                }
                p++;
            } while (--dig > STRSCAN_MAXDIG);
            dig = STRSCAN_MAXDIG;
        } else { /* Simplify exponent. */
            while (ex10 > 0 && dig <= 18)
                *xip++ = 0, ex10 -= 2, dig += 2;
        }
    } else { /* Only got zeros. */
        ex10 = 0;
        xi[0] = 0;
    }

    /* Fast path for numbers in integer format (but handles e.g. 1e6, too). */
    if (dig <= 20 && ex10 == 0) {
        uint8_t* xis;
        uint64_t x = xi[0];
        double n;
        for (xis = xi + 1; xis < xip; xis++)
            x = x * 100 + *xis;
        if (!(dig == 20 && (xi[0] > 18 || (int64_t)x >= 0))) { /* No overflow? */
            /* Format-specific handling. */
            switch (fmt) {
            case STRSCAN_INT:
                if (!(opt & STRSCAN_OPT_TONUM) && x < 0x80000000u + neg) {
                    o->i = neg ? static_cast<int32_t>(0u - static_cast<uint32_t>(x))
                               : static_cast<int32_t>(x);
                    return STRSCAN_INT; /* Fast path for 32 bit integers. */
                }
                if (!(opt & STRSCAN_OPT_C)) {
                    fmt = STRSCAN_NUM;
                    goto plainnumber;
                }
                /* fallthrough */
            case STRSCAN_U32:
                if ((x >> 32) != 0)
                    return STRSCAN_ERROR;
                o->i = neg ? static_cast<int32_t>(0u - static_cast<uint32_t>(x))
                           : static_cast<int32_t>(x);
                return STRSCAN_U32;
            case STRSCAN_I64:
            case STRSCAN_U64:
                o->u64 = neg ? uint64_t(0) - x : x;
                return fmt;
            default:
            plainnumber: /* Fast path for plain numbers < 2^63. */
                if ((int64_t)x < 0)
                    break;
                n = (double)(int64_t)x;
                if (neg)
                    n = -n;
                o->n = n;
                return fmt;
            }
        }
    }

    /* Slow non-integer path. */
    if (fmt == STRSCAN_INT) {
        if ((opt & STRSCAN_OPT_C))
            return STRSCAN_ERROR;
        fmt = STRSCAN_NUM;
    } else if (fmt > STRSCAN_INT) {
        return STRSCAN_ERROR;
    }
    {
        uint32_t hi = 0, lo = (uint32_t)(xip - xi);
        int32_t ex2 = 0, idig = (int32_t)lo + (ex10 >> 1);

        lua_assert(lo > 0 && (ex10 & 1) == 0);

        /* Handle simple overflow/underflow. */
        if (idig > 310 / 2) {
            if (neg)
                setminfV(o);
            else
                setpinfV(o);
            return fmt;
        } else if (idig < -326 / 2) {
            o->n = neg ? -0.0 : 0.0;
            return fmt;
        }

        /* Scale up until we have at least 17 or 18 integer part digits. */
        while (idig < 9 && idig < DLEN(lo, hi)) {
            uint32_t i, cy = 0;
            ex2 -= 6;
            for (i = DPREV(lo);; i = DPREV(i)) {
                uint32_t d = (xi[i] << 6) + cy;
                cy = (((d >> 2) * 5243) >> 17);
                d = d - cy * 100; /* Div/mod 100. */
                xi[i] = (uint8_t)d;
                if (i == hi)
                    break;
                if (d == 0 && i == DPREV(lo))
                    lo = i;
            }
            if (cy) {
                hi = DPREV(hi);
                if (xi[DPREV(lo)] == 0)
                    lo = DPREV(lo);
                else if (hi == lo) {
                    lo = DPREV(lo);
                    xi[DPREV(lo)] |= xi[lo];
                }
                xi[hi] = (uint8_t)cy;
                idig++;
            }
        }

        /* Scale down until no more than 17 or 18 integer part digits remain. */
        while (idig > 9) {
            uint32_t i = hi, cy = 0;
            ex2 += 6;
            do {
                cy += xi[i];
                xi[i] = (cy >> 6);
                cy = 100 * (cy & 0x3f);
                if (xi[i] == 0 && i == hi)
                    hi = DNEXT(hi), idig--;
                i = DNEXT(i);
            } while (i != lo);
            while (cy) {
                if (hi == lo) {
                    xi[DPREV(lo)] |= 1;
                    break;
                }
                xi[lo] = (cy >> 6);
                lo = DNEXT(lo);
                cy = 100 * (cy & 0x3f);
            }
        }

        /* Collect integer part digits and convert to rescaled double. */
        {
            uint64_t x = xi[hi];
            uint32_t i;
            for (i = DNEXT(hi); --idig > 0 && i != lo; i = DNEXT(i))
                x = x * 100 + xi[i];
            if (i == lo) {
                while (--idig >= 0)
                    x = x * 100;
            } else { /* Gather round bit from remaining digits. */
                x <<= 1;
                ex2--;
                do {
                    if (xi[i]) {
                        x |= 1;
                        break;
                    }
                    i = DNEXT(i);
                } while (i != lo);
            }
            strscan_double(x, o, ex2, neg);
        }
    }
    return fmt;
}

/* Parse binary number. */
static StrScanFmt strscan_bin(const uint8_t* p, TValue* o, StrScanFmt fmt, uint32_t opt,
                              int32_t ex2, int32_t neg, uint32_t dig) {
    uint64_t x = 0;
    uint32_t i;

    if (ex2 || dig > 64)
        return STRSCAN_ERROR;

    /* Scan binary digits. */
    for (i = dig; i; i--, p++) {
        if ((*p & ~1) != '0')
            return STRSCAN_ERROR;
        x = (x << 1) | (*p & 1);
    }

    /* Format-specific handling. */
    switch (fmt) {
    case STRSCAN_INT:
        if (!(opt & STRSCAN_OPT_TONUM) && x < 0x80000000u + neg) {
            o->i =
                neg ? static_cast<int32_t>(0u - static_cast<uint32_t>(x)) : static_cast<int32_t>(x);
            return STRSCAN_INT; /* Fast path for 32 bit integers. */
        }
        if (!(opt & STRSCAN_OPT_C)) {
            fmt = STRSCAN_NUM;
            break;
        }
        /* fallthrough */
    case STRSCAN_U32:
        if (dig > 32)
            return STRSCAN_ERROR;
        o->i = neg ? static_cast<int32_t>(0u - static_cast<uint32_t>(x)) : static_cast<int32_t>(x);
        return STRSCAN_U32;
    case STRSCAN_I64:
    case STRSCAN_U64:
        o->u64 = neg ? uint64_t(0) - x : x;
        return fmt;
    default:
        break;
    }

    /* Reduce range, then convert to double. */
    if ((x & U64x(c0000000, 0000000))) {
        x = (x >> 2) | (x & 3);
        ex2 += 2;
    }
    strscan_double(x, o, ex2, neg);
    return fmt;
}

/* Scan string containing a number. Returns format. Returns value in o. */
static StrScanFmt scan(const uint8_t* p, TValue* o, uint32_t opt) {
    int32_t neg = 0;

    /* Remove leading space, parse sign and non-numbers. */
    if (LJ_UNLIKELY(!lj_char_isdigit(*p))) {
        while (lj_char_isspace(*p))
            p++;
        if (*p == '+' || *p == '-')
            neg = (*p++ == '-');
        if (LJ_UNLIKELY(*p >= 'A')) { /* Parse "inf", "infinity" or "nan". */
            TValue tmp;
            setnanV(&tmp);
            if (casecmp(p[0], 'i') && casecmp(p[1], 'n') && casecmp(p[2], 'f')) {
                if (neg)
                    setminfV(&tmp);
                else
                    setpinfV(&tmp);
                p += 3;
                if (casecmp(p[0], 'i') && casecmp(p[1], 'n') && casecmp(p[2], 'i') &&
                    casecmp(p[3], 't') && casecmp(p[4], 'y'))
                    p += 5;
            } else if (casecmp(p[0], 'n') && casecmp(p[1], 'a') && casecmp(p[2], 'n')) {
                p += 3;
            }
            while (lj_char_isspace(*p))
                p++;
            if (*p)
                return STRSCAN_ERROR;
            o->u64 = tmp.u64;
            return STRSCAN_NUM;
        }
    }

    /* Parse regular number. */
    {
        StrScanFmt fmt = STRSCAN_INT;
        int cmask = LJ_CHAR_DIGIT;
        int base = (opt & STRSCAN_OPT_C) && *p == '0' ? 0 : 10;
        const uint8_t *sp, *dp = NULL;
        uint32_t dig = 0, hasdig = 0, x = 0;
        int32_t ex = 0;

        /* Determine base and skip leading zeros. */
        if (LJ_UNLIKELY(*p <= '0')) {
            if (*p == '0') {
                if (casecmp(p[1], 'x'))
                    base = 16, cmask = LJ_CHAR_XDIGIT, p += 2;
                else if (casecmp(p[1], 'b'))
                    base = 2, cmask = LJ_CHAR_DIGIT, p += 2;
            }
            for (;; p++) {
                if (*p == '0') {
                    hasdig = 1;
                } else if (*p == '.') {
                    if (dp)
                        return STRSCAN_ERROR;
                    dp = p;
                } else {
                    break;
                }
            }
        }

        /* Preliminary digit and decimal point scan. */
        for (sp = p;; p++) {
            if (LJ_LIKELY(lj_char_isa(*p, cmask))) {
                x = x * 10 + (*p & 15); /* For fast path below. */
                dig++;
            } else if (*p == '.') {
                if (dp)
                    return STRSCAN_ERROR;
                dp = p;
            } else {
                break;
            }
        }
        if (!(hasdig | dig))
            return STRSCAN_ERROR;

        /* Handle decimal point. */
        if (dp) {
            fmt = STRSCAN_NUM;
            if (dig) {
                ex = (int32_t)(dp - (p - 1));
                dp = p - 1;
                while (ex < 0 && *dp-- == '0')
                    ex++, dig--; /* Skip trailing zeros. */
                if (base == 16)
                    ex *= 4;
            }
        }

        /* Parse exponent. */
        if (base >= 10 && casecmp(*p, (uint32_t)(base == 16 ? 'p' : 'e'))) {
            uint32_t xx;
            int negx = 0;
            fmt = STRSCAN_NUM;
            p++;
            if (*p == '+' || *p == '-')
                negx = (*p++ == '-');
            if (!lj_char_isdigit(*p))
                return STRSCAN_ERROR;
            xx = (*p++ & 15);
            while (lj_char_isdigit(*p)) {
                if (xx < 65536)
                    xx = xx * 10 + (*p & 15);
                p++;
            }
            ex += negx ? -(int32_t)xx : (int32_t)xx;
        }

        /* Parse suffix. */
        if (*p) {
            /* I (IMAG), U (U32), LL (I64), ULL/LLU (U64), L (long), UL/LU (ulong). */
            /* NYI: f (float). Not needed until cp_number() handles non-integers. */
            if (casecmp(*p, 'i')) {
                if (!(opt & STRSCAN_OPT_IMAG))
                    return STRSCAN_ERROR;
                p++;
                fmt = STRSCAN_IMAG;
            } else if (fmt == STRSCAN_INT) {
                if (casecmp(*p, 'u'))
                    p++, fmt = STRSCAN_U32;
                if (casecmp(*p, 'l')) {
                    p++;
                    if (casecmp(*p, 'l'))
                        p++, fmt += STRSCAN_I64 - STRSCAN_INT;
                    else if (!(opt & STRSCAN_OPT_C))
                        return STRSCAN_ERROR;
                    else if (sizeof(long) == 8)
                        fmt += STRSCAN_I64 - STRSCAN_INT;
                }
                if (casecmp(*p, 'u') && (fmt == STRSCAN_INT || fmt == STRSCAN_I64))
                    p++, fmt += STRSCAN_U32 - STRSCAN_INT;
                if ((fmt == STRSCAN_U32 && !(opt & STRSCAN_OPT_C)) ||
                    (fmt >= STRSCAN_I64 && !(opt & STRSCAN_OPT_LL)))
                    return STRSCAN_ERROR;
            }
            while (lj_char_isspace(*p))
                p++;
            if (*p)
                return STRSCAN_ERROR;
        }

        /* Fast path for decimal 32 bit integers. */
        if (fmt == STRSCAN_INT && base == 10 &&
            (dig < 10 || (dig == 10 && *sp <= '2' && x < 0x80000000u + neg))) {
            int32_t y =
                neg ? static_cast<int32_t>(0u - static_cast<uint32_t>(x)) : static_cast<int32_t>(x);
            if ((opt & STRSCAN_OPT_TONUM)) {
                o->n = (double)y;
                return STRSCAN_NUM;
            } else {
                o->i = y;
                return STRSCAN_INT;
            }
        }

        /* Dispatch to base-specific parser. */
        if (base == 0 && !(fmt == STRSCAN_NUM || fmt == STRSCAN_IMAG))
            return strscan_oct(sp, o, fmt, neg, dig);
        if (base == 16)
            fmt = strscan_hex(sp, o, fmt, opt, ex, neg, dig);
        else if (base == 2)
            fmt = strscan_bin(sp, o, fmt, opt, ex, neg, dig);
        else
            fmt = strscan_dec(sp, o, fmt, opt, ex, neg, dig);

        /* Try to convert number to integer, if requested. */
        if (fmt == STRSCAN_NUM && (opt & STRSCAN_OPT_TOINT)) {
            double n = o->n;
            int32_t i = lj_num2int(n);
            if (n == (lua_Number)i) {
                o->i = i;
                return STRSCAN_INT;
            }
        }
        return fmt;
    }
}

#undef DNEXT
#undef DPREV
#undef DLEN
#undef STRSCAN_DIG
#undef STRSCAN_MAXDIG
#undef STRSCAN_DDIG
#undef STRSCAN_DMASK
#undef casecmp
#undef LJ_LIKELY
#undef LJ_UNLIKELY
#undef LJ_64
#undef lua_assert
#undef U64x
#undef LJ_CHAR_DIGIT
#undef LJ_CHAR_XDIGIT
#undef lj_char_isa
#undef lj_char_isdigit
#undef lj_char_isspace
}
bool json_scan_number(const char* text, double* output) {
    TValue value{};
    const auto format = scan(reinterpret_cast<const uint8_t*>(text), &value, STRSCAN_OPT_TONUM);
    if (format != STRSCAN_NUM)
        return false;
    *output = std::bit_cast<double>(value);
    return true;
}
} // namespace skeet::script::cjson_detail
