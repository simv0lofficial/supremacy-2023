#include "skeet/script/entity_api.hpp"
#include <array>
#include <stdexcept>

namespace skeet::script {
namespace {
void* (*owner)(void*, int) = nullptr;
}
template <int Index> int EntityModule::entry(void* state) {
    auto& module = *static_cast<EntityModule*>(owner(state, -10003));
    constexpr std::array methods{&EntityModule::get_local_player,
                                 &EntityModule::get_all,
                                 &EntityModule::get_players,
                                 &EntityModule::get_game_rules,
                                 &EntityModule::get_player_resource,
                                 &EntityModule::get_classname,
                                 &EntityModule::set_prop,
                                 &EntityModule::get_prop,
                                 &EntityModule::new_prop,
                                 &EntityModule::is_enemy,
                                 &EntityModule::is_alive,
                                 &EntityModule::is_dormant,
                                 &EntityModule::get_player_name,
                                 &EntityModule::get_player_weapon,
                                 &EntityModule::hitbox_position,
                                 &EntityModule::get_steam64,
                                 &EntityModule::get_bounding_box,
                                 &EntityModule::get_origin,
                                 &EntityModule::get_esp_data};
    return (module.*methods[Index])(state);
}
void EntityModule::register_exports(Bridge& bridge) {
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Entity exports require an initialized Lua state");
    bridge_ = &bridge;
    rules_handle_ = UINT32_MAX;
    custom_properties_.clear();
    resolved_properties_.clear();
    owner = bridge.api->touserdata;
    constexpr std::array names{"get_local_player",
                               "get_all",
                               "get_players",
                               "get_game_rules",
                               "get_player_resource",
                               "get_classname",
                               "set_prop",
                               "get_prop",
                               "new_prop",
                               "is_enemy",
                               "is_alive",
                               "is_dormant",
                               "get_player_name",
                               "get_player_weapon",
                               "hitbox_position",
                               "get_steam64",
                               "get_bounding_box",
                               "get_origin",
                               "get_esp_data"};
    constexpr std::array functions{entry<0>,  entry<1>,  entry<2>,  entry<3>,  entry<4>,
                                   entry<5>,  entry<6>,  entry<7>,  entry<8>,  entry<9>,
                                   entry<10>, entry<11>, entry<12>, entry<13>, entry<14>,
                                   entry<15>, entry<16>, entry<17>, entry<18>};
    lua().createtable(bridge.state, 0, 19);
    for (std::size_t i = 0; i < functions.size(); ++i) {
        lua().pushlightuserdata(bridge.state, this);
        lua().pushcclosure(bridge.state, functions[i], 1);
        lua().setfield(bridge.state, -2, names[i]);
    }
    lua().setfield(bridge.state, -10002, "entity");
}
} // namespace skeet::script
