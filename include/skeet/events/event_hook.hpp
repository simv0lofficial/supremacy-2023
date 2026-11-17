#pragma once
#include "event_dispatch.hpp"
#include "skeet/engine/hooks.hpp"
#include "skeet/network/client_state_descriptor.hpp"

namespace skeet::events {

// Captured initialization operands0860/0864 bind slot34 of descriptor43475A3C
// to433AE68A. Its thirteen replacements belong to the earlier protected hook
// scheduler;4341F57F does not install this hook.
inline constexpr std::size_t event_packet_slot = network::client_state_event_slot;

// Owns the source dispatcher behind the actual engine-vtable entry. The event
// registry, observer vector and local-player pointer slot are the same objects
// populated by feature constructors. In particular, no engine/manager copy is
// kept apart from the registry used to publish manager+A0 subscription changes.
class EventPacketHookRuntime final {
public:
    EventPacketHookRuntime(EventRegistry& registry, engine::OwnerRegistry& observers,
                           void* const* local_player) noexcept;
    EventPacketHookRuntime(const EventPacketHookRuntime&) = delete;
    EventPacketHookRuntime& operator=(const EventPacketHookRuntime&) = delete;

    bool process(void* receiver, void* packet);

private:
    EventMasterBindings bindings_;
    EventMaster master_;
};

// Publishes the existing dispatcher only; the complete ClientStateHookFamily
// prepares all thirteen replacements and performs one shared installation.
void bind_event_packet_hook_runtime(EventPacketHookRuntime& runtime) noexcept;

// The caller supplies the actual prepared descriptor43475A3C and receiver.
// Source-global43468D20 is engine.dll;747CA973 is the RTTI CClientState type
// hash, with base-type hash0. The genuine RTTI resolver/descriptor clone and
// the other twelve replacements must already have completed. This per-entry
// adapter invokes complete4331E834 using the canonical cleanup sentinel; the
// whole-family production caller uses the publisher above and installs once.
// Runtime and metadata must outlive restoration by UnloadRuntime::cleanup.
engine::VtableHookNode* install_event_packet_hook(EventPacketHookRuntime& runtime,
                                                  engine::VtableHookRegistry& hooks, void* receiver,
                                                  engine::VtableHookMetadata& metadata);

#if defined(_M_IX86) || defined(__i386__)
bool __fastcall event_packet_hook(void* receiver, void* ignored_edx, void* packet);
#else
bool event_packet_hook(void* receiver, void* ignored_edx, void* packet);
#endif
} // namespace skeet::events
