#pragma once
#include "core.hpp"
#include <limits>

namespace skeet {

struct RagePointScore {
    std::int32_t damage = 0;
    std::int32_t hitgroup = 0;
    std::uint32_t flags = 0; // bit 0 safe; bit 1 carries the native 0x80 priority gate.
    float chance = 0;
    std::uint32_t priority = 0;
};
struct RagePointRequirements {
    std::int32_t target_health = 0;
    std::int32_t minimum_damage = 0;
    float minimum_chance = 0;
    bool prefer_body = false;
};
struct RagePointReference {
    std::array<float, 3> position;
    std::int32_t damage;
};
struct RagePointAdmissionState {
    RagePointReference reference;
    std::uint32_t priority_mask;
    std::uint32_t evaluation_flags;
    std::uint32_t body_disabler_reasons;
};
// 0x4336060A..0x433606E2. A descriptor without bit 2 becomes the reference
// even when rejected. Accepted records inherit this reference's position/damage.
// Failure may set point flag 0x80 or request the evaluation flag 0x100.
bool admit_rage_point(RagePointScore&, const RagePointRequirements&, RagePointAdmissionState&,
                      const std::array<float, 3>& descriptor_position,
                      std::uint32_t descriptor_flags) noexcept;
// 0x433604AC..0x433605ED. chance*100 uses the current x86 rounding mode.
std::uint32_t rage_point_priority(const RagePointScore&, const RagePointRequirements&) noexcept;

inline constexpr std::size_t no_rage_point = std::numeric_limits<std::size_t>::max();
struct RagePointSelection {
    std::size_t primary = no_rage_point;
    std::size_t alternate_first = no_rage_point;
    std::size_t alternate_second = no_rage_point;
    std::uint32_t candidate_flags = 0;
};
// Complete selection loop 0x4336070E..0x43360869. Returned indices identify
// the original evaluated point records to copy; unassigned slots stay untouched.
// Equal priorities retain the first point. Masking applies only to primary choice.
RagePointSelection select_rage_points(std::span<const RagePointScore> points,
                                      std::uint32_t priority_mask, bool prefer_body,
                                      bool prefer_safe, std::uint32_t candidate_flags) noexcept;

struct RageCandidatePriority {
    std::int32_t target_health = 0;
    RagePointScore primary;
    bool prefer_body = false;
    bool prefer_safe = false;
    bool target_weapon_priority_enabled = false; // candidate +0x10C bit 0
    // Result of the entity-list/weapon-handle lookup at 0x4336466A..0x433646D0.
    bool target_weapon_is_taser = false;
    bool target_priority = false; // target table +3
    std::uint32_t target_state_flags = 0;
};
std::uint32_t rage_candidate_priority(const RageCandidatePriority&) noexcept;
// Native qsort comparator 0x433FF5C7 preserves wrapping 32-bit subtraction.
std::int32_t compare_rage_candidate_priorities(std::uint32_t left, std::uint32_t right) noexcept;

} // namespace skeet
