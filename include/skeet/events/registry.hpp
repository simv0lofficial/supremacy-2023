#pragma once
#include "game_events.hpp"
#include "event_name_storage.hpp"
#include <span>

namespace skeet::events {
using EventCallback = void (*)(void* owner, GameEventReader&, EventPlayerIndices&);
struct EventListener {
    std::uint32_t name_hash;
    void* owner;
    EventCallback callback;
};
struct EventRegistry {
private:
    void* standalone_engine_ = nullptr;
    void* standalone_manager_ = nullptr;

public:
    explicit EventRegistry(EventNameStorage& names) noexcept
        : name_storage(names), engine_interface(standalone_engine_),
          event_manager(standalone_manager_) {}
    EventRegistry(EventNameStorage& names, void*& engine, void*& manager) noexcept
        : name_storage(names), engine_interface(engine), event_manager(manager) {}
    EventRegistry(const EventRegistry&) = delete;
    EventRegistry& operator=(const EventRegistry&) = delete;
    EventRegistry(EventRegistry&&) = delete;
    EventRegistry& operator=(EventRegistry&&) = delete;
    EventNameStorage& name_storage;
    std::vector<EventListener> listeners;
    bool subscriptions_dirty = false;
    void*& engine_interface; // live4346A514, IsConnected virtual26
    void*& event_manager; // live43468364, dirty byte+A0

    // Native 4336BAAD / 4341B241. Exact triple equality, stable insertion/erase.
    bool add(EventListener listener, bool engine_connected);
    bool remove(EventListener listener, bool engine_connected);
    // Complete engine-bound entries: the connection query happens only after
    // an actual insert/erase, then publishes the real event-manager dirty byte.
    bool add(EventListener listener);
    bool remove(EventListener listener);
    // 433AE7A5..433AE7E4, after the engine's pre-event observers have run.
    void dispatch(std::uint32_t name_hash, GameEventReader&, EventPlayerIndices* local_player);
    bool contains(std::uint32_t name_hash) const noexcept;
    EventNameRef name(EventName id) const {
        return name_storage.alias(id);
    }
};

// Constructor 4347F89E..4347F8E4 registers these three event types. The impact
// triple is stored separately at owner+100, with subscription state at+10C.
void bind_shot_events(EventRegistry&, GameEventState&, bool engine_connected);
void unbind_shot_events(EventRegistry&, GameEventState&, bool engine_connected);
void bind_shot_events(EventRegistry&, GameEventState&);
void unbind_shot_events(EventRegistry&, GameEventState&);
void dispatch_shot_event(GameEventState&, std::uint32_t name_hash, GameEventReader&,
                         EventPlayerIndices&);

struct SubscriptionDescriptor {
    std::int32_t identifier;
    const char* name;
};
// Native 4336B72D..4336B820: OR interested engine events into existing bits.
// Engine descriptors supply valid nonnegative identifiers within mask storage.
void enable_event_subscriptions(const EventRegistry&, std::span<const std::uint32_t> lua_names,
                                std::span<const SubscriptionDescriptor>,
                                std::span<std::uint32_t> mask);
// Complete4336B72D raw-engine owner walk. Manager+4/+10 is an array of0x34-byte
// descriptors; manager+78 resolves descriptor+4 names through0x18-byte entries.
// The native mask object stores its writable word pointer at+C.
void enable_engine_event_subscriptions(const EventRegistry&,
                                       std::span<const std::uint32_t> lua_names,
                                       const void* event_manager, void* mask_object);
} // namespace skeet::events
