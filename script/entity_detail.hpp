#pragma once
#include "skeet/script/entity_api.hpp"
#include "skeet/engine/interface_call.hpp"
#include <bit>
#include <cstring>

namespace skeet::script::entity_detail {
template <class T> T read(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> void write(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
inline void* networkable(void* entity) {
    return static_cast<std::byte*>(entity) + 8;
}
inline std::int32_t convar_integer(const void* variable) {
    for (;;) {
        const auto parent = read<const void*>(variable, 0x1c);
        if (parent == variable)
            break;
        variable = parent;
    }
    return std::bit_cast<std::int32_t>(read<std::uint32_t>(variable, 0x30) ^
                                       std::uint32_t(reinterpret_cast<std::uintptr_t>(variable)));
}
inline bool teams_enemy(std::int32_t a, std::int32_t b, bool teammates) {
    return a == 2 ? b == 3 || teammates : a == 3 && (b == 2 || teammates);
}
bool receive_property(const char*, const engine::RecvTable&, std::uint32_t base, EntityProperty&,
                      const engine::RecvVarProxy* byte_proxy,
                      const engine::RecvVarProxy* short_proxy);
bool data_map_property(const char*, engine::DataMap*, EntityProperty&);
std::uint32_t maximum_data_map_offset(engine::DataMap*);
inline void property_name_type(const char* name, EntityProperty& property) {
    auto& type = property.type == 6 ? property.element_type : property.type;
    const auto capital = [](unsigned char byte) { return std::uint8_t(byte + 0xbfu) < 0x19u; };
    if (name[0] != 'm' || name[1] != '_')
        return;
    if (name[2] == 'b' && capital(name[3]))
        type = 9;
    else if (name[2] == 'f' && name[3] == 'l' && capital(name[4]))
        type = 1;
    else if (name[2] == 'h' && capital(name[3]))
        type = 11;
}
}
