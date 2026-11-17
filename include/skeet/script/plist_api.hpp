#pragma once
#include "skeet/players/state.hpp"
#include "skeet/script/bridge.hpp"

namespace skeet::script {

// Both Lua closures address the same player records as the PLAYERS menu.
class PlistModule {
public:
    explicit PlistModule(players::State& players) : players_(players) {}
    void register_exports(Bridge&);
    int set(void* state); // 0x43325068
    int get(void* state); // 0x433C8E61

private:
    players::State& players_;
    Bridge* bridge_ = nullptr;
    static int set_entry(void*);
    static int get_entry(void*);
};

} // namespace skeet::script
