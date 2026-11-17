#pragma once
#include "skeet/script/client_api.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/script/lua_text.hpp"
#include <bit>
#include <cstring>
#include <limits>

namespace skeet::script::client_detail {
constexpr int registry = -10000, globals = -10002, first_upvalue = -10003, second_upvalue = -10004;
template <class T> T& field(void* object, std::ptrdiff_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::byte*>(object) + offset);
}
template <class T> const T& field(const void* object, std::ptrdiff_t offset) {
    return *reinterpret_cast<const T*>(static_cast<const std::byte*>(object) + offset);
}
inline std::int32_t truncate(float value) {
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return INT32_MIN;
    return static_cast<std::int32_t>(value);
}
inline std::int32_t truncate(double value) {
    if (!(value >= -2147483648.0 && value < 2147483648.0))
        return INT32_MIN;
    return static_cast<std::int32_t>(value);
}
inline int argument_count_error(const LuaApi& api, void* state) {
    return api.lerror(state, "invalid number of arguments");
}
inline std::uint32_t raw_name_hash(const char* name, bool insensitive = false) noexcept {
    std::uint32_t hash = 0x811c9dc5;
    if (name)
        for (auto* text = reinterpret_cast<const unsigned char*>(name); *text; ++text) {
            auto value = *text;
            if (insensitive && value >= 'A' && value <= 'Z')
                value += 32;
            hash = (hash ^ value) * 0x1000193u;
        }
    return hash;
}
inline void push_vector(const LuaApi& api, void* state, const std::array<float, 3>& value) {
    for (const auto component : value)
        api.pushnumber(state, double(component));
}
} // namespace skeet::script::client_detail
