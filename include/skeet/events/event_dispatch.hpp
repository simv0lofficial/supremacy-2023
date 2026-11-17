#pragma once
#include "engine_event.hpp"
#include "registry.hpp"
#include "skeet/engine/owner_registry.hpp"

namespace skeet::misc {
class PersistentKillFeedRuntime;
}
namespace skeet::script {
class ClientModule;
}
namespace skeet::events {
// The two methods of native observer vector434758E0/E4/E8. Destruction is not
// an interface slot. Constructors append borrowed observers in their own order.
struct EventObserver {
    virtual void before(EngineGameEventReader& event, std::uint32_t name_hash) = 0;
    virtual void after(EngineGameEventReader& event, std::uint32_t name_hash) = 0;
};
void register_event_observer(engine::OwnerRegistry&, EventObserver&, engine::JobMemory);

struct EventMasterBindings {
    void*& engine; //4346A514, aliases EventRegistry::engine_interface
    void*& event_manager; //43468364, aliases EventRegistry::event_manager
    void* const* local_player; //43475808, read after the pre-event pass
};
class EventMaster {
public:
    EventMaster(EventRegistry& registry, engine::OwnerRegistry& observers,
                EventMasterBindings& bindings) noexcept
        : registry_(registry), observers_(observers), bindings_(bindings) {}
    // Complete433AE68A..433AE847. Demo/HLTV invokes the hook metadata's original
    // virtual34; the ordinary path consumes the packet and returns true.
    bool process(void* hook_receiver, void* event_packet);

private:
    EventRegistry& registry_; //43478DD0/DD4/DD8
    engine::OwnerRegistry& observers_;
    EventMasterBindings& bindings_;
};

// Constructor4347E910 appends owner+8 with vtable4346FB0C. Its second method
// really resets the HUD lifetime after every manager dispatch, even if disabled.
class KillFeedEventObserver final : public EventObserver {
public:
    explicit KillFeedEventObserver(misc::PersistentKillFeedRuntime& runtime) noexcept
        : runtime_(runtime) {}
    void before(EngineGameEventReader&, std::uint32_t name_hash) override; //433FA644
    void after(EngineGameEventReader&, std::uint32_t name_hash) override; //4332B844
private:
    misc::PersistentKillFeedRuntime& runtime_;
};

// Constructor4347FFB5 appends owner+40 with vtable4346F6C8. The Lua registry
// is separate from the C++ registry; only its first matching hash is dispatched.
class LuaEventObserver final : public EventObserver {
public:
    LuaEventObserver(script::ClientModule& module, void* const* local_player) noexcept
        : module_(module), local_(local_player) {}
    void before(EngineGameEventReader&, std::uint32_t name_hash) override; //4335B986
    void after(EngineGameEventReader&, std::uint32_t) override {} //433C95B7
private:
    script::ClientModule& module_;
    void* const* local_;
};
} // namespace skeet::events
