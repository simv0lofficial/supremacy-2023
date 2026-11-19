#pragma once
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/animation/entity_update.hpp"
#include <cmath>
#include <bit>

namespace skeet::animation {
// Rebuild a captured pose without mixing in the entity's current layers.
class RecordPoseScope {
    void* player_;
    lagcomp::EntityNetvars offsets_{};
    lagcomp::AnimationState saved_{};
    bool valid_ = true;

public:
    RecordPoseScope(void* player, const lagcomp::LagRecord& record, const BoneRuntime& bones,
                    bool enabled)
        : player_(enabled ? player : nullptr) {
        if (!player_)
            return;
        offsets_.animation_layers = std::uint32_t(bones.fields.animation_layers_b538bba5);
        offsets_.pose_parameters = std::uint32_t(bones.fields.pose_parameters);
        capture_entity_animation(saved_, player_, offsets_);
        valid_ = record.animation.layer_count == saved_.layer_count &&
                 record.animation.layer_count >= 0 && record.animation.layer_count <= 13;
        for (float pose : record.animation.pose_parameters)
            valid_ = valid_ && std::isfinite(pose);
        if (valid_)
            for (int index = 0; index < record.animation.layer_count; ++index)
                for (unsigned field : {2u, 3u, 4u, 5u, 6u})
                    valid_ = valid_ && std::isfinite(
                        std::bit_cast<float>(record.animation.layers[index].words[field]));
        if (!valid_) {
            player_ = nullptr;
            return;
        }
        restore_entity_animation(record.animation, player_, offsets_);
    }
    bool valid() const noexcept {
        return valid_;
    }
    ~RecordPoseScope() {
        if (player_)
            restore_entity_animation(saved_, player_, offsets_);
    }
    RecordPoseScope(const RecordPoseScope&) = delete;
    RecordPoseScope& operator=(const RecordPoseScope&) = delete;
};

inline std::int32_t build_captured_hitbox_matrices(void* player,
                                                  const lagcomp::LagRecord& record,
                                                  const BoneRuntime& bones,
                                                  std::span<Matrix3x4> scratch,
                                                  std::span<Matrix3x4> output,
                                                  std::uint32_t mask, float time = 0.0f) {
    RecordPoseScope pose(player, record, bones, true);
    if (!pose.valid() || !std::isfinite(record.header.animation_yaw))
        return 0;
    for (float coordinate : record.header.origin)
        if (!std::isfinite(coordinate))
            return 0;
    const BoneRequest request{{0.0f, record.header.animation_yaw, 0.0f},
                              record.header.origin, time, mask, -1, 0, false};
    if (!bones.build_bone_matrices(player, request, scratch))
        return 0;
    return bones.select_hitbox_bone_matrices(player, scratch, output);
}
}
