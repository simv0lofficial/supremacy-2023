#pragma once
#include "collision.hpp"

namespace skeet {

struct MovementEngineCollisionBindings {
    void* const& engine_trace; //43468304; actual EngineTrace slot5
    void* const& surface_properties; //43469268; actual surface physics slot4
    void* const* const& skip_filter_vtable; //live genuine engine table434758B4
};

// Production engine boundary for the recovered movement collision routines.
// The adapter only packs Ray_t/filter ABI and invokes the genuine interfaces;
// all stepping, clipping, surface scaling and collision decisions stay there.
class EngineMovementCollision final : public MovementCollisionEngine {
    MovementEngineCollisionBindings bindings_;

public:
    explicit EngineMovementCollision(const MovementEngineCollisionBindings& bindings)
        : bindings_(bindings) {}
    void trace_ray(const MovementCollisionRay&, std::uint32_t mask, MovementCollisionFilter,
                   MovementCollisionTrace&) override;
    float surface_friction(std::int16_t surface_property) override;
};

} //namespace skeet
