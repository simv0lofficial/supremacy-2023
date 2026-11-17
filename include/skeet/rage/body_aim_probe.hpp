#pragma once
#include "acquisition.hpp"
#include "ballistics.hpp"
#include "skeet/movement/collision.hpp"

namespace skeet::anti_aim {
class TargetTraceEntities;
}
namespace skeet::rage {

struct BodyAimProbeOffsets {
    std::ptrdiff_t network_origin; // A9EE6062
    std::ptrdiff_t velocity; // 36375C23
    std::ptrdiff_t studio_header; // 433FD30D decoded pointer field
    std::ptrdiff_t hitbox_set; // C2B6FB10
    std::ptrdiff_t cached_bones; // 17978090
    std::ptrdiff_t weapon_item;
};
struct BodyAimProbeContext {
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    BallisticEntity source; // actual local player43475808
    void* weapon; // frame+0C, resolved by the enclosing scheduler
    BodyAimProbeOffsets offsets;
    std::uint32_t context_flags; // acquisition owner+170
    std::uint32_t weapon_flags; // acquisition owner+174
    const std::uint8_t& allow_penetration; //live BooleanConfig.value4346A3E0
    std::uint8_t* trace_flag; // optional434683D0
    void* const* trace_filter_vtable; // actual external table43475B90
    const anti_aim::TargetTraceEntities& trace_entities;
};

// Exact433533CB..433537A1, including the repeated normalizations and the
// ordered endpoint comparisons. The capsule is already transformed by its
// current live bone; hitbox-angle rotation is not applied by this branch.
std::array<RageVector, 2> body_probe_surface_points(const RageCapsule&,
                                                    const RageVector& origin) noexcept;

// Complete optional acquisition branch433528CC..433538BF. Valid live studio
// hitbox6 and its cached bone are an enclosing acquisition contract, as in the
// native code. On admission, only masks C0/C4 and frame flag01000000 change.
bool probe_body_aim(client::CommandFrame&, AcquisitionTargetSettings&, void* target_player,
                    const BodyAimProbeContext&);

} // namespace skeet::rage
