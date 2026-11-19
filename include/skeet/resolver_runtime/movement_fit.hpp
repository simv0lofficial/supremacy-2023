#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <limits>

namespace skeet::resolver_runtime {
inline constexpr unsigned movement_pose_evaluation_limit = 192;
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
        !std::isfinite(maximum - minimum) || branches < 1 || branches > 4 ||
        !(tolerance > 0.0f) || !std::isfinite(tolerance))
        return best;
    struct Solution {
        float pose, error;
        unsigned branch;
    };
    std::array<Solution, 12> solutions{};
    unsigned count = 0;
    bool valid_domain = true;
    const auto evaluate = [&](float pose, unsigned branch) {
        if (best.evaluations == movement_pose_evaluation_limit) {
            valid_domain = false;
            return std::numeric_limits<float>::infinity();
        }
        ++best.evaluations;
        const float error = residual(pose, branch);
        if (!std::isfinite(error) || error < 0.0f) {
            valid_domain = false;
            return std::numeric_limits<float>::infinity();
        }
        return error;
    };
    constexpr unsigned intervals = 16;
    const float step = (maximum - minimum) / float(intervals);
    for (unsigned branch = 0; branch < branches; ++branch) {
        std::array<float, intervals + 1> errors{};
        for (unsigned i = 0; i <= intervals; ++i)
            errors[i] = evaluate(minimum + float(i) * step, branch);
        if (!valid_domain)
            return best;
        const auto [lowest, highest] = std::minmax_element(errors.begin(), errors.end());
        // A flat model contains no directional information. Do not refine it.
        if (*highest - *lowest <= tolerance) {
            if (*lowest <= tolerance)
                return best;
            continue;
        }
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
            // Golden-section refinement reuses one expensive model evaluation.
            constexpr float ratio = 0.61803398875f;
            float left = high - ratio * (high - low), right = low + ratio * (high - low);
            float a = evaluate(left, branch), b = evaluate(right, branch);
            for (unsigned iteration = 0; iteration <= 8; ++iteration) {
                if (a < error) {
                    pose = left;
                    error = a;
                }
                if (b < error) {
                    pose = right;
                    error = b;
                }
                if (iteration == 8)
                    break;
                if (a <= b) {
                    high = right;
                    right = left;
                    b = a;
                    left = high - ratio * (high - low);
                    a = evaluate(left, branch);
                } else {
                    low = left;
                    left = right;
                    a = b;
                    right = low + ratio * (high - low);
                    b = evaluate(right, branch);
                }
            }
            if (!valid_domain)
                return best;
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
    best.identified = valid_domain;
    return best;
}
}
