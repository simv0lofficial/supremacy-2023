#pragma once
#include "projection.hpp"
#include "skeet/render_queue.hpp"

namespace skeet::esp {

struct SkeletonBone {
    std::uint32_t flags;
    std::uint32_t parent;
};
// 0x4340ACA4..0x4340AE26, after native model/setup-bones validation.
// Source studio bone flags/parent indices and cached matrix storage are
// supplied by the engine adapter. The native workspace holds 256 bones.
void draw_skeleton(ui::RenderQueue& queue, const Projection& projection,
                   std::span<const SkeletonBone> bones, std::span<const Matrix3x4> transforms,
                   std::uint32_t color);

} // namespace skeet::esp
