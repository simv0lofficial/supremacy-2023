#include "entity_detail.hpp"
#include <cstring>
#include <vector>

namespace skeet::script {
using namespace entity_detail;
int EntityModule::get_all(void* state) {
    if (!in_game()) {
        lua().createtable(state, 0, 0);
        return 1;
    }
    engine::ClientClass* requested = nullptr;
    if (lua().gettop(state) >= 1) {
        std::size_t length = 0;
        const auto* name = lua().checklstring(state, 1, &length);
        requested = engine::invoke<engine::ClientClass*>(bindings_.base_client, 8);
        for (; requested; requested = requested->next)
            if (std::strcmp(requested->network_name, name) == 0)
                break;
        if (!requested) {
            lua().createtable(state, 0, 0);
            return 1;
        }
        // 43416B94 reads upvalue2, but publishes its class pointer into
        // upvalue1, the property map. This replaces a same-key PI record.
        const std::string key(name, length);
        custom_properties_.erase(key);
        resolved_properties_.erase(key);
    }
    std::vector<std::int16_t> found;
    const auto highest = read<std::int32_t>(bindings_.entity_list, 0x24);
    for (std::int32_t index = 1; index <= highest; ++index) {
        auto* value = entity(std::uint32_t(index));
        if (!value || dormant(value))
            continue;
        if (requested && requested != engine::invoke<engine::ClientClass*>(networkable(value), 2))
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
int EntityModule::get_classname(void* state) {
    if (!in_game() || lua().gettop(state) < 1 || lua().type(state, 1) == 0) {
        lua().pushnil(state);
        return 1;
    }
    const auto index = argument_index(state);
    if (index >= 8192)
        return lua().argerror(state, 1, "invalid entity index");
    auto* value = entity(index);
    if (!value)
        lua().pushnil(state);
    else if (index < 2048)
        lua().pushstring(state,
                         engine::invoke<engine::ClientClass*>(networkable(value), 2)->network_name);
    else
        lua().pushstring(state, engine::invoke<const char*>(value, 143));
    return 1;
}
} // namespace skeet::script
