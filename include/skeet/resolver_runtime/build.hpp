#pragma once
#include "skeet/resolver_runtime/hypotheses.hpp"
#include "skeet/resolver.hpp"
#include "skeet/animation/bones.hpp"

namespace skeet::lagcomp {
struct LagRecord;
}
namespace skeet::animation {
struct BoneRuntime;
}

namespace skeet::resolver_runtime {

struct HypothesisBuildContext {
    animation::BoneEntityState& entity;
    const animation::BoneModel& model;
    const RageVector& origin;
    std::span<float, 24> pose_parameters;
    const animation::BoneEngine& engine;
    const RageTrigonometry& trigonometry;
    bool strict = false;
};

// Full 0x433FAC08 orchestration: three hypotheses, the recovered skeletal
// builder for each, native early termination, and restoration of all 24 poses.
// The returned count is the number of successful candidate builds (0..3).
std::uint32_t build_hypotheses(ResolverRecord& record, const HypothesisInput& input,
                               HypothesisBuildContext& context);
std::uint32_t build_hypotheses(lagcomp::LagRecord& record, const HypothesisInput& input,
                               HypothesisBuildContext& context);
// Live433FAC08 composition. Every433FF63C call reads the supplied entity's
// current origin, model, studio set and genuine engine pose virtual; no model
// or BoneEntityState projection is retained between the three candidates.
std::uint32_t build_hypotheses(lagcomp::LagRecord& record, const HypothesisInput& input,
                               void* player, const animation::BoneRuntime& runtime,
                               bool strict = false);

} // namespace skeet::resolver_runtime
