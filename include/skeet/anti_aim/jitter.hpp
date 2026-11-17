#pragma once
#include <array>
#include <cstdint>

namespace skeet::anti_aim {
struct JitterSettings {
    std::int32_t mode; // owner + 0x40
    std::int32_t offset; // owner + 0x54
    float minimum_skitter; // owner + 0x14C; default 20
    float maximum_skitter; // owner + 0x148; default 90
    std::array<float, 12> skitter = {-1, 1, 0, -1, 1, -1, 0, 1, -1, 0, 1, 0}; // +0x15C
};
struct JitterRandom {
    void* context;
    void (*seed)(void*, std::int32_t); // vstdlib!RandomSeed
    float (*range)(void*, float, float); // vstdlib!RandomFloat
};

// Native scheduler block 0x4332E63D..0x4332E837, including the yaw-off gate,
// all five jitter modes and the command-number seed. Randomness stays at its
// original external vstdlib boundary; no random generator is substituted.
void apply_yaw_jitter(float& yaw, std::int32_t yaw_mode, const JitterSettings&, std::uint32_t phase,
                      std::int32_t command_number, const JitterRandom&);
} // namespace skeet::anti_aim
