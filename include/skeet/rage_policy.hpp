#pragma once
#include "core.hpp"
#include <limits>

namespace skeet {

struct RageHitboxMasks {
    std::uint32_t regular = 0;
    std::uint32_t safe = 0;
};
// Settings preparation at 0x433C15E8..0x433C16CD. Force-safe's hotkey
// evaluator is a caller input; the remaining mask transformations are native.
RageHitboxMasks prepare_rage_hitboxes(std::uint32_t selected, bool prefer_safe, bool force_safe,
                                      std::uint32_t avoid_unsafe, std::uint32_t configuration_flags,
                                      bool prefer_body, std::uint8_t body_disablers) noexcept;

struct RageBodyAimPolicy {
    RageHitboxMasks hitboxes;
    std::uint32_t target_state_flags = 0;
    std::uint32_t disabler_reasons = 0;
    bool context_ready = false;
};
// 0x43351A85..0x43351B51, including the final target-state safe-mask gate.
// Configuration bit 1 (Low inaccuracy) has no effect in this consumer.
void apply_rage_body_disablers(RageBodyAimPolicy&, std::uint8_t options) noexcept;

struct RageThresholdSettings {
    std::int32_t minimum_chance = 50; // config hash 0x3F5E5553, +0x2C
    std::int32_t minimum_damage = 10; // 0x18C0E6BE, +0x68
    bool override_enabled = false; // 0xB80303B4, +0x7C
    std::int32_t override_damage = 10; // 0x7DC413FE, +0xB8
    bool delay_shot = false; // 0xA09C0D52, +0x108
};
struct RageThresholds {
    std::int32_t raw_minimum_damage;
    float minimum_chance;
    bool override_active;
    bool delay_shot;
};
// Native consumers in 0x433C175D..0x433C1977. Config flag 0x20 selects
// the integer at global 0x4346A4B0; its value is an input, not assumed zero.
// Override activation is the result of hotkey 0x7EB3A142 evaluation.
RageThresholds prepare_rage_thresholds(const RageThresholdSettings&, std::uint16_t weapon_id,
                                       std::uint32_t configuration_flags,
                                       std::int32_t forced_chance,
                                       bool override_hotkey_active) noexcept;

} // namespace skeet
