#pragma once

#if defined(__clang__)
#include_next <math.h>
#else
#include <../ucrt/math.h>
#endif
#include <cmath>

namespace math {
    constexpr float pi   = 3.1415926535897932384f;
    constexpr float pi_2 = pi * 2.f;

    inline constexpr float deg_to_rad(float val) {
        return val * (pi / 180.f);
    }

    inline constexpr float rad_to_deg(float val) {
        return val * (180.f / pi);
    }
}
