#include "skeet/script/bridge.hpp"
#include "detail.hpp"
#include <cstdint>
#include <stdexcept>

namespace skeet::script {
namespace {
using namespace detail;
constexpr int registry = -10000;
constexpr int function_type = 6;

void push_integer_field(const LuaApi& api, void* state, const char* key, std::int32_t value) {
    api.pushlstring(state, key, 1);
    api.pushinteger(state, value);
    api.settable(state, -3);
}

int& exclusive_ref(Bridge& bridge, ExclusiveCallback kind) noexcept {
    return kind == ExclusiveCallback::indicator ? bridge.indicator_ref : bridge.output_ref;
}

void check_binding(const Bridge& bridge) {
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Lua callback bridge requires an initialized state");
}

thread_local Bridge* shutdown_bridge = nullptr;
int notify_shutdown_callback(void* state) {
    auto& bridge = *shutdown_bridge;
    const auto& api = *bridge.api;
    api.rawgeti(state, registry, bridge.runtime_ref);
    api.rawgeti(state, -1, 2);
    api.remove(state, -2);
    api.call(state, 0, 0);
    return 0;
}
void release_callback_storage(std::unordered_map<std::uint32_t, bool>& callbacks) noexcept {
    //433CF5BD traverses buckets forwards and unlinks each node before freeing
    //the bucket allocation. These are the maps the source dispatcher updates.
    const auto buckets = callbacks.bucket_count();
    for (std::size_t index = 0; index < buckets; ++index)
        while (callbacks.begin(index) != callbacks.end(index))
            callbacks.erase(callbacks.begin(index)->first);
    std::unordered_map<std::uint32_t, bool>{}.swap(callbacks);
}
}

void Bridge::finalize_active_callback_storage() noexcept {
    release_callback_storage(callback_active);
}
void Bridge::finalize_dirty_callback_storage() noexcept {
    release_callback_storage(callback_dirty);
}

bool has_indicator(const Bridge& bridge) noexcept {
    const std::lock_guard lock(bridge.vm_mutex);
    return bridge.state != nullptr && bridge.indicator_ref != -1;
}

bool has_output(const Bridge& bridge) noexcept {
    const std::lock_guard lock(bridge.vm_mutex);
    return bridge.state != nullptr && bridge.output_ref != -1;
}

void CallbackState::initialize(Bridge& bridge) const {
    check_binding(bridge);
    bridge.callback_active.clear();
    bridge.callback_dirty.clear();
    bridge.recent_callback_errors = {};
    bridge.api->createtable(bridge.state, 0, 0);
    bridge.callback_table_ref = bridge.api->ref(bridge.state, registry);
}

void CallbackState::release(Bridge& bridge) const {
    check_binding(bridge);
    release_reference(*bridge.api, bridge.state, bridge.callback_table_ref);
    release_reference(*bridge.api, bridge.state, bridge.output_ref);
    release_reference(*bridge.api, bridge.state, bridge.indicator_ref);
}

void dispatch_indicator(Bridge& bridge, RenderContext context, std::string_view text,
                        std::int32_t red, std::int32_t green, std::int32_t blue,
                        std::int32_t alpha) {
    const std::lock_guard lock(bridge.vm_mutex);
    check_binding(bridge);
    if (bridge.indicator_active || bridge.indicator_ref == -1)
        return;
    bridge.indicator_active = true;

    const auto previous_context = bridge.render_context;
    bridge.render_context = context;
    const auto& api = *bridge.api;
    void* const state = bridge.state;
    CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = api.gettop(state);
    RtlPushFrame(&guard.active);
    auto invoke = [&] {
        api.rawgeti(state, registry, bridge.indicator_ref);
        if (api.type(state, -1) == function_type) {
            api.createtable(state, 0, 5);
            api.pushlstring(state, bridge.indicator_text_key.get(), 4);
            api.pushlstring(state, text.data(), text.size());
            api.settable(state, -3);
            push_integer_field(api, state, "r", red);
            push_integer_field(api, state, "g", green);
            push_integer_field(api, state, "b", blue);
            push_integer_field(api, state, "a", alpha);
            if (callback_pcall(state, 1, 0) != 0)
                report_callback_error(bridge, state);
        } else {
            api.settop(state, -2);
        }
    };
    protected_region(invoke, guard);
    api.settop(state, guard.saved_top);
    RtlPopFrame(&guard.active);
    bridge.indicator_active = false;
    bridge.render_context = previous_context;
}

void dispatch_indicator(Bridge& bridge, const void* queue, std::int32_t* cursor,
                        std::string_view text, std::uint32_t abgr) {
    dispatch_indicator(bridge, {queue, cursor}, text, abgr & 255, (abgr >> 8) & 255,
                       (abgr >> 16) & 255, abgr >> 24);
}

void dispatch_output(Bridge& bridge, std::string_view text, std::int32_t red, std::int32_t green,
                     std::int32_t blue, std::int32_t alpha) {
    const std::lock_guard lock(bridge.vm_mutex);
    check_binding(bridge);
    if (bridge.output_active || bridge.output_ref == -1)
        return;
    bridge.output_active = true;
    const auto& api = *bridge.api;
    void* const state = bridge.state;
    CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = api.gettop(state);
    RtlPushFrame(&guard.active);
    auto invoke = [&] {
        api.rawgeti(state, registry, bridge.output_ref);
        if (api.type(state, -1) == function_type) {
            api.createtable(state, 0, 5);
            api.pushlstring(state, bridge.indicator_text_key.get(), 4);
            api.pushlstring(state, text.data(), text.size());
            api.settable(state, -3);
            push_integer_field(api, state, "r", red);
            push_integer_field(api, state, "g", green);
            push_integer_field(api, state, "b", blue);
            push_integer_field(api, state, "a", alpha);
            if (callback_pcall(state, 1, 0) != 0)
                report_callback_error(bridge, state);
        } else {
            api.settop(state, -2);
        }
    };
    protected_region(invoke, guard);
    api.settop(state, guard.saved_top);
    RtlPopFrame(&guard.active);
    bridge.output_active = false;
}

void dispatch_output(Bridge& bridge, std::string_view text, std::uint32_t abgr) {
    dispatch_output(bridge, text, abgr & 255, (abgr >> 8) & 255, (abgr >> 16) & 255, abgr >> 24);
}

void release_reference(const LuaApi& api, void* state, int& reference) {
    if (reference != -1) {
        api.unref(state, registry, reference);
        reference = -1;
    }
}

void register_exclusive(Bridge& bridge, ExclusiveCallback kind, int function_index) {
    register_exclusive(bridge, bridge.state, kind, function_index);
}

void register_exclusive(Bridge& bridge, void* state, ExclusiveCallback kind, int function_index) {
    check_binding(bridge);
    auto& reference = exclusive_ref(bridge, kind);
    release_reference(*bridge.api, state, reference);
    bridge.api->pushvalue(state, function_index);
    reference = bridge.api->ref(state, registry);
}

void unregister_exclusive(Bridge& bridge, ExclusiveCallback kind) {
    unregister_exclusive(bridge, bridge.state, kind);
}

void unregister_exclusive(Bridge& bridge, void* state, ExclusiveCallback kind) {
    check_binding(bridge);
    release_reference(*bridge.api, state, exclusive_ref(bridge, kind));
}

bool push_callback_list(const Bridge& bridge, void* state, const char* name) {
    if (bridge.callback_table_ref == -1)
        return false;
    check_binding(bridge);
    const auto& api = *bridge.api;
    api.rawgeti(state, registry, bridge.callback_table_ref);
    api.pushstring(state, name);
    api.gettable(state, -2);
    if (api.type(state, -1) == 5)
        return true;
    api.settop(state, -3);
    return false;
}

int find_callback(const LuaApi& api, void* state, int table_index, int value_index) {
    const auto count = static_cast<std::int32_t>(api.objlen(state, table_index));
    for (int index = 1; index <= count; ++index) {
        api.rawgeti(state, table_index, index);
        const auto equal = api.rawequal(state, -1, value_index);
        api.settop(state, -2);
        if (equal == 1)
            return index;
    }
    return 0;
}

void remove_callback(const LuaApi& api, void* state, int table_index, int index) {
    api.checktype(state, table_index, 5);
    const auto count = static_cast<std::int32_t>(api.objlen(state, table_index));
    if (index < 1 || index > count)
        return;
    while (++index <= count) {
        api.rawgeti(state, table_index, index);
        api.rawseti(state, table_index, index - 1);
    }
    api.pushnil(state);
    api.rawseti(state, table_index, count);
}

void compact_callback_list(const LuaApi& api, void* state, int table_index) {
    api.checktype(state, table_index, 5);
    const auto count = static_cast<int>(api.objlen(state, table_index));
    const auto table = table_index < 0 ? api.gettop(state) + table_index + 1 : table_index;
    int next = 1;
    for (int index = 1; index <= count; ++index) {
        api.rawgeti(state, table, index);
        if (api.type(state, -1) == function_type) {
            if (next != index)
                api.rawseti(state, table, next);
            else
                api.settop(state, -2);
            ++next;
        } else {
            api.settop(state, -2);
        }
    }
    for (int index = next; index <= count; ++index) {
        api.pushnil(state);
        api.rawseti(state, table, index);
    }
}

namespace {
int dispatch_event(Bridge& bridge, void* state, const char* name, Marshaller marshal, void* context,
                   ArgumentConsumer consume, bool has_argument = false) {
    check_binding(bridge);
    const auto hash = callback_hash(name);
    auto& active = bridge.callback_active[hash];
    if (active)
        return 0;
    const auto& api = *bridge.api;
    CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = api.gettop(state);
    RtlPushFrame(&guard.active);
    active = true;
    int completed = 0;
    auto invoke = [&] {
        api.lcheckstack(state, 12, "dispatching Lua callbacks");
        if (!push_callback_list(bridge, state, name))
            return;
        const auto list = api.gettop(state);
        const auto count = static_cast<int>(api.objlen(state, list));
        if (count == 0)
            return;
        int arguments = 0;
        if (marshal) {
            arguments = marshal(bridge, context);
        } else if (has_argument) {
            api.pushvalue(state, 2);
            arguments = 1;
        }
        if (arguments >= 0) {
            for (int index = 1; index <= count; ++index) {
                api.rawgeti(state, list, index);
                if (api.type(state, -1) != function_type) {
                    api.settop(state, -2);
                    continue;
                }
                for (int argument = 1; argument <= arguments; ++argument)
                    api.pushvalue(state, list + argument);
                if (callback_pcall(state, arguments, 0) == 0)
                    ++completed;
                else
                    report_callback_error(bridge, state);
            }
            if (consume)
                consume(bridge, context, completed);
        }
        api.settop(state, list);
        auto dirty = bridge.callback_dirty.find(hash);
        if (dirty != bridge.callback_dirty.end() && dirty->second) {
            compact_callback_list(api, state, list);
            dirty->second = false;
        }
    };
    const bool dispatched = protected_region(invoke, guard);
    api.settop(state, guard.saved_top);
    active = false;
    RtlPopFrame(&guard.active);
    if (!dispatched) {
        report_native_exception(bridge, guard, name, true);
        return 0;
    }
    return completed;
}
}

int dispatch_callbacks(Bridge& bridge, const char* name, Marshaller marshal, void* context) {
    return dispatch_callbacks(bridge, name, marshal, context, nullptr);
}

int dispatch_callbacks(Bridge& bridge, const char* name, Marshaller marshal, void* context,
                       ArgumentConsumer consume) {
    const std::lock_guard lock(bridge.vm_mutex);
    return dispatch_event(bridge, bridge.state, name, marshal, context, consume);
}

void dispatch_custom_event(Bridge& bridge, void* state, const char* name, bool has_argument) {
    const std::lock_guard lock(bridge.vm_mutex);
    dispatch_event(bridge, state, name, nullptr, nullptr, nullptr, has_argument);
}

namespace {
struct IntegerEvent {
    const char* field;
    std::int32_t value;
};
struct NamedIntegerEvent {
    events::EventNameRef field;
    std::int32_t value;
};
int marshal_integer_event(Bridge& bridge, void* context) {
    const auto& event = *static_cast<const IntegerEvent*>(context);
    const auto& api = *bridge.api;
    api.createtable(bridge.state, 0, 1);
    api.pushlstring(bridge.state, event.field, 14);
    api.pushinteger(bridge.state, event.value);
    api.settable(bridge.state, -3);
    return 1;
}
int marshal_named_integer_event(Bridge& bridge, void* context) {
    const auto& event = *static_cast<const NamedIntegerEvent*>(context);
    const auto& api = *bridge.api;
    api.createtable(bridge.state, 0, 1);
    api.pushlstring(bridge.state, event.field.get(), 14);
    api.pushinteger(bridge.state, event.value);
    api.settable(bridge.state, -3);
    return 1;
}
}

int dispatch_integer_event(Bridge& bridge, const char* name, const char* field,
                           std::int32_t value) {
    IntegerEvent event{field, value};
    return dispatch_callbacks(bridge, name, marshal_integer_event, &event);
}
int dispatch_integer_event(Bridge& bridge, const char* name, events::EventNameRef field,
                           std::int32_t value) {
    NamedIntegerEvent event{field, value};
    return dispatch_callbacks(bridge, name, marshal_named_integer_event, &event);
}

namespace {
void integer_field(Bridge& bridge, std::string_view name, std::int32_t value) {
    const auto& api = *bridge.api;
    api.pushlstring(bridge.state, name.data(), name.size());
    api.pushinteger(bridge.state, value);
    api.settable(bridge.state, -3);
}
void integer_field(Bridge& bridge, events::EventNameRef name, std::size_t length,
                   std::int32_t value) {
    const auto& api = *bridge.api;
    api.pushlstring(bridge.state, name.get(), length);
    api.pushinteger(bridge.state, value);
    api.settable(bridge.state, -3);
}

void chance_field(Bridge& bridge, const AimShot& shot) {
    const auto& api = *bridge.api;
    api.pushlstring(bridge.state, bridge.name(events::EventName::hit_chance).get(), 10);
    const float percent = shot.hit_chance * 100.0f;
    api.pushnumber(bridge.state, static_cast<double>(percent));
    api.settable(bridge.state, -3);
}

struct MissEvent {
    const AimShot* shot;
    std::uint32_t* reason;
};
int marshal_aim_miss(Bridge& bridge, void* context) {
    auto& event = *static_cast<MissEvent*>(context);
    const auto& shot = *event.shot;
    const auto& api = *bridge.api;
    api.createtable(bridge.state, 0, 5);
    integer_field(bridge, "id", shot.id);
    integer_field(bridge, "target", shot.target);
    integer_field(bridge, bridge.name(events::EventName::hitgroup), 8, shot.hitgroup);
    chance_field(bridge, shot);
    integer_field(bridge, "tick", shot.tick);
    if (*event.reason > 4)
        *event.reason = 0;
    api.pushlstring(bridge.state, "reason", 6);
    api.pushstring(bridge.state, bridge.aim_miss_reasons[*event.reason].c_str());
    api.settable(bridge.state, -3);
    return 1;
}

struct HitEvent {
    const AimShot* shot;
    std::int32_t target, hitgroup, damage;
};
int marshal_aim_hit(Bridge& bridge, void* context) {
    const auto& event = *static_cast<HitEvent*>(context);
    const auto& shot = *event.shot;
    bridge.api->createtable(bridge.state, 0, 8);
    integer_field(bridge, "id", shot.id);
    integer_field(bridge, "target", event.target);
    integer_field(bridge, bridge.name(events::EventName::hitgroup), 8, event.hitgroup);
    integer_field(bridge, bridge.name(events::EventName::damage), 6, event.damage);
    chance_field(bridge, shot);
    integer_field(bridge, "tick", shot.tick);
    return 1;
}
}

int dispatch_aim_miss(Bridge& bridge, const AimShot& shot, std::uint32_t reason) {
    if (!bridge.state)
        return 0;
    MissEvent event{&shot, &reason};
    return dispatch_callbacks(bridge, bridge.name(events::EventName::aim_miss).get(),
                              marshal_aim_miss, &event);
}

int dispatch_aim_hit(Bridge& bridge, const AimShot& shot, std::int32_t target,
                     std::int32_t hitgroup, std::int32_t damage) {
    if (!bridge.state)
        return 0;
    HitEvent event{&shot, target, hitgroup, damage};
    return dispatch_callbacks(bridge, bridge.name(events::EventName::aim_hit).get(),
                              marshal_aim_hit, &event);
}

void notify_shutdown(Bridge& bridge) {
    const std::lock_guard lock(bridge.vm_mutex);
    if (!bridge.state || bridge.runtime_ref == -1)
        return;
    check_binding(bridge);
    const auto& api = *bridge.api;
    CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = api.gettop(bridge.state);
    RtlPushFrame(&guard.active);
    auto* previous_bridge = shutdown_bridge;
    shutdown_bridge = &bridge;
    auto invoke = [&] {
        if (api.cpcall(bridge.state, notify_shutdown_callback, nullptr) != 0)
            report_lua_error(bridge);
    };
    protected_region(invoke, guard);
    shutdown_bridge = previous_bridge;
    api.settop(bridge.state, guard.saved_top);
    RtlPopFrame(&guard.active);
}

std::uint32_t callback_hash(const char* name) noexcept {
    std::uint32_t hash = 0x811c9dc5;
    if (name) {
        for (auto current = reinterpret_cast<const unsigned char*>(name); *current; ++current) {
            const auto byte = *current;
            const auto folded = byte >= 'A' && byte <= 'Z' ? byte + 0x20u : byte;
            hash = (hash ^ folded) * 0x01000193u;
        }
    }
    return hash ^ 0x92ca5df1u;
}

} // namespace skeet::script
