#pragma once
#include "skeet/lagcomp/entity_state.hpp"

namespace skeet::animation {
// Fix hooks use these five offsets and the two engine interfaces. They do
// not consume EntityStateRuntime's frame/command/time transaction snapshot.
struct FixAnimationEntityFields {
    std::uint32_t field_0471e13c;
    std::uint32_t field_432b1f5b;
    std::uint32_t field_0d560fac;
    std::uint32_t player_flags;
    std::uint32_t pose_parameters;
};
struct FixAnimationEntityView {
    void* engine;
    void* entity_list;
    FixAnimationEntityFields offsets;
    FixAnimationEntityView(void* engine, void* entity_list,
                           FixAnimationEntityFields fields) noexcept
        : engine(engine), entity_list(entity_list), offsets(fields) {}
    // Existing callers with a real transaction runtime preserve their exact
    // interface/field identities, without retaining its clock snapshot.
    FixAnimationEntityView(const lagcomp::EntityStateRuntime& entity) noexcept
        : engine(entity.engine), entity_list(entity.entity_list),
          offsets{entity.offsets.field_0471e13c, entity.offsets.field_432b1f5b,
                  entity.offsets.field_0d560fac, entity.offsets.player_flags,
                  entity.offsets.pose_parameters} {}
};
} //namespace skeet::animation
