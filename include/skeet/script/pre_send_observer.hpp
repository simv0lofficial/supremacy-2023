#pragma once
#include "skeet/events/registry.hpp"
#include "skeet/misc/network_console_handlers.hpp"
#include "skeet/script/bridge.hpp"

namespace skeet::script {
class ClientModule;

// These are the live slots read by43381F73, rather than a snapshot made when
// the script owner is constructed. ClientModule owns the separate Lua event
// hash list consumed by the canonical engine-event subscription implementation.
struct PreSendObserverBindings {
    const events::EventRegistry& event_registry;
    void* const& client_state; //43468E44, signon state at+100
    const std::uint8_t& subscription_gate; //434691B8
    void** const& original_channel_vtable; //43475B0C
};

// Complete433E0A74. All callbacks receive the same {text=...} table and one
// return value is requested from each. Only boolean true suppresses sending.
// A changed text is sent once through the saved engine virtual40 with both
// reliability arguments zero; the caller must then suppress its original send.
bool dispatch_string_command(Bridge&, void* channel, void* message, void** original_channel_vtable);

// Complete43381F73. Type5 dispatches string_cmd; type12 conditionally adds the
// native and Lua event subscriptions to the message's existing bit mask.
void observe_pre_send(Bridge&, const ClientModule&, const PreSendObserverBindings&,
                      misc::NetworkMessageCall&);

// Native script-owner secondary endpoint at+20, vtable43472C2C. Construction
// registers through4336F73C in the actual43475B94/B98/BC SendNetMsg observer
// registry. SendNetMsg433877FF invokes its slot0 before its cancellation test.
class PreSendObserver final : public misc::NetworkMessageCallback {
public:
    PreSendObserver(Bridge&, const ClientModule&, PreSendObserverBindings,
                    engine::OwnerRegistryBinding callbacks);
    PreSendObserver(const PreSendObserver&) = delete;
    PreSendObserver& operator=(const PreSendObserver&) = delete;

private:
    Bridge& bridge_;
    const ClientModule& client_;
    PreSendObserverBindings bindings_;
    static const misc::NetworkMessageEntry entries_[1];
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall observe_entry(void*, misc::NetworkMessageCall*);
#else
    static void observe_entry(void*, misc::NetworkMessageCall*);
#endif
};
} // namespace skeet::script
