#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <limits>

namespace skeet::resolver_runtime {
struct MovementPoseFit {
    float pose = 0.0f, error = std::numeric_limits<float>::infinity();
    unsigned branch = 0, evaluations = 0;
    bool identified = false;
};
// Invert the model's displacement curve, without fitting across players or
// learning from shots. Multiple compatible yaw intervals remain unresolved.
template <class Residual>
MovementPoseFit fit_movement_pose(float minimum, float maximum, unsigned branches, float tolerance,
                                  Residual residual) {
    MovementPoseFit best;
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || !(maximum > minimum) ||
        branches < 1 || branches > 4 || !(tolerance > 0.0f))
        return best;
    struct Solution {
        float pose, error;
        unsigned branch;
    };
    std::array<Solution, 12> solutions{};
    unsigned count = 0;
    const auto evaluate = [&](float pose, unsigned branch) {
        ++best.evaluations;
        const float error = residual(pose, branch);
        return std::isfinite(error) && error >= 0.0f ? error
                                                     : std::numeric_limits<float>::infinity();
    };
    constexpr unsigned intervals = 16;
    const float step = (maximum - minimum) / float(intervals);
    for (unsigned branch = 0; branch < branches; ++branch) {
        std::array<float, intervals + 1> errors{};
        for (unsigned i = 0; i <= intervals; ++i)
            errors[i] = evaluate(minimum + float(i) * step, branch);
        unsigned minima = 0;
        for (unsigned i = 0; i <= intervals; ++i) {
            if ((i && errors[i] > errors[i - 1]) || (i < intervals && errors[i] > errors[i + 1]))
                continue;
            // A flat or highly oscillatory curve does not identify one pose.
            if (++minima > 3 || count == solutions.size())
                return best;
            float low = minimum + float(i ? i - 1 : 0) * step;
            float high = minimum + float(std::min(i + 1, intervals)) * step;
            float pose = minimum + float(i) * step, error = errors[i];
            for (unsigned iteration = 0; iteration < 8; ++iteration) {
                const float left = low + (high - low) / 3.0f, right = high - (high - low) / 3.0f;
                const float a = evaluate(left, branch), b = evaluate(right, branch);
                if (a < error) {
                    pose = left;
                    error = a;
                }
                if (b < error) {
                    pose = right;
                    error = b;
                }
                if (a <= b)
                    high = right;
                else
                    low = left;
            }
            solutions[count++] = {pose, error, branch};
            if (error < best.error) {
                best.pose = pose;
                best.error = error;
                best.branch = branch;
            }
        }
    }
    if (!(best.error <= tolerance))
        return best;
    for (unsigned i = 0; i < count; ++i)
        if (solutions[i].error <= best.error + tolerance &&
            (solutions[i].branch != best.branch || std::fabs(solutions[i].pose - best.pose) > 8.0f))
            return best;
    // Also reject shallow minima: low residual alone cannot constrain yaw.
    for (float side : {-8.0f, 8.0f}) {
        const float probe = best.pose + side;
        if (probe >= minimum && probe <= maximum &&
            evaluate(probe, best.branch) <= best.error + tolerance)
            return best;
    }
    best.identified = true;
    return best;
}
}
