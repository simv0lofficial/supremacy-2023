#pragma once
#include "rage_hitchance.hpp"
#include <vector>

namespace skeet {
struct RageLocalHitbox {
    std::int32_t hitgroup;
    RageVector minimum;
    RageVector maximum;
    RageVector rotation;
    float radius;
    const void* engine_hitbox;
};
struct RageTrigonometry {
    // Imported __libm_sse2_sinf / __libm_sse2_cosf semantics. The bindings are
    // required: choosing a different CRT may change the final matrix by one ULP.
    float (*sine)(float);
    float (*cosine)(float);
};
// Native preparation loop 0x4335D23C..0x4335D5F9, also used for resolver
// hypotheses. Matrices are the per-hitbox matrices produced by the record's
// bone lookup. The loop caps the model's count at 19 and preserves the original
// descriptor pointer for box clipping while constructing world-space capsules.
std::vector<RageScanHitbox> prepare_rage_scan_hitboxes(std::span<const RageLocalHitbox>,
                                                       const HitboxMatrices&,
                                                       const RageTrigonometry&);
} // namespace skeet
