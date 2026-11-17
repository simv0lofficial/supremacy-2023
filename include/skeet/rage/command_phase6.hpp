#pragma once
#include "skeet/animation/entity_update.hpp"
#include "skeet/client/command_frame.hpp"

namespace skeet::rage {
struct CommandPhase6Bindings {
    const void* const* game_rules; // native4346A3A4; both levels may be null
    std::ptrdiff_t rule_guard; // decoded43467910 / D720AEA6, byte
    std::ptrdiff_t weapon_identifier; // decoded4346908C / 5C9F0FBB, uint16
    // The actual global pair434759DC/E8, also read by animation43384C9E
    // and anti-aim433470E4. The RAGE gameplay owner has no private copy.
    std::array<animation::AnimationAngleSample, 2>& pending_angles;
};

// Complete433DF660. A different command shifts the current three raw words
// into the previous sample before publishing the new command/pitch/yaw.
// Repeating the same command only replaces its two angle words.
void publish_command_animation_angles(std::array<animation::AnimationAngleSample, 2>&,
                                      std::int32_t command_number,
                                      const std::array<float, 3>& angles) noexcept;

// Complete RAGE vtable phase6 owner4335CBFB. Its owner/player arguments are
// unused natively. The two weapon pointers, shared rule byte, exact firearm
// classification and firing-button branches precede the shared publication.
void process_rage_phase6(const client::CommandFrame&, const client::Command&,
                         const CommandPhase6Bindings&) noexcept;
} // namespace skeet::rage
