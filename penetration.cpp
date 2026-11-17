// Penetration & Autowall Implementation
#include "skeet/rage/ballistics.hpp"
#include <bit>
#include <cmath>
#include <limits>
#include <xmmintrin.h>
#include "skeet/rage/engine_ballistics.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/engine/trace.hpp"
#include <cstring>
#include <stdexcept>
#include "skeet/rage/point_penetration.hpp"


// ============================================================================
// Subcomponent: ballistics.cpp
// ============================================================================

namespace skeet {
namespace {
constexpr std::uint32_t exit_contents = 0x600400bU;
BallisticVector subtract(const BallisticVector& a, const BallisticVector& b) noexcept {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}
float length(const BallisticVector& v) noexcept {
    return std::sqrt((v[0] * v[0] + v[1] * v[1]) + v[2] * v[2]);
}
BallisticVector advance(const BallisticVector& p, const BallisticVector& d, float f) noexcept {
    return {d[0] * f + p[0], d[1] * f + p[1], d[2] * f + p[2]};
}
BallisticRay ray(const BallisticVector& start, const BallisticVector& end) {
    const auto delta = subtract(end, start);
    return {start, delta,
            (delta[1] * delta[1] + delta[0] * delta[0]) + delta[2] * delta[2] != 0.0f};
}
bool is_world(BallisticEngine& engine, const BallisticTrace& trace) {
    return trace.entity == engine.entity(0);
}
std::int32_t filter_type(BallisticEngine& engine, const BallisticFilter& filter) {
    // 4346FDD4 uses client GetTraceType 433686FC, a concrete constant 1.
    return filter.kind == BallisticFilterKind::world_only ? 1 : engine.filter_type(filter);
}
bool damageable_model(BallisticEngine& engine, std::int32_t index) {
    if (index <= 0)
        return false;
    const auto model = engine.model_info(index - 1);
    return model.entity_type == 6 && model.model_present && model.model_type == 3 &&
           model.studio_model_present && (model.activity_flags & 0x7fffU) != 0;
}
// 4338590E: negative Euclidean distance outside the finite segment.
float distance_to_segment(const BallisticVector& point, const BallisticVector& start,
                          const BallisticVector& end) {
    const auto d = subtract(end, start), p = subtract(point, start);
    const float segment_length = length(d);
    const float inverse = 1.0f / (segment_length + std::numeric_limits<float>::epsilon());
    const BallisticVector unit{inverse * d[0], inverse * d[1], inverse * d[2]};
    const float projection = (unit[1] * p[1] + unit[0] * p[0]) + unit[2] * p[2];
    if (projection < 0.0f)
        return -length(p);
    if (projection > segment_length)
        return -length(subtract(point, end));
    return length(subtract(point, advance(start, unit, projection)));
}
void clip_one(BallisticEngine& engine, BallisticEntity entity, const BallisticVector& start,
              const BallisticVector& end, float minimum, BallisticTrace& trace, float& fraction) {
    const auto info = engine.entity_info(entity);
    auto center = info.origin;
    center[2] = (info.maximum_z + info.minimum_z) * 0.5f + center[2];
    const float distance = distance_to_segment(center, start, end);
    if (distance < minimum || distance > 60.0f)
        return;
    BallisticTrace clipped;
    engine.clip_ray_to_entity(ray(start, end), 0x4600400bU, entity, clipped);
    if (fraction > clipped.fraction) {
        fraction = clipped.fraction;
        trace = clipped;
    }
}
}

void reset_ballistic_query(BallisticQuery& query) noexcept {
    query.trace.fraction = 1.0f;
    query.trace.hitgroup = -1;
    query.trace.hitbox = -1;
    query.remaining_penetrations = query.allow_penetration ? 4 : 0;
    query.damage = 0.0f;
    query.trace.all_solid = 0;
    query.trace.start_solid = 0;
    query.trace.entity = 0;
    query.point_count = 0;
    query.flags = 0;
}

bool ballistic_breakable(BallisticEngine& engine, BallisticEntity entity) {
    if (!entity)
        return false;
    const auto info = engine.breakable_info(entity);
    if (info.primary_class_matches)
        return info.primary_class_state == 0;
    const auto saved = engine.damage_mode(entity);
    if (info.alternate_class_matches && info.solid_type == 1 && info.collision_group == 7 &&
        (info.solid_flags & 0x800U) && info.health == -1 && !(info.effects & 0x2000000U) &&
        (info.first_scale != 0.0f || info.second_scale != 0.0f))
        engine.set_damage_mode(entity, 2);
    const bool result = engine.has_engine_breakable_callback() && engine.engine_breakable(entity);
    engine.set_damage_mode(entity, saved);
    return result;
}

bool ballistic_enemy(BallisticEngine& engine, BallisticEntity source, BallisticEntity target) {
    if (!source || !target || source == target)
        return false;
    const auto a = engine.entity_info(source), b = engine.entity_info(target);
    const auto rules = engine.rules();
    if (rules.game_type == 6)
        return a.survival_team == -1 || a.survival_team != b.survival_team;
    if (a.team != 2 && a.team != 3)
        return false;
    if ((a.team == 2 && b.team == 3) || (a.team == 3 && b.team == 2))
        return true;
    return rules.teammates_are_enemies;
}

bool trace_ballistic_exit(BallisticEngine& engine, const BallisticVector& direction,
                          const BallisticTrace& entry, const BallisticFilter& filter,
                          BallisticTrace& exit, BallisticEntity source, float step,
                          float maximum_distance) {
    std::uint32_t initial_contents = 0;
    float distance = 0.0f;
    const bool world_only = filter_type(engine, filter) == 1;
    if (!(maximum_distance >= 0.0f))
        return false;
    do {
        distance += step;
        const auto point = advance(entry.end, direction, distance);
        const auto contents = engine.point_contents(point, filter.mask, world_only);
        if (!initial_contents)
            initial_contents = contents;
        if (!(contents & exit_contents) ||
            ((contents & 0x40000000U) && contents != initial_contents)) {
            BallisticTraceRequest request{ray(point, advance(point, direction, -step)),
                                          filter.exit_mask,
                                          filter.kind,
                                          &filter,
                                          source,
                                          0};
            if (filter.exit_mask & 0x40000000U)
                request.filter_kind = BallisticFilterKind::skip_source;
            engine.trace_ray(request, exit);
            if (exit.start_solid && std::int16_t(exit.surface_flags) < 0) {
                request = {ray(point, entry.end),
                           exit_contents,
                           BallisticFilterKind::skip_source_and_hit,
                           &filter,
                           source,
                           exit.entity};
                engine.trace_ray(request, exit);
                if ((exit.fraction < 1.0f || exit.all_solid) && !exit.start_solid)
                    return true;
            } else if ((exit.fraction < 1.0f || exit.all_solid) && !exit.start_solid) {
                if (!(exit.surface_flags & 0x80U) ||
                    (ballistic_breakable(engine, exit.entity) &&
                     ballistic_breakable(engine, entry.entity)) ||
                    (entry.surface_flags & 0x80U))
                    return true;
            } else if (entry.entity && !is_world(engine, entry) &&
                       ballistic_breakable(engine, entry.entity)) {
                exit.surface_property = entry.surface_property;
                exit.end = {direction[0] + entry.end[0], direction[1] + entry.end[1],
                            direction[2] + entry.end[2]};
                return true;
            }
        }
    } while (distance <= maximum_distance);
    return false;
}

float ballistic_exit_budget(float damage, float penetration, const PenetrationSurface& surface,
                            const BallisticTrace& trace, float filter_minimum) noexcept {
    float budget = 90.0f;
    if (!(filter_minimum > 10.0f))
        return budget;
    float combined, damage_modifier;
    if (surface.material == 0x47 || surface.material == 0x59) {
        combined = 3.0f;
        damage_modifier = 0.05f;
    } else {
        combined = (trace.contents & 8U) || (trace.surface_flags & 0x80U)
                       ? 1.0f
                       : (surface.penetration_modifier + 1.0f) * 0.5f;
        damage_modifier = 0.16f;
    }
    if (surface.material == 0x55 || surface.material == 0x57)
        combined = 3.0f;
    else if (surface.material == 0x4c)
        combined = 2.0f;
    const float inverse = maximum_number(0.0f, 1.0f / combined);
    // The pre-trace budget divides in float; the post-trace damage loss uses double.
    const float weapon = maximum_number(0.0f, float(double(3.0f / penetration) * 1.25));
    const float loss =
        float(double(weapon) * (double(inverse) * 3.0) + double(damage * damage_modifier));
    const float available = (damage - (filter_minimum - 1.0f)) - loss;
    const float root = std::sqrt(available) * 4.898979663848877f / std::sqrt(inverse);
    const float magnitude = std::bit_cast<float>(std::bit_cast<std::uint32_t>(root) & 0x7fffffffU);
    const float candidate = (magnitude + 4.0f) + 1.0f;
    return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(candidate), _mm_set_ss(budget)));
}

