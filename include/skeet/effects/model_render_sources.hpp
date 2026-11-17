#pragma once
#include "model_render.hpp"

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::engine {
struct NativeOffsetGlobals;
}
namespace skeet::effects {
// The shadow branch samples these actual cells only after its record-count
// and movement gates. No draw-time clock or history projection is retained.
class ModelShadowTimingSource {
public:
    ModelShadowTimingSource(engine::NativeDataImage&, const engine::NativeOffsetGlobals&);
    ModelShadowTiming capture() const;

private:
    engine::NativeDataImage& data_;
    const engine::NativeOffsetGlobals& offsets_;
};
} //namespace skeet::effects
