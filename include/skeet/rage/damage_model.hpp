#pragma once
#include "skeet/rage/ballistics.hpp"
#include "skeet/rage/scanning_geometry.hpp"

namespace skeet::rage {
struct BulletDamageModel {
    float armor_ratio = 1, head_multiplier = 4, head_scale = 1, body_scale = 1;
    std::int32_t armor = 0;
    std::uint16_t weapon_id = 0;
    bool helmet = false, heavy_armor = false, cooperative = false, valid = true;
};
// Full floating-point damage is retained until the integer threshold boundary.
float bullet_health_damage(float raw_damage, std::int32_t hitgroup,
                            const BulletDamageModel&) noexcept;
std::int32_t damage_lower_bound(float damage) noexcept;

struct ExperimentalDamageContext {
    std::array<float, 4> scales{}; // CT head/body, T head/body.
    std::ptrdiff_t heavy_armor_offset = 0;
    float head_multiplier = 4;
    bool enabled = false, valid = false, cooperative = false;
};
class DamageRulesCache {
    void* owner_ = nullptr;
    std::array<void*, 4> variables_{};
    bool warned_ = false;
public:
    ExperimentalDamageContext capture(bool enabled, void* cvar, const void* weapon_data,
                                       std::ptrdiff_t heavy_armor_offset, int game_type);
};
BulletDamageModel capture_damage_model(BallisticEngine&, BallisticEntity,
                                        const ExperimentalDamageContext&, const BallisticQuery&);
struct DamageIntersection {
    RageVector position{};
    std::int32_t hitbox = -1, hitgroup = -1;
    bool found = false;
};
DamageIntersection intersect_damage_ray(const RageVector& origin, const RageVector& direction,
                                         float range, std::span<const RageScanHitbox>,
                                         const RageBoxClipper&);
struct DamageRayResult {
    DamageIntersection hit;
    float health_damage = 0;
    std::uint32_t flags = 0;
};
DamageRayResult trace_damage_ray(BallisticEngine&, BallisticEntity source,
                                  const RageVector& origin, const RageVector& direction,
                                  std::span<const RageScanHitbox>, const RageBoxClipper&,
                                  const BallisticFilter&, BallisticQuery,
                                  const BulletDamageModel&);
}