bool handle_ballistic_penetration(BallisticEngine& engine, const BallisticVector& direction,
                                  BallisticVector& position, BallisticEntity source,
                                  const BallisticFilter& filter, BallisticQuery& query,
                                  const PenetrationSurface& surface, float& damage) {
    const auto& entry = query.trace;
    if (!(query.remaining_penetrations || (entry.contents & 8U) || (entry.surface_flags & 0x80U) ||
          surface.material == 0x59 || surface.material == 0x47) ||
        !(query.penetration > 0.0f) || query.remaining_penetrations <= 0)
        return true;
    const float budget =
        ballistic_exit_budget(damage, query.penetration, surface, entry, filter.minimum_damage);
    BallisticTrace exit;
    if (entry.hitbox > 0 && is_world(engine, entry) && damageable_model(engine, entry.hitbox))
        query.flags |= 2U;
    if (!trace_ballistic_exit(engine, direction, entry, filter, exit, source, 4.0f, budget)) {
        const bool world_only = filter_type(engine, filter) == 1;
        if (!(engine.point_contents(entry.end, exit_contents, world_only) & exit_contents))
            return true;
    }
    const auto exit_surface = engine.surface(exit.surface_property);
    SurfacePenetration penetration{entry.end,
                                   exit.end,
                                   surface,
                                   exit_surface,
                                   (entry.contents & 8U) != 0,
                                   (entry.surface_flags & 0x80U) != 0,
                                   query.penetration};
    BulletPenetrationState state{position, damage, query.remaining_penetrations};
    const bool continued = apply_surface_penetration(state, penetration);
    damage = state.damage;
    if (continued) {
        position = state.position;
        query.remaining_penetrations = state.remaining_penetrations;
    }
    return !continued;
}

