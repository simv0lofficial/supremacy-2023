#pragma once
#include "skeet/rage/scanning.hpp"
#include "skeet/rage/scanning_geometry.hpp"
#include "skeet/rage/scanning_chance.hpp"

namespace skeet::rage::scan_detail {
struct Model {
    const void* header = nullptr;
    std::vector<RageLocalHitbox> hitboxes;
    std::vector<std::uint32_t> bones;
};
Model model(void* player, const ScanModelOffsets&);
bool teammate_blocks(const ScanContext&, const RageVector& point);
struct Geometry {
    std::vector<RageScanHitbox> primary;
    std::array<std::vector<RageScanHitbox>, 3> alternatives;
    RageResolverHitboxes views() const noexcept {
        return {alternatives[0], alternatives[1], alternatives[2]};
    }
};
struct ProjectedPoint {
    RageVector position{};
    RageVector direction{};
    std::int32_t hitbox = -1, hitgroup = -1;
    bool found = false;
};
ProjectedPoint project(const ScanContext&, const Geometry&, const RageVector&, float range);
float minimum_multiplier(const ScanPolicy&, std::int32_t health, std::int32_t damage,
                         std::int32_t hitgroup, std::int32_t weapon_id) noexcept;
RageSpreadBasis spread_basis(const RageVector&) noexcept;
RageHitchance normal_chance(const ScanContext&, void* player, const Geometry&,
                            const RageSpreadBasis&, float range, std::int32_t hitgroup,
                            float minimum_multiplier);
void refine(const ScanContext&, const ScanPolicy&, std::int32_t health, void* player,
            BallisticEntity target, const Geometry&, TargetPoint&);
}
