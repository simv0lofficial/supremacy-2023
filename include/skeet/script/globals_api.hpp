#pragma once
#include "skeet/script/bridge.hpp"

namespace skeet::script {

// Engine global-vars memory borrowed by native global 43467B04. Offset 0x0C
// is not consumed by this module and retains its native storage position.
struct GlobalsVariables {
    float realtime; // +00
    std::int32_t frame_count; // +04
    float absolute_frame_time; // +08
    std::uint32_t reserved_0c; // +0C
    float current_time; // +10
    float frame_time; // +14
    std::int32_t max_clients; // +18
    std::int32_t tick_count; // +1C
    float interval_per_tick; // +20
    float interpolation_amount; // +24
};
static_assert(sizeof(GlobalsVariables) == 0x28);
static_assert(offsetof(GlobalsVariables, interpolation_amount) == 0x24);

struct GlobalsBindings {
    const GlobalsVariables* variables; // *43467B04
    void* engine_client; // 4346A514, IVEngineClient
    const float* server_tick_interval; // *4346A338
    const std::int32_t* last_outgoing_command; // *43475B2C
    const std::int32_t* old_command_ack; // *43475794
    const std::int32_t* command_ack; // *43475BA4
    const std::int32_t* choked_commands; // *434757DC
};

// Actual 16-entry globals table from decoded record 4347538C / array434753E0.
// The engine and pointed-to memory must outlive the registered Lua closures.
class GlobalsModule {
public:
    explicit GlobalsModule(GlobalsBindings bindings) noexcept : bindings_(bindings) {}
    GlobalsModule(const GlobalsModule&) = delete;
    GlobalsModule& operator=(const GlobalsModule&) = delete;
    void register_exports(Bridge&); // generic binder43323193

private:
    GlobalsBindings bindings_;
    const LuaApi* api_ = nullptr;
    enum class Value {
        realtime,
        curtime,
        frametime,
        absoluteframetime,
        maxplayers,
        tickcount,
        tickinterval,
        framecount,
        framelerp,
        mapname,
        lastoutgoingcommand,
        oldcommandack,
        commandack,
        chokedcommands,
        servertickcount,
        interpolationamount
    };
    int push(void*, Value);
    static GlobalsModule& owner(void*);
    static int realtime_entry(void*);
    static int curtime_entry(void*);
    static int frametime_entry(void*);
    static int absoluteframetime_entry(void*);
    static int maxplayers_entry(void*);
    static int tickcount_entry(void*);
    static int tickinterval_entry(void*);
    static int framecount_entry(void*);
    static int framelerp_entry(void*);
    static int mapname_entry(void*);
    static int lastoutgoingcommand_entry(void*);
    static int oldcommandack_entry(void*);
    static int commandack_entry(void*);
    static int chokedcommands_entry(void*);
    static int servertickcount_entry(void*);
    static int interpolationamount_entry(void*);
};

} // namespace skeet::script
