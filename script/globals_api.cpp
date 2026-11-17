#include "skeet/script/globals_api.hpp"
#include "skeet/engine/interface_call.hpp"
#include <array>
#include <stdexcept>
#include <utility>
#include <emmintrin.h>

namespace skeet::script {
namespace {
void* (*export_context)(void*, int) = nullptr;
constexpr int first_upvalue = -10003;

double native_number(float value) noexcept {
    return _mm_cvtsd_f64(_mm_cvtss_sd(_mm_setzero_pd(), _mm_set_ss(value)));
}
}

GlobalsModule& GlobalsModule::owner(void* state) {
    return *static_cast<GlobalsModule*>(export_context(state, first_upvalue));
}

int GlobalsModule::push(void* state, Value value) {
    const auto& globals = *bindings_.variables;
    const auto number = [&](float field) { api_->pushnumber(state, native_number(field)); };
    const auto integer = [&](std::int32_t field) { api_->pushinteger(state, field); };
    switch (value) {
    case Value::realtime:
        number(globals.realtime);
        break; // 4339FD3A
    case Value::curtime:
        number(globals.current_time);
        break; // 43384277
    case Value::frametime:
        number(globals.frame_time);
        break; // 4339FB35
    case Value::absoluteframetime:
        number(globals.absolute_frame_time);
        break; // 4334D223
    case Value::maxplayers:
        integer(globals.max_clients);
        break; // 43384020
    case Value::tickcount:
        integer(globals.tick_count);
        break; // 43402A2E
    case Value::tickinterval:
        number(globals.interval_per_tick);
        break; // 433B75A4
    case Value::framecount:
        integer(globals.frame_count);
        break; // 434030F0
    case Value::framelerp:
        number(globals.interpolation_amount);
        break; // 43347A64
    case Value::mapname: // 433AD1FD
        if (engine::invoke<bool>(bindings_.engine_client, 26))
            api_->pushstring(state, engine::invoke<const char*>(bindings_.engine_client, 53));
        else
            api_->pushnil(state);
        break;
    case Value::lastoutgoingcommand:
        integer(*bindings_.last_outgoing_command);
        break; // 43419FE2
    case Value::oldcommandack:
        integer(*bindings_.old_command_ack);
        break; // 4341CBE5
    case Value::commandack:
        integer(*bindings_.command_ack);
        break; // 4339FB5D
    case Value::chokedcommands:
        integer(*bindings_.choked_commands);
        break; // 433E0842
    case Value::servertickcount: { // 433E1246
        // The x86 engine return is stored as binary32 before the SSE division.
        const float time = engine::invoke<float>(bindings_.engine_client, 14);
        const auto ticks =
            _mm_div_ss(_mm_set_ss(time), _mm_set_ss(*bindings_.server_tick_interval));
        integer(_mm_cvttss_si32(_mm_add_ss(ticks, _mm_set_ss(0.5f))));
        break;
    }
    case Value::interpolationamount: // 433D0B64
        integer(_mm_cvttss_si32(_mm_set_ss(globals.interpolation_amount)));
        break;
    }
    return 1;
}

#define GLOBALS_ENTRY(name)                                                                        \
    int GlobalsModule::name##_entry(void* state) {                                                 \
        return owner(state).push(state, Value::name);                                              \
    }
GLOBALS_ENTRY(realtime)
GLOBALS_ENTRY(curtime)
GLOBALS_ENTRY(frametime)
GLOBALS_ENTRY(absoluteframetime)
GLOBALS_ENTRY(maxplayers)
GLOBALS_ENTRY(tickcount)
GLOBALS_ENTRY(tickinterval)
GLOBALS_ENTRY(framecount)
GLOBALS_ENTRY(framelerp)
GLOBALS_ENTRY(mapname)
GLOBALS_ENTRY(lastoutgoingcommand)
GLOBALS_ENTRY(oldcommandack)
GLOBALS_ENTRY(commandack)
GLOBALS_ENTRY(chokedcommands)
GLOBALS_ENTRY(servertickcount)
GLOBALS_ENTRY(interpolationamount)
#undef GLOBALS_ENTRY

void GlobalsModule::register_exports(Bridge& bridge) {
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Lua globals require an initialized state");
    if (!bindings_.variables || !bindings_.engine_client || !bindings_.server_tick_interval ||
        !bindings_.last_outgoing_command || !bindings_.old_command_ack || !bindings_.command_ack ||
        !bindings_.choked_commands)
        throw std::logic_error("Lua globals require engine and command bindings");
    const auto& api = *bridge.api;
    api_ = &api;
    export_context = api.touserdata;
    constexpr std::array<std::pair<const char*, LuaApi::CFunction>, 16> functions{
        {{"realtime", realtime_entry},
         {"curtime", curtime_entry},
         {"frametime", frametime_entry},
         {"absoluteframetime", absoluteframetime_entry},
         {"maxplayers", maxplayers_entry},
         {"tickcount", tickcount_entry},
         {"tickinterval", tickinterval_entry},
         {"framecount", framecount_entry},
         {"framelerp", framelerp_entry},
         {"mapname", mapname_entry},
         {"lastoutgoingcommand", lastoutgoingcommand_entry},
         {"oldcommandack", oldcommandack_entry},
         {"commandack", commandack_entry},
         {"chokedcommands", chokedcommands_entry},
         {"servertickcount", servertickcount_entry},
         {"interpolationamount", interpolationamount_entry}}};
    api.createtable(bridge.state, 0, 0);
    for (const auto& [name, function] : functions) {
        api.pushlightuserdata(bridge.state, this);
        api.pushcclosure(bridge.state, function, 1);
        api.setfield(bridge.state, -2, name);
    }
    api.setfield(bridge.state, -10002, "globals");
}

} // namespace skeet::script
