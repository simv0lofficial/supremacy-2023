#pragma once
#include "types.hpp"
#include "skeet/engine/trace.hpp"

namespace skeet {

struct MovementCollisionRay {
    MovementVector start, delta, start_offset, extents;
    bool is_ray, swept;
};

struct MovementCollisionTrace {
    MovementVector start, end, plane_normal;
    float plane_distance;
    std::uint8_t plane_type, plane_signs;
    std::uint16_t plane_padding;
    float fraction;
    std::uint32_t contents;
    std::uint16_t displacement_flags;
    std::uint8_t all_solid, start_solid;
    float fraction_left_solid;
    std::uint32_t surface_name;
    std::int16_t surface_property;
    std::uint16_t surface_flags;
    std::int32_t hitgroup;
    std::int16_t physics_bone;
    std::uint16_t physics_padding;
    std::uint32_t entity;
    std::int32_t hitbox;
};
static_assert(sizeof(MovementCollisionTrace) == engine::trace_result_size);
static_assert(offsetof(MovementCollisionTrace, fraction) == engine::trace_fraction_offset);
static_assert(offsetof(MovementCollisionTrace, entity) == engine::trace_entity_offset);

enum class MovementCollisionFilterKind {
    // 0x434727C0: ShouldHitEntity always false, GetTraceType returns zero.
    reject_entities,
    skip_player,
    // 0x434701D0: ShouldHitEntity always false, GetTraceType returns one.
    // Kept distinct from reject_entities, whose native trace type is zero.
    world_only,
};

struct MovementCollisionFilter {
    MovementCollisionFilterKind kind;
    std::uint32_t player;
    std::int32_t collision_group;
    std::uint32_t extra;
};

class MovementCollisionEngine {
public:
    virtual ~MovementCollisionEngine() = default;
    // Actual EngineTrace::TraceRay boundary. Collision decisions stay in source.
    virtual void trace_ray(const MovementCollisionRay&, std::uint32_t mask, MovementCollisionFilter,
                           MovementCollisionTrace&) = 0;
    // Actual surface-properties interface +0x10, friction output only.
    virtual float surface_friction(std::int16_t surface_property) = 0;
};

// Native 0x433AC5BD / 0x4341873D. The engine adapter supplies the optional
// byte pointed to by the native trace flag; nested guards restore in order.
class MovementTraceFlagGuard {
    std::uint8_t* flag_;
    std::uint8_t saved_;

public:
    explicit MovementTraceFlagGuard(std::uint8_t* flag) noexcept;
    ~MovementTraceFlagGuard();
    MovementTraceFlagGuard(const MovementTraceFlagGuard&) = delete;
    MovementTraceFlagGuard& operator=(const MovementTraceFlagGuard&) = delete;
    void restore() noexcept;
};

// Complete wrapper 0x433CC0F2..0x433CC130, including flag restoration.
void trace_movement_world(MovementCollisionEngine& engine, const MovementCollisionRay& ray,
                          std::uint32_t mask, MovementCollisionTrace& trace,
                          std::uint8_t* engine_trace_flag);

MovementCollisionRay make_movement_hull_ray(MovementVector start, MovementVector end,
                                            MovementVector minimum,
                                            MovementVector maximum) noexcept;

} // namespace skeet
