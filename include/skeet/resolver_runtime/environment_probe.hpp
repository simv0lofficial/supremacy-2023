#pragma once
#include "skeet/rage/ballistics.hpp"
#include "skeet/rage_hitboxes.hpp"

namespace skeet::resolver_runtime {
struct EnvironmentRay {
    RageRay ray;
    RageVector start_offset;
    RageVector extents;
    bool is_ray;
    bool swept;
};
struct EnvironmentTraceRequest {
    EnvironmentRay ray;
    std::uint32_t mask;
    // Both native filters reject every entity (43382057); their trace types
    // differ: initial line=0 (43336942), hulls=1 (433686FC).
    std::uint32_t trace_type;
};
struct EnvironmentEngine {
    void* context;
    void (*trace_ray)(void*, const EnvironmentTraceRequest&, BallisticTrace&);
    std::uint32_t (*point_contents)(void*, const RageVector&, std::uint32_t mask);
    void (*view_angles)(void*, RageVector&); // genuine engine virtual +48
    BallisticEngine& penetration;
    RageTrigonometry trigonometry;
    std::uint8_t* suppression_flag = nullptr; // native global434683D0 pointed byte
    const std::uint8_t* rework = nullptr;
    bool reworked() const noexcept {
        return rework && *rework;
    }
};
struct EnvironmentProbe {
    bool selected;
    RageVector direction; // chosen vector before native scoring sign inversion
    std::array<RageVector, 2> shifted_target_points;
    std::array<std::int32_t, 2> damage;
};

// 433486FC..43349119 environment probing. Input origins are unraised; the
// native +64 eye offset is applied here. No internal ballistic call is bound
// through a callback: penetration uses the recovered 433C0AE9 implementation.
EnvironmentProbe probe_environment(const RageVector& local_origin, const RageVector& target_origin,
                                   const EnvironmentEngine&);
} // namespace skeet::resolver_runtime
