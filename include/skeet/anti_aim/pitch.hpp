#pragma once
#include <cstdint>

namespace skeet::anti_aim {
struct PitchSettings {
    std::int32_t mode;
    std::int32_t custom_pitch; // owner + 0x128
    float special_angle; // owner + 0x138, default 180
    float maximum_pitch; // owner + 0x150, default 89
};
struct PitchFrame {
    std::uint32_t flags;
    std::uint32_t buttons;
    std::int32_t movement_type; // decoded player field
    std::int32_t command_number;
    float angle_9c;
    bool has_weapon;
    float weapon_pitch_340;
};
struct PitchInterfaces {
    void* context;
    void (*random_seed)(void*, std::int32_t); // vstdlib!RandomSeed
    std::int32_t (*random_integer)(void*, std::int32_t, std::int32_t); // vstdlib!RandomInt
    std::int32_t (*player_code)(void*); // external player virtual method +0x1E8
};

// Native scheduler block 0x4332D70D..0x4332D8DE. The caller supplies decoded
// player/weapon fields. Native random choice 4 preserves the incoming pitch.
void apply_pitch(float& pitch, const PitchSettings&, const PitchFrame&, const PitchInterfaces&);
} // namespace skeet::anti_aim
