#pragma once
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/rage_hitboxes.hpp"

namespace skeet::animation {
struct RemoteAnimationPreparationFields {
    explicit RemoteAnimationPreparationFields(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset frame, ik_mask, flags, previous_flags, active_weapon;
    engine::DecodedOffset animation_state, view_gate, observer_mode, observer_target, world_model;
    engine::DecodedOffset eye_origin, duck_amount; //433849B5/433E533F
};
struct RemoteAnimationPreparationBindings {
    const RemoteAnimationPreparationFields& fields;
    const RageTrigonometry& trigonometry;
    void* engine; //4346A514, virtual93
    void* entity_list; //4346A890, virtual4
    void* input; //4346A894, virtual32
    void* const& local_player; //43475808
    const std::int32_t& frame; //43467B04+4
    const RageVector& view_origin; //43479CCC
    const RageVector& view_angles; //43479CD8
};

// Complete ordinary43402385. Recomputes the remote player's animation LOD
// once per frame and propagates its flags to the active weapon's world model.
// The caller owns the real per-thread guard; this body does not change it.
class RemoteAnimationPreparationRuntime {
public:
    explicit RemoteAnimationPreparationRuntime(RemoteAnimationPreparationBindings bindings) noexcept
        : bindings(bindings) {}
    const RemoteAnimationPreparationBindings bindings;
    void prepare(void* player) const;

private:
    bool allows_view_animation(void* player) const; //433BFF50
};
} // namespace skeet::animation
