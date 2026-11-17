#include "skeet/script/say_text_message.hpp"
#include <cstdint>
#include <cstring>

namespace skeet::script {
namespace {
SayTextMessageRuntime* active_runtime;

template <class T> T field(const void* object, std::ptrdiff_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
const char* parameter(const void* packet, std::int32_t index) {
    if (field<std::int32_t>(packet, 0x18) <= index)
        return "?"; //native4346FB68
    const auto* entries = field<const void*>(packet, 0x14);
    const auto* string = field<const void*>(entries, index * 4);
    // This is the engine's MSVC string representation, not a source string.
    return field<std::uint32_t>(string, 0x14) < 16 ? static_cast<const char*>(string)
                                                   : field<const char*>(string, 0);
}
struct Arguments {
    const void* packet;
    const SayTextMessageNames& names;
};
int marshal_say_text_message(Bridge& bridge, void* context) { //4332296D/43327A0C
    const auto& args = *static_cast<const Arguments*>(context);
    const auto& api = *bridge.api;
    auto* const state = bridge.state;
    api.createtable(state, 0, 4);

    const bool team_only = (field<std::uint32_t>(args.packet, 0x28) & 0x10u) &&
                           field<std::uint8_t>(args.packet, 0xD) == 0;
    api.pushlstring(state, args.names.team_only.get(), 8);
    api.pushboolean(state, team_only);
    api.settable(state, -3);

    const auto entity =
        (field<std::uint8_t>(args.packet, 0x28) & 1u) ? field<std::int32_t>(args.packet, 8) : -1;
    api.pushlstring(state, args.names.entity.get(), 6);
    api.pushinteger(state, entity);
    api.settable(state, -3);

    const auto* name = parameter(args.packet, 0);
    api.pushlstring(state, args.names.name.get(), 4);
    api.pushstring(state, name);
    api.settable(state, -3);

    const auto* text = parameter(args.packet, 1);
    api.pushlstring(state, args.names.text.get(), 4);
    api.pushstring(state, text);
    api.settable(state, -3);
    return 1;
}
}

void dispatch_say_text_message(SayTextMessageRuntime& runtime, const void* packet) {
    if (!runtime.bridge.state)
        return;
    Arguments arguments{packet, runtime.names};
    dispatch_callbacks(runtime.bridge, runtime.names.event.get(), marshal_say_text_message,
                       &arguments);
}
void bind_say_text_message_runtime(SayTextMessageRuntime& runtime) noexcept {
    active_runtime = &runtime;
}
void unbind_say_text_message_runtime(SayTextMessageRuntime& runtime) noexcept {
    if (active_runtime == &runtime)
        active_runtime = nullptr;
}
#if defined(_M_IX86) || defined(__i386__)
void __cdecl say_text_message_hook(const void* packet) {
#else
void say_text_message_hook(const void* packet) {
#endif
    dispatch_say_text_message(*active_runtime, packet);
}
} //namespace skeet::script
