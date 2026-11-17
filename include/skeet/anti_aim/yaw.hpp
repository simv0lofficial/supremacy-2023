#pragma once
#include <cstdint>

namespace skeet::anti_aim {
struct YawLimits {
    float backward_angle; // owner + 0x138, constructor default 180
    float maximum_angle; // owner + 0x148, constructor default 90
};
struct YawFrame {
    float angle_b8;
    float origin_x_dc, origin_y_e0;
    float interpolation_158;
};
struct YawTarget {
    std::int32_t handle; // global 0x4346DB1C, -1 means absent
    float x, y; // globals 0x4346DB20/24
};
struct EntityLookup {
    void* object;
    // External entity-list virtual method at byte offset 0x10 in the x86 ABI.
    const void* (*by_handle)(void*, std::int32_t);
};

// Complete native helper 0x433D2118..0x433D231E. The saved-angle argument is
// read only in mode 3; mode 4 resolves the entity only for a non--1 handle.
float transform_yaw(float base, float offset, std::int32_t mode, const float& saved_angle,
                    const YawLimits&, const YawFrame&, const YawTarget&, const EntityLookup&);

// Complete 0x433B76BE..0x433B76D7. The 0x80 flag lowers a nonzero signed
// limit by one with x86 wrapping before the signed comparison.
bool lag_step_pending(std::uint32_t flags, std::int32_t limit_fc,
                      std::int32_t current_15c) noexcept;
// Full4332E6BD..4332E79D branch. Each target/local coordinate is separately
// quantized to0.25 with native SSErounding before the planar angle calculation.
bool apply_target_yaw_base(std::int32_t yaw_base, const YawTarget&, float origin_x, float origin_y,
                           const EntityLookup&, float& yaw);
} // namespace skeet::anti_aim
