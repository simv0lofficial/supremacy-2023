#include "skeet/script/cvar_api.hpp"
#include "skeet/engine/interface_call.hpp"
#include <algorithm>
#include <cstring>
#include <string_view>
#include <vector>

namespace skeet::script {
namespace {
template <class T> T read(void* object, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<std::byte*>(object) + offset, sizeof(value));
    return value;
}
using ChangeCallback = void (*)(void*, const char*, float);
using CommandCallback = void (*)(const CvarCommandArguments*);
using LegacyCommandCallback = void (*)();
}

void CvarModule::invoke_variable(void* state, void* variable) {
    auto count = read<std::int32_t>(variable, 0x60);
    if (!count)
        return;
    const char* text = "";
    if (!(engine::invoke<std::uint32_t>(variable, 5) & 0x1000u)) {
        text = read<const char*>(variable, 0x24);
        if (!text) {
            text = read<const char*>(variable, 0x20);
            if (!text)
                text = "";
        }
    }
    const auto value = cvar_float(variable);
    const auto& api = *bridge_->api;
    auto* receiver = static_cast<std::byte*>(variable) + 0x18;
    if (api.gettop(state) >= 2) {
        const auto index = static_cast<std::int32_t>(api.checkinteger(state, 2));
        if (index < 0 || index >= read<std::int32_t>(variable, 0x60))
            return;
        const auto callbacks = read<ChangeCallback*>(variable, 0x54);
        callbacks[index](receiver, text, value);
        return;
    }
    for (std::int32_t index = 0; index < read<std::int32_t>(variable, 0x60); ++index) {
        const auto callbacks = read<ChangeCallback*>(variable, 0x54);
        callbacks[index](receiver, text, value);
    }
}

void CvarModule::invoke_command(void* state, void* command) {
    const auto flags = read<std::uint8_t>(command, 0x20);
    void* callback = read<void*>(command, 0x18);
    if (!callback)
        return;
    if (!(flags & 6)) {
        reinterpret_cast<LegacyCommandCallback>(callback)();
        return;
    }
    const auto& api = *bridge_->api;
    const char* name = engine::invoke<const char*>(command, 6);
    std::vector<std::string_view> values;
    values.emplace_back(name);
    const auto top = api.gettop(state);
    for (int index = 1; index <= top; ++index) {
        std::size_t length = 0;
        if (const char* value = api.tolstring(state, index, &length); value && length)
            values.emplace_back(value, length);
    }
    CvarCommandArguments arguments;
    arguments.command_name_length = static_cast<std::int32_t>(values[0].size());
    std::size_t buffer_used = 0;
    std::size_t line_used = 0;
    const auto limit = (std::min)(values.size(), arguments.arguments.size());
    for (std::size_t index = 0; index < limit; ++index) {
        const auto value = values[index];
        if (buffer_used + value.size() >= arguments.argument_buffer.size())
            break;
        char* destination = arguments.argument_buffer.data() + buffer_used;
        std::memcpy(destination, value.data(), value.size());
        destination[value.size()] = '\0';
        std::memcpy(arguments.command_line.data() + line_used, value.data(), value.size());
        line_used += value.size();
        arguments.arguments[arguments.count++] = destination;
        buffer_used += value.size() + 1;
        if (index != limit - 1)
            arguments.command_line[line_used++] = ' ';
    }
    if (flags & 2)
        reinterpret_cast<CommandCallback>(callback)(&arguments);
    else
        engine::invoke<void>(callback, 0, &arguments);
}

} // namespace skeet::script
