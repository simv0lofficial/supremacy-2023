#include "client_detail.hpp"
#include "skeet/misc/logs.hpp"
#include <array>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script {
namespace {
using ColorPrint = void (*)(void*, const std::uint32_t*, const char*, ...);
using ConsolePrint = void (*)(void*, const char*, ...);
using DebugText = void (*)(void*, const std::array<float, 3>&, std::int32_t, float, std::int32_t,
                           std::int32_t, std::int32_t, std::int32_t, const char*, ...);
void color_print(void* cvar, const std::uint32_t& color, const char* format, const char* message) {
    reinterpret_cast<ColorPrint>((*static_cast<void***>(cvar))[25])(cvar, &color, format, message);
}
}
int ClientModule::logging(Export operation, void* state) {
    const auto& api = *bridge().api;
    const auto count = api.gettop(state);
    std::array<char, 8192> message{};
    if (operation == Export::error_log) { //43387611
        if (count > 0)
            if (const auto* text = api.tolstring(state, 1, nullptr)) {
                if (!bindings.error_log)
                    throw std::logic_error("Client error log requires ErrorLog");
                bindings.error_log->write(text);
            }
        return 0;
    }
    if (operation == Export::exec) { //4340126A
        if (concatenate_lua(api, state, 1, message.data(), message.size()))
            engine::invoke<void>(bindings.engine, 108, message.data());
        return 0;
    }
    if (operation == Export::draw_debug_text) { //433F184D
        if (count <= 9)
            return 0;
        const std::array<float, 3> position{float(api.tonumber(state, 1)),
                                            float(api.tonumber(state, 2)),
                                            float(api.tonumber(state, 3))};
        const auto line = std::int32_t(api.tointeger(state, 4));
        const auto duration = float(api.tonumber(state, 5));
        const auto red = std::int32_t(api.tointeger(state, 6));
        const auto green = std::int32_t(api.tointeger(state, 7));
        const auto blue = std::int32_t(api.tointeger(state, 8));
        const auto alpha = std::int32_t(api.tointeger(state, 9));
        if (concatenate_lua(api, state, 10, message.data(), message.size()))
            reinterpret_cast<DebugText>((*static_cast<void***>(bindings.debug_overlay))[18])(
                bindings.debug_overlay, position, line, duration, red, green, blue, alpha, "%s",
                message.data());
        return 0;
    }
    if (operation == Export::set_clan_tag) { //4340C608
        if (!bindings.set_clan_tag || !engine::invoke<bool>(bindings.engine, 26) ||
            engine::invoke<bool>(bindings.engine, 82) || engine::invoke<bool>(bindings.engine, 93))
            return 0;
        if (!bindings.frame_number)
            throw std::logic_error("Clan tag requires frame counter");
        if (last_clan_frame_ == *bindings.frame_number)
            return 0;
        last_clan_frame_ = *bindings.frame_number;
        if (count < 1 || !concatenate_lua(api, state, 1, message.data(), 4096))
            message[0] = 0;
        bindings.set_clan_tag(message.data(), message.data());
        return 0;
    }
    if (operation == Export::color_log) { //433FFDAB
        if (count <= 3)
            return 0;
        const auto red = std::int32_t(api.checkinteger(state, 1));
        const auto green = std::int32_t(api.checkinteger(state, 2));
        const auto blue = std::int32_t(api.checkinteger(state, 3));
        const auto length = concatenate_lua(api, state, 4, message.data(), 8191);
        if (!length)
            return 0;
        const auto color = 0xff000000u | (std::uint32_t(blue) & 255) << 16 |
                           (std::uint32_t(green) & 255) << 8 | (std::uint32_t(red) & 255);
        message[length] = '\n';
        message[length + 1] = 0;
        color_print(bindings.cvar, color, "%s", message.data());
        if (has_output(bridge())) {
            message[length] = 0;
            dispatch_output(bridge(), std::string_view(message.data(), length), red, green, blue,
                            255);
        } else if (bindings.hud_log)
            bindings.hud_log->append(message.data(), color);
        return 0;
    }
    if (!bindings.normal_log)
        throw std::logic_error("Client log requires NormalLog");
    if (count == 1 && api.type(state, 1) == 4) { //4341CDD9 bypasses the fixed formatting buffer
        const auto* text = api.tolstring(state, 1, nullptr);
        if (bindings.normal_log->disabled && *bindings.normal_log->disabled)
            return 0;
        constexpr std::uint32_t prefix = 0xff1de6b5;
        color_print(bindings.cvar, prefix, "%s", "[supremacy] ");
        reinterpret_cast<ConsolePrint>((*static_cast<void***>(bindings.cvar))[26])(bindings.cvar,
                                                                                   "%s\n", text);
        if (has_output(bridge()))
            dispatch_output(bridge(), text, 255, 255, 255, 255);
        else if (bindings.hud_log) {
            bindings.hud_log->append(text, 0xffffffff);
            bindings.hud_log->append("\n", 0xffffffff);
        }
    } else if (concatenate_lua(api, state, 1, message.data(), message.size()))
        bindings.normal_log->write(message.data());
    return 0;
}
} // namespace skeet::script
