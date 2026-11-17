#pragma once
#include "skeet/anti_aim/yaw.hpp"
#include "skeet/rage/ballistics.hpp"

namespace skeet::anti_aim {
struct FreestandingScore {
    std::int32_t maximum_distance = 0, total_distance = 0, damage = 0;
    BallisticVector direction{};
};
struct FreestandingRay {
    bool enabled = false;
    BallisticVector start{}, end{};
};
struct FreestandingWorkspace {
    BallisticQuery query;
    std::array<FreestandingScore, 3> scores{};
    // 43381817 leaves ray 5's coordinates untouched, including when disabled.
    // The caller retains this scratch slot, matching native preexisting memory.
    std::array<FreestandingRay, 12> rays{};
};
void build_freestanding_rays(std::array<FreestandingRay, 12>&,
                             const std::array<FreestandingScore, 3>&, const BallisticVector& start,
                             const BallisticVector& target,
                             const BallisticVector& side); // 43381817
bool freestanding_difference(const std::array<FreestandingScore, 3>&) noexcept; // 43407226
std::int32_t select_freestanding_side(const std::array<FreestandingScore, 3>&) noexcept; // 433B87F9
std::int32_t score_freestanding_ray(BallisticEngine&, BallisticQuery&, FreestandingScore&,
                                    const BallisticVector& start,
                                    const BallisticVector& target); // 433D094E
// Full native trace/score/selection owner 433F41AF. Only engine access crosses
// the interface; bullet tracing, penetration and ranking stay in recovered code.
bool find_freestanding_direction(BallisticEngine&, const BallisticVector& start,
                                 const BallisticVector& target, BallisticVector& direction,
                                 std::int32_t* total_damage, FreestandingWorkspace&);

struct FreestandingSettings {
    bool enabled;
    HotkeyBinding hotkey;
};
struct FreestandingFrame {
    float scalar_c0, scalar_38, height_164, forward_move;
    BallisticVector origin_10c, origin_168;
};
struct FreestandingContext {
    std::uint32_t flags; // global 434759C0
    HotkeyBinding height_hotkey; // global 43467C8C, mode at43467CA0
    InputBitmap input;
    std::int32_t target_handle;
    BallisticVector target;
    EntityLookup entities;
};
struct FreestandingState {
    std::uint32_t flags_130;
    float yaw_1a8;
    std::uint32_t value_1ac;
};
// Complete native 4338A172, with internal 433305FB/4341D036 activation tests.
bool apply_freestanding(const FreestandingSettings&, const FreestandingFrame&,
                        const FreestandingContext&, FreestandingState&, float& yaw,
                        BallisticEngine&, FreestandingWorkspace&);
} // namespace skeet::anti_aim
