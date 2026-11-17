#include "entity_detail.hpp"
#include "skeet/misc/runtime.hpp"
#include <array>
#include <stdexcept>
#include <vector>

namespace skeet::script {
using namespace entity_detail;
bool EntityModule::in_game() const {
    return engine::invoke<bool>(bindings_.engine, 26);
}
void* EntityModule::entity(std::uint32_t index) const {
    return index & 0xffff0000u ? nullptr : engine::invoke<void*>(bindings_.entity_list, 3, index);
}
std::uint32_t EntityModule::argument_index(void* state, int index) const {
    return static_cast<std::uint32_t>(lua().tointeger(state, index));
}
void* EntityModule::player(void* state, std::uint32_t& index) const {
    const auto arguments = lua().gettop(state);
    if (!in_game() || arguments < 1 || lua().type(state, 1) != 3)
        return nullptr;
    index = argument_index(state);
    if (index - 1u >= 64u)
        return nullptr;
    auto* result = entity(index);
    return result && engine::invoke<bool>(result, 158) ? result : nullptr;
}
bool EntityModule::dormant(void* value) const {
    return engine::invoke<bool>(networkable(value), 9);
}
bool EntityModule::enemy(void* value, bool sleeping) const {
    auto target = bindings_.targeting;
    target.local_player = bindings_.local_player ? *bindings_.local_player : nullptr;
    if (!target.local_player || value == target.local_player)
        return false;
    if (!sleeping || convar_integer(target.game_type) == 6)
        return rage::target_is_enemy(target, value);
    auto* resource = bindings_.player_resource ? *bindings_.player_resource : nullptr;
    if (!resource)
        return false;
    const auto first = engine::invoke<std::int32_t>(networkable(target.local_player), 10);
    const auto second = engine::invoke<std::int32_t>(networkable(value), 10);
    const auto& o = bindings_.offsets;
    if (!read<std::uint8_t>(resource, o.resource_connected + first) ||
        !read<std::uint8_t>(resource, o.resource_connected + second))
        return false;
    return teams_enemy(read<std::int32_t>(resource, o.resource_team + first * 4),
                       read<std::int32_t>(resource, o.resource_team + second * 4),
                       convar_integer(target.teammates_are_enemies) != 0);
}
int EntityModule::get_local_player(void* state) {
    auto* const local = bindings_.local_player ? *bindings_.local_player : nullptr;
    if (!in_game() || !local)
        lua().pushnil(state);
    else
        lua().pushinteger(state, engine::invoke<std::int32_t>(bindings_.engine, 12));
    return 1;
}
int EntityModule::get_player_resource(void* state) {
    if (!in_game()) {
        lua().pushnil(state);
        return 1;
    }
    auto* resource = bindings_.player_resource ? *bindings_.player_resource : nullptr;
    if (!resource)
        lua().pushnil(state);
    else
        lua().pushinteger(state, engine::invoke<std::int32_t>(networkable(resource), 10));
    return 1;
}
int EntityModule::get_game_rules(void* state) {
    if (!in_game()) {
        lua().pushnil(state);
        return 1;
    }
    auto handle = rules_handle_;
    if (handle != 0 && handle != UINT32_MAX) {
        auto* rules = engine::invoke<void*>(bindings_.entity_list, 4, handle);
        if (!rules || read<void*>(rules, 0) != bindings_.game_rules_proxy_table)
            handle = UINT32_MAX;
    }
    if (!handle || handle == UINT32_MAX) {
        handle = UINT32_MAX;
        const auto highest = read<std::int32_t>(bindings_.entity_list, 0x24);
        for (auto index = *bindings_.max_players + 1; index < highest; ++index) {
            auto* value = entity(std::uint32_t(index));
            if (value && read<void*>(value, 0) == bindings_.game_rules_proxy_table) {
                handle = *engine::invoke<const std::uint32_t*>(value, 2);
                rules_handle_ = handle;
                break;
            }
        }
    }
    if (handle == UINT32_MAX)
        lua().pushnil(state);
    else
        lua().pushinteger(state, handle & 0xffffu);
    return 1;
}
int EntityModule::get_players(void* state) {
    const auto arguments = lua().gettop(state);
    if (!in_game()) {
        lua().createtable(state, 0, 0);
        return 1;
    }
    auto* const local = bindings_.local_player ? *bindings_.local_player : nullptr;
    const bool enemies = arguments > 0 && lua().type(state, 1) == 1 && lua().toboolean(state, 1);
    std::vector<std::int16_t> found;
    for (std::int32_t index = 1; index <= *bindings_.max_players; ++index) {
        auto* value = entity(std::uint32_t(index));
        if (!value || !engine::invoke<bool>(value, 158) ||
            read<std::uint8_t>(value, bindings_.offsets.life_state) ||
            engine::invoke<std::int32_t>(value, 122) <= 0 || dormant(value) ||
            read<std::uint8_t>(value, bindings_.offsets.suppressed))
            continue;
        if (enemies && (read<std::uint8_t>(value, bindings_.offsets.enemy_excluded) || !local ||
                        !enemy(value, false)))
            continue;
        found.push_back(std::int16_t(index));
    }
    lua().createtable(state, static_cast<int>(found.size()), 0);
    int index = 0;
    for (auto value : found) {
        lua().pushinteger(state, value);
        lua().rawseti(state, -2, ++index);
    }
    return 1;
}
int EntityModule::is_enemy(void* state) {
    std::uint32_t index = 0;
    auto* value = player(state, index);
    lua().pushboolean(state, value && bindings_.local_player && *bindings_.local_player &&
                                 enemy(value, dormant(value)));
    return 1;
}
int EntityModule::is_alive(void* state) {
    std::uint32_t index = 0;
    auto* value = player(state, index);
    bool alive = false;
    if (value) {
        if (!dormant(value))
            alive = !read<std::uint8_t>(value, bindings_.offsets.life_state);
        else if (auto* resource = bindings_.player_resource ? *bindings_.player_resource : nullptr)
            alive = read<std::uint8_t>(resource, bindings_.offsets.resource_alive + index) != 0;
    }
    lua().pushboolean(state, alive);
    return 1;
}
int EntityModule::is_dormant(void* state) {
    bool result = true;
    const auto arguments = lua().gettop(state);
    if (in_game() && arguments > 0 && lua().type(state, 1) == 3) {
        const auto index = argument_index(state);
        if (index < 8192) {
            auto* value = engine::invoke<void*>(bindings_.entity_list, 0, index);
            if (value)
                result = engine::invoke<bool>(value, 9);
        }
    }
    lua().pushboolean(state, result);
    return 1;
}
int EntityModule::get_player_name(void* state) {
    misc::PlayerInfo info;
    bool found = false;
    if (in_game() && lua().gettop(state) > 0 && lua().type(state, 1) == 3) {
        const auto index = argument_index(state);
        found =
            index - 1u < 64u && engine::invoke<bool>(bindings_.engine, 8, index, info.bytes.data());
    }
    if (!found)
        lua().pushlstring(state, "unknown", 7);
    else {
        auto* destination = reinterpret_cast<char*>(info.bytes.data() + 0x10);
        auto* source = destination;
        auto* first = destination;
        while (*source) {
            const auto byte = *source++;
            *destination = byte;
            if (byte != ';' && byte != '"' && byte != '\n' && byte != '\r' && byte != '\\')
                ++destination;
        }
        *destination = 0;
        lua().pushstring(state, first);
    }
    return 1;
}
int EntityModule::get_player_weapon(void* state) {
    void* value = nullptr;
    if (in_game() && lua().gettop(state) > 0 && lua().type(state, 1) == 3) {
        const auto index = argument_index(state);
        if (index - 1u < 64u) {
            if (auto* target = entity(index)) {
                const auto handle = read<std::uint32_t>(target, bindings_.offsets.active_weapon);
                if (handle != UINT32_MAX)
                    value = engine::invoke<void*>(bindings_.entity_list, 4, handle);
            }
        }
    }
    if (!value)
        lua().pushnil(state);
    else
        lua().pushinteger(state, engine::invoke<std::int32_t>(networkable(value), 10));
    return 1;
}
int EntityModule::get_steam64(void* state) {
    if (lua().gettop(state) < 1)
        return lua().lerror(state, "invalid number of arguments");
    misc::PlayerInfo info;
    if (in_game()) {
        const auto index = argument_index(state);
        if (index - 1u < 64u &&
            engine::invoke<bool>(bindings_.engine, 8, index, info.bytes.data())) {
            lua().pushinteger(state, read<std::int32_t>(info.bytes.data(), 8));
            return 1;
        }
    }
    lua().pushnil(state);
    return 1;
}
} // namespace skeet::script
