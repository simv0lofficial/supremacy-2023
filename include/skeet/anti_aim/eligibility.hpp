#pragma once
#include "skeet/engine/interface_call.hpp"
#include "skeet/anti_aim/proximity.hpp"

namespace skeet::anti_aim {
struct EligibilitySettings {
    bool enabled;
    std::int32_t pitch, yaw, body_yaw;
};
struct EligibilityFrame {
    std::uint32_t flags, secondary_flags;
};
struct EligibilityGuard {
    bool shared_player_available, guard;
    float current_time; //433FC87C: shared player's decoded9F8E7F71 field
    float reference_time; // pointed43467EB8
};
struct EligibilityBindings {
    void* input_system; //4346A89C
    void* engine_client; //4346A514
    void* hud_context; //43468994
    engine::MemberFunction<void*, const char*> find_hud; //4346A374, optional
};
// Complete43384AE8 helper, retaining actual FindHudElement/virtual34 calls.
bool hud_chat_active(const EligibilityBindings&);
// Phase4 entry4332D1C6..4332D28B, including its unconditional flag update.
bool command_eligible(const EligibilitySettings&, const EligibilityFrame&, const EligibilityGuard&,
                      std::int32_t command_handle_1c4, const EligibilityBindings&,
                      std::uint32_t& flags_130);
// Native4332D28B..4332D2FF, before writing owner flag8 and advancing the cycle.
bool command_uses_cached_angles(bool player_a15c3105, std::int32_t movement_type,
                                std::uint32_t frame_flags, std::uint32_t buttons, ProximityEngine&,
                                ProximityEntity local);
} // namespace skeet::anti_aim
