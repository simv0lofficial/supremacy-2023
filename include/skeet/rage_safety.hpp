#pragma once
#include "rage_hitchance.hpp"
#include "rage_segments.hpp"

namespace skeet {
using RageResolverHitboxes = std::array<std::span<const RageScanHitbox>, 3>;
// Scanner's three-hypothesis safe-point gate, 0x4335F44A..0x4335F6FB.
// Head checks use the engine hitbox clipper and reject closer body intersections.
// Other hitgroups require a qualifying capsule intersection in every hypothesis.
// The head path requires a populated first hitbox in all three input sets.
bool rage_point_safe(const RageRay&, std::int32_t requested_hitgroup,
                     float minimum_hitgroup_multiplier, const RageResolverHitboxes&,
                     const RageBoxClipper&);
} // namespace skeet