void clip_ballistic_entities(BallisticEngine& engine, const BallisticVector& start,
                             const BallisticVector& end, float minimum, float fraction_limit,
                             const BallisticFilter& filter, BallisticTrace& trace) {
    float fraction = fraction_limit;
    if (fraction_limit < 0.0f)
        fraction = trace.fraction;
    if (filter.preferred_entity) {
        clip_one(engine, filter.preferred_entity, start, end, minimum, trace, fraction);
        return;
    }
    const auto count = engine.maximum_clients();
    for (std::int32_t index = 1; index <= count; ++index) {
        const auto entity = engine.entity(index);
        if (!entity)
            continue;
        const auto info = engine.clip_entity_info(entity);
        if (info.virtual_278 && !info.dormant && !info.target_state &&
            filter.kind != BallisticFilterKind::world_only &&
            engine.accepts(filter, entity, 0x4600400bU))
            clip_one(engine, entity, start, end, minimum, trace, fraction);
    }
}

std::int32_t trace_ballistic_bullet(BallisticEngine& engine, BallisticEntity source,
                                    const BallisticVector& start, const BallisticVector& target,
                                    const BallisticFilter& filter, BallisticQuery& query) {
    reset_ballistic_query(query);
    engine.bind_source(filter, source);
    engine.bind_last_hit(filter, 0);
    const auto delta = subtract(target, start);
    const float distance = length(delta);
    const float inverse = 1.0f / (distance + std::numeric_limits<float>::epsilon());
    const BallisticVector direction{inverse * delta[0], inverse * delta[1], inverse * delta[2]};
    float damage = query.base_damage;
    const float minimum = maximum_number(1.0f, filter.minimum_damage - 5.0f);
    if (distance > query.range + 16.0f)
        return 0;
    if (minimum > damage) {
        query.damage = 0.0f;
        return 0;
    }
    const float endpoint_distance = distance - 0.1f;
    BallisticVector position = start;
    float travelled = 0.0f;
    for (;;) {
        const float remaining = query.range - travelled;
        const auto end = advance(position, direction, remaining);
        engine.trace_ray({ray(position, end), filter.mask, filter.kind, &filter, source, 0},
                         query.trace);
        if (filter.mask & 0x40000000U) {
            const auto extended = advance(end, direction, 40.0f);
            const float fraction =
                length(subtract(query.trace.end, position)) / length(subtract(position, extended));
            clip_ballistic_entities(engine, position, extended, 0.0f, fraction, filter,
                                    query.trace);
        }
        if (length(subtract(query.trace.end, start)) >= endpoint_distance) {
            const float fraction =
                length(subtract(position, target)) / length(subtract(position, end));
            const float exponent = (fraction * remaining + travelled) * 0.002f;
            damage =
                float(std::pow(double(query.range_modifier), double(exponent)) * double(damage));
            query.trace.end = target;
            break;
        }
        travelled = query.trace.fraction * remaining + travelled;
        damage = float(std::pow(double(query.range_modifier), double(travelled * 0.002f)) *
                       double(damage));
        if (query.trace.fraction == 1.0f)
            break;
        query.points[query.point_count++] = query.trace.end;
        if (minimum > damage) {
            damage = 0.0f;
            break;
        }
        const auto surface = engine.surface(query.trace.surface_property);
        if ((travelled > 3000.0f && query.penetration > 0.0f) ||
            double(surface.penetration_modifier) < 0.1)
            query.remaining_penetrations = 0;
        if (!(filter.mask & 0x40000000U))
            query.trace.hitgroup = 1;
        BallisticEntity hit = 0;
        if (query.trace.entity && engine.entity_info(query.trace.entity).virtual_278)
            hit = query.trace.entity;
        engine.bind_last_hit(filter, hit);
        if (query.trace.entity && engine.entity_info(query.trace.entity).virtual_280) {
            if (!hit) {
                damage = 0.0f;
                break;
            }
            const auto info = engine.entity_info(hit);
            if (!info.target_state) {
                if (info.target_index_blocked || (source && !ballistic_enemy(engine, source, hit)))
                    damage = 0.0f;
                else if ((filter.mask & 0x40000000U) &&
                         std::uint32_t(query.trace.hitgroup) - 1U <= 7U)
                    damage = hitgroup_damage_scale(info.armor, info.helmet, query.trace.hitgroup,
                                                   query.weapon_id, query.armor_ratio) *
                             damage;
                break;
            }
        }
        if (query.remaining_penetrations <= 0) {
            damage = 0.0f;
            break;
        }
        if (handle_ballistic_penetration(engine, direction, position, source, filter, query,
                                         surface, damage)) {
            query.flags |= 4U;
            damage = 0.0f;
            break;
        }
        if (minimum > damage) {
            damage = 0.0f;
            break;
        }
    }
    if (damage < 0.0f)
        damage = 0.0f;
    query.damage = damage;
    return _mm_cvttss_si32(_mm_set_ss(damage));
}
} // namespace skeet

