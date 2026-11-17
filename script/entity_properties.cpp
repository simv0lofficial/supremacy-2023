#include "entity_detail.hpp"
#include <algorithm>
#include <cstring>

namespace skeet::script {
using namespace entity_detail;
bool EntityModule::property(void* state, void* value, EntityProperty& out) {
    std::size_t length = 0;
    const char* name = nullptr;
    if (lua().type(state, 2) == 4) {
        name = lua().tolstring(state, 2, &length);
        const std::string key(name, length);
        if (const auto found = custom_properties_.find(key); found != custom_properties_.end()) {
            out = found->second;
            return true;
        }
        if (const auto found = resolved_properties_.find(key);
            found != resolved_properties_.end()) {
            out = found->second;
            return true;
        }
    }
    name = lua().checklstring(state, 2, &length);
    if (!name || !length)
        return false;
    out = {};
    auto* client_class = engine::invoke<engine::ClientClass*>(networkable(value), 2);
    const auto& bindings = bindings_;
    if (receive_property(name, *client_class->table, 0, out, bindings.byte_proxy,
                         bindings.short_proxy)) {
        EntityProperty mapped;
        if (data_map_property(name, engine::invoke<engine::DataMap*>(value, 17), mapped)) {
            if (out.type == 6)
                out.element_type = mapped.type;
            else
                out.type = mapped.type;
        }
        property_name_type(name, out);
    } else if (!data_map_property(name, engine::invoke<engine::DataMap*>(value, 17), out)) {
        auto* related = engine::invoke<void*>(value, 7);
        if (!related || !engine::invoke<void*>(related, 44))
            return false;
        if (!receive_property(name, *bindings.related_receive_class->table, 0, out,
                              bindings.byte_proxy, bindings.short_proxy))
            return false;
        property_name_type(name, out);
    }
    resolved_properties_.emplace(std::string(name, length), out);
    return true;
}
bool EntityModule::property_in_bounds(void* value, const EntityProperty& property) {
    auto* client_class = engine::invoke<engine::ClientClass*>(networkable(value), 2);
    const std::string name(client_class->network_name);
    auto found = class_sizes_.find(name);
    if (found == class_sizes_.end()) {
        std::uint32_t maximum = 0;
        if (bindings_.class_size_interface) {
            const auto* class_name = engine::invoke<const char*>(value, 143);
            const auto size =
                engine::invoke<std::int32_t>(bindings_.class_size_interface, 4, class_name);
            if (size > 4)
                maximum = std::uint32_t(size - 4);
        }
        if (!maximum) {
            const auto data_map_maximum =
                maximum_data_map_offset(engine::invoke<engine::DataMap*>(value, 17));
            client_class = engine::invoke<engine::ClientClass*>(networkable(value), 2);
            maximum =
                (std::max)(data_map_maximum, engine::maximum_property_offset(*client_class->table));
        }
        found = class_sizes_.emplace(name, maximum).first;
    }
    return property.offset <= found->second;
}
bool EntityModule::property_address(void* state, int argument, const EntityProperty& property,
                                    std::byte*& address, std::uint8_t& type) const {
    std::uint32_t count = property.element_count;
    if (type == 254) { // Native dynamic vector: pointer + allocation fields + count.
        const auto* vector = address;
        address = read<std::byte*>(vector, 0);
        if (!address)
            return false;
        const auto dynamic_count = read<std::uint32_t>(vector, 12);
        count = dynamic_count < count ? dynamic_count : count;
        type = 6;
    } else if (type != 6)
        return true;
    if (property.element_stride && lua().gettop(state) >= argument) {
        const auto index = static_cast<std::uint32_t>(lua().tointeger(state, argument));
        if (index >= count)
            return false;
        address += std::uint32_t(property.element_stride) * index;
        type = property.element_type;
    }
    return true;
}
int EntityModule::new_prop(void* state) {
    const auto arguments = lua().gettop(state);
    if (arguments < 3)
        return 0;
    std::size_t name_length = 0;
    const auto* name = lua().checklstring(state, 1, &name_length);
    const auto offset = static_cast<std::int32_t>(lua().tointeger(state, 2));
    const auto type = static_cast<std::int32_t>(lua().tointeger(state, 3));
    if (offset <= 0 || type >= 9)
        return 0;
    EntityProperty value;
    value.offset = std::uint32_t(offset);
    value.type = std::uint8_t(type);
    if (type == 5) {
        if (arguments < 6)
            return 0;
        const auto element = static_cast<std::int32_t>(lua().tointeger(state, 4));
        const auto stride = static_cast<std::int32_t>(lua().tointeger(state, 5));
        const auto count = static_cast<std::int32_t>(lua().tointeger(state, 6));
        if (element >= 8 || stride < 1 || count < 1)
            return 0;
        value.element_type = std::uint8_t(element);
        value.element_stride = std::uint16_t(stride);
        value.element_count = std::uint16_t(count);
    }
    const std::string key(name, name_length);
    if (!custom_properties_.contains(key) && !resolved_properties_.contains(key))
        custom_properties_.emplace(key, value);
    return 0;
}
int EntityModule::get_prop(void* state) {
    auto unavailable = [&] {
        lua().pushnil(state);
        return 1;
    };
    if (!in_game() || lua().gettop(state) < 2 || lua().type(state, 1) != 3)
        return unavailable();
    const auto index = argument_index(state);
    if (index >= 8192)
        return lua().argerror(state, 1, "invalid entity index");
    auto* value = entity(index);
    if (!value || lua().type(state, 2) != 4)
        return unavailable();
    EntityProperty metadata;
    if (!property(state, value, metadata) || !metadata.offset ||
        !property_in_bounds(value, metadata))
        return unavailable();
    if (read<void*>(value, 0) == bindings_.game_rules_proxy_table) {
        value = bindings_.game_rules ? *bindings_.game_rules : nullptr;
        if (!value)
            return unavailable();
    }
    auto* address = static_cast<std::byte*>(value) + metadata.offset;
    auto type = metadata.type;
    if (!property_address(state, 3, metadata, address, type))
        return unavailable();
    switch (type) {
    case 0:
    case 7:
        lua().pushinteger(state, read<std::int32_t>(address, 0));
        return 1;
    case 1:
        lua().pushnumber(state, read<float>(address, 0));
        return 1;
    case 2:
        lua().pushnumber(state, read<float>(address, 0));
        lua().pushnumber(state, read<float>(address, 4));
        lua().pushnumber(state, read<float>(address, 8));
        return 3;
    case 3:
        lua().pushnumber(state, read<float>(address, 0));
        lua().pushnumber(state, read<float>(address, 4));
        return 2;
    case 4:
        lua().pushstring(state, reinterpret_cast<const char*>(address));
        return 1;
    case 8:
        lua().pushlightuserdata(state, read<void*>(address, 0));
        return 1;
    case 9:
        lua().pushinteger(state, read<std::uint8_t>(address, 0));
        return 1;
    case 10:
        lua().pushinteger(state, read<std::int16_t>(address, 0));
        return 1;
    case 11: {
        const auto handle = read<std::uint32_t>(address, 0);
        if (handle == UINT32_MAX)
            lua().pushnil(state);
        else
            lua().pushinteger(state, handle & 0xffffu);
        return 1;
    }
    default:
        return 0;
    }
}
int EntityModule::set_prop(void* state) {
    if (!in_game() || lua().gettop(state) < 3 || lua().type(state, 1) != 3)
        return 0;
    const auto index = argument_index(state);
    if (index >= 8192)
        return lua().argerror(state, 1, "invalid entity index");
    auto* value = entity(index);
    if (!value)
        return 0;
    EntityProperty metadata;
    if (!property(state, value, metadata) || !metadata.offset ||
        !property_in_bounds(value, metadata))
        return 0;
    auto* address = static_cast<std::byte*>(value) + metadata.offset;
    auto type = metadata.type;
    if (!property_address(state, 4, metadata, address, type))
        return 0;
    switch (type) {
    case 0:
        write(address, 0, static_cast<std::int32_t>(lua().checkinteger(state, 3)));
        break;
    case 1:
        write(address, 0, static_cast<float>(lua().checknumber(state, 3)));
        break;
    case 2:
    case 3:
        for (int i = 3, count = lua().gettop(state); i < 6 && i <= count; ++i)
            write(address, (i - 3) * 4, static_cast<float>(lua().checknumber(state, i)));
        break;
    case 4: {
        std::size_t length = 0;
        const auto* string = lua().tolstring(state, 3, &length);
        if (length < metadata.element_count)
            std::memcpy(address, string, length);
        break;
    }
    case 7:
        write(address, 0,
              static_cast<std::int64_t>(static_cast<std::int32_t>(lua().checkinteger(state, 3))));
        break;
    case 9:
        write(address, 0, static_cast<std::uint8_t>(lua().checkinteger(state, 3)));
        break;
    case 10:
        write(address, 0, static_cast<std::int16_t>(lua().checkinteger(state, 3)));
        break;
    case 11: {
        const auto argument = static_cast<std::uint32_t>(lua().tointeger(state, 3));
        if (lua().type(state, 3) == 0 || argument == UINT32_MAX)
            write(address, 0, UINT32_MAX);
        else if (argument >= 8192)
            lua().argerror(state, 3, "invalid entity index");
        else if (auto* target = entity(argument))
            write(address, 0, *engine::invoke<const std::uint32_t*>(target, 2));
        break;
    }
    default:
        break;
    }
    return 0;
}
} // namespace skeet::script
