#pragma once
#include "incoming_update.hpp"
#include "skeet/engine/decoded_offset.hpp"

namespace skeet::network {
// These references share the SendNetMsg handshake, incoming update, startup,
// animation and command state. The publishers do not own another copy.
struct TickUpdateGlobals {
    client::EntityObserverRegistry& receivers; //43475BA8/AC/B0
    IncomingUpdatePatch*& patches; //4347644C
    std::int32_t& previous_command; //4346D4B4
    std::int32_t& previous_processed_command; //4346D4B8
    std::int32_t& transition_status; //4347C9EC
    float& acknowledged_at; //43479924
    float& interpolation; //43476928
    const std::int32_t*& sequence; //434758D8
    const float*& current_time; //4346A334
};
struct TickUpdateBindings {
    void* const& engine_client; //4346A514
    void* const*& game_rules; //4346A3A4
    engine::DecodedOffset game_rule_guard; //canonical43468378/key43468244, BF567619
    std::span<const client::StartupAcknowledgement, 150> acknowledgements; //43475BE0
    const misc::PingSpikeNetworkHistory& latency; //canonical4347A500
    client::EntityUpdateBindings::ExchangeWord exchange; //kernel32!InterlockedExchange
};

// Complete433841D8. status is the incoming object's actual flags+24/byte+10
// view; its meaning is not inferred from the hook's engine vtable slot.
void update_tick_status(const void* status, TickUpdateGlobals&, const TickUpdateBindings&);
// Complete4333CA14: update_tick_status, actual BA8slot0 then original slot37.
// The callbacks receive status in EDX and have no stack arguments.
void publish_update_status(void* hooked_object, const void* status, TickUpdateGlobals&,
                           const TickUpdateBindings&);
// Complete4341EFED: guard/acknowledgement selection, command reset, channel
// interpolation, actual BA8slot2, patch+10 exchanges, then original slot5.
void publish_incoming_tick(void* hooked_object, std::uintptr_t first_argument,
                           std::int32_t incoming_tick, TickUpdateGlobals&,
                           const TickUpdateBindings&);

struct TickUpdateRuntime {
    TickUpdateGlobals globals;
    TickUpdateBindings bindings;
    void status(void* object, const void* value) {
        publish_update_status(object, value, globals, bindings);
    }
    void incoming(void* object, std::uintptr_t first, std::int32_t tick) {
        publish_incoming_tick(object, first, tick, globals, bindings);
    }
};
void bind_tick_update_runtime(TickUpdateRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __fastcall update_status_hook(void* object, void* register_edx, const void* status);
void __fastcall incoming_tick_hook(void* object, void* register_edx, std::uintptr_t first_argument,
                                   std::int32_t incoming_tick);
#else
void update_status_hook(void* object, void* register_edx, const void* status);
void incoming_tick_hook(void* object, void* register_edx, std::uintptr_t first_argument,
                        std::int32_t incoming_tick);
#endif
} //namespace skeet::network