// ============================================================================
// Subcomponent: engine_ballistics.cpp
// ============================================================================

namespace skeet {
namespace {
template <class T> T field(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
void* pointer(BallisticEntity value) noexcept {
    return reinterpret_cast<void*>(std::uintptr_t(value));
}
BallisticEntity address(void* value) noexcept {
    return static_cast<BallisticEntity>(reinterpret_cast<std::uintptr_t>(value));
}
std::uint32_t offset(const EngineBallisticBindings& b, std::uint32_t slot, std::uint32_t key,
                     std::uint32_t salt) {
    return *b.offsets.pointers.at(slot) ^ b.offsets.keys.at(key) ^ salt;
}
std::int32_t convar_integer(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1c);
        if (parent == object)
            break;
        object = parent;
    }
    return std::bit_cast<std::int32_t>(field<std::uint32_t>(object, 0x30) ^
                                       std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
engine::TraceRay native_ray(const BallisticRay& source) noexcept {
    engine::TraceRay ray;
    ray.start = {source.start[0], source.start[1], source.start[2], 0.0f};
    ray.delta = {source.delta[0], source.delta[1], source.delta[2], 0.0f};
    ray.is_ray = 1;
    ray.swept = source.swept;
    return ray;
}

#if defined(_M_IX86) || defined(__i386__)
#define BALLISTIC_MEMBER __thiscall
#else
#define BALLISTIC_MEMBER
#endif
//4346EDC0 forwards only the trace-facing methods needed here into the live
//434758B4 table. The receiver remains the original four-word native prefix.
struct SkipFilter {
    engine::TraceSkipFilterPrefix prefix;
    const EngineBallisticBindings* bindings;
    static bool BALLISTIC_MEMBER should_hit(void* receiver, void* entity, std::uint32_t mask) {
        const auto& self = *static_cast<SkipFilter*>(receiver);
        const auto table = self.bindings->skip_filter_vtable;
        if (!table)
            throw std::logic_error("Required ballistic skip filter table is not bound");
        return reinterpret_cast<engine::MemberFunction<bool, void*, std::uint32_t>>(table[0])(
            receiver, entity, mask);
    }
    static std::int32_t BALLISTIC_MEMBER trace_type(void* receiver) {
        const auto& self = *static_cast<SkipFilter*>(receiver);
        const auto table = self.bindings->skip_filter_vtable;
        if (!table)
            throw std::logic_error("Required ballistic skip filter table is not bound");
        return reinterpret_cast<engine::MemberFunction<std::int32_t>>(table[1])(receiver);
    }
    SkipFilter(BallisticEntity source, const EngineBallisticBindings& b)
        : prefix{table(), pointer(source), 0, 0}, bindings(&b) {}
    static void* const* table() noexcept {
        static void* const methods[]{reinterpret_cast<void*>(should_hit),
                                     reinterpret_cast<void*>(trace_type)};
        return methods;
    }
};

//4346DCEC forwards all of its used methods to43475B90.4346EEBC replaces
//ShouldHitEntity/GetTraceType with false/0 and retains the same real setters.
struct ExtendedFilter {
    void* const* vtable;
    std::array<std::uint32_t, 5> fields_04{};
    std::uint32_t mask = 0, exit_mask = 0;
    float minimum_damage = 0;
    const EngineBallisticBindings* bindings;
    BallisticFilterKind kind;
    std::array<void*, 5> methods{};
    static ExtendedFilter& self(void* receiver) noexcept {
        return *static_cast<ExtendedFilter*>(receiver);
    }
    static void* method(void* receiver, std::size_t slot) {
        const auto table = self(receiver).bindings->extended_filter_vtable;
        if (!table)
            throw std::logic_error("Required ballistic extended filter table is not bound");
        return table[slot];
    }
    static bool BALLISTIC_MEMBER should_hit(void* receiver, void* entity, std::uint32_t mask) {
        const auto& filter = self(receiver);
        if (filter.kind == BallisticFilterKind::reject_entities ||
            filter.kind == BallisticFilterKind::world_only)
            return false;
        if (filter.kind == BallisticFilterKind::reject_players && entity) {
            auto current = engine::invoke<void*>(entity, 7);
            const auto& b = *filter.bindings;
            while (current) {
                if (field<std::int32_t>(current, offset(b, 0x434683c8, 0x4346758c, 0xe4e2d1a7)) ==
                        11 ||
                    engine::invoke<bool>(current, 158))
                    return false;
                const auto handle = field<std::int32_t>(
                    current, offset(b, 0x43469344, 0x43467018, 0x49d93e62) - 0x18);
                if (handle == -1)
                    break;
                current = engine::invoke<void*>(b.entity_list, 4, handle);
            }
        }
        return reinterpret_cast<engine::MemberFunction<bool, void*, std::uint32_t>>(
            method(receiver, 0))(receiver, entity, mask);
    }
    static std::int32_t BALLISTIC_MEMBER trace_type(void* receiver) {
        if (self(receiver).kind == BallisticFilterKind::reject_entities)
            return 0;
        if (self(receiver).kind == BallisticFilterKind::world_only)
            return 1;
        return reinterpret_cast<engine::MemberFunction<std::int32_t>>(method(receiver, 1))(
            receiver);
    }
    static void BALLISTIC_MEMBER set_source(void* receiver, void* entity) {
        reinterpret_cast<engine::MemberFunction<void, void*>>(method(receiver, 2))(receiver,
                                                                                   entity);
    }
    static void BALLISTIC_MEMBER set_last_hit(void* receiver, void* entity) {
        reinterpret_cast<engine::MemberFunction<void, void*>>(method(receiver, 4))(receiver,
                                                                                   entity);
    }
    ExtendedFilter(const EngineBallisticBindings& b)
        : vtable(nullptr), bindings(&b), kind(BallisticFilterKind::supplied) {
        initialize();
    }
    ExtendedFilter(const EngineBallisticBindings& b, const BallisticFilter& f)
        : vtable(nullptr), fields_04(f.native_fields_04), mask(f.mask), exit_mask(f.exit_mask),
          minimum_damage(f.minimum_damage), bindings(&b), kind(f.kind) {
        initialize();
        fields_04[4] = f.preferred_entity;
    }
    ExtendedFilter(const ExtendedFilter&) = delete;
    ExtendedFilter& operator=(const ExtendedFilter&) = delete;
    void save(const BallisticFilter& f) const noexcept {
        f.native_fields_04 = fields_04;
    }
    void initialize() {
        const auto external = bindings->extended_filter_vtable;
        if (!external)
            throw std::logic_error("Required ballistic extended filter table is not bound");
        //The owners use0/1/2/4. Preserve the genuine slot3 pointer without
        //inventing a C++ signature for the unused external engine method.
        methods = {reinterpret_cast<void*>(should_hit), reinterpret_cast<void*>(trace_type),
                   reinterpret_cast<void*>(set_source), external[3],
                   reinterpret_cast<void*>(set_last_hit)};
        vtable = methods.data();
    }
};
#undef BALLISTIC_MEMBER
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(ExtendedFilter, fields_04) == 4);
static_assert(offsetof(ExtendedFilter, mask) == 0x18);
static_assert(offsetof(ExtendedFilter, minimum_damage) == 0x20);
#endif
}

std::int32_t EngineBallistics::filter_type(const BallisticFilter& filter) {
    if (filter.kind == BallisticFilterKind::world_only)
        return 1;
    if (filter.kind == BallisticFilterKind::reject_entities)
        return 0;
    if (filter.kind == BallisticFilterKind::reject_players) {
        ExtendedFilter native(bindings_, filter);
        return engine::invoke<std::int32_t>(&native, 1);
    }
    return engine::invoke<std::int32_t>(filter.object, 1);
}
void EngineBallistics::bind_source(const BallisticFilter& filter, BallisticEntity source) {
    if (filter.kind == BallisticFilterKind::reject_entities ||
        filter.kind == BallisticFilterKind::reject_players ||
        filter.kind == BallisticFilterKind::world_only) {
        ExtendedFilter native(bindings_, filter);
        engine::invoke<void>(&native, 2, pointer(source));
        native.save(filter);
        return;
    }
    engine::invoke<void>(filter.object, 2, pointer(source));
}
void EngineBallistics::bind_last_hit(const BallisticFilter& filter, BallisticEntity hit) {
    if (filter.kind == BallisticFilterKind::reject_entities ||
        filter.kind == BallisticFilterKind::reject_players ||
        filter.kind == BallisticFilterKind::world_only) {
        ExtendedFilter native(bindings_, filter);
        engine::invoke<void>(&native, 4, pointer(hit));
        native.save(filter);
        return;
    }
    engine::invoke<void>(filter.object, 4, pointer(hit));
}
bool EngineBallistics::accepts(const BallisticFilter& filter, BallisticEntity entity,
                               std::uint32_t mask) {
    if (filter.kind == BallisticFilterKind::world_only ||
        filter.kind == BallisticFilterKind::reject_entities)
        return false;
    if (filter.kind == BallisticFilterKind::reject_players) {
        ExtendedFilter native(bindings_, filter);
        return engine::invoke<bool>(&native, 0, pointer(entity), mask);
    }
    return engine::invoke<bool>(filter.object, 0, pointer(entity), mask);
}
std::uint32_t EngineBallistics::point_contents(const BallisticVector& point, std::uint32_t mask,
                                               bool world_only) {
    //433C098C uses slot1 with two arguments;433E111C uses slot0 and a null
    //out-entity argument. These are different genuine engine entrypoints.
    if (world_only)
        return engine::invoke<std::uint32_t>(bindings_.engine_trace, 1, &point, mask);
    return engine::invoke<std::uint32_t>(bindings_.engine_trace, 0, &point, mask,
                                         static_cast<void**>(nullptr));
}
void EngineBallistics::trace_ray(const BallisticTraceRequest& request, BallisticTrace& result) {
    auto ray = native_ray(request.ray);
    switch (request.filter_kind) {
    case BallisticFilterKind::supplied:
        if (!request.filter || !request.filter->object)
            throw std::logic_error("Required supplied ballistic trace filter is not bound");
        engine::invoke<void>(bindings_.engine_trace, 5, &ray, request.mask, request.filter->object,
                             &result);
        return;
    case BallisticFilterKind::skip_source: {
        SkipFilter filter(request.source, bindings_);
        engine::invoke<void>(bindings_.engine_trace, 5, &ray, request.mask, &filter, &result);
        return;
    }
    case BallisticFilterKind::skip_source_and_hit: {
        ExtendedFilter filter(bindings_);
        filter.fields_04[0] = request.source;
        filter.fields_04[3] = request.skip; //native exit local+10
        engine::invoke<void>(bindings_.engine_trace, 5, &ray, request.mask, &filter, &result);
        return;
    }
    case BallisticFilterKind::world_only: {
        if (!request.filter)
            throw std::logic_error("Required ballistic world filter prefix is not bound");
        ExtendedFilter filter(bindings_, *request.filter);
        engine::invoke<void>(bindings_.engine_trace, 5, &ray, request.mask, &filter, &result);
        filter.save(*request.filter);
        return;
    }
    case BallisticFilterKind::reject_entities:
    case BallisticFilterKind::reject_players: {
        if (!request.filter)
            throw std::logic_error("Required ballistic filter prefix is not bound");
        ExtendedFilter filter(bindings_, *request.filter);
        engine::invoke<void>(bindings_.engine_trace, 5, &ray, request.mask, &filter, &result);
        filter.save(*request.filter);
        return;
    }
    }
    throw std::logic_error("Invalid ballistic filter kind");
}
void EngineBallistics::clip_ray_to_entity(const BallisticRay& source, std::uint32_t mask,
                                          BallisticEntity entity, BallisticTrace& result) {
    auto ray = native_ray(source);
    engine::invoke<void>(bindings_.engine_trace, 3, &ray, mask, pointer(entity), &result);
}
PenetrationSurface EngineBallistics::surface(std::int16_t property) {
    const auto data =
        engine::invoke<const void*>(bindings_.surface_properties, 5, std::int32_t(property));
    return {field<std::uint16_t>(data, 0x60), field<float>(data, 0x58)};
}
BallisticEntity EngineBallistics::entity(std::int32_t index) {
    return address(engine::invoke<void*>(bindings_.entity_list, 3, index));
}
std::int32_t EngineBallistics::maximum_clients() {
    return *bindings_.maximum_clients;
}

BallisticEntityInfo EngineBallistics::read_entity(BallisticEntity value, bool clipping) {
    auto* entity = pointer(value);
    BallisticEntityInfo result;
    result.virtual_278 = engine::invoke<bool>(entity, 158);
    result.virtual_280 = engine::invoke<bool>(entity, 160);
    if (!result.virtual_278)
        return result;
    const auto networkable = static_cast<std::byte*>(entity) + 8;
    result.dormant = engine::invoke<bool>(networkable, 9);
    const auto index = engine::invoke<std::int32_t>(networkable, 10);
    result.target_index_blocked =
        bindings_.player_adjustments.records[std::uint32_t(index)].adjustments.whitelist != 0;
    result.target_state =
        clipping
            ? field<std::uint8_t>(entity, offset(bindings_, 0x43468e78, 0x434684e4, 0x9d920e35))
            : field<std::uint8_t>(entity, offset(bindings_, 0x4346a5dc, 0x43468430, 0xcde290d2));
    result.helmet =
        field<std::uint8_t>(entity, offset(bindings_, 0x43468c04, 0x43468fe8, 0x7d6280c7));
    result.armor =
        field<std::int32_t>(entity, offset(bindings_, 0x43467908, 0x434682e4, 0x47a6f50c));
    result.team =
        field<std::int32_t>(entity, offset(bindings_, 0x434688b0, 0x4346a518, 0x8b5586f6));
    result.survival_team =
        field<std::int32_t>(entity, offset(bindings_, 0x434678ac, 0x43467b7c, 0x430c8993));
    result.origin = *engine::invoke<const BallisticVector*>(entity, 10);
    result.minimum_z = (*engine::invoke<const BallisticVector*>(entity, 76))[2];
    result.maximum_z = (*engine::invoke<const BallisticVector*>(entity, 77))[2];
    return result;
}
BallisticEntityInfo EngineBallistics::entity_info(BallisticEntity entity) {
    return read_entity(entity, false);
}
BallisticEntityInfo EngineBallistics::clip_entity_info(BallisticEntity entity) {
    return read_entity(entity, true);
}

BallisticBreakableInfo EngineBallistics::breakable_info(BallisticEntity value) {
    void* entity = pointer(value);
    BallisticBreakableInfo result;
    if (bindings_.primary_breakable_class) {
        const auto unknown = engine::invoke<void*>(entity, 4);
        if (unknown) {
            const auto type = engine::invoke<void*>(unknown, 2);
            result.primary_class_matches = type && type == bindings_.primary_breakable_class;
            if (result.primary_class_matches) {
                result.primary_class_state = field<std::uint8_t>(
                    entity, offset(bindings_, 0x4346a360, 0x43468314, 0xf0374295));
                return result;
            }
        }
    }
    if (!bindings_.alternate_breakable_class)
        return result;
    const auto unknown = engine::invoke<void*>(entity, 4);
    if (!unknown)
        return result;
    const auto type = engine::invoke<void*>(unknown, 2);
    result.alternate_class_matches = type && type == bindings_.alternate_breakable_class;
    if (!result.alternate_class_matches)
        return result;
    auto collideable = engine::invoke<void*>(entity, 3);
    result.solid_type = engine::invoke<std::int32_t>(collideable, 11);
    if (result.solid_type != 1)
        return result;
    result.collision_group =
        field<std::uint8_t>(entity, offset(bindings_, 0x43468bd4, 0x4346a678, 0x7501cb91));
    if (result.collision_group != 7)
        return result;
    collideable = engine::invoke<void*>(entity, 3);
    result.solid_flags = engine::invoke<std::uint32_t>(collideable, 12);
    if (!(result.solid_flags & 0x800))
        return result;
    result.health =
        field<std::int32_t>(entity, offset(bindings_, 0x43467fcc, 0x43467938, 0x8d1bd8d0));
    if (result.health != -1)
        return result;
    result.effects =
        field<std::uint32_t>(entity, offset(bindings_, 0x434688d0, 0x43468c24, 0xe2fbe695));
    if (result.effects & 0x2000000)
        return result;
    result.first_scale =
        field<float>(entity, offset(bindings_, 0x43468bd0, 0x43469278, 0x8dbc9874));
    if (result.first_scale == 0.0f)
        result.second_scale =
            field<float>(entity, offset(bindings_, 0x43467948, 0x4346919c, 0x68140cd8));
    return result;
}
BallisticModelInfo EngineBallistics::model_info(std::int32_t index) {
    BallisticModelInfo result;
    const auto object = engine::invoke<void*>(bindings_.model_collision, 4, index);
    if (!object)
        return result;
    result.entity_type = engine::invoke<std::int32_t>(object, 11);
    if (result.entity_type != 6)
        return result;
    const auto model = engine::invoke<const void*>(object, 7);
    result.model_present = model != nullptr;
    if (!model)
        return result;
    result.model_type = field<std::int32_t>(model, 0x110);
    if (result.model_type != 3)
        return result;
    result.studio_model_present = engine::invoke<void*>(bindings_.model_info, 32, model) != nullptr;
    if (!result.studio_model_present)
        return result;
    const auto cycle = engine::invoke<float>(object, 19);
    const auto flags = engine::invoke<const std::uint16_t*>(
        bindings_.model_cache, 16, std::int32_t(field<std::uint16_t>(model, 0x138)), cycle);
    if (flags)
        result.activity_flags = *flags;
    return result;
}
std::uint8_t EngineBallistics::damage_mode(BallisticEntity entity) {
    return field<std::uint8_t>(pointer(entity),
                               offset(bindings_, 0x43469198, 0x4346a538, 0xd5fd2e2c) + 0x18);
}
void EngineBallistics::set_damage_mode(BallisticEntity entity, std::uint8_t value) {
    auto* destination = static_cast<std::byte*>(pointer(entity)) +
                        offset(bindings_, 0x43469198, 0x4346a538, 0xd5fd2e2c) + 0x18;
    std::memcpy(destination, &value, 1);
}
bool EngineBallistics::has_engine_breakable_callback() {
    return bindings_.breakable != nullptr;
}
bool EngineBallistics::engine_breakable(BallisticEntity entity) {
    return bindings_.breakable(pointer(entity));
}
BallisticRules EngineBallistics::rules() {
    const auto game_type = convar_integer(bindings_.game_type.get());
    if (game_type == 6)
        return {game_type, false};
    return {game_type, convar_integer(bindings_.teammates_are_enemies.get()) != 0};
}
} //namespace skeet

