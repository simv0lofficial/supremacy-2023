#include "skeet/script/pre_send_observer.hpp"
#include "skeet/script/client_api.hpp"
#include "skeet/engine/interface_call.hpp"
#include "detail.hpp"
#include <array>
#include <cstring>
#include <stdexcept>

namespace skeet::script {
namespace {
template <class T> T field(const void* object, std::ptrdiff_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> void set_field(void* object, std::ptrdiff_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}

// The engine's string-command message points to the MSVC x86 basic_string
// representation. The replacement is a borrowed view: native433E0C51..0CC0
// neither allocates nor frees the Lua string used by the long-string branch.
struct CommandText {
    union {
        std::array<char, 16> inline_text;
        const char* pointer;
    } storage{};
    std::uint32_t size = 0;
    std::uint32_t capacity = 0;
};
static_assert(offsetof(CommandText, size) == 0x10);
static_assert(offsetof(CommandText, capacity) == 0x14);
static_assert(sizeof(CommandText) == 0x18);

const char* text_bytes(const void* text) {
    return field<std::uint32_t>(text, 0x14) < 16 ? static_cast<const char*>(text)
                                                 : field<const char*>(text, 0);
}
}

bool dispatch_string_command(Bridge& bridge, void* channel, void* message,
                             void** original_channel_vtable) {
    const std::lock_guard lock(bridge.vm_mutex);
    if (!bridge.state || !(field<std::uint8_t>(message, 0x14) & 1) || !field<void*>(message, 0xC))
        return false;
    if (!bridge.api)
        throw std::logic_error("Lua string_cmd requires its initialized API");

    const auto* event_name = bridge.name(events::EventName::string_cmd).get();
    const auto hash = callback_hash(event_name);
    auto& active = bridge.callback_active[hash];
    if (active)
        return false;
    const auto& api = *bridge.api;
    void* const state = bridge.state;
    detail::CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = api.gettop(state);
    detail::RtlPushFrame(&guard.active);
    bool cancelled = false;
    active = true;
    auto invoke = [&] {
        api.lcheckstack(state, 12, "dispatching string command callbacks");
        if (!push_callback_list(bridge, state, event_name))
            return;
        const auto count = static_cast<std::int32_t>(api.objlen(state, -1));
        if (count > 0) {
            api.createtable(state, 0, 1);
            api.pushlstring(state, bridge.indicator_text_key.get(), 4);
            const auto* text = field<const void*>(message, 0xC);
            api.pushlstring(state, text_bytes(text), field<std::uint32_t>(text, 0x10));
            api.settable(state, -3);
            for (std::int32_t index = 1; index <= count; ++index) {
                api.rawgeti(state, -2, index);
                if (api.type(state, -1) == 6) {
                    api.pushvalue(state, -2);
                    if (detail::callback_pcall(state, 1, 1) != 0) {
                        detail::report_callback_error(bridge, state);
                        continue;
                    }
                    if (api.type(state, -1) == 1 && api.toboolean(state, -1))
                        cancelled = true;
                }
                api.settop(state, -2);
            }

            if (!cancelled) {
                api.getfield(state, -1, bridge.indicator_text_key.get());
                std::size_t length = 0;
                const auto* changed = api.tolstring(state, -1, &length);
                auto* const original = field<void*>(message, 0xC);
                if (length != field<std::uint32_t>(original, 0x10) ||
                    (length && std::memcmp(text_bytes(original), changed, length) != 0)) {
                    CommandText replacement{};
                    replacement.size = static_cast<std::uint32_t>(length);
                    if (length >= 16) {
                        replacement.storage.pointer = changed;
                        replacement.capacity = static_cast<std::uint32_t>(length + 1);
                    } else {
                        if (length)
                            std::memcpy(replacement.storage.inline_text.data(), changed, length);
                        replacement.capacity = 15;
                    }
                    set_field(message, 0xC, &replacement);
                    using Send = engine::MemberFunction<bool, void*, std::uint8_t, std::uint8_t>;
                    reinterpret_cast<Send>(original_channel_vtable[40])(channel, message, 0, 0);
                    set_field(message, 0xC, original);
                    cancelled = true;
                }
                api.settop(state, -2);
            }
            api.settop(state, -2);
            const auto dirty = bridge.callback_dirty.find(hash);
            if (dirty != bridge.callback_dirty.end() && dirty->second) {
                dirty->second = false;
                compact_callback_list(api, state, -1);
            }
        }
        api.settop(state, -3);
    };
    detail::protected_region(invoke, guard);
    api.settop(state, guard.saved_top);
    active = false;
    detail::RtlPopFrame(&guard.active);
    return cancelled;
}

void observe_pre_send(Bridge& bridge, const ClientModule& client,
                      const PreSendObserverBindings& bindings, misc::NetworkMessageCall& call) {
    const auto type = engine::invoke<std::int32_t>(call.message, 7);
    if (type == 5) {
        if (dispatch_string_command(bridge, call.channel, call.message,
                                    bindings.original_channel_vtable))
            call.cancelled = 1;
    } else if (type == 12 && field<std::int32_t>(bindings.client_state, 0x100) == 6 &&
               bindings.subscription_gate) {
        events::enable_engine_event_subscriptions(
            bindings.event_registry, client.event_subscriptions(),
            bindings.event_registry.event_manager, call.message);
    }
}

const misc::NetworkMessageEntry PreSendObserver::entries_[1]{observe_entry};

PreSendObserver::PreSendObserver(Bridge& bridge, const ClientModule& client,
                                 PreSendObserverBindings bindings,
                                 engine::OwnerRegistryBinding callbacks)
    : NetworkMessageCallback{nullptr}, bridge_(bridge), client_(client), bindings_(bindings) {
    callbacks.append(this);
    vtable = entries_;
}

#if defined(_M_IX86) || defined(__i386__)
void __fastcall PreSendObserver::observe_entry(void* endpoint, misc::NetworkMessageCall* call) {
#else
void PreSendObserver::observe_entry(void* endpoint, misc::NetworkMessageCall* call) {
#endif
    auto& observer =
        *static_cast<PreSendObserver*>(static_cast<misc::NetworkMessageCallback*>(endpoint));
    observe_pre_send(observer.bridge_, observer.client_, observer.bindings_, *call);
}
} // namespace skeet::script
