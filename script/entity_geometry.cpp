#include "entity_detail.hpp"
#include "skeet/esp/player_engine.hpp"
#include <cstring>
#include <stdexcept>

namespace skeet::script {
using namespace entity_detail;
namespace {
void push_vector(const LuaApi& lua, void* state, const esp::Vector3& value) {
    lua.pushnumber(state, value.x);
    lua.pushnumber(state, value.y);
    lua.pushnumber(state, value.z);
}
}
int EntityModule::get_origin(void* state) {
    if (lua().gettop(state) < 1)
        return lua().lerror(state, "invalid number of arguments");
    if (in_game()) {
        const auto index = argument_index(state);
        if (index >= 8192)
            return lua().argerror(state, 1, "invalid entity index");
        if (auto* value = entity(index)) {
            if (!dormant(value)) {
                push_vector(lua(), state, *engine::invoke<const esp::Vector3*>(value, 10));
                return 3;
            }
            if (engine::invoke<bool>(value, 158)) {
                const auto& geometry = cache_.geometry(index);
                if (geometry.alpha != 0.0f) {
                    push_vector(lua(), state, geometry.origin);
                    return 3;
                }
            }
        }
    }
    lua().pushnil(state);
    lua().pushnil(state);
    lua().pushnil(state);
    return 3;
}
int EntityModule::get_bounding_box(void* state) {
    const auto arguments = lua().gettop(state);
    if (arguments < 1)
        return lua().lerror(state, "invalid number of arguments");
    float alpha = 0;
    if (in_game()) {
        const auto index = argument_index(state, arguments > 1 ? 2 : 1);
        if (index - 1u < 64u && entity(index)) {
            const auto& geometry = cache_.geometry(index);
            if (!bindings_.projection)
                throw std::logic_error("Entity bounding boxes require engine projection");
            Rectangle box;
            if (esp::player_box(*bindings_.projection, geometry.upper, geometry.lower, box)) {
                if (!bindings_.esp_activation)
                    throw std::logic_error(
                        "Entity bounding boxes require the ESP activation state");
                alpha = *bindings_.esp_activation ? geometry.alpha : 0.0f;
                lua().pushinteger(state, box.x);
                lua().pushinteger(state, box.y);
                lua().pushinteger(state,
                                  std::int32_t(std::uint32_t(box.x) + std::uint32_t(box.width)));
                lua().pushinteger(state,
                                  std::int32_t(std::uint32_t(box.y) + std::uint32_t(box.height)));
                lua().pushnumber(state, alpha);
                return 5;
            }
        }
    }
    lua().pushnil(state);
    lua().pushnil(state);
    lua().pushnil(state);
    lua().pushnil(state);
    lua().pushnumber(state, alpha);
    return 5;
}
int EntityModule::hitbox_position(void* state) {
    auto unavailable = [&] {
        lua().pushnil(state);
        return 1;
    };
    if (!in_game())
        return unavailable();
    if (lua().gettop(state) < 2)
        return lua().lerror(state, "invalid number of arguments");
    const auto index = argument_index(state);
    if (index - 1u >= 64u)
        return unavailable();
    auto* value = entity(index);
    if (!value)
        return unavailable();
    const auto& offsets = bindings_.offsets;
    const auto* studio = read<const std::byte*>(value, offsets.studio_header);
    if (!studio)
        return unavailable();
    const auto* header = read<const std::byte*>(studio, 0);
    if (!header)
        return unavailable();
    const auto set_index = read<std::int32_t>(value, offsets.hitbox_set);
    const auto* set = header + read<std::int32_t>(header, 0xb0) + set_index * 12;
    const auto count = read<std::int32_t>(set, 4);
    if (!count)
        return unavailable();
    const auto* first = set + read<std::int32_t>(set, 8);
    const std::byte* hitbox = nullptr;
    std::int32_t hitbox_index = 0;
    const auto type = lua().type(state, 2);
    if (type == 3) {
        hitbox_index = static_cast<std::int32_t>(lua().checkinteger(state, 2));
        if (hitbox_index < 0 || hitbox_index >= count)
            return unavailable();
        hitbox = first + hitbox_index * 0x44;
    } else if (type == 4) {
        const auto* name = lua().tolstring(state, 2, nullptr);
        if (!name)
            return unavailable();
        for (; hitbox_index < count; ++hitbox_index) {
            const auto* candidate = first + hitbox_index * 0x44;
            const auto bone_index = read<std::int32_t>(candidate, 0);
            if (bone_index < 0 || bone_index >= read<std::int32_t>(header, 0x9c))
                continue;
            const auto* bone = header + read<std::int32_t>(header, 0xa0) + bone_index * 0xd8;
            const auto* bone_name =
                reinterpret_cast<const char*>(bone + read<std::int32_t>(bone, 0));
            if (!std::strcmp(name, bone_name)) {
                hitbox = candidate;
                break;
            }
        }
    }
    if (!hitbox)
        return unavailable();
    const Matrix3x4* matrix = nullptr;
    auto* record = cache_.get(index);
    if (!bridge_->render_context.cursor && record->history.count &&
        (record->history.at(0).header.flags & 4))
        matrix = &record->history.at(0).matrices[std::uint32_t(hitbox_index)];
    if (!matrix) {
        const auto* cache = static_cast<const std::byte*>(value) + offsets.bone_cache;
        const auto bone_index = read<std::int32_t>(hitbox, 0);
        if (read<std::int32_t>(cache, 0xc) <= bone_index)
            return unavailable();
        matrix = read<const Matrix3x4*>(cache, 0) + bone_index;
    }
    const auto minimum = read<esp::Vector3>(hitbox, 8), maximum = read<esp::Vector3>(hitbox, 0x14);
    const esp::Vector3 center{(maximum.x - minimum.x) * 0.5f + minimum.x,
                              (maximum.y - minimum.y) * 0.5f + minimum.y,
                              (maximum.z - minimum.z) * 0.5f + minimum.z};
    esp::Vector3 result;
    auto coordinate = [&](int row) {
        const auto& m = matrix->rows[row];
        const auto pair =
            row == 0 ? center.x * m[0] + center.y * m[1] : center.y * m[1] + center.x * m[0];
        return (pair + center.z * m[2]) + m[3];
    };
    result = {coordinate(0), coordinate(1), coordinate(2)};
    push_vector(lua(), state, result);
    return 3;
}
int EntityModule::get_esp_data(void* state) {
    if (lua().gettop(state) < 1)
        return lua().lerror(state, "invalid number of arguments");
    if (in_game()) {
        const auto index = argument_index(state);
        if (index >= 8192)
            return lua().argerror(state, 1, "invalid entity index");
        auto* value = entity(index);
        if (value && engine::invoke<bool>(value, 158)) {
            auto& record = *cache_.get(index);
            lua().createtable(state, 0, 3);
            lua().pushnumber(state, record.geometry.alpha);
            lua().setfield(state, -2, "alpha");
            std::int32_t health = 0;
            if (!dormant(value))
                health = engine::invoke<std::int32_t>(value, 122);
            else if (auto* resource =
                         bindings_.player_resource ? *bindings_.player_resource : nullptr) {
                const auto resource_index = engine::invoke<std::int32_t>(networkable(value), 10);
                if (read<std::uint8_t>(resource,
                                       bindings_.offsets.resource_connected + resource_index))
                    health = read<std::int32_t>(resource, bindings_.offsets.resource_health +
                                                              resource_index * 4);
            }
            lua().pushinteger(state, health);
            lua().setfield(state, -2, "health");
            lua().pushnumber(state, record.cached_definition);
            lua().setfield(state, -2, "weapon_id");
            if (!bridge_->frame_number)
                throw std::logic_error("Entity ESP data requires the shared engine frame number");
            if (record.native.cached_esp_frame != *bridge_->frame_number) {
                const auto& draw = bindings_.players.draw_context(bindings_.flag_context);
                auto& players = bindings_.players.get();
                if (&draw.cache != &cache_)
                    throw std::logic_error(
                        "Entity ESP flags require the canonical player draw context");
                const auto* player = players.identity(value);
                if (!player || player->engine_entity != value)
                    throw std::logic_error(
                        "Entity ESP flags require the matching engine player view");
                record.native.cached_esp_frame = *bridge_->frame_number;
                record.native.cached_esp_flags = esp::player_overlay_flags(*player, draw, players);
            }
            lua().pushinteger(state, record.native.cached_esp_flags);
            lua().setfield(state, -2, "flags");
            return 1;
        }
    }
    lua().pushnil(state);
    return 1;
}
} // namespace skeet::script
