#pragma once
#include "skeet/rage/ballistics.hpp"
#include "skeet/engine/interface_call.hpp"
#include <stdexcept>

namespace skeet::legit {
// Complete4346DCEC ABI prefix. Its five native entries only tail-forward to
// the actual engine table43475B90; the prefix owns source/last-hit state.
struct BulletTraceFilter {
    void* const* vtable;
    std::array<std::uint32_t, 5> fields_04{};
    std::uint32_t mask = 0x4600400b, exit_mask = 0x4600400b;
    float minimum_damage = 1.0f;
    explicit BulletTraceFilter(void* const* table) : vtable(table) {
        if (!table)
            throw std::logic_error("LEGIT requires the actual engine trace-filter table");
    }
    BallisticFilter binding() noexcept {
        return {this, fields_04[4], mask, exit_mask, minimum_damage};
    }
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(BulletTraceFilter) == 0x24);
static_assert(offsetof(BulletTraceFilter, mask) == 0x18);
static_assert(offsetof(BulletTraceFilter, minimum_damage) == 0x20);
#endif
// Both software entity clipping and genuine TraceRay use the same native ABI
// filter object. No should-hit decision is delegated to a generic substitute.
class FilterBallistics final : public BallisticEngine {
    BallisticEngine& inner;

public:
    explicit FilterBallistics(BallisticEngine& engine) : inner(engine) {}
    std::int32_t filter_type(const BallisticFilter& f) override {
        return engine::invoke<std::int32_t>(f.object, 1);
    }
    void bind_source(const BallisticFilter& f, BallisticEntity e) override {
        engine::invoke<void>(f.object, 2, e);
    }
    void bind_last_hit(const BallisticFilter& f, BallisticEntity e) override {
        engine::invoke<void>(f.object, 4, e);
    }
    bool accepts(const BallisticFilter& f, BallisticEntity e, std::uint32_t m) override {
        return engine::invoke<bool>(f.object, 0, e, m);
    }
    std::uint32_t point_contents(const BallisticVector& p, std::uint32_t m, bool w) override {
        return inner.point_contents(p, m, w);
    }
    void trace_ray(const BallisticTraceRequest& r, BallisticTrace& t) override {
        inner.trace_ray(r, t);
    }
    void clip_ray_to_entity(const BallisticRay& r, std::uint32_t m, BallisticEntity e,
                            BallisticTrace& t) override {
        inner.clip_ray_to_entity(r, m, e, t);
    }
    PenetrationSurface surface(std::int16_t p) override {
        return inner.surface(p);
    }
    BallisticEntity entity(std::int32_t i) override {
        return inner.entity(i);
    }
    std::int32_t maximum_clients() override {
        return inner.maximum_clients();
    }
    BallisticEntityInfo entity_info(BallisticEntity e) override {
        return inner.entity_info(e);
    }
    BallisticEntityInfo clip_entity_info(BallisticEntity e) override {
        return inner.clip_entity_info(e);
    }
    BallisticBreakableInfo breakable_info(BallisticEntity e) override {
        return inner.breakable_info(e);
    }
    BallisticModelInfo model_info(std::int32_t i) override {
        return inner.model_info(i);
    }
    std::uint8_t damage_mode(BallisticEntity e) override {
        return inner.damage_mode(e);
    }
    void set_damage_mode(BallisticEntity e, std::uint8_t m) override {
        inner.set_damage_mode(e, m);
    }
    bool has_engine_breakable_callback() override {
        return inner.has_engine_breakable_callback();
    }
    bool engine_breakable(BallisticEntity e) override {
        return inner.engine_breakable(e);
    }
    BallisticRules rules() override {
        return inner.rules();
    }
};
} // namespace skeet::legit
