#pragma once
#include "skeet/lagcomp/records.hpp"
#include "skeet/rage_hitchance.hpp"
#include "skeet/engine/build_eye_adjustment_bindings.hpp"
#include <functional>
#include <variant>

namespace skeet::resolver_runtime {

struct AttemptOwner {
    float previous_selection_time; // native owner +88
    float previous_pose_time; // native owner +1A0
};
struct AttemptShot {
    const lagcomp::LagRecord& record; // native shot +180
    std::uint32_t prefix_flags; // native shot +00
    std::uint32_t shot_flags; // native shot +C4
    std::uint32_t target_handle; // native shot +94
    float distance; // native shot +74
};
struct AttemptTarget {
    bool hidden; // entity byte decoded with 9D920E35
    float* animation_adjustment; // animation state +80; null means no state
    const void* studio_model; // non-null only when wrapper and header exist
    bool hitbox_set;
    std::uint32_t hitbox_count;
    float body_pose_parameter; // entity pose parameter 11
    std::array<const void*, 19> hitboxes;
};
struct AttemptEngine {
    void* context;
    // Actual entity-list virtual +0C, receiving the native target handle.
    AttemptTarget* (*lookup_handle)(void*, std::uint32_t);
    // Actual engine target 43468CA4, called through 433A0EC6.
    float (*convert_pose_parameter)(void*, const void* studio_model, std::uint32_t, float);
    RageBoxClipper clipper;
};
// The live4337CB8A path obtains each field only after its preceding gates.
// A semantic AttemptTarget projection cannot preserve those partial reads.
struct RawAttemptEngine {
    void* const& entity_list;
    float* const& current_time; //4346A334, read after the actual lookup
    std::ptrdiff_t hidden, animation_state, studio_wrapper, pose_parameters, hitbox_set;
    const engine::PoseRangeRuntime& pose;
    RageBoxClipper clipper;
};
class AttemptEngineBinding {
    std::variant<std::reference_wrapper<const AttemptEngine>,
                 std::reference_wrapper<const RawAttemptEngine>>
        source_;

public:
    AttemptEngineBinding(const AttemptEngine& source) noexcept : source_(std::cref(source)) {}
    AttemptEngineBinding(const RawAttemptEngine& source) noexcept : source_(std::cref(source)) {}
    const auto& source() const noexcept {
        return source_;
    }
};

// Complete 433D725B. Uses all 19 model descriptors and the corresponding
// hypothesis matrix; the trace persists between failed engine clip calls.
bool intersects_hypothesis(const RageRay&, const std::array<const void*, 19>& hitboxes,
                           const HitboxMatrices&, const RageBoxClipper&);

// Complete 4337CB8A. The mode-0 path updates animation_adjustment; other modes
// only choose the next attempt. start/direction are nullable native arguments.
std::uint8_t choose_next_attempt(const AttemptOwner&, const AttemptShot&, float current_time,
                                 bool geometry_enabled, const RageVector* start,
                                 const RageVector* direction, const AttemptEngine&);
std::uint8_t choose_next_attempt(const AttemptOwner&, const AttemptShot&, float current_time,
                                 bool geometry_enabled, const RageVector* start,
                                 const RageVector* direction, const RawAttemptEngine&);
std::uint8_t choose_next_attempt(const AttemptOwner&, const AttemptShot&, float current_time,
                                 bool geometry_enabled, const RageVector* start,
                                 const RageVector* direction, const AttemptEngineBinding&);

} // namespace skeet::resolver_runtime
