#pragma once
#include "historical_bones.hpp"
#include "skeet/animation/bone_runtime.hpp"

namespace skeet::client {
struct NativePlayerStateLayout;
}
namespace skeet::legit {
struct LiveHistoricalBoneBindings {
    const HistoricalBoneOffsets& offsets;
    const client::NativePlayerStateLayout& cached;
    const animation::BoneRuntime& bones;
    const float* interval;
    const float* time;
    const float* frame_time;
    const std::int32_t* current_tick;
};
// Whole43367778 with the actual cache and BoneRuntime. Cached fields are read
// at their native transaction sites; model/set lookup occurs inside each raw
// scatter/select, including after the genuine attachment engine call.
std::int32_t historical_target_bones_live(const lagcomp::RecordRing&, void* target,
                                          std::int32_t requested_tick, std::span<Matrix3x4, 19>,
                                          std::span<Matrix3x4, 256>,
                                          const LiveHistoricalBoneBindings&);
// Whole4336EC3B live entry. SetupBones is followed by a fresh model selection.
bool prepare_target_bones_live(const ProfileState&, const lagcomp::RecordRing&, void* target,
                               std::span<Matrix3x4, 19>, std::span<Matrix3x4, 256>, float& interval,
                               const LiveHistoricalBoneBindings&);
} // namespace skeet::legit
