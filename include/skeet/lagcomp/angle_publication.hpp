#pragma once
#include "entity_state.hpp"

namespace skeet::lagcomp {
// Complete433F19CC. Entries have a native stride of16; capacity and first
// are the original unsigned16-bit fields. Only one subtraction is performed.
std::uint32_t history_angle_entry_index(std::uint16_t capacity, std::uint16_t first,
                                        std::int32_t ordinal) noexcept;
// Complete43416836..1698F: normalize/clamp pitch to[-90,90], then publish
// changed pitch/yaw to the native interpolation ring and optional live target.
// state_fac21eaf is the actual decoded entity-property offset. Its pointers
// are native pointers on the x86 target, not copied 32-bit addresses on a host.
void publish_history_final_angles(void* player, const Vector& previous_angles,
                                  const EntityStateRuntime&, std::uint32_t state_fac21eaf);
} // namespace skeet::lagcomp
