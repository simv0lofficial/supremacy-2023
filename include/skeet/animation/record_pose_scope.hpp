#pragma once
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/animation/entity_update.hpp"
#include <cmath>

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
}
