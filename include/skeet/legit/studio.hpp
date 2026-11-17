#pragma once
#include "acquisition.hpp"
#include "skeet/animation/bones.hpp"

namespace skeet::legit {
struct StudioOffsets {
    std::ptrdiff_t wrapper, hitbox_set;
};
struct StudioHitboxSet {
    const std::byte* descriptors;
    std::int32_t count;
    bool studio;
};
// Full nullable433FD30D wrapper/header gate and native set lookup. Descriptor
// indexing remains a caller operation; the native accuracy search uses0/3.
StudioHitboxSet load_target_hitbox_set(const void* entity, const StudioOffsets&);
struct TargetStudio {
    bool ready = false;
    std::int32_t count = 0;
    std::array<AcquisitionHitbox, 19> hitboxes;
    std::array<std::uint32_t, 19> bones;
    std::span<const AcquisitionHitbox> descriptors() const noexcept {
        return {hitboxes.data(), std::size_t(count)};
    }
    animation::BoneModel bone_model() const noexcept {
        return {true, 0, ready, false, false, {}, {}, {bones.data(), std::size_t(count)}};
    }
};
// Concrete433FD30D plus the shared studio/header/set/bone descriptor reads.
// The native hitbox-to-bone helpers cap the model's descriptor count at19.
TargetStudio load_target_studio(const void* entity, const StudioOffsets&);
} // namespace skeet::legit
