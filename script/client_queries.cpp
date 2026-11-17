#include "client_detail.hpp"
#include "skeet/script/cvar_api.hpp"
#include "skeet/menu/players_menu.hpp"
#include "skeet/movement/steering_math.hpp"
#include <cmath>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script {
using namespace client_detail;
void* ClientModule::client_cvar(const char* name) { //433B90BE, successful lookups only
    if (const auto cached = cvar_cache_.find(name); cached != cvar_cache_.end())
        return cached->second;
    auto* result = engine::invoke<void*>(bindings.cvar, 17, name);
    if (result)
        cvar_cache_.emplace(name, result);
    return result;
}
void* ClientModule::eye_player() const { //43359AFA
    auto* player = bindings.local_player;
    if (!player || field<std::uint8_t>(player, bindings.offsets.life_state) == 0 ||
        field<std::uint8_t>(player, bindings.offsets.movement_type) != 10)
        return player;
    const auto mode = field<std::int32_t>(player, bindings.offsets.observer_mode);
    if (mode != 4 && mode != 5)
        return player;
    const auto handle = field<std::int32_t>(player, bindings.offsets.observer_target);
    if (handle == -1)
        return player;
    auto* observer = engine::invoke<void*>(bindings.entity_list, 4, handle);
    return observer && engine::invoke<bool>(observer, 158) ? observer : player;
}
int ClientModule::queries(Export operation, void* state) {
    const auto& api = *bridge().api;
    switch (operation) {
    case Export::userid_to_entindex: { //43330913
        std::int32_t index = 0;
        if (api.gettop(state) == 1 && api.isnumber(state, 1))
            index =
                engine::invoke<std::int32_t>(bindings.engine, 9, truncate(api.tonumber(state, 1)));
        api.pushinteger(state, index);
        return 1;
    }
    case Export::get_cvar: { //4339FCC2
        const auto* name = api.gettop(state) >= 1 ? api.checklstring(state, 1, nullptr) : nullptr;
        auto* variable = name && *name ? client_cvar(name) : nullptr;
        const auto* value = variable ? field<const char*>(variable, 0x24) : nullptr;
        if (value && *value)
            api.pushlstring(state, value, std::strlen(value));
        else
            api.pushnil(state);
        return 1;
    }
    case Export::set_cvar: { //433CBF27
        if (api.gettop(state) != 2)
            return 0;
        const auto* name = api.checklstring(state, 1, nullptr);
        if (!name || !*name)
            return api.argerror(state, 1, "invalid name");
        auto* variable = client_cvar(name);
        if (!variable)
            return 0;
        const auto type = api.type(state, 2);
        if (type == 1) {
            const auto value = api.toboolean(state, 2) & 1;
            set_cvar_raw_int(variable, value);
            set_cvar_raw_float(variable, float(value));
        } else if (type == 3) {
            const auto value = float(api.tonumber(state, 2));
            set_cvar_raw_int(variable, truncate(value + 0.5f));
            set_cvar_raw_float(variable, value);
        } else if (type == 4) {
            if (const auto* value = api.tolstring(state, 2, nullptr))
                engine::invoke<void>(variable, 14, value);
        }
        return 0;
    }
    case Export::screen_size: { //433952AD
        std::int32_t width = 0, height = 0;
        engine::invoke<void, std::int32_t&, std::int32_t&>(bindings.engine, 5, width, height);
        api.pushinteger(state, width);
        api.pushinteger(state, height);
        return 2;
    }
    case Export::latency: { //433E846D
        float result = 0;
        if (auto* channel = engine::invoke<void*>(bindings.engine, 78)) {
            result = engine::invoke<float>(channel, 10, 0);
            const auto rate = cvar_float(const_cast<void*>(bindings.latency_rate_cvar.get()));
            if (rate > 0)
                result -= 0.5f / rate;
            result = maximum_number(0, result);
        }
        api.pushnumber(state, double(result));
        return 1;
    }
    case Export::real_latency: { //433CF01E
        float result = 0;
        if (engine::invoke<bool>(bindings.engine, 27))
            result = maximum_number(
                0, float(*bindings.real_latency_incoming + *bindings.real_latency_outgoing));
        api.pushnumber(state, double(result));
        return 1;
    }
    case Export::camera_angles: { //433ADFBE
        if (api.gettop(state) < 2) {
            std::array<float, 3> angles{};
            engine::invoke<void, std::array<float, 3>&>(bindings.engine, 18, angles);
            push_vector(api, state, angles);
            return 3;
        }
        MovementAngles angles{float(api.checknumber(state, 1)), float(api.checknumber(state, 2)),
                              0};
        if (std::isnan(angles.pitch))
            angles.pitch = 0;
        if (std::isnan(angles.yaw))
            angles.yaw = 0;
        const bool unrestricted =
            !(bindings.angle_restriction_switch && *bindings.angle_restriction_switch) &&
            bindings.game_rules && *bindings.game_rules &&
            field<std::uint8_t>(*bindings.game_rules, bindings.offsets.valve_matchmaking) == 0;
        if (!unrestricted)
            normalize_movement_angles(angles);
        angles.roll = 0;
        engine::invoke<void, const MovementAngles&>(bindings.engine, 19, angles);
        return 0;
    }
    case Export::camera_position:
        push_vector(api, state, *bindings.camera_position);
        return 3; //4334F4DF
    case Export::eye_position: { //434184C1
        auto* player = eye_player();
        if (!player) {
            api.pushnil(state);
            return 1;
        }
        const auto& origin = engine::invoke<const std::array<float, 3>&>(player, 10);
        const auto& offset = field<std::array<float, 3>>(player, bindings.offsets.view_offset);
        const std::array<float, 3> position{origin[0] + offset[0], origin[1] + offset[1],
                                            origin[2] + offset[2]};
        push_vector(api, state, position);
        return 3;
    }
    case Export::reload_active_scripts:
        *bindings.reload_scripts = 1;
        return 0; //433C8748
    case Export::key_state: { //433319E1
        bool down = false;
        if (api.gettop(state) > 0) {
            const auto key = std::uint32_t(api.checkinteger(state, 1));
            if ((key & 0xffffff00u) == 0) {
                HotkeyBinding binding{key << 2, HotkeyMode::hold};
                down = hotkey_active(binding, bindings.input);
            }
        }
        api.pushboolean(state, down);
        return 1;
    }
    case Export::update_player_list: { //4334F461
        if (engine::invoke<bool>(bindings.engine, 27)) {
            const auto time = engine::invoke<float>(bindings.engine, 14);
            const auto tick = truncate(time / *bindings.engine_interval + 0.5f);
            const auto delta = std::bit_cast<std::int32_t>(std::uint32_t(tick) -
                                                           std::uint32_t(last_player_update_));
            if (*bindings.interval_per_tick <
                std::fabs(float(delta) * *bindings.interval_per_tick)) {
                last_player_update_ = tick;
                if (!bindings.players_menu)
                    throw std::logic_error("Client player list requires PlayersMenu");
                bindings.players_menu->activate();
            }
        }
        return 0;
    }
    case Export::get_model_name: { //433B25A0
        const char* name = nullptr;
        if (engine::invoke<bool>(bindings.engine, 27) && api.gettop(state) > 0 &&
            api.type(state, 1) == 3) {
            const auto index = std::int32_t(api.tointeger(state, 1));
            if (index != -1)
                if (auto* model = engine::invoke<void*>(bindings.model_info, 1, index))
                    name = engine::invoke<const char*>(bindings.model_info, 3, model);
        }
        if (name)
            api.pushstring(state, name);
        else
            api.pushnil(state);
        return 1;
    }
    case Export::register_esp_flag: { //43386339
        if (flags.entries.size() >= 32)
            return 0;
        const auto* text = api.checklstring(state, 1, nullptr);
        if (!text)
            return 0;
        const auto red = std::uint32_t(api.checkinteger(state, 2));
        const auto green = std::uint32_t(api.checkinteger(state, 3));
        const auto blue = std::uint32_t(api.checkinteger(state, 4));
        api.checktype(state, 5, 6);
        api.pushvalue(state, 5);
        const auto reference = api.ref(state, registry);
        if (reference == -1)
            return 0;
        ReferenceString flag;
        flag.reference = reference;
        const auto length = static_cast<int>(std::strlen(text));
        const auto units = MultiByteToWideChar(CP_UTF8, 0, text, length, nullptr, 0);
        if (units > 0) {
            flag.text.resize(units);
            MultiByteToWideChar(CP_UTF8, 0, text, length,
                                reinterpret_cast<wchar_t*>(flag.text.data()), units);
            CharUpperW(reinterpret_cast<wchar_t*>(flag.text.data()));
        }
        flag.color = 0xff000000u | (blue & 255) << 16 | (green & 255) << 8 | (red & 255);
        flags.entries.push_back(std::move(flag));
        return 0;
    }
    case Export::current_threat: { //4339DED2
        const auto handle = *bindings.threat_handle;
        if (!engine::invoke<bool>(bindings.engine, 26) || handle == -1 ||
            !engine::invoke<void*>(bindings.entity_list, 4, handle))
            api.pushnil(state);
        else
            api.pushinteger(state, std::uint32_t(handle) & 0xffffu);
        return 1;
    }
    case Export::request_full_update: //4337D1EC
        if (engine::invoke<bool>(bindings.engine, 26))
            *bindings.full_update = 1;
        return 0;
    default:
        throw std::logic_error("Incorrect client query dispatch");
    }
}
} // namespace skeet::script
