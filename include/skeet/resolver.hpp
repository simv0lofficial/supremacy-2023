#pragma once
#include "core.hpp"

namespace skeet {

struct ResolverVariant {
    float yaw;
    float pose_yaw;
    HitboxMatrices matrices;
};

struct ResolverRecord {
    float reference_yaw; // native LagRecord +0x04
    float observed_yaw; // +0x2C
    std::uint8_t variant_mode; // +0x58; understood values 0..3
    std::uint8_t selected_attempt; // +0x59
    std::uint32_t state_flags; // +0x5C; bit 0x80 enables variants
    float selected_yaw; // +0x28
    float selected_pose_parameter; // +0x1FC
    HitboxMatrices selected_matrices; // +0x230
    ResolverVariant first; // +0x5C0 (matrices +0x5D0)
    ResolverVariant center; // +0x960 (matrices +0x970)
    ResolverVariant third; // +0xD00 (matrices +0xD10)
};

float round_half_away_from_zero(float value) noexcept;
void select_resolver_variant(ResolverRecord& record, std::uint8_t attempt) noexcept;

} // namespace skeet