// ============================================================================
// Subcomponent: point_penetration.cpp
// ============================================================================

namespace skeet {
namespace {
struct RestoreSuppression {
    std::uint8_t* flag;
    std::uint8_t saved = 0;
    explicit RestoreSuppression(std::uint8_t* value) : flag(value) {
        if (flag) {
            saved = *flag;
            *flag = 0;
        }
    }
    ~RestoreSuppression() {
        if (flag)
            *flag = saved;
    }
};
}
std::int32_t trace_point_penetration(BallisticEngine& engine, const BallisticVector& start,
                                     const BallisticVector& target, std::int32_t requested_damage,
                                     BallisticQuery& query, std::uint8_t* suppression_flag) {
    RestoreSuppression suppression(suppression_flag);
    reset_ballistic_query(query);
    const BallisticFilter filter{nullptr,   0,    0x600400b,
                                 0x600400b, 1.0f, BallisticFilterKind::world_only};
    const auto offset = subtract(target, start);
    const float distance = length(offset);
    const float inverse = 1.0f / (distance + std::numeric_limits<float>::epsilon());
    const BallisticVector direction{offset[0] * inverse, offset[1] * inverse, offset[2] * inverse};
    const float minimum = maximum_number(1.0f, float(requested_damage) - 5.0f);
    float damage = 0.0f;
    if (!(distance > query.range) && !(minimum > query.base_damage)) {
        damage = query.base_damage;
        BallisticVector position = start;
        float travelled = 0.0f;
        for (;;) {
            const float remaining = query.range - travelled;
            const auto endpoint = advance(position, direction, remaining);
            const auto delta = subtract(endpoint, position);
            const bool swept =
                (delta[1] * delta[1] + delta[0] * delta[0]) + delta[2] * delta[2] != 0.0f;
            engine.trace_ray({{position, delta, swept}, filter.mask, filter.kind, &filter, 0, 0},
                             query.trace);
            if (length(subtract(query.trace.end, start)) >= distance) {
                const float fraction =
                    length(subtract(position, target)) / length(subtract(position, endpoint));
                const float exponent = (fraction * remaining + travelled) * 0.002f;
                damage = float(std::pow(double(query.range_modifier), double(exponent)) *
                               double(damage));
                query.trace.end = target;
                break;
            }
            travelled = query.trace.fraction * remaining + travelled;
            damage = float(std::pow(double(query.range_modifier), double(travelled * 0.002f)) *
                           double(damage));
            if (query.trace.fraction == 1.0f)
                break;
            query.points[query.point_count++] = query.trace.end;
            if (minimum > damage) {
                damage = 0.0f;
                break;
            }
            const auto surface = engine.surface(query.trace.surface_property);
            if ((travelled > 3000.0f && query.penetration > 0.0f) ||
                double(surface.penetration_modifier) < 0.1) {
                query.remaining_penetrations = 0;
                damage = 0.0f;
                break;
            }
            if (query.remaining_penetrations <= 0) {
                damage = 0.0f;
                break;
            }
            if (handle_ballistic_penetration(engine, direction, position, 0, filter, query, surface,
                                             damage)) {
                query.flags |= 4;
                damage = 0.0f;
                break;
            }
            if (minimum > damage) {
                damage = 0.0f;
                break;
            }
        }
        if (damage < 0.0f)
            damage = 0.0f;
    }
    query.damage = damage;
    return _mm_cvttss_si32(_mm_set_ss(damage));
}
} // namespace skeet
