#pragma once
#include "config.hpp"
#include "rage_policy.hpp"

namespace skeet {
// A live view of the descriptor storage used by the recovered RAGE policy.
// It owns no values; the registry and its descriptors must outlive the view.
class RageRuntimeConfigView {
public:
    explicit RageRuntimeConfigView(ConfigRegistries& registries);
    RageThresholdSettings threshold_settings() const noexcept;
    RageThresholds thresholds(std::uint16_t weapon_id, std::uint32_t configuration_flags,
                              bool override_hotkey_active) const noexcept;
    RageHitboxMasks hitboxes(std::uint32_t configuration_flags,
                             bool force_safe_active) const noexcept;
    void apply_body_disablers(RageBodyAimPolicy& policy) const noexcept;

private:
    const IntegerConfig *minimum_chance_, *minimum_damage_, *override_damage_, *double_tap_chance_;
    const BooleanConfig *override_enabled_, *delay_shot_;
    const IntegerConfig *selected_hitboxes_, *avoid_unsafe_, *body_disablers_;
    const BooleanConfig *prefer_safe_, *prefer_body_;
};
} // namespace skeet
