#pragma once
#include "core.hpp"
#include <limits>

namespace skeet {

bool hitgroup_armored(std::int32_t armor, std::uint8_t helmet, std::int32_t hitgroup) noexcept;
float hitgroup_damage_scale(std::int32_t armor, std::uint8_t helmet, std::int32_t hitgroup,
                            std::uint16_t weapon_id, float armor_ratio) noexcept;
// 0x433E9ECD: truncate each pellet's scaled damage, then multiply with x86 wrap.
std::int32_t final_bullet_damage(float ballistic_damage, float armor_ratio, std::int32_t pellets,
                                 std::uint16_t weapon_id, std::int32_t hitgroup,
                                 std::int32_t target_armor, std::uint8_t target_helmet) noexcept;

struct PenetrationSurface {
    std::uint16_t material = 0;
    float penetration_modifier = 0;
};
struct SurfacePenetration {
    std::array<float, 3> entry;
    std::array<float, 3> exit;
    PenetrationSurface entry_surface;
    PenetrationSurface exit_surface;
    bool contents_bit_3 = false;
    bool surface_flag_bit_7 = false;
    float weapon_penetration = 0;
};
struct BulletPenetrationState {
    std::array<float, 3> position;
    float damage = 0;
    std::int32_t remaining_penetrations = 0;
};
// Standard post-exit-trace path, 0x4339E320..0x4339E4C9. Trace-to-exit,
// entity access and the earlier trace-distance budget are outside this function.
// False clears damage; only success moves position and consumes penetration.
bool apply_surface_penetration(BulletPenetrationState&, const SurfacePenetration&) noexcept;

// Primary rage normalization 0x433C1D85..0x433C1FBC, also present in the
// Visualize aimbot rescan at 0x43332A2B..0x43332C3B. source_factor/source_limit
// are results of two native virtual getters; their engine meanings are unresolved.
std::int32_t rage_minimum_damage(std::int32_t raw, std::int32_t target_health,
                                 std::uint32_t configuration_flags, float source_factor,
                                 std::int32_t source_limit) noexcept;

} // namespace skeet
