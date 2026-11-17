#pragma once
#include "skeet/lagcomp/records.hpp"

namespace skeet::lagcomp {
// 433E4F87. NaN components become positive zero. Otherwise clamp against
// maximum, then its sign-bit-flipped lower bound; true means no store occurred.
// Ordered SSE comparisons preserve the native NaN-bound and DAZ behavior.
bool validate_velocity_component(float& value, float maximum) noexcept;

// 4332FB8A's component pass. All three components are visited in x/y/z order,
// even when an earlier component was invalid; true means all were unchanged.
bool validate_velocity(Vector& velocity, float maximum) noexcept;

// Complete 4332FB8A with its ConVar dependency supplied by the caller.
// maximum_convar is the already decoded 43468C2C ^ 3FE3AE0C object. Follow
// parent +1C to a self-parent, then XOR encoded float +2C with its low address.
// Resolve the maximum once before reading or updating any vector component.
bool validate_velocity(Vector& velocity, const void* maximum_convar) noexcept;
} // namespace skeet::lagcomp
