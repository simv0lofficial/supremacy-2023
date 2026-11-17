#include "skeet/script/lua_text.hpp"
#include <cstring>

namespace skeet::script {
namespace {
bool call_to_string(const LuaApi& api, void* state, int index) {
    if (!api.getmetatable(state, index))
        return false;
    api.getfield(state, -1, "__tostring");
    if (api.type(state, -1) == 0) {
        api.settop(state, -3);
        return false;
    }
    api.remove(state, -2);
    api.pushvalue(state, index);
    api.call(state, 1, 1);
    return true;
}
}
std::size_t concatenate_lua(const LuaApi& api, void* state, int first, char* output,
                            std::size_t capacity) {
    if (capacity == 0)
        return 0;
    const auto count = api.gettop(state);
    auto remaining = capacity - 1;
    auto* cursor = output;
    auto append = [&](const char* text, std::size_t length) {
        if (length > remaining)
            return false;
        if (length)
            std::memcpy(cursor, text, length);
        cursor += length;
        remaining -= length;
        return true;
    };
    for (auto index = first; index <= count; ++index) {
        switch (api.type(state, index)) {
        case 0:
            if (!append("nil", 3))
                return 0;
            break;
        case 1:
            if (api.toboolean(state, index)) {
                if (!append("true", 4))
                    return 0;
            } else if (!append("false", 5))
                return 0;
            break;
        case 2:
            break;
        case 3: { // LuaJIT's lua_tolstring uses the same STRFMT_G14 converter.
            api.pushvalue(state, index);
            std::size_t length = 0;
            const auto* text = api.tolstring(state, -1, &length);
            const bool result = !text || !length || append(text, length);
            api.settop(state, -2);
            if (!result)
                return 0;
            break;
        }
        case 4: {
            std::size_t length = 0;
            const auto* text = api.tolstring(state, index, &length);
            if (text && !append(text, length))
                return 0;
            break;
        }
        case 5:
        case 7:
        case 10:
            if (call_to_string(api, state, index)) {
                std::size_t length = 0;
                const auto* text = api.tolstring(state, -1, &length);
                const bool appended = !text || append(text, length);
                api.settop(state, -2);
                if (!appended)
                    return 0;
            }
            break;
        default:
            return 0;
        }
    }
    *cursor = 0;
    return std::size_t(cursor - output);
}
} // namespace skeet::script
