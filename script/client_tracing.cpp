#include "client_detail.hpp"
#include "skeet/rage/targeting_commit.hpp"
#include <algorithm>
#include <stdexcept>

namespace skeet::script {
namespace {
using namespace client_detail;
BallisticEntity entity_token(void* object) {
    return static_cast<BallisticEntity>(reinterpret_cast<std::uintptr_t>(object));
}
BallisticRay line(const BallisticVector& start, const BallisticVector& end) {
    const BallisticVector delta{end[0] - start[0], end[1] - start[1], end[2] - start[2]};
    return {start, delta,
            (delta[1] * delta[1] + delta[0] * delta[0]) + delta[2] * delta[2] != 0.0f};
}
std::int32_t entity_index(BallisticEntity entity) {
    return engine::invoke<std::int32_t>(reinterpret_cast<void*>(std::uintptr_t(entity) + 8), 10);
}
BallisticQuery weapon_query(ClientBindings& bindings, void* source) {
    if (source && engine::invoke<bool>(source, 158)) {
        const auto handle = field<std::int32_t>(source, bindings.offsets.active_weapon);
        if (handle != -1)
            if (auto* weapon = engine::invoke<void*>(bindings.entity_list, 4, handle))
                return rage::prepare_target_bullet(weapon, bindings.offsets.item_definition, true);
    }
    BallisticQuery query;
    query.allow_penetration = 1;
    query.weapon_id = 38;
    query.penetration = 2.5f;
    query.range = 8192;
    query.range_modifier = 0.98f;
    query.armor_ratio = 1.65f;
    query.base_damage = 80;
    query.reserved = 1;
    return query;
}
// Layout of the inherited engine trace filter. The first four methods remain
// actual engine entries; only433B4EB6's player/owner rejection is client code.
struct BulletFilter {
    void** virtuals;
    void* source = nullptr;
    void* last_hit = nullptr;
    std::uint32_t field_c = 0, field_10 = 0;
    void* preferred = nullptr;
    std::uint32_t mask, exit_mask;
    float minimum = 1;
    ClientBindings* bindings;
};
#if defined(_M_IX86) || defined(__i386__)
bool __fastcall exclude_players(BulletFilter* self, void*, void* handle_entity,
                                std::uint32_t mask) {
#else
bool exclude_players(BulletFilter* self, void* handle_entity, std::uint32_t mask) {
#endif
    auto& bindings = *self->bindings;
    if (handle_entity) {
        auto* entity = engine::invoke<void*>(handle_entity, 7);
        while (entity) {
            if (field<std::int32_t>(entity, bindings.offsets.collision_group) == 11 ||
                engine::invoke<bool>(entity, 158))
                return false;
            const auto owner = field<std::int32_t>(entity, bindings.offsets.owner_handle);
            if (owner == -1)
                break;
            entity = engine::invoke<void*>(bindings.entity_list, 4, owner);
        }
    }
    using Accept = engine::MemberFunction<bool, void*, std::uint32_t>;
    return reinterpret_cast<Accept>(bindings.bullet_filter_vtable[0])(self, handle_entity, mask);
}
}
int ClientModule::tracing(Export operation, void* state) {
    using namespace client_detail;
    const auto& api = *bridge().api;
    const auto count = api.gettop(state);
    if (operation == Export::scale_damage) { //4338654D
        if (count != 3)
            return argument_count_error(api, state);
        const auto index = std::int32_t(api.checkinteger(state, 1));
        auto* target = index ? engine::invoke<void*>(bindings.entity_list, 3, index) : nullptr;
        const auto query = weapon_query(bindings, target);
        const auto hitgroup = std::int32_t(api.checkinteger(state, 2));
        auto damage = std::int32_t(api.checkinteger(state, 3));
        if (target && std::uint32_t(hitgroup) - 1 < 8) {
            if (!bindings.ballistics)
                throw std::logic_error("Client damage requires BallisticEngine");
            const auto info = bindings.ballistics->entity_info(entity_token(target));
            damage = truncate(hitgroup_damage_scale(info.armor, info.helmet, hitgroup,
                                                    query.weapon_id, query.armor_ratio) *
                              float(damage));
        }
        api.pushinteger(state, damage);
        return 1;
    }
    const auto required = operation == Export::visible ? 3 : 7;
    if (count < required)
        return argument_count_error(api, state);
    if (operation == Export::visible) { //433E74B5
        auto* player = eye_player();
        bool visible = false;
        if (player) {
            if (!bindings.ballistics)
                throw std::logic_error("Client tracing requires BallisticEngine");
            const BallisticVector target{float(api.checknumber(state, 1)),
                                         float(api.checknumber(state, 2)),
                                         float(api.checknumber(state, 3))};
            const auto& origin = engine::invoke<const BallisticVector&>(player, 10);
            const auto& offset = field<BallisticVector>(player, bindings.offsets.view_offset);
            const BallisticVector start{origin[0] + offset[0], origin[1] + offset[1],
                                        origin[2] + offset[2]};
            BallisticTrace trace;
            bindings.ballistics->trace_ray({line(start, target), 0x0200400b,
                                            BallisticFilterKind::skip_source, nullptr,
                                            entity_token(player), 0},
                                           trace);
            visible = trace.fraction > 0.97f;
        }
        api.pushboolean(state, visible);
        return 1;
    }
    if (!bindings.ballistics)
        throw std::logic_error("Client tracing requires BallisticEngine");
    auto& engine = *bindings.ballistics;
    void* source = nullptr;
    if (operation == Export::trace_line) { //433E4FC4
        const auto index = std::uint32_t(api.checkinteger(state, 1));
        if ((index & 0xffff0000u) == 0)
            source = engine::invoke<void*>(bindings.entity_list, 3, index);
    } else if (api.type(state, 1) != 0) {
        const auto index = std::int32_t(api.checkinteger(state, 1));
        if (index)
            source = engine::invoke<void*>(bindings.entity_list, 3, index);
    }
    auto query =
        operation == Export::trace_bullet ? weapon_query(bindings, source) : BallisticQuery{};
    const BallisticVector start{float(api.checknumber(state, 2)), float(api.checknumber(state, 3)),
                                float(api.checknumber(state, 4))};
    const BallisticVector target{float(api.checknumber(state, 5)), float(api.checknumber(state, 6)),
                                 float(api.checknumber(state, 7))};
    if (operation == Export::trace_line) {
        BallisticTrace trace;
        engine.trace_ray({line(start, target), 0x4600400b, BallisticFilterKind::skip_source,
                          nullptr, entity_token(source), 0},
                         trace);
        api.pushnumber(state, double(trace.fraction));
        api.pushinteger(state, trace.entity ? entity_index(trace.entity) : -1);
        return 2;
    }
    const bool skip_players = count >= 8 && api.toboolean(state, 8) != 0;
    if (!bindings.bullet_filter_vtable)
        throw std::logic_error("Client bullet trace requires actual engine filter vtable");
    std::array<void*, 4> virtuals;
    std::copy_n(bindings.bullet_filter_vtable, 4, virtuals.begin());
    if (skip_players)
        virtuals[0] = reinterpret_cast<void*>(exclude_players);
    const auto mask = skip_players ? 0x0600400bu : 0x4600400bu;
    BulletFilter object{virtuals.data(), nullptr, nullptr, 0, 0, nullptr, mask, mask, 1, &bindings};
    BallisticFilter filter{&object, 0, mask, mask, 1, BallisticFilterKind::supplied};
    const auto result =
        trace_ballistic_bullet(engine, entity_token(source), start, target, filter, query);
    if (!skip_players && result && query.trace.entity)
        api.pushinteger(state, entity_index(query.trace.entity));
    else
        api.pushnil(state);
    api.pushinteger(state, truncate(query.damage));
    return 2;
}
} // namespace skeet::script
