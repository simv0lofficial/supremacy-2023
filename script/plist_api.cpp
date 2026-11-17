#include "skeet/script/plist_api.hpp"
#include <array>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace skeet::script {
namespace {
void* (*export_context)(void*, int) = nullptr;
constexpr int globals = -10002;
constexpr int first_upvalue = -10003;

struct Field {
    std::uint8_t type;
    std::uint8_t length;
    std::int16_t minimum;
    std::int16_t maximum;
    std::uint32_t hash;
    std::uint32_t offset;
};
// 0x4346D578..0x4346D628. Lookup 0x433AFCF7 requires both the
// Lua string's byte length and the ASCII-insensitive hash of its C string.
constexpr std::array<Field, 11> fields{{
    {3, 19, 0, 2, 0xd7e29989u, 20},
    {3, 24, 0, 3, 0x1013f8f7u, 16},
    {1, 17, 0, 0, 0x0d8c921cu, 6},
    {1, 14, 0, 0, 0x6afe5bc2u, 5},
    {3, 20, -60, 60, 0xfbbaa527u, 8},
    {1, 15, 0, 0, 0x28aff203u, 1},
    {1, 24, 0, 0, 0x6d5de974u, 2},
    {1, 13, 0, 0, 0x6641805eu, 3},
    {1, 11, 0, 0, 0x181de9f3u, 4},
    {3, 17, -90, 90, 0x013a5d56u, 12},
    {1, 16, 0, 0, 0xb64b3871u, 0},
}};
const Field* find_field(const char* name, std::size_t length) noexcept {
    const auto hash = callback_hash(name);
    for (const auto& field : fields)
        if (length == field.length && hash == field.hash)
            return &field;
    return nullptr;
}
std::int32_t checked_integer(const LuaApi& api, void* state, int index) {
    const auto value = api.checkinteger(state, index);
    // The original LuaJIT uses CVTTSD2SI to a signed 32-bit lua_Integer.
    // A bound 64-bit Lua library has the same parser with a wider result.
    if (value < std::numeric_limits<std::int32_t>::min() ||
        value > std::numeric_limits<std::int32_t>::max())
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}
}

int PlistModule::set(void* state) {
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 3)
        return 0;
    const auto index = checked_integer(api, state, 1);
    if (static_cast<std::uint32_t>(index) - 1u >= 64u)
        return 0;
    std::size_t length = 0;
    const auto* name = api.checklstring(state, 2, &length);
    if (!name || !length)
        return 0;
    const auto* field = find_field(name, length);
    if (!field)
        return 0;
    auto* value = players_.settings_base() + index * 32 + field->offset;
    if (field->type == 1) {
        *value = static_cast<std::uint8_t>(api.toboolean(state, 3) != 0);
    } else if (field->type == 3) {
        const auto integer = checked_integer(api, state, 3);
        if (integer >= field->minimum && integer <= field->maximum)
            std::memcpy(value, &integer, sizeof(integer));
    }
    return 0;
}

int PlistModule::get(void* state) {
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 2)
        return 0;
    const auto index = checked_integer(api, state, 1);
    if (static_cast<std::uint32_t>(index) - 1u >= 64u)
        return 0;
    std::size_t length = 0;
    const auto* name = api.checklstring(state, 2, &length);
    if (!name || !length)
        return 0;
    const auto* field = find_field(name, length);
    if (!field)
        return 0;
    const auto* value = players_.settings_base() + index * 32 + field->offset;
    if (field->type == 1) {
        api.pushboolean(state, *value);
    } else if (field->type == 3) {
        std::int32_t integer;
        std::memcpy(&integer, value, sizeof(integer));
        api.pushnumber(state, static_cast<double>(integer));
    } else {
        api.pushnil(state);
    }
    return 1;
}

int PlistModule::set_entry(void* state) {
    return static_cast<PlistModule*>(export_context(state, first_upvalue))->set(state);
}
int PlistModule::get_entry(void* state) {
    return static_cast<PlistModule*>(export_context(state, first_upvalue))->get(state);
}
void PlistModule::register_exports(Bridge& bridge) {
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Lua player-list exports require an initialized state");
    bridge_ = &bridge;
    const auto& api = *bridge.api;
    export_context = api.touserdata;
    api.createtable(bridge.state, 0, 2);
    for (const auto& function : std::array<std::pair<const char*, LuaApi::CFunction>, 2>{
             {{"set", set_entry}, {"get", get_entry}}}) {
        api.pushlightuserdata(bridge.state, this);
        api.pushcclosure(bridge.state, function.second, 1);
        api.setfield(bridge.state, -2, function.first);
    }
    api.setfield(bridge.state, globals, "plist");
}

} // namespace skeet::script
