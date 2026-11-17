#pragma once
#include "skeet/core.hpp"

namespace skeet::esp {

struct Vector3 {
    float x, y, z;
};
struct Projection {
    // Native global matrix 0x434798E0. Rows 0, 1 and 3 are used.
    std::array<float, 16> matrix;
    float horizontal_scale, vertical_scale;
};
bool project_point(const Projection& projection, Vector3 position, Point& screen) noexcept;
// Draw-state endpoints +0x214 and +0x1F0. Failure leaves output unchanged.
bool player_box(const Projection& projection, Vector3 first, Vector3 second,
                Rectangle& output) noexcept;
float convert_field_of_view(float degrees, float aspect_scale) noexcept;
float angle_to_target(Vector3 origin, Vector3 target, Vector3 axis) noexcept;

} // namespace skeet::esp
